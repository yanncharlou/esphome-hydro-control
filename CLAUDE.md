# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

ESPHome firmware for a DIY hydroponics controller running on an ESP8266 (Wemos D1). It measures pH, EC/TDS and water temperature, and automatically doses pH-down and A/B nutrients via peristaltic pumps, exposing everything to Home Assistant. There is no application code to build in the usual sense — the YAML is compiled to firmware by ESPHome and flashed OTA.

## How this repo is consumed (important)

This directory is **not** a standalone ESPHome device config. It is included as a package by a parent device config that lives one level up in the ESPHome config root (`/config/esphome/`). See `README.md` for the parent snippet. Consequences:

- The parent config provides `wifi:` networks, `esphome.name`, and the `!secret` values (`api_encryption_key`, `ota_password`, wifi credentials). They are intentionally absent here.
- `includes:` paths in `main.yaml` (`hydro-control/sliding_window_limiter.h`, `hydro-control/globals.h`) are resolved **relative to the ESPHome config root**, i.e. `/config/esphome/`, not relative to this folder.
- Calibration constants come in as **substitutions** injected by the parent (`ec_ratio`, `pump{1,2,3}_ml_to_ms_ratio`). `main.yaml` defines fallback defaults for them, but the authoritative per-device values live in the parent config.

## Validate / compile / flash

There is no lint or unit-test suite. "Correctness" means the config compiles under ESPHome. Because the includes and secrets resolve from the parent config, run ESPHome against the **parent device YAML**, not against `main.yaml`:

```bash
esphome config   <parent-device>.yaml   # expand substitutions & validate schema
esphome compile  <parent-device>.yaml   # full C++ build (catches lambda/header errors)
esphome run      <parent-device>.yaml   # compile + OTA upload
```

In this Home Assistant setup this is normally driven from the ESPHome dashboard add-on. `esphome compile` is the real check for anything touching lambdas, `globals.h`, or `sliding_window_limiter.h`.

## Architecture

- **`main.yaml`** — the whole device: I2C bus, ADS1115 ADC, Dallas 1-Wire temp sensor, SH1106 OLED (3 pages), sensors, pump switches, calibration buttons, dosing scripts, and the regulation `interval:` loops. It `!include`s the two limit packages.
- **`nutrient_limit.yaml` / `ph_minus_limit.yaml`** — self-contained HA-facing packages (a daily-limit `number`, a flow `sensor`, a reset `button`) that wrap the two sliding-window limiters.
- **`sliding_window_limiter.h`** — `SlidingWindowLimiter`, a fixed-capacity ring buffer that enforces a rolling 24h dosing cap (guards against runaway dosing from a bad probe reading).
- **`globals.h`** — instantiates the two limiter objects as raw C++ pointers (`nutrient_limiter`, `ph_minus_limiter`).

### Why the limiters live in a C++ header

ESPHome `globals:` cannot hold instances of a custom C++ class (see the commented-out block and issue link in `ph_minus_limit.yaml`). So the limiter objects are declared as free C++ pointers in `globals.h` and referenced by name from lambdas across all three YAML files (e.g. `ph_minus_limiter->addIfPossible(...)`). This cross-file dependency on symbols defined in a header is the main non-obvious coupling in the project — the YAML lambdas will only compile because `main.yaml` includes `globals.h`.

### Regulation flow

`interval:` loops (in `main.yaml`) run every minute. When a `*_regulation_enabled` switch is on, the reading is out of range, the tank isn't flagged empty, and no error/pulse is active, they fire a dosing **script** (`ph_minus_pulse` / `nutrient_pulse`). Each script asks its limiter `addIfPossible(ml)` first; if the rolling 24h cap is exceeded the dose is skipped. Doses convert ml → pump-on milliseconds via the `pump*_ml_to_ms_ratio` substitutions, and trigger a 5-min `mixer_pulse`.

### pH measurement & calibration

Two-point calibration with Nernst temperature compensation. Raw ADS1115 voltage is smoothed and stored in `ph_raw_median`. Calibration buttons capture the buffer voltages into flash-persisted globals `ph_cal_v1` (pH 7, the module's hardware iso-electric offset, stored **raw** because it doesn't follow Nernst) and `ph_cal_v2` (pH 4, stored **normalized to 25 °C**). The pH sensor lambda re-projects `ph_cal_v2` to the current temperature each read and interpolates. When editing this math, keep the store-time vs. read-time Nernst handling consistent — the asymmetry between the two cal points is deliberate.

### EC / TDS measurement

ADS1115 voltage → median filter → temperature-compensated cubic TDS polynomial (DFRobot Gravity formula) scaled by the `ec_ratio` substitution. `ec_base` (pre-ratio) and `ec_raw_median` are exposed on OLED page 2 for calibration.

## Conventions

- UI-facing strings, comments and `name:` fields are in **French**; keep that when adding entities.
- Persisted tuning values use `restore_value: true` (template `number`s) or `restore_from_flash` (globals) — changing an `id:` or default resets the stored value on device.
- `home-assistant/dashboard.yaml` is the companion Lovelace dashboard; keep entity ids in sync when renaming.
