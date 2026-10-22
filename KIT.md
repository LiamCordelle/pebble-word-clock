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
