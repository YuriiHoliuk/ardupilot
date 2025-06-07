# ArduCopter Failsafe_Compass Mode Implementation

## Overview

Failsafe_Compass is a minimalistic failsafe mode for ArduCopter that enables the aircraft to fly in a predetermined direction using only IMU data (gyroscope and accelerometer), without requiring GPS, compass, or other positioning systems. This mode activates during radio failsafe events to help the aircraft escape from its current location.

## Core Concept

When radio communication is lost, the aircraft will:

1. Turn to a preset heading
2. Maintain current altitude
3. Fly forward using configurable pitch control
4. Continue until pilot regains control

## Implementation Phases

### POC (Proof of Concept)

**Goal**: Validate basic heading-based flight without position control

**Features**:

- Single parameter: `FS_COMPASS_HDG` (0-359 degrees)
- Configurable pitch angle parameter: `FS_COMPASS_PITCH` (default 5 degrees)
- Maintain current altitude - no altitude changes
- No adjustable parameters during flight

**Behavior**:

```
On Radio Failsafe:
1. Maintain current altitude
2. Turn to FS_COMPASS_HDG
3. Apply FS_COMPASS_PITCH forward pitch
4. Continue until manual recovery
```

### MVP (Minimum Viable Product)

**Goal**: Add essential configurability and safety features

**New Parameters**:

- `FS_COMPASS_HDG`: Target heading (0-359°)
- `FS_COMPASS_PITCH`: Forward pitch angle (5-20°, default 5°)

**Features**:

- Configurable pitch angle
- Maintain current altitude - no altitude control needed

**Behavior**:

```
On Radio Failsafe (if FS_COMPASS_MODE failsafe action is selected):
1. Maintain current altitude
2. Turn to FS_COMPASS_HDG heading
3. Apply FS_COMPASS_PITCH forward pitch
4. Continue until manual recovery
```

### V2 (Enhanced Version)

**Goal**: Add dynamic control and improved safety

**Additional Parameters**:

- `FS_COMPASS_HDG_CH`: RC channel for heading adjustment (0=disabled)
- `FS_COMPASS_OSD_ENABLE`: Enable OSD display of target heading (0=disabled, 1=enabled)
- `FS_COMPASS_OSD_ITEM`: OSD item slot for heading display (configurable without ground station modification)

**Features**:

- RC-adjustable heading (when link available before failsafe)
- Real-time OSD display of configured target heading
- Live heading updates visible during flight as pilot adjusts RC channel

**Enhanced Behavior**:

```
Before failsafe:
- Monitor FS_COMPASS_HDG_CH for heading updates
- Display current target heading on OSD (if enabled)
- Update OSD display in real-time as pilot adjusts heading

On Radio Failsafe:
1. Use last known heading from RC channel (if configured), otherwise use FS_COMPASS_HDG
2. Maintain current altitude
3. Apply configured pitch angle and maintain heading
4. Continue until manual recovery
```

## Technical Implementation Notes

### Altitude Control

- Maintain current altitude when failsafe activates
- Use barometer for altitude hold
- No climbing or altitude changes to avoid obstacles

### Heading Control

- Use IMU data only (gyroscope and accelerometer)
- AHRS provides heading estimate without compass dependency
- Implement turn rate limiting for smooth transitions
- Note: Heading may drift over time without compass reference

### Pitch Control

- Direct pitch angle command
- No position or velocity feedback required
- Let natural aircraft dynamics determine speed

### State Management

- New flight mode: `FAILSAFE_COMPASS`
- Integrate with existing failsafe system
- Clear mode exit only through manual pilot intervention

## Safety Considerations

### Limitations

- No obstacle avoidance
- No position awareness
- Wind drift not compensated
- Altitude based only on barometer

### Mitigations

- Maintain current altitude to avoid unnecessary movement
- Conservative pitch angles
- Battery monitoring
- Pilot can override immediately upon RC recovery

## Integration Points

### Failsafe System

- Add new failsafe action option
- Priority relative to other failsafe modes
- Compatibility with GCS failsafe

### Flight Mode System

- Register as new flight mode
- Ensure proper mode switching logic
- Display status to GCS

### Parameter System

- Group parameters under FS_COMPASS_*
- Set reasonable defaults
- Validate parameter ranges

## Testing Plan

### POC Testing

1. Test if a drone flies well in alt hold.
2. Detect where north is and a safe pitch during that flight.
3. Hardcode the needed pitch and set the needed heading parameter. Set the mode to some switch.
4. Fly in the opposite direction. Enable failsafe mode manually.
5. The drone should go more or less back.
6. Disable failsafe mode manually.

### MVP Testing

1. Parameter validation
2. Home heading detection (if implemented)
3. Various pitch angles
4. Wind resistance testing

### V2 Testing

1. RC channel heading adjustment
2. OSD heading display functionality
3. Real-time heading updates on OSD
4. OSD configuration without ground station modification
5. Timeout and auto-land
6. Battery failsafe integration
7. Extended flight duration