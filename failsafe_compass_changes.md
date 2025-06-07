# Failsafe Compass Mode - Code Changes Documentation

## Overview
This document details all code changes made to implement the Failsafe Compass mode MVP and V2 enhanced features in ArduCopter.

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
- **Lines 2064-2100**: Added `ModeFailsafeCompass` class declaration with:
  - Mode configuration (no GPS required, autopilot mode, no manual throttle)
  - Navigation interface methods
  - Private member variable for heading target only

#### `/ArduCopter/Copter.h`
- **Line 611**: Added `FAILSAFE_COMPASS = 8` to FailsafeAction enumeration

#### `/ArduCopter/config.h`
- **Lines 241-245**: Added `MODE_FAILSAFE_COMPASS_ENABLED` definition (enabled by default)

#### `/ArduCopter/Copter.h`
- **Lines 1105-1107**: Added mode instance `ModeFailsafeCompass mode_failsafe_compass;`

#### `/ArduCopter/Parameters.h`
- **Lines 670-675**: Added compass failsafe parameters to ParametersG2 class:
  - `fs_compass_heading`: Target heading (0-359 degrees)
  - `fs_compass_pitch`: Forward pitch angle (5-20 degrees)
  - `fs_compass_hdg_src`: Heading source selection (0=fixed, 1=home)

#### `/ArduCopter/Parameters.cpp`
- **Lines 1209-1234**: Added parameter definitions in var_info2 table:
  - `FS_COMPASS_HDG`: Target heading (default 0°, range 0-359)
  - `FS_COMPASS_PITCH`: Forward pitch angle (default 5°, range 5-20)
  - `FS_COMPASS_HDG_SRC`: Heading source (default 0=fixed heading)
- **Line 208**: Updated `FS_THR_ENABLE` parameter documentation to include option 8

#### `/ArduCopter/mode.cpp`
- **Lines 155-158**: Added case for `FAILSAFE_COMPASS` in `mode_from_mode_num()` switch

#### `/ArduCopter/defines.h`
- **Line 127**: Added `FS_THR_ENABLED_COMPASS = 8` failsafe option

#### `/ArduCopter/events.cpp`
- **Lines 42-46**: Added handling for `FS_THR_ENABLED_COMPASS` in radio failsafe event
  - Uses proper failsafe action framework instead of direct mode switch
- **Lines 512-524**: Added `FailsafeAction::FAILSAFE_COMPASS` case in `do_failsafe_action()`
  - Attempts to switch to FAILSAFE_COMPASS mode
  - Falls back to LAND mode if compass failsafe unavailable or disabled

## Key Implementation Details

### Mode Behavior
1. **Altitude Control**: 
   - Climbs to RTL altitude parameter value
   - Uses barometer-based altitude controller
   - Maintains altitude once reached (±2m tolerance)

2. **Heading Control**:
   - Turns to heading specified in `FS_COMPASS_HDG` parameter
   - Uses compass data with AHRS fusion
   - Maintains heading using yaw control

3. **Forward Flight**:
   - Open-loop control with configurable pitch angle (FS_COMPASS_PITCH parameter)
   - No velocity feedback required (GPS-free operation)
   - Simple pitch command in target heading direction

### Technical Notes
- Uses UP axis position controller APIs (unified position)
- Open-loop control - no velocity feedback needed
- No position control - pure heading and pitch based flight
- No obstacle avoidance or terrain following
- Completely GPS-free operation
- Integrated with ArduCopter's FailsafeAction framework for consistent failsafe behavior
- Includes fallback logic to LAND mode if compass failsafe is unavailable

## Build System
- No changes needed to wscript - mode files are automatically included
- Successfully builds for MatekH743 board
- All compilation errors resolved

## Parameter Summary
- `FS_COMPASS_HDG`: Target heading in degrees (0-359, default 0)
- `FS_COMPASS_PITCH`: Forward pitch angle in degrees (5-20, default 5)
- `FS_COMPASS_HDG_SRC`: Heading source (0=fixed heading, 1=home direction, default 0)
- `FS_THR_ENABLE = 8`: New option to activate compass failsafe mode

## MVP Features Added
- **Configurable Pitch Control**: Uses FS_COMPASS_PITCH parameter instead of hardcoded value
- **Home Direction Option**: Can calculate heading to home using EKF relative position (GPS-free)
- **Parameter Validation**: Validates parameter ranges on mode initialization
- **GCS Status Reporting**: Provides feedback on mode activation and heading source used
- **Proper Failsafe Action Integration**: Uses ArduCopter's FailsafeAction framework instead of direct mode switching

## V2 Features Added
- **RC Channel Heading Adjustment**: FS_COMPASS_HDG_CH parameter allows heading to be adjusted via RC channel before failsafe
- **Advanced Throttle Control**: FS_COMPASS_THR_CTRL_ENABLED enables automatic pitch adjustment to maintain target throttle
- **Dynamic Pitch Range**: FS_COMPASS_PITCH_MIN/MAX parameters define the pitch adjustment range
- **PID Controller Integration**: Integrated AC_PID controller for smooth pitch adjustments based on throttle feedback

## V2 Parameter Summary
- `FS_COMPASS_HDG_CH`: RC channel for heading adjustment (0=disabled, 1-16 for channel selection)
- `FS_COMPASS_THR_CTRL_ENABLED`: Enable/disable advanced throttle control (0=disabled, 1=enabled)
- `FS_COMPASS_TARGET_THR`: Target throttle percentage (20-80%, default 50%)
- `FS_COMPASS_PITCH_MIN`: Minimum pitch angle (3-15°, default 3°)
- `FS_COMPASS_PITCH_MAX`: Maximum pitch angle (10-30°, default 20°)

## V2 Implementation Details
1. **RC Heading Control**: 
   - Maps RC channel input (-100% to 100%) to heading (0° to 360°)
   - Uses last known heading value when failsafe triggers
   - Falls back to fixed heading if RC channel is invalid

2. **Throttle Control System**:
   - Monitors current throttle output
   - Compares with target throttle (±5% tolerance)
   - Uses PID controller to calculate pitch adjustments
   - Constrains pitch within min/max limits
   - Resets integrator when within tolerance to prevent windup

3. **PID Controller Configuration**:
   - P=1.0: 1% throttle error = 1° pitch change
   - I=0.1: Slow integration for stability
   - D=0: No derivative term for simplicity
   - I_max=5.0: Limits integral windup to ±5°

## Testing Status
- Code compiles successfully for MatekH743 board
- MVP implementation complete
- V2 enhanced features implemented
- Ready for SITL and real-world testing for validation