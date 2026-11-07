# External Valorant Cheat

Read-only external cheat. No injection. Overlay is its own click-through window.

## Build

```sh
cmake -B build -A x64
cmake --build build --config Release
```

Run build/Release/ValoBoggiee.exe as Administrator.

## Backends

- handle (default) - OpenProcess + ReadProcessMemory. Works immediately, slightly higher exposure.
- driver - \\.\iqvw64e. Install the signed Intel driver first, then set "backend": "driver".

## Offsets

data/offsets.json holds one RVA per field. On attach every field is scanned. A failed scan falls
back to the JSON value and logs WARN. Update after each patch.

## Risk table

| Feature      | Access       | Risk |
|--------------|--------------|------|
| Radar        | read         | low  |
| Head marker  | read         | low  |
| Recoil       | read + input | low  |
| Movement     | input        | low  |
| Utility      | read         | low  |
| Skin changer | write        | med  |

## Calibration

Recoil tables live in data/recoil.json, per weapon, indexed by shot. Run the debug view, fire one
magazine per weapon, scale with the multiplier until the reticle holds. Sensitivity dependent.

## Keys

Insert opens the menu. Everything else unbound until you bind it.
