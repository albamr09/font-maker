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

This generates the `.wasm` and `.js` inside `app/public/`

To compile **library (CLI)**

```bash
docker-compose run --rm --remove-orphans cli-build
```

Use the CLI inside the container

```bash
docker-compose run --rm --remove-orphans cli
```

```bash
./font-maker --name "Noto Sans" fonts/$OUTPUT_FOLDER fonts/$FONT_PATH.ttf
```
