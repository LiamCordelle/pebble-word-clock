# Word Clock

A standalone Pebble Time 2 watchface: time in a lit-letter grid, four minute dots,
date and a configurable caption. Tap the screen or flick your wrist for an
eight-second detail page: heart rate, steps, UV, temperature/condition, upcoming
sunrise/sunset and a second time zone (New Zealand by default). Tap/flick again
to return immediately.

The default main face preserves Watchface Lab's approved design. Status warnings
reuse its caption: charging, low battery and phone disconnected. When a low
battery and disconnection coincide, both appear. The date-caption setting moves
the date from the header to the caption and puts WORD CLOCK in the header so the
same value is shown once.

## Build

```sh
git submodule update --init shared-kit
npm ci
pebble build
python3 tests/run.py
python3 tools/render_scenes.py
python3 tools/create_assets.py
```

Output: `build/pebble-word-clock.pbw` (1.0.1). Target: Emery only (200×228).
Ordinary builds use live data (`LAB_SCENARIO=3`) and do not fetch dependencies.
`LAB_SCENARIO=0/1/2 pebble build` builds deterministic demo/stress/empty data.
Do not ship a scenario build.

The kit is a Git submodule plus source symlinks, as in Mondrian and Underground.
See [KIT.md](KIT.md). Kit 2.1.0 is committed and shared on GitHub; the submodule
pins its exact revision. `npm run release` fetches current kit main, tests, cleans
and builds live data. `npm run release -- --pinned` skips the fetch for a
reproducible historical build. Both refuse dirty or unpublished kit changes.
Neither command publishes to the store.

## Settings

Clay settings apply live and persist on the watch and phone:

- Soft or fully lit IT IS; steps/date/battery caption; dark or light ground.
- Watch-default, 12-hour or 24-hour format (sun/second-zone values; the English
  word grid naturally uses twelve hours and a soft AM/PM marker).
- Celsius/Fahrenheit, step goal (8,000), weather on/off, health demo fallback
  (off), NZ/UTC/London/New York/Los Angeles/Tokyo/Sydney second zone.

No seconds or animation. Health events update the shared model at most once per
minute. The kit owns health permissions/history, weather validation/three-hour
cache/watchdog/retries, serialized phone messaging and all shared settings.
The face owns its three extra settings and persist key 2000. All glyphs use
firmware fonts; no bundled font assets or degree glyph are needed (C/F is explicit).

## Visual acceptance

`tools/render_scenes.py` uses Watchface Lab's real PebbleOS host renderer. It
never starts QEMU and writes `shots/coverage.png` and `shots/coverage.json`.

48 scenes cover demo/stress/empty face and detail, 12-hour mode, midnight, sunset,
missing phone/health/weather, low battery, charging, negative temperature,
Fahrenheit, classic preamble, both captions, light mode and every weather kind.
The demo main face is pixel-identical to the approved prototype. Stress/empty
header, grid and dots are identical; their production status captions intentionally
differ. The small prototype captions and 14px grid caps are retained per the
face brief's explicit instruction to preserve its approved art.

Tap values use Gothic 24 bold (14px cap ink); condition and labels use Gothic 18
bold (11px caps). Labels were enlarged/brightened after the first glance check.
Eight half-resolution glance outputs are visually reviewed, including stress,
empty, light and 12-hour detail. Main and tap first-pass images are retained as
`shots/first-face.png` / `shots/first-detail.png`.

Shared tests cover health/history permission loss, weather protocol/cache/retries,
phone boot/configuration, release workflow and 143,192 DST comparisons. Consumer
native tests cover valid/partial/invalid settings, corrupt records, idempotence,
persistence and restart. The candidate kit passes isolated SDK builds plus 36
Mondrian and 62 Underground render scenes without editing either consumer.

## Final emulator and hardware limits

Run with pebble-tool's Python:

```sh
/Users/lcordelle/.local/share/uv/tools/pebble-tool/bin/python tools/emulator_check.py
```

The harness refuses to interrupt an existing emulator, temporarily saves the
Emery profile, verifies the active app UUID and captures evidence under
`shots/emulator/`. It restores the original profile and stops QEMU on exit.
It uses emulator health inputs and a current synthetic weather snapshot through
the production correlated AppMessage protocol; it does not claim a phone
geolocation fix or physical sensor measurements.

Final QEMU check passed: three fresh starts, three clean exits with zero app
allocations, Clay settings, persisted theme/preamble/caption, cached forecast,
eight-second return, status pixels and injected 64 BPM / 6,842 steps. Draw wall
times: **4–16ms**, with no app faults. The accepted restart test explicitly stops
the app; pressing Back alone did not terminate it. The synthetic weather sender
replies once, so cache verification is not masked by a new snapshot; the later
watchdog expiry is expected. Screenshots were visually reviewed and checked by
`python3 tools/verify_emulator.py`. Timings/results are in
`shots/emulator/result.json`.
The SDK build uses 19,294 bytes RAM plus 4.2KB resources, leaving over
111KB heap; exact build output is under `build/`.

Physical-watch checks remain: bright-daylight visibility of the original unlit
hairlines and the optional light theme; real health/geolocation permissions;
touch-controller battery consumption. QEMU cannot measure those. Touch stays
always available like the reference faces; `KIT_TOUCH_WINDOW_MS=10000 pebble build`
provides the kit's alternate ten-second touch window after a wrist flick for
on-device battery comparison. There is no continuous draw/animation timer.

## Store files

`screenshot_emery.png`, `screenshot_detail.png`, `icon_80x80.png`, `icon_144x144.png`
and [STORE.md](STORE.md). Static face: no animation GIF is needed.
See [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md).
Source: https://github.com/LiamCordelle/pebble-word-clock.
Version 1.0.1 is the first store release, approved on 22 October 2026.
A native 25×25 MENU_ICON is included in the PBW; the 80×80 and 144×144 images
are supplied separately as store icons. Store uploads use emery_face.png and
emery_detail.png, copied from the verified host-rendered screenshots.

## Published release

[Word Clock 1.0.1 on Pebble](https://apps.repebble.com/c9a837c5a40841b898ec9376) — published 22 October 2026.
The live listing contains the face and detail gallery images and declares native
launcher icon resource 1. The published PBW matches the clean, tested local PBW
byte for byte. Public release metadata and build provenance are recorded in
[store/published.json](store/published.json).

## Shared behaviour maintenance

This release uses kit 2.2.0 for shared data and calculations.
Artwork and explicit render fixtures remain face-owned. Run
`tools/emulator_smoke.py` with pebble-tool's Python for common runtime checks;
local emulator/visual checkers retain the face-specific assertions.
The published 8,000-step-default release is recorded in store/published.json.
