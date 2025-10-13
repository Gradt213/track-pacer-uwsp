<p align="center">
  <img src="https://github.com/erikberrg/tp-prod/blob/master/assets/images/icon.png" width="96" alt="Track Pacer icon" />
</p>

# Track Pacer System

Track Pacer is an interactive LED pacing system for a 200 m indoor track. The Expo (React Native) controller app lets coaches and athletes design pacing presets that sync color-coded LED segments around the track. ESP32 junction boxes relay timing data over BLE and ESP-NOW to keep each section in lockstep.

## Features
- Custom pacing presets with selectable distance, target time, repeats, delay, and color
- Real-time LED animation in the app that mirrors the live hardware state
- Multi-unit ESP32 orchestration using BLE for command intake and ESP-NOW for peer-to-peer distribution
- Light and dark UI themes optimized for quick adjustments trackside

## System Overview
- **Mobile app**: Expo Router project (`app/`, `components/`, `helpers/`) with AsyncStorage persistence and BLE control.
- **Hardware**: ESP32 master (`sketches/mainUnit1.cpp`) plus satellite units (`sketches/mainUnit2.cpp`, etc.) driving WS2812B LED strips via FastLED.
- **Connectivity**: App → master over BLE UART service, master → satellites via ESP-NOW, satellites manage their own animation loops.

## Repository Layout
- `app/` – Expo Router screens (`(main)/index.jsx`, `Modal.jsx`, `_layout.jsx`)
- `components/` – shared UI, pacing controls, and animation context
- `helpers/` – BLE transport, pacing calculations, and workout orchestration
- `constants/theme.js` – shared color palette, typography weights, radius tokens
- `sketches/` – Arduino/PlatformIO-ready firmware for each ESP32 unit
- `assets/` – icons, Lottie files, and other static media
- `docs/` – engineering notes such as `sync-audit.md`

## Getting Started
1. **Prerequisites**
   - Node.js ≥ 18 and npm (ships with Node)
   - Expo CLI (`npm install --global expo-cli`) for convenience
   - Xcode (iOS) or Android Studio (Android) if you test on simulators/emulators
2. **Install dependencies**
   ```bash
   npm install
   ```
3. **Run the development server**
   ```bash
   npm run start
   ```
   - Press `i` for iOS simulator, `a` for Android emulator, or scan the QR code with Expo Go on a device.

## Daily Development Commands
- `npm run ios` / `npm run android` – create a development build on the connected simulator/emulator.
- `npm run web` – launch the web preview (limited hardware integrations).
- `npm run lint` – lint the project using Expo’s configuration.
- `npm run test` – run Jest tests (use `--watch` while iterating).

## Build & Release Workflow

### Managed Expo builds (recommended)
1. Authenticate with Expo: `npx expo login`.
2. Configure `eas.json` for the target profile if needed.
3. Kick off a cloud build:
   - iOS: `npx expo run:ios --profile production`
   - Android: `npx expo run:android --profile production`
4. Download the artifacts from the Expo dashboard and distribute through TestFlight/Play Console.

### Bare builds (manual)
1. Create native projects: `npx expo prebuild`.
2. Bundle JavaScript: `npx expo export`.
3. Open `ios/` in Xcode or `android/` in Android Studio, drop in the generated bundle, and archive/install as required.

## Firmware Flash Guide

### Prerequisites
1. Install the Arduino IDE (2.x recommended).
2. Add the ESP32 board package via **Preferences → Additional Board Manager URLs**:  
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Install the **FastLED** library from the Library Manager.

### Configure & Upload
1. Connect the target ESP32 via USB-C and select the correct port and board (e.g., **ESP32 Dev Module**).
2. Open the relevant sketch from `sketches/`:
   - `mainUnit1.cpp` for the master controller (handles BLE + ESP-NOW distribution).
   - `mainUnit2.cpp` (and additional copies) for satellite sections.
3. Update configuration constants before flashing:
   - Verify `NUM_LEDS`, `PACER_LEDS`, and `STEP_SIZE` match the physical strip.
   - Confirm the MAC addresses for each downstream unit in `unit*Address[]`.
   - Align the pacing math with the mobile app (segment count and timing).
4. Click **Verify** to compile, then **Upload** to flash the firmware.

### Optional: Flash via CLI
If you prefer `esptool.py`, first compile for the board to obtain the `.bin`, then run:
```bash
esptool.py --chip esp32 --port /dev/tty.usbserial-XXXX --baud 460800 write_flash -z 0x1000 firmware.bin
```
Replace the port and firmware path with your local values.

## Troubleshooting & Support
- The in-app **Support** tab mirrors the guidance in `support.txt` (BLE pairing, pacing setup, FAQ).
- Ensure Bluetooth is enabled and ESP32 units are powered before starting a workout.
- Use the `sync-audit` document in `docs/` for current known issues around controller synchronization.

## License

This project is licensed under the MIT License. See the accompanying LICENSE file for details.
