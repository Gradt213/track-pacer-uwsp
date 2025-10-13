# Repository Guidelines

## Project Structure & Module Organization
Track Pacer runs on Expo Router. Screen entry points live in `app/`; `_layout.jsx` drives navigation and `(main)/` holds tabs like `index.jsx`. Reusable UI sits in `components/` (see `components/ui/AnimatedPressable.jsx`), domain logic in `helpers/`, and theme tokens in `constants/theme.js`; never hard-code colors. Assets go in `assets/`, hardware sketches in `sketches/`. Limit files to 200 lines and split shared patterns early.

## Build, Test, and Development Commands
Run `npm install` after cloning. `npm run start` launches the Expo dev server; pick the target from the CLI. Use `npm run ios`, `npm run android`, or `npm run web` for platform previews. `npm run test` runs Jest (add `--watch` while iterating). `npm run lint` enforces the Expo lint config—fix issues before commits.

## Coding Style & Naming Conventions
Write ES modules with semicolons and 2-space indentation. Components and hooks use PascalCase and camelCase; screens mirror their route folder names (e.g. `app/(main)/presets.jsx`). Keep business logic in helpers and presentation in components. Centralize colors, typography, and spacing through `theme`. UI must support both `lightColors` and `darkColors`; branch styles with `useColorScheme` or props. Prefer `StyleSheet.create` for larger style blocks. Document public components with TSDoc/JSDoc headers.

## Testing Guidelines
Jest with `jest-expo` is the default harness. Keep tests beside code in `__tests__/` folders or `*.test.tsx` files. Every feature ships with at least one unit test plus an integration check for flows like BLE pacing or preset management. Name files `ComponentName.behavior.test.tsx`, mock native modules carefully, and note coverage expectations in pull requests.

## Commit & Pull Request Guidelines
Use Conventional Commits (e.g. `feat: add lap timing summary card`) and squash before merging. Pull requests need a concise summary, linked issue or todo entry, UI captures when visuals change, and a checklist of `npm run test`, lint, and relevant device smoke tests. Flag hardware prerequisites or new env vars so reviewers can validate behavior.

## Security & Configuration Tips
Store secrets in `.env` and load them through `react-native-dotenv`; never commit credentials. Update BLE commands via `helpers/ble.js` and keep Android permission prompts localized. Review `support.txt` and hardware docs before altering command sequences so deployments stay reliable.
