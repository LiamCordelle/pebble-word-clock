# Word Clock kit dependency

Uses `shared-kit`, a Git submodule of
https://github.com/LiamCordelle/pebble-watchface-kit.git (branch `main`).
Kit **2.1.0**, revision `ae4bbc8aeca34eed1e7bfad2372bd5022f38d623`. The shared extension is
committed and pushed to its GitHub main branch before this consumer release.
The app pins that revision as a submodule; ordinary builds do not fetch.
Release commands require a clean, published dependency and record provenance.
No other local consumer or central kit checkout has been changed.

The C kit, phone kit, lab.h and lab_common.c are source symlinks into the
submodule, exactly as in Mondrian and Underground. There is no copied kit or
shared runtime. Every PBW includes its dependency.

## 2.1.0 — 22 October 2026

- Optional generic `faceSettings` schema entries flow through the kit's existing
  Clay, normalization, namespace storage and acknowledged AppMessage queue.
- Weak consumer init/receive hooks let a face own its settings and persistence
  keys. Existing consumers use no-op defaults; common protocol numbering and
  persisted structures are unchanged.
- Common drawing helpers compile when optional PDC/font resources are omitted.
  Missing PDCs draw nothing; font helpers fall back to system fonts.
- Word Clock owns PREAMBLE_LIT/CAPTION_MODE/LIGHT_THEME (wire keys 10012–10014),
  stored as a validated three-byte record at watch persistence key 2000.
  Phone namespace: `word-clock-kit-settings-v1`. Common wire keys remain 10000–10011.

Validation: shared health/history/protocol/weather/config/bootstrap/workflow
suite, 143,192 independent DST comparisons, consumer settings tests; isolated
SDK builds and 36 Mondrian / 62 Underground render scenes pass. Word Clock
covers 48 renders, demo/stress/empty reference checks and eight glance outputs.
See README.md for final emulator evidence and physical-watch limits.

Canonical extension documentation: `shared-kit/src/c/kit/KIT.md`.

## Final Emery check — 22 October 2026

Passed on the live PBW: verified UUID, three fresh starts with explicit
AppRunStateStop/Start, Clay-applied theme/preamble/caption, watch/phone persistence,
eight-second return, cached forecast with no second injected weather reply,
low battery, charging, disconnect/reconnect and synthetic health values (64 BPM,
6,842 steps). Screenshot comparisons pass; frames 4–16ms; no app faults; three
exits report zero app allocations. Evidence: `shots/emulator/` and result.json.
The original SDK profile was restored and QEMU stopped. The deliberate missing
second weather reply exercises the watchdog while retaining the valid cache.

The initial Back-only restart check was replaced because Back did not terminate
the face. Only the explicit-stop run is accepted as persistence evidence.

## Release 1.0.1 — 22 October 2026

Shared kit 2.1.0 is committed/pushed as ae4bbc8aeca34eed1e7bfad2372bd5022f38d623.
The consumer's release workflow passed from a clean committed tree, including
all tests, live build and 48 renders. Word Clock 1.0.1 is published on Pebble;
store/published.json records its exact source/kit revisions and matching local
and public PBW hashes. No other consumer checkout was modified.

## 8,000-step default — 22 October 2026

Updated to shared kit 2.1.2 (`71a935d`). Fresh watch/phone settings and
face fallbacks use 8,000 steps. Valid saved goals are preserved. Protocol keys,
persistence layouts and explicit screenshot fixtures are unchanged. Shared and
consumer tests, all five render suites and fresh-profile QEMU checks cover the
new default and a saved 12,500-step goal across explicit stop/start.

## Shared behaviour extraction — kit 2.2.0

Moon/live-data assembly, temperature/event clocks, bounded forecast queries,
daylight state and activity estimates now use the shared kit. Drawing and
scenario fixtures remain here. Underground Fahrenheit now rounds consistently;
unknown daylight stays unknown and activity math uses 64-bit intermediates.
The thin tools/emulator_smoke.py entrypoint uses shared profile/transport/
fixture/log utilities. Existing face-specific visual checkers remain available.

Final dependency: `c737afcfadf8cdd62dec58941cf8718fed0d6017` (kit 2.2.0). Shared/native/phone
tests, all 271 cross-consumer scenes and the local shared emulator smoke pass.
Runtime evidence is in shots/emulator-smoke/result.json; fresh 8,000 and saved
12,500 goals, cold weather-cache restoration, detail timeout and clean exits
are verified. The original SDK profile was restored.

## 1.1.0 timezone upgrade — 1 November 2026

Updated the clean shared-kit dependency from kit 2.2.0
`c737afcfadf8cdd62dec58941cf8718fed0d6017` to kit 2.4.1
`2dcfa7302bf07aabb9a1f5f0974064b638dd5ac8` after fetching origin/main.
The selected revision is the published, tested main used by Mondrian 1.1.0.
No shared-kit edits were required. This face opts into `fullTimezones: true`;
the kit owns search, transport, validation, persistence and offline transitions.
Existing storage namespace, message keys and valid saved goals are preserved.

Display → Find a time zone filters by city, region or IANA name; explicitly
select a result and save. Empty search shows common choices plus the selected
zone. Search text is temporary. The bundled tzdata 2026f covers 2020–January
2038; future timezone rule changes arrive in watchface updates.

The face owns the preview WORLD override and render scenarios for Mumbai,
Abu Dhabi, Moscow, Kathmandu, Chatham, Kiritimati and UTC−12 in both time formats.
Release receipts in build/ record the final kit SHA and PBW hash.

Validation completed for 1.1.0: shared/native and consumer tests passed;
`npm run release -- --render` passed 76 scenes. QEMU saved and restarted
eight timezone selections through Clay, retained a 12,500-step saved goal,
and matched the live clock glyphs against deterministic renders in both time
formats. Fresh defaults remained 8,000. All nine app exits freed allocations;
no runtime faults were reported. The owned emulator was stopped and the
original SDK profile restored. Screenshots and reports are in
`shots/emulator-timezones/`. Default prototype artwork still passes its pixel checks.
Pre-commit QEMU-tested PBW SHA-256: `de48923734167263c30e1b847f91f314e1ad88f65258e74deb1f0a3609211393`.
Release build provenance records the consumer source revision and dependency.
Store publication and verification are recorded in store/published.json.

## Published 1.1.0 — 1 November 2026

Source release commit: `13ee2aca9f55fcc3b2ceb49dc811cc426097e1d9`.
Published build SHA-256: `c405e4b52a2fd4dc882a1c1ff6132f0188c21d58576dcd63870071b5e9108210`.
The committed-source clean build changes SDK resource timestamps and JSON
capability ordering; executable bytes outside the SDK resource-timestamp field,
resources and phone code match the broader eight-zone runtime-tested build.
The publish command uploaded this final artifact without changing its hash.
Authenticated store records confirm the existing app, visible published 1.1.0,
PBW and retained gallery. See `store/published.json` and `store/publish-1.1.0.log`.
Final uploaded-artifact QEMU verification also passes Mumbai (WORLD 289),
Clay save/restart, retained 12,500-step goal, pixel-exact clock glyphs and
clean exits without runtime faults. Its screenshots and report are in
`shots/emulator-publication/`; the emulator is stopped and SDK profile restored.
