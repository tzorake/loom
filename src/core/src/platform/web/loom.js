/**
 * loom.js — JavaScript loader for the Loom WebAssembly module.
 *
 * Responsibilities
 * ────────────────
 *  1. Provide a minimal WASI snapshot_preview1 shim so the module (compiled
 *     with the WASI SDK) can perform basic I/O (stderr → console.error,
 *     timekeeping, random, etc.).
 *
 *  2. Provide the custom "env" imports that the C++ platform backend calls
 *     (canvas rendering, title, animation-frame scheduling).
 *
 *  3. Wire DOM events (mouse, keyboard, resize) to the exported C++ entry
 *     points (loom_mouse_move, loom_key_event, loom_resize, …).
 *
 *  4. Call the exported loom_init() once after instantiation.
 *
 * The C++ side calls js_request_animation_frame() at the end of every tick;
 * this file schedules requestAnimationFrame → exports.loom_tick() in response.
 */

(function () {
  'use strict';

  // ── Canvas setup ──────────────────────────────────────────────────────────

  const canvas  = document.getElementById('loom-canvas');
  // Both contexts are obtained lazily so that acquiring one does not
  // permanently block the other.  A canvas may only have one active
  // rendering context; the first getContext() call wins.
  let ctx2d = null;
  function getCtx2D() {
    if (!ctx2d) ctx2d = canvas.getContext('2d');
    return ctx2d;
  }
  let gl2 = null;
  function getGL2() {
    if (!gl2) gl2 = canvas.getContext('webgl2');
    return gl2;
  }

  // WASM linear memory — set after instantiation.
  let memory;

  // Cached exports reference.
  let exports;

  // ── Helpers ───────────────────────────────────────────────────────────────

  // Read a UTF-8 string from WASM memory given a pointer and byte length.
  function wasmStr(ptr, len) {
    return new TextDecoder().decode(new Uint8Array(memory.buffer, ptr, len));
  }

  // ── WASI shim ─────────────────────────────────────────────────────────────
  //
  // Implements the subset of wasi_snapshot_preview1 that the WASI SDK libc
  // actually calls for a single-threaded app with no filesystem access.

  const WASI_ESUCCESS  = 0;
  const WASI_EBADF     = 8;
  const WASI_EINVAL    = 28;
  const WASI_ENOENT    = 44;
  const WASI_ENOSYS    = 52;

  // fd_write — used by fprintf/printf for stdout (1) and stderr (2).
  function fd_write(fd, iovs_ptr, iovs_len, nwritten_ptr) {
    const view = new DataView(memory.buffer);
    const mem8 = new Uint8Array(memory.buffer);

    let text  = '';
    let total = 0;

    for (let i = 0; i < iovs_len; i++) {
      const base = view.getUint32(iovs_ptr + i * 8,     true);
      const len  = view.getUint32(iovs_ptr + i * 8 + 4, true);
      text  += new TextDecoder().decode(mem8.subarray(base, base + len));
      total += len;
    }

    view.setUint32(nwritten_ptr, total, true);

    // Strip trailing newline — console already adds one.
    const msg = text.replace(/\n$/, '');
    if (fd === 1) console.log(msg);
    else          console.error(msg);

    return WASI_ESUCCESS;
  }

  // clock_time_get — used by std::chrono::steady_clock and system_clock.
  //   id 0 = CLOCK_REALTIME, id 1 = CLOCK_MONOTONIC
  //
  // WASM signature: (i32 id, i64 precision, i32 time_ptr) -> i32
  // With the WebAssembly/JavaScript BigInt integration (all modern browsers),
  // the i64 precision is passed as a JS BigInt, giving 3 JS arguments —
  // NOT 4 split-i32 arguments.  The third argument is the output pointer.
  function clock_time_get(id, _precision, time_ptr) {
    const view = new DataView(memory.buffer);
    const ms   = (id === 1) ? performance.now() : Date.now();
    const ns   = BigInt(Math.round(ms * 1_000_000));
    view.setBigUint64(time_ptr, ns, true);
    return WASI_ESUCCESS;
  }

  function clock_res_get(id, resolution_ptr) {
    const view = new DataView(memory.buffer);
    // Report 1 ms resolution.
    view.setBigUint64(resolution_ptr, 1_000_000n, true);
    return WASI_ESUCCESS;
  }

  // random_get — used by std::random_device, hash seeds, etc.
  function random_get(buf_ptr, buf_len) {
    const buf = new Uint8Array(memory.buffer, buf_ptr, buf_len);
    crypto.getRandomValues(buf);
    return WASI_ESUCCESS;
  }

  // fd_fdstat_get — called for fd 0/1/2 validity checks.
  function fd_fdstat_get(fd, stat_ptr) {
    if (fd > 2) return WASI_EBADF;
    const view = new DataView(memory.buffer);
    // filetype = 2 (character device), flags = 0, rights_base = ~0, rights_inheriting = ~0
    view.setUint8(stat_ptr,     2);               // filetype
    view.setUint8(stat_ptr + 1, 0);               // padding
    view.setUint16(stat_ptr + 2, 0, true);         // fs_flags
    view.setUint32(stat_ptr + 4, 0, true);         // padding
    view.setBigUint64(stat_ptr +  8, 0xFFFFFFFFFFFFFFFFn, true); // rights_base
    view.setBigUint64(stat_ptr + 16, 0xFFFFFFFFFFFFFFFFn, true); // rights_inheriting
    return WASI_ESUCCESS;
  }

  // fd_prestat_get — signals no pre-opened directories exist.
  function fd_prestat_get(_fd, _buf) { return WASI_EBADF; }

  // Stubs for syscalls the runtime generates references to but we never hit.
  function fd_close(_fd)                              { return WASI_ESUCCESS; }
  function fd_seek(_fd, _off, _w, _ptr)               { return WASI_ESUCCESS; }
  function fd_read(_fd, _iovs, _len, _ptr)            { return WASI_ESUCCESS; }

  function environ_sizes_get(count_ptr, buf_size_ptr) {
    const view = new DataView(memory.buffer);
    view.setUint32(count_ptr,    0, true);
    view.setUint32(buf_size_ptr, 0, true);
    return WASI_ESUCCESS;
  }
  function environ_get(_env_ptr, _buf_ptr) { return WASI_ESUCCESS; }

  function args_sizes_get(argc_ptr, argv_buf_ptr) {
    const view = new DataView(memory.buffer);
    view.setUint32(argc_ptr,    0, true);
    view.setUint32(argv_buf_ptr, 0, true);
    return WASI_ESUCCESS;
  }
  function args_get(_argv_ptr, _buf_ptr) { return WASI_ESUCCESS; }

  function proc_exit(code) {
    console.log('[loom] proc_exit', code);
  }

  function sched_yield() { return WASI_ESUCCESS; }

  // ── Remaining WASI snapshot_preview1 stubs ────────────────────────────────
  //
  // The WASI SDK libc references many syscalls at link time even when they are
  // never reached at runtime.  WebAssembly.instantiate() requires every import
  // to be a callable, so we provide no-op / error stubs for the full set.
  //
  // Safe responses:
  //   ENOSYS  — syscall exists but is not implemented here (generic fallback)
  //   EBADF   — operation on a file descriptor we don't own
  //   ESUCCESS — harmless acknowledgement (e.g. flag-setting no-ops)

  // fd group
  function fd_fdstat_set_flags(_fd, _flags)                 { return WASI_ESUCCESS; }
  function fd_prestat_dir_name(_fd, _path, _len)            { return WASI_EBADF;    }
  function fd_allocate(_fd, _off, _len)                     { return WASI_ENOSYS;   }
  function fd_datasync(_fd)                                 { return WASI_ENOSYS;   }
  function fd_filestat_get(_fd, _buf)                       { return WASI_ENOSYS;   }
  function fd_filestat_set_size(_fd, _size)                 { return WASI_ENOSYS;   }
  function fd_filestat_set_times(_fd, _at, _mt, _flags)     { return WASI_ENOSYS;   }
  function fd_pread(_fd, _iovs, _len, _off, _nread)         { return WASI_ENOSYS;   }
  function fd_pwrite(_fd, _iovs, _len, _off, _nwritten)     { return WASI_ENOSYS;   }
  function fd_readdir(_fd, _buf, _len, _cookie, _bufused)   { return WASI_ENOSYS;   }
  function fd_renumber(_fd, _to)                            { return WASI_ENOSYS;   }
  function fd_sync(_fd)                                     { return WASI_ENOSYS;   }
  function fd_tell(_fd, _offset)                            { return WASI_ENOSYS;   }

  // path group
  function path_create_directory(_fd, _path, _len)          { return WASI_ENOSYS;   }
  function path_filestat_get(_fd, _flags, _path, _len, _buf){ return WASI_ENOSYS;   }
  function path_filestat_set_times(
    _fd, _flags, _path, _len, _at, _mt, _fflags)            { return WASI_ENOSYS;   }
  function path_link(
    _old_fd, _old_flags, _old_path, _old_len,
    _new_fd, _new_path, _new_len)                            { return WASI_ENOSYS;   }
  function path_open(
    _fd, _dir_flags, _path, _path_len, _o_flags,
    _fs_rights_base, _fs_rights_inheriting,
    _fd_flags, _opened_fd)                                   { return WASI_ENOSYS;   }
  function path_readlink(
    _fd, _path, _path_len, _buf, _buf_len, _bufused)         { return WASI_ENOSYS;   }
  function path_remove_directory(_fd, _path, _len)          { return WASI_ENOSYS;   }
  function path_rename(
    _old_fd, _old_path, _old_len,
    _new_fd, _new_path, _new_len)                            { return WASI_ENOSYS;   }
  function path_symlink(_old, _old_len, _fd, _new, _new_len){ return WASI_ENOSYS;   }
  function path_unlink_file(_fd, _path, _len)               { return WASI_ENOSYS;   }

  // poll / proc / sock
  function poll_oneoff(_in, _out, _nsubs, _nevents)         { return WASI_ENOSYS;   }
  function proc_raise(_sig)                                  { return WASI_ENOSYS;   }
  function sock_accept(_fd, _flags, _conn)                  { return WASI_ENOSYS;   }
  function sock_recv(_fd, _iovs, _len, _flags, _nrecv, _oflags) { return WASI_ENOSYS; }
  function sock_send(_fd, _iovs, _len, _flags, _nsent)      { return WASI_ENOSYS;   }
  function sock_shutdown(_fd, _how)                         { return WASI_ENOSYS;   }

  const wasiImports = {
    // I/O
    fd_write,
    fd_read,
    fd_seek,
    fd_close,
    fd_fdstat_get,
    fd_fdstat_set_flags,
    fd_prestat_get,
    fd_prestat_dir_name,
    fd_allocate,
    fd_datasync,
    fd_filestat_get,
    fd_filestat_set_size,
    fd_filestat_set_times,
    fd_pread,
    fd_pwrite,
    fd_readdir,
    fd_renumber,
    fd_sync,
    fd_tell,
    // path
    path_create_directory,
    path_filestat_get,
    path_filestat_set_times,
    path_link,
    path_open,
    path_readlink,
    path_remove_directory,
    path_rename,
    path_symlink,
    path_unlink_file,
    // clock / random / env / args
    clock_time_get,
    clock_res_get,
    random_get,
    environ_sizes_get,
    environ_get,
    args_sizes_get,
    args_get,
    // poll / proc / sched / sock
    poll_oneoff,
    proc_exit,
    proc_raise,
    sched_yield,
    sock_accept,
    sock_recv,
    sock_send,
    sock_shutdown,
  };

  // ── C++ exception ABI stubs ───────────────────────────────────────────────
  //
  // wasm32-wasi does not bundle __cxa_throw / __cxa_allocate_exception into
  // the sysroot; the linker emits them as imports from the "env" module.
  // Loom's throw sites are programmer-error guards that should never fire at
  // runtime.  If one does, we allocate a scratch page in WASM memory for the
  // exception object and then throw a JavaScript Error so the failure surfaces
  // as an uncaught exception in the browser console.

  let _cxaExceptionPtr = 0;

  function cxa_allocate_exception(size) {
    if (!_cxaExceptionPtr) {
      // Grow the WASM linear memory by one page (64 KiB) for scratch space.
      const old_pages = memory.grow(1);
      _cxaExceptionPtr = old_pages * 65536;
    }
    return _cxaExceptionPtr;
  }

  function cxa_throw(thrown_object, tinfo, dest) {
    console.error('[loom] Unhandled C++ exception — aborting');
    throw new Error('[loom] unhandled C++ exception');
  }

  // ── Custom "env" imports (called by tzwebimports.hpp declarations) ─────────

  const envImports = {
    __cxa_allocate_exception: cxa_allocate_exception,
    __cxa_throw:              cxa_throw,

    // Draw a pixel buffer onto the canvas.
    // pixels is a pointer to packed uint32 values in RGBA byte order
    // (the C++ render() method converts from internal ARGB before calling here).
    js_draw_pixels(pixels_ptr, x, y, w, h) {
      if (w <= 0 || h <= 0) return;
      // View directly into WASM memory — no copy needed.  putImageData is
      // synchronous so the WASM buffer cannot be detached during the call.
      const bytes = new Uint8ClampedArray(memory.buffer, pixels_ptr, w * h * 4);
      const imageData = new ImageData(bytes, w, h);
      getCtx2D().putImageData(imageData, x, y);
    },

    js_set_title(ptr, len) {
      document.title = wasmStr(ptr, len);
    },

    js_get_canvas_width()  { return canvas.width;  },
    js_get_canvas_height() { return canvas.height; },

    js_show_canvas(visible) {
      canvas.style.display = visible ? 'block' : 'none';
    },

    // Schedule the next animation frame → calls exports.loom_tick().
    js_request_animation_frame() {
      requestAnimationFrame(() => { if (exports) exports.loom_tick(); });
    },
  };

  // ── WebGL 2 import object (loom-rhi WebGL backend) ───────────────────────
  //
  // All GL object handles are 1-based integer indices into JS arrays.
  // Index 0 is reserved as "null handle" (maps to null in all calls).

  const glBufs        = [null];   // TzRhiBuffer handles
  const glTexs        = [null];   // TzRhiTexture handles
  const glSamplers    = [null];   // TzRhiSampler handles
  const glShaders     = [null];   // shader objects (transient)
  const glProgs       = [null];   // program handles
  const glVAOs        = [null];   // vertex array objects
  const glFBOs        = [null];   // framebuffer objects
  const glUniformLocs = [null];   // WebGLUniformLocation handles

  // Helper: read a UTF-8 string of known byte length from WASM memory.
  // (wasmStr is defined earlier but requires memory to be initialised.)
  function glStr(ptr, len) { return wasmStr(ptr, len); }

  const webglImports = {
    // ── Buffer ──────────────────────────────────────────────────────────────
    gl_create_buffer() {
      const gl = getGL2(); glBufs.push(gl.createBuffer()); return glBufs.length - 1;
    },
    gl_bind_buffer(target, handle) {
      getGL2().bindBuffer(target, glBufs[handle] ?? null);
    },
    gl_buffer_data_static(target, ptr, byteSize) {
      const gl = getGL2();
      if (ptr && byteSize > 0)
        gl.bufferData(target, new Uint8Array(memory.buffer, ptr, byteSize), gl.STATIC_DRAW);
      else
        gl.bufferData(target, byteSize, gl.STATIC_DRAW);
    },
    gl_buffer_sub_data(target, offset, ptr, byteSize) {
      getGL2().bufferSubData(target, offset,
        new Uint8Array(memory.buffer, ptr, byteSize));
    },
    gl_delete_buffer(handle) {
      const gl = getGL2(); gl.deleteBuffer(glBufs[handle]); glBufs[handle] = null;
    },

    // ── Texture ─────────────────────────────────────────────────────────────
    gl_create_texture() {
      const gl = getGL2(); glTexs.push(gl.createTexture()); return glTexs.length - 1;
    },
    gl_bind_texture(target, handle) {
      getGL2().bindTexture(target, glTexs[handle] ?? null);
    },
    gl_tex_image_2d(target, level, internalFormat, width, height, border,
                    format, type, ptr, byteSize) {
      const gl = getGL2();
      const data = (ptr && byteSize > 0)
        ? new Uint8Array(memory.buffer, ptr, byteSize) : null;
      gl.texImage2D(target, level, internalFormat, width, height, border,
                    format, type, data);
    },
    gl_tex_sub_image_2d(target, level, xoffset, yoffset, width, height,
                        format, type, ptr, byteSize) {
      const gl = getGL2();
      gl.texSubImage2D(target, level, xoffset, yoffset, width, height,
                       format, type,
                       new Uint8Array(memory.buffer, ptr, byteSize));
    },
    gl_tex_parameteri(target, pname, param) {
      getGL2().texParameteri(target, pname, param);
    },
    gl_active_texture(texture) { getGL2().activeTexture(texture); },
    gl_delete_texture(handle) {
      const gl = getGL2(); gl.deleteTexture(glTexs[handle]); glTexs[handle] = null;
    },

    // ── Sampler ─────────────────────────────────────────────────────────────
    gl_create_sampler() {
      const gl = getGL2(); glSamplers.push(gl.createSampler()); return glSamplers.length - 1;
    },
    gl_bind_sampler(unit, handle) {
      getGL2().bindSampler(unit, glSamplers[handle] ?? null);
    },
    gl_sampler_parameteri(handle, pname, param) {
      getGL2().samplerParameteri(glSamplers[handle], pname, param);
    },
    gl_delete_sampler(handle) {
      const gl = getGL2(); gl.deleteSampler(glSamplers[handle]); glSamplers[handle] = null;
    },

    // ── Shader / Program ────────────────────────────────────────────────────
    gl_create_shader(type) {
      const gl = getGL2(); glShaders.push(gl.createShader(type)); return glShaders.length - 1;
    },
    gl_shader_source(handle, ptr, len) {
      getGL2().shaderSource(glShaders[handle], glStr(ptr, len));
    },
    gl_compile_shader(handle) { getGL2().compileShader(glShaders[handle]); },
    gl_get_shader_compile_status(handle) {
      return getGL2().getShaderParameter(glShaders[handle], 0x8B81 /*COMPILE_STATUS*/) ? 1 : 0;
    },
    gl_delete_shader(handle) {
      const gl = getGL2(); gl.deleteShader(glShaders[handle]); glShaders[handle] = null;
    },
    gl_create_program() {
      const gl = getGL2(); glProgs.push(gl.createProgram()); return glProgs.length - 1;
    },
    gl_attach_shader(program, shader) {
      getGL2().attachShader(glProgs[program], glShaders[shader]);
    },
    gl_link_program(program) { getGL2().linkProgram(glProgs[program]); },
    gl_get_program_link_status(program) {
      return getGL2().getProgramParameter(glProgs[program], 0x8B82 /*LINK_STATUS*/) ? 1 : 0;
    },
    gl_use_program(program) {
      getGL2().useProgram(program ? glProgs[program] : null);
    },
    gl_delete_program(program) {
      const gl = getGL2(); gl.deleteProgram(glProgs[program]); glProgs[program] = null;
    },
    gl_get_uniform_block_index(program, namePtr, nameLen) {
      const gl = getGL2();
      const idx = gl.getUniformBlockIndex(glProgs[program], glStr(namePtr, nameLen));
      return (idx === 0xFFFFFFFF) ? -1 : idx;
    },
    gl_uniform_block_binding(program, blockIndex, binding) {
      getGL2().uniformBlockBinding(glProgs[program], blockIndex, binding);
    },
    gl_get_active_uniform_block_count(program) {
      return getGL2().getProgramParameter(glProgs[program], 0x8A36 /*ACTIVE_UNIFORM_BLOCKS*/) | 0;
    },
    gl_get_active_uniform_count(program) {
      return getGL2().getProgramParameter(glProgs[program], 0x8B86 /*ACTIVE_UNIFORMS*/) | 0;
    },
    gl_get_active_uniform_type(program, index) {
      const info = getGL2().getActiveUniform(glProgs[program], index);
      return info ? info.type : 0;
    },
    gl_get_active_uniform_name(program, index, namePtr, maxLen) {
      const info = getGL2().getActiveUniform(glProgs[program], index);
      if (!info || maxLen <= 0) return;
      const encoded = new TextEncoder().encode(info.name);
      const n = Math.min(encoded.length, maxLen - 1);
      new Uint8Array(memory.buffer, namePtr, n).set(encoded.subarray(0, n));
      new Uint8Array(memory.buffer, namePtr + n, 1)[0] = 0;
    },
    gl_get_uniform_location(program, namePtr, nameLen) {
      const gl = getGL2();
      const loc = gl.getUniformLocation(glProgs[program], wasmStr(namePtr, nameLen));
      if (loc === null) return -1;
      glUniformLocs.push(loc);
      return glUniformLocs.length - 1;
    },
    gl_uniform1i(location, value) {
      if (location < 0) return;
      getGL2().uniform1i(glUniformLocs[location], value);
    },

    // ── VAO ─────────────────────────────────────────────────────────────────
    gl_create_vertex_array() {
      const gl = getGL2(); glVAOs.push(gl.createVertexArray()); return glVAOs.length - 1;
    },
    gl_bind_vertex_array(handle) {
      getGL2().bindVertexArray(handle ? glVAOs[handle] : null);
    },
    gl_enable_vertex_attrib_array(index) { getGL2().enableVertexAttribArray(index); },
    gl_vertex_attrib_pointer(index, size, type, normalized, stride, offset) {
      getGL2().vertexAttribPointer(index, size, type, !!normalized, stride, offset);
    },
    gl_delete_vertex_array(handle) {
      const gl = getGL2(); gl.deleteVertexArray(glVAOs[handle]); glVAOs[handle] = null;
    },

    // ── Draw ────────────────────────────────────────────────────────────────
    gl_draw_arrays(mode, first, count)          { getGL2().drawArrays(mode, first, count); },
    gl_draw_elements(mode, count, type, offset) { getGL2().drawElements(mode, count, type, offset); },

    // ── State ────────────────────────────────────────────────────────────────
    gl_enable(cap)                          { getGL2().enable(cap); },
    gl_disable(cap)                         { getGL2().disable(cap); },
    gl_blend_func_separate(sRGB, dRGB, sA, dA) { getGL2().blendFuncSeparate(sRGB, dRGB, sA, dA); },
    gl_blend_equation_separate(mRGB, mA)    { getGL2().blendEquationSeparate(mRGB, mA); },
    gl_depth_func(func)                     { getGL2().depthFunc(func); },
    gl_depth_mask(flag)                     { getGL2().depthMask(!!flag); },
    gl_cull_face(mode)                      { getGL2().cullFace(mode); },
    gl_front_face(mode)                     { getGL2().frontFace(mode); },
    gl_viewport(x, y, w, h)                 { getGL2().viewport(x, y, w, h); },
    gl_clear_color(r, g, b, a)              { getGL2().clearColor(r, g, b, a); },
    gl_clear(mask)                          { getGL2().clear(mask); },
    gl_scissor(x, y, w, h)                  { getGL2().scissor(x, y, w, h); },
    gl_line_width(width)                    { getGL2().lineWidth(width); },

    // ── Framebuffer ──────────────────────────────────────────────────────────
    gl_create_framebuffer() {
      const gl = getGL2(); glFBOs.push(gl.createFramebuffer()); return glFBOs.length - 1;
    },
    gl_bind_framebuffer(target, handle) {
      getGL2().bindFramebuffer(target, handle ? glFBOs[handle] : null);
    },
    gl_framebuffer_texture_2d(target, attachment, textarget, texture, level) {
      getGL2().framebufferTexture2D(target, attachment, textarget,
                                    glTexs[texture] ?? null, level);
    },
    gl_delete_framebuffer(handle) {
      const gl = getGL2(); gl.deleteFramebuffer(glFBOs[handle]); glFBOs[handle] = null;
    },

    // ── UBO ──────────────────────────────────────────────────────────────────
    gl_bind_buffer_range(target, index, buffer, offset, size) {
      getGL2().bindBufferRange(target, index, glBufs[buffer] ?? null, offset, size);
    },
  };

  // ── DOM event wiring ──────────────────────────────────────────────────────

  function encodeUtf8Bytes(str) {
    // Returns up to 4 bytes of the first code-point as individual ints.
    const buf = new TextEncoder().encode(str);
    return [buf[0] ?? 0, buf[1] ?? 0, buf[2] ?? 0, buf[3] ?? 0];
  }

  function wireEvents() {
    // Mouse position relative to canvas top-left.
    function canvasXY(e) {
      const r = canvas.getBoundingClientRect();
      return [Math.round(e.clientX - r.left), Math.round(e.clientY - r.top)];
    }

    canvas.addEventListener('mousemove', e => {
      const [x, y] = canvasXY(e);
      exports.loom_mouse_move(x, y);
    });

    canvas.addEventListener('mousedown', e => {
      const [x, y] = canvasXY(e);
      exports.loom_mouse_button(e.button, 1, x, y);
    });

    canvas.addEventListener('mouseup', e => {
      const [x, y] = canvasXY(e);
      exports.loom_mouse_button(e.button, 0, x, y);
    });

    canvas.addEventListener('wheel', e => {
      const [x, y] = canvasXY(e);
      exports.loom_mouse_scroll(x, y, e.deltaX, e.deltaY);
      e.preventDefault();
    }, { passive: false });

    // Keyboard — attach to window so focus isn't needed on the canvas.
    window.addEventListener('keydown', e => {
      if (e.repeat) return;
      const [b0, b1, b2, b3] = encodeUtf8Bytes(e.key.length === 1 ? e.key : '');
      exports.loom_key_event(
        e.keyCode, 1,
        e.shiftKey ? 1 : 0, e.ctrlKey ? 1 : 0, e.altKey ? 1 : 0,
        b0, b1, b2, b3
      );
      // Prevent browser shortcuts (Tab focus jump, etc.) for keys the app handles.
      if (e.keyCode !== 116 /* F5 */) e.preventDefault();
    });

    window.addEventListener('keyup', e => {
      exports.loom_key_event(
        e.keyCode, 0,
        e.shiftKey ? 1 : 0, e.ctrlKey ? 1 : 0, e.altKey ? 1 : 0,
        0, 0, 0, 0
      );
    });

    // Resize — keep the canvas matching the window and notify C++.
    function onResize() {
      const w = window.innerWidth;
      const h = window.innerHeight;
      canvas.width  = w;
      canvas.height = h;
      if (exports) exports.loom_resize(w, h);
    }
    window.addEventListener('resize', onResize);
    // Size the canvas to fill the viewport on first load.
    onResize();

    // Right-click context menu would be confusing; suppress it.
    canvas.addEventListener('contextmenu', e => e.preventDefault());
  }

  // ── WASM instantiation ────────────────────────────────────────────────────

  const importObject = {
    wasi_snapshot_preview1: wasiImports,
    env: envImports,
    webgl: webglImports,
  };

  WebAssembly.instantiateStreaming(fetch('loom_app.wasm'), importObject)
    .then(({ instance }) => {
      exports = instance.exports;
      memory  = instance.exports.memory;

      wireEvents();

      // Bootstrap the C++ application.
      exports.loom_init();
    })
    .catch(err => {
      console.error('[loom] Failed to load loom_app.wasm:', err);
    });

})();
