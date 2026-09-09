# Sample (config-driven local / remote)

A C++ project template based on [Foundation](https://github.com/pierre-quelin/Foundation)

Thin application shell. One generic `sample` executable; **behavior comes from config**. It consumes **Foundation** through [`Description.xml`](Description.xml).

## License

This project is licensed under the **MIT License**.

For the full license text, see: https://opensource.org/license/mit/

## Shared configuration

| File | Role |
|------|------|
| [`cfg/main.json`](cfg/main.json) | **Unique** factory graph — Local vs Remote via optional `Platform` branches |
| [`cfg/local/main.ini`](cfg/local/main.ini) | `platformName=Local` |
| [`cfg/remote/main.ini`](cfg/remote/main.ini) | `platformName=Remote` |

`platformName` in the `.ini` is copied to `ApplicationServices::platformName` and selects `Platform.<name>` where present. Nodes without `Platform` are unchanged.

Local exposes selected IO / Serport with `"Bridged": "<BridgeType>"`. Remote uses Ghosts under the same instance paths (e.g. `IOBoard.pres24v`).

## Build

```bat
Build.bat fetch
Build.bat gen
```

Install layout (same binary + same `main.json`, different `.ini`):

```text
dist/<BUILD_TARGET>/local/sample.exe
dist/<BUILD_TARGET>/local/main.ini
dist/<BUILD_TARGET>/local/cfg/main.json
dist/<BUILD_TARGET>/remote/sample.exe
dist/<BUILD_TARGET>/remote/main.ini
dist/<BUILD_TARGET>/remote/cfg/main.json
```

## Run (lab)

1. MQTT broker on `tcp://127.0.0.1:1883` (or change `EventBus.Platform.*.Transport.Broker`).
2. Set `Serport.DeviceName` in `cfg/main.json` as needed.
3. Start **local** first, then **remote**:

```bat
cd dist\<BUILD_TARGET>\local
sample.exe

cd dist\<BUILD_TARGET>\remote
sample.exe
```

Logger TCP: **8023** (local) / **8024** (remote).

## Layout

| Path | Role |
|------|------|
| `src/main.cpp` | Generic boot (`platformName`, `wireEventBus`, GlobalObjects) |
| `cfg/main.json` | Shared config |
| `cfg/local/`, `cfg/remote/` | Per-deploy `main.ini` only |
| `lib/Foundation` | Fetched Foundation tree |