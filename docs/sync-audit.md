# BLE Synchronization Audit

## Overview
This note captures the April 2025 investigation into pacing drift across the Track Pacer ESP32 controllers. The audit focused on the Expo app’s BLE command pipeline and the embedded firmware in `sketches/mainUnit*.cpp`.

## Critical Bugs
- **Segment math mismatch (`helpers/ble.js:75-119`) — resolved**  
  The app previously divided each lap into five equal delays while the firmware assumed four segments, causing downstream units to fire early. The mobile app now uses a four-way division to match firmware timing.
- **Unit 5 peer never registered (`sketches/mainUnit1.cpp:18`, `:134`, `:192-195`) — resolved**  
  The master controller now registers the Unit 5 MAC address during setup, allowing ESP-NOW broadcasts to reach the final junction box.
- **Dangling trigger timers (`helpers/ble.js:105-139`, `:163-192`) — resolved**  
  Timeout handles are tracked and cleared whenever a workout stops or restarts, preventing stale triggers from reigniting downstream units.

## Post-Fix Follow-Ups
1. Validate the refreshed pacing cadence on hardware to confirm lap timing stays consistent across all units.
2. Surface `esp_now_send` failures prominently in serial logs to aid future field diagnostics.
3. Expand automated coverage in the Expo app around pacing calculations to catch regressions early.

With the protocol now consistent, focus on on-track validation and additional monitoring to ensure long-term reliability.
