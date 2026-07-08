# font-maker

The easiest way to turn your custom fonts into files compatible with [MapLibre GL](https://maplibre.org) (and Mapbox GL too).

For other prepared fonts, look at [maplibre/demotiles/font](https://github.com/maplibre/demotiles/tree/gh-pages/font) instead.

For an example of using font-maker on the command line to cover as much of Unicode as possible, see the [protomaps/basemaps-assets](https://github.com/protomaps/basemaps-assets) repository.

## Development

### Compile library

Create the image:

```bash
docker build -f Dockerfile.wasm -t font-maker-wasm --platform linux/amd64 .
```

Compile the library to wasm:

```bash
docker run --rm --platform linux/amd64 -v "$(pwd):/app" font-maker-wasm
```

This generates the `.wasm` and `.js` inside `app/public/`

### Compile app

Create the image:

```bash
docker build -f Dockerfile.app -t font-maker --platform linux/amd64 .
```

Run the web app:

```bash
docker run --rm -it --platform linux/amd64 -p 5174:5174 font-maker
```

Now you'll be able to open the web app hosted on `localhost:5174`
