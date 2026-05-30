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

  const canvas = document.getElementById('loom-canvas');
  const ctx    = canvas.getContext('2d');

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

  const wasiImports = {
    fd_write,
    fd_read,
    fd_seek,
    fd_close,
    fd_fdstat_get,
    fd_prestat_get,
    clock_time_get,
    clock_res_get,
    random_get,
    environ_sizes_get,
    environ_get,
    args_sizes_get,
    args_get,
    proc_exit,
    sched_yield,
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
      ctx.putImageData(imageData, x, y);
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
