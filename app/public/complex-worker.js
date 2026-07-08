importScripts("sdfglyph-complex.js");

const COMPLEX_GLYPH_OFFSET = 0x10000; // must match src/complex_range.hpp

// Copies a generated glyph_buffer out of the WASM heap into a transferable
// ArrayBuffer, then frees the WASM-side buffer.
function readGlyphBuffer(glyph_buffer_ptr) {
  const data_ptr = Module.ccall(
    "glyph_buffer_data",
    "number",
    ["number"],
    [glyph_buffer_ptr]
  );
  const size = Module.ccall(
    "glyph_buffer_size",
    "number",
    ["number"],
    [glyph_buffer_ptr]
  );
  const result = new Uint8Array(size);
  result.set(new Uint8Array(Module.HEAPU8.buffer, data_ptr, size));
  Module.ccall("free_glyph_buffer", null, ["number"], [glyph_buffer_ptr]);
  return result;
}

self.onmessage = function (message) {
  const { stackId, name, buffers } = message.data;

  // Pass an explicit name so the output folder equals the complex font's name
  // rather than the auto-combined base+complex fontstack name.
  const fontstack_ptr = Module.ccall(
    "create_fontstack",
    "number",
    ["string"],
    [name]
  );

  const font_datas = [];

  for (let ab of buffers) {
    let uint8Arr = new Uint8Array(ab);
    const num_bytes = uint8Arr.length * uint8Arr.BYTES_PER_ELEMENT;
    const data_ptr = Module._malloc(num_bytes);
    font_datas.push(data_ptr);
    const data_on_heap = new Uint8Array(
      Module.HEAPU8.buffer,
      data_ptr,
      num_bytes
    );
    data_on_heap.set(uint8Arr);
    Module.ccall(
      "fontstack_add_face",
      null,
      ["number", "number", "number"],
      [fontstack_ptr, data_ptr, num_bytes]
    );
  }

  const s = Module.ccall(
    "fontstack_name",
    "number",
    ["number"],
    [fontstack_ptr]
  );
  const stack_name = UTF8ToString(s);

  // Report the expected number of .pbf files up front so the UI can show a
  // progress bar: 256 fixed unicode ranges + one range per 256 complex glyphs.
  const num_glyphs = Module.ccall(
    "fontstack_face_num_glyphs",
    "number",
    ["number", "number"],
    [fontstack_ptr, 1]
  );
  const total = 256 + Math.ceil(num_glyphs / 256);
  self.postMessage({ stackId, stackName: stack_name, total });

  // Unicode-indexed ranges from the base face (index 0).
  for (let i = 0; i < 65536; i += 256) {
    const glyph_buffer_ptr = Module.ccall(
      "generate_unicode_buffer",
      "number",
      ["number", "number"],
      [fontstack_ptr, i]
    );
    const result = readGlyphBuffer(glyph_buffer_ptr);
    self.postMessage(
      {
        stackId,
        stackName: stack_name,
        glyph: { name: `${i}-${i + 255}.pbf`, buffer: result.buffer },
      },
      [result.buffer]
    );
  }

  // Glyph-indexed ranges from the complex face (index 1).
  for (let gid = 0; gid < num_glyphs; gid += 256) {
    const glyph_buffer_ptr = Module.ccall(
      "generate_glyph_buffer",
      "number",
      ["number", "number"],
      [fontstack_ptr, gid]
    );
    const result = readGlyphBuffer(glyph_buffer_ptr);
    const start = COMPLEX_GLYPH_OFFSET + gid;
    const end = start + 255;
    self.postMessage(
      {
        stackId,
        stackName: stack_name,
        glyph: { name: `${start}-${end}.pbf`, buffer: result.buffer },
      },
      [result.buffer]
    );
  }

  for (let data_ptr of font_datas) {
    Module._free(data_ptr);
  }

  Module._free(s);
  Module.ccall("free_fontstack", "number", ["number"], [fontstack_ptr]);

  self.postMessage({ stackId, done: true });
};
