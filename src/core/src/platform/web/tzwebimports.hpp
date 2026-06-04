#pragma once

#include <cstdint>

// ── WASM import / export helpers ──────────────────────────────────────────
//
// WASM_IMPORT(module, name)
//   Declares a function that is resolved from the JavaScript import object
//   at instantiation time. `module` maps to the top-level key of the import
//   object; `name` maps to the property within that object.
//
// WASM_EXPORT_AS(sym)
//   Marks an extern "C" function so wasm-ld exports it under the given
//   symbol name. JavaScript can then call it via instance.exports.<sym>().

#define WASM_IMPORT(module, name) \
    __attribute__((import_module(module), import_name(name)))

#define WASM_EXPORT_AS(sym) \
    extern "C" __attribute__((export_name(sym)))

// ── JS functions callable from C++ ───────────────────────────────────────

extern "C" {

// Rendering ─────────────────────────────────────────────────────────────
// Blit a pixel buffer onto the canvas.
// pixels : pointer into WASM linear memory; each uint32 is packed RGBA
//          (byte order in memory: R, G, B, A — matches Canvas ImageData).
// x, y   : destination origin on the canvas.
// w, h   : dimensions of the buffer.
WASM_IMPORT("env", "js_draw_pixels")
void js_draw_pixels(const uint32_t *pixels, int x, int y, int w, int h);

// Window management ─────────────────────────────────────────────────────
// Set the browser tab / page title.
// title  : UTF-8 string in WASM memory.
// len    : byte length (no null terminator required).
WASM_IMPORT("env", "js_set_title")
void js_set_title(const char *title, int len);

// Return the current CSS pixel size of the canvas element.
WASM_IMPORT("env", "js_get_canvas_width")
int js_get_canvas_width();

WASM_IMPORT("env", "js_get_canvas_height")
int js_get_canvas_height();

// Show (visible=1) or hide (visible=0) the canvas element.
WASM_IMPORT("env", "js_show_canvas")
void js_show_canvas(int visible);

// Animation loop ────────────────────────────────────────────────────────
// Suspend WASM until the next animation frame.
//
// This is a JSPI (JavaScript Promise Integration) suspending import.  The JS
// side wraps it with WebAssembly.Suspending and returns a Promise that
// resolves when requestAnimationFrame fires.  From C++ it behaves as an
// ordinary blocking call — processEvents() calls it in a loop and execution
// resumes once per frame.
WASM_IMPORT("env", "js_yield")
void js_yield();

} // extern "C"
