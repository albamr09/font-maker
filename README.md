# font-maker

The easiest way to turn your custom fonts into files compatible with [MapLibre GL](https://maplibre.org) (and Mapbox GL too).

For other prepared fonts, look at [maplibre/demotiles/font](https://github.com/maplibre/demotiles/tree/gh-pages/font) instead.

For an example of using font-maker on the command line to cover as much of Unicode as possible, see the [protomaps/basemaps-assets](https://github.com/protomaps/basemaps-assets) repository.

## Development

To run the **web app**

```bash
docker-compose up app
```

Now you'll be able to open the web app hosted on `localhost:5174`

To compile the **library (WASM)**

```bash
docker-compose run --rm --remove-orphans wasm
```

This generates the `.wasm` and `.js` for both modes inside `app/public/`:

- `sdfglyph.js` / `sdfglyph.wasm` - normal mode
- `sdfglyph-complex.js` / `sdfglyph-complex.wasm` - complex mode

To compile **library (CLI)**

```bash
docker-compose run --rm --remove-orphans cli-build
```

This produces two binaries, `font-maker` and `complex-font-maker`.

Use the CLI inside the container

```bash
docker-compose run --rm --remove-orphans cli
```

### Normal mode

Convert as many fonts as you want into a single fontstack of unicode-indexed PBFs:

```bash
./font-maker --name "Noto Sans" fonts/$OUTPUT_FOLDER fonts/$FONT_PATH_1.ttf fonts/$FONT_PATH_2.ttf
```

### Complex mode

For complex scripts, `complex-font-maker` takes exactly two fonts: a base font
(emitted as unicode-indexed glyphs) and one complex-script font (emitted as
glyph-indexed glyphs, offset by `0x10000`):

```bash
./complex-font-maker fonts/$OUTPUT_FOLDER fonts/$BASE_FONT.ttf fonts/$COMPLEX_FONT.ttf
```
