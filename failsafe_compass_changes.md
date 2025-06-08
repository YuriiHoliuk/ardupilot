# Failsafe Compass Mode - Code Changes Documentation

## Overview
This document details all code changes made to implement the Failsafe Compass mode POC in ArduCopter.

## Files Modified/Added

### 1. New Files Created

#### `/ArduCopter/mode_failsafe_compass.cpp`
- New mode implementation file
- Contains `ModeFailsafeCompass` class implementation
- Key methods:
  - `init()`: Initializes altitude controller and sets target heading
  - `run()`: Main control loop - handles altitude, heading, and forward flight
  - `wp_distance_m()`, `wp_bearing()`: Navigation interface methods

### 2. Modified Files

#### `/ArduCopter/mode.h`
- **Line 103**: Added `FAILSAFE_COMPASS = 29` to Mode::Number enum
- **Lines 2064-2101**: Added `ModeFailsafeCompass` class declaration with:
  - Mode configuration (no GPS required, autopilot mode, no manual throttle)
  - Navigation interface methods
  - Private member variable for heading target only

#### `/ArduCopter/config.h`
- **Lines 241-245**: Added `MODE_FAILSAFE_COMPASS_ENABLED` definition (enabled by default)

#### `/ArduCopter/Copter.h`
- **Lines 1105-1107**: Added mode instance `ModeFailsafeCompass mode_failsafe_compass;`

#### `/ArduCopter/Parameters.h`
- **Lines 670-672**: Added parameters to ParametersG2 class:
  - `fs_compass_heading`: Target heading in degrees (0-359)
  - `fs_compass_pitch`: Forward pitch angle in degrees (-45 to +45)
  - `fs_compass_hdg_ch`: RC channel for heading adjustment (0=disabled)

#### `/ArduCopter/Parameters.cpp`
- **Lines 1238-1243**: Added `FS_COMPASS_HDG` parameter definition
  - Range: 0-359 degrees
  - Default: 0 (North)
  - ID: 11 in var_info2 table
- **Lines 1245-1252**: Added `FS_COMPASS_PITCH` parameter definition
  - Range: -45 to +45 degrees
  - Default: -5 degrees (forward flight)
  - ID: 12 in var_info2 table
- **Lines 1254-1259**: Added `FS_COMPASS_HDG_CH` parameter definition
  - Values: 0=Disabled, 5-16 for CH5-CH16
  - Default: 0 (disabled)
  - ID: 13 in var_info2 table
- **Line 208**: Updated `FS_THR_ENABLE` parameter documentation to include option 8

#### `/ArduCopter/mode.cpp`
- **Lines 155-158**: Added case for `FAILSAFE_COMPASS` in `mode_from_mode_num()` switch

#### `/ArduCopter/defines.h`
- **Line 127**: Added `FS_THR_ENABLED_COMPASS = 8` failsafe option

#### `/ArduCopter/events.cpp`
- **Lines 42-47**: Added handling for `FS_THR_ENABLED_COMPASS` in radio failsafe event
  - Directly switches to FAILSAFE_COMPASS mode when this option is selected

#### `/ArduCopter/radio.cpp`
- **Lines 196-222**: Added `update_fs_compass_heading_from_rc()` function
  - Monitors configured RC channel continuously
  - Maps RC input (-100% to +100%) to heading (0-360 degrees)
  - Updates FS_COMPASS_HDG parameter in real-time (without EEPROM save)

#### `/ArduCopter/Copter.cpp`
- **Lines 529-532**: Added call to `update_fs_compass_heading_from_rc()` in rc_loop
  - Called at 100Hz with other RC processing
  - Only active when FS_COMPASS_HDG_CH is configured

#### `/ArduCopter/Copter.h`
- **Lines 949-951**: Added function declaration for `update_fs_compass_heading_from_rc()`

## Key Implementation Details

### Mode Behavior
1. **Altitude Control**: 
   - Maintains current altitude when failsafe activates
   - Uses barometer-based altitude controller
   - No climbing or altitude changes

2. **Heading Control**:
   - Turns to heading specified in `FS_COMPASS_HDG` parameter
   - Uses IMU data with AHRS fusion (no compass dependency)
   - Maintains heading using yaw control

3. **Forward Flight**:
   - Open-loop control with configurable pitch angle (FS_COMPASS_PITCH parameter, range -45 to +45 degrees)
   - Negative values for forward flight, positive values for backward flight
   - No velocity feedback required (GPS-free operation)
   - Simple pitch command in target heading direction

### Technical Notes
- Uses UP axis position controller APIs (unified position)
- Open-loop control - no velocity feedback needed
- No position control - pure heading and pitch based flight
- No obstacle avoidance or terrain following
- Completely GPS-free and compass-free operation
- Uses IMU/AHRS for heading without compass dependency

## Build System
- No changes needed to wscript - mode files are automatically included
- Successfully builds for MatekH743 board
- All compilation errors resolved

## Parameter Summary
- `FS_COMPASS_HDG`: Target heading in degrees (0-359)
- `FS_COMPASS_PITCH`: Pitch angle in degrees (-45 to +45, default -5°, negative=forward flight)
- `FS_COMPASS_HDG_CH`: RC channel for heading adjustment (0=disabled, 5-16 for CH5-CH16)
- `FS_THR_ENABLE = 8`: New option to activate compass failsafe mode

### RC Channel Heading Control
- When `FS_COMPASS_HDG_CH` is configured to a valid channel (5-16):
  - RC input is continuously monitored at 100Hz
  - Input mapping: -100% = 0°, 0% = 180°, +100% = 360°
  - Updates `FS_COMPASS_HDG` parameter in real-time
  - Changes persist until power cycle (not saved to EEPROM)
  - Allows pilot to adjust escape heading during flight

## Testing Status
- Code compiles successfully
- Not yet flight tested
- Requires SITL and real-world testing for validation