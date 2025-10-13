# BLE Synchronization Audit

## Overview
This note captures the April 2025 investigation into pacing drift across the Track Pacer ESP32 controllers. The audit focused on the Expo app’s BLE command pipeline and the embedded firmware in `sketches/mainUnit*.cpp`.

## Critical Bugs
- **Segment math mismatch (`helpers/ble.js:74-145`)**  
  The app divides each lap into five equal delays (`segmentDelayMs = lapDuration * 1000 / 5`), but the firmware assumes four segments (`sketches/mainUnit1.cpp:92` and `sketches/mainUnit2.cpp:49`). Unit 2 therefore lights up after 20% of the lap while its internal pacing expects a 25% window, cascading desync for Units 3–5.
- **Unit 5 peer never registered (`sketches/mainUnit1.cpp:18`, `:134`, `:154`)**  
  The mobile app emits a `five` command, yet the master controller never calls `addPeer(unit5Address)`. The ESP-NOW send silently fails, so the last controller never starts with the rest.
- **Dangling trigger timers (`helpers/ble.js:103-145`, `:158-167`)**  
  `setTimeout` calls queue downstream triggers, but `sendStop` lacks `clearTimeout`. Stopping or restarting mid-lap still fires stale `two`/`three`/`four`/`five` commands, reawakening remote units out of phase.

## Recommended Fixes
1. Align the segment count on both app and firmware (either shift the app to four segments or adjust every ESP sketch to five). Validate the resulting lap timing on hardware.
2. Register every active peer in `setup()` and surface `esp_now_send` failures so missing devices are obvious during field tests.
3. Track timeout handles in `sendPacer` and cancel them inside `sendStop()` and before launching a fresh lap to prevent orphaned triggers.

Tackle the segment math first; it drives the largest observed drift. Once the protocol is consistent, re-run pacing trials and expand unit tests around the timing utilities to catch future regressions.
