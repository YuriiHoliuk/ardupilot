# ArduCopter IMU Rescue Mode Implementation

## Overview

IMU Rescue is a minimalistic failsafe mode for ArduCopter that enables the aircraft to fly in a predetermined direction using only IMU data (gyroscope and accelerometer), without requiring GPS, compass, or other positioning systems. This mode activates during radio failsafe events to help the aircraft escape from its current location.

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

- Single parameter: `IMU_RESCUE_HDG` (0-359 degrees)
- Configurable pitch angle parameter: `IMU_RESCUE_PITCH` (range -45 to +45 degrees, default -30 degrees)
- Maintain current altitude - no altitude changes
- No adjustable parameters during flight

**Behavior**:

```
On Radio Failsafe:
1. Maintain current altitude
2. Turn to IMU_RESCUE_HDG
3. Apply IMU_RESCUE_PITCH forward pitch
4. Continue until manual recovery
```

### MVP (Minimum Viable Product)

**Goal**: Add essential configurability and safety features

**New Parameters**:

- `IMU_RESCUE_HDG`: Target heading (0-359°)
- `IMU_RESCUE_PITCH`: Pitch angle (-45 to +45°, default -30°, negative = forward flight)

**Features**:

- Configurable pitch angle
- Maintain current altitude - no altitude control needed

**Behavior**:

```
On Radio Failsafe (if FS_THR_ENABLE=8 failsafe action is selected):
1. Maintain current altitude
2. Turn to IMU_RESCUE_HDG heading
3. Apply IMU_RESCUE_PITCH forward pitch
4. Continue until manual recovery
```

### V2 (Enhanced Version)

**Goal**: Add dynamic control and improved safety

**Additional Parameters**:

- `IMU_RESCUE_CH`: RC channel for heading adjustment (0=disabled, 5-16 for CH5-CH16)
- OSD display integration using existing CLIMBEFF OSD item (repurposed to show "IRH:XXX°")

**Features**:

- RC-adjustable heading during flight (continuously updates IMU_RESCUE_HDG parameter)
- Real-time OSD display of configured target heading via repurposed CLIMBEFF item
- Live heading updates visible during flight as pilot adjusts RC channel
- RC input mapping: full range maps to 0-360 degrees

**Enhanced Behavior**:

```
During normal flight (when IMU_RESCUE_CH is configured):
- Continuously monitor RC channel input
- Map RC input to heading (0-360 degrees)
- Update IMU_RESCUE_HDG parameter in real-time (without saving to EEPROM)
- Display current target heading on OSD as "IRH:XXX°" (if CLIMBEFF OSD item enabled)

On Radio Failsafe:
1. Use the last updated IMU_RESCUE_HDG value
2. Maintain current altitude
3. Apply configured pitch angle and maintain heading
4. Continue until manual recovery
```

## Technical Implementation Notes

### Mode Integration

- New flight mode: `IMU_RESCUE` (mode number 29)
- Enabled via `MODE_IMU_RESCUE_ENABLED` configuration flag
- Triggered by `FS_THR_ENABLE = 8` parameter setting
- Integrated with existing failsafe system

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

- Direct pitch angle command via IMU_RESCUE_PITCH parameter
- Negative values for forward flight, positive values for backward flight
- Range: -45 to +45 degrees for various flight speeds
- No position or velocity feedback required
- Let natural aircraft dynamics determine speed

### RC Channel Integration

- Configurable via `IMU_RESCUE_CH` parameter (0=disabled, 5-16 for channels)
- Real-time heading adjustment during normal flight
- Updates parameter in memory only (not saved to EEPROM)
- Full RC range maps linearly to 0-360 degrees

### OSD Integration

- Repurposed CLIMBEFF OSD item to display IMU Rescue heading
- Shows "IRH:XXX°" format where XXX is current target heading
- Only active in ArduCopter builds with IMU_RESCUE enabled
- No new OSD item creation required

### State Management

- New flight mode: `IMU_RESCUE`
- Integrate with existing failsafe system
- Clear mode exit only through manual pilot intervention

## Safety Considerations

### Limitations

- No obstacle avoidance
- No position awareness
- Wind drift not compensated
- Altitude based only on barometer
- Heading drift over time without compass

### Mitigations

- Maintain current altitude to avoid unnecessary movement
- Conservative pitch angles
- Battery monitoring
- Pilot can override immediately upon RC recovery

## Integration Points

### Failsafe System

- Add new failsafe action option (FS_THR_ENABLE = 8)
- Priority relative to other failsafe modes
- Compatibility with GCS failsafe

### Flight Mode System

- Register as new flight mode (Number::IMU_RESCUE = 29)
- Ensure proper mode switching logic
- Display status to GCS

### Parameter System

- Group parameters under IMU_RESCUE_*
- Set reasonable defaults
- Validate parameter ranges

### OSD System

- Repurpose existing CLIMBEFF item for heading display
- Conditional compilation for ArduCopter only
- Backward compatibility maintained

## Testing Plan

### POC Testing

1. Test if a drone flies well in alt hold.
2. Detect where north is and a safe pitch during that flight.
3. Configure the needed pitch and set the needed heading parameter. Set FS_THR_ENABLE=8.
4. Fly in the opposite direction. Trigger radio failsafe.
5. The drone should go more or less back.
6. Regain radio control to exit mode.

### MVP Testing

1. Parameter validation
2. Heading accuracy testing
3. Various pitch angles
4. Wind resistance testing

### V2 Testing

1. RC channel heading adjustment
2. OSD heading display functionality
3. Real-time heading updates on OSD
4. Extended flight duration
5. Battery failsafe integration