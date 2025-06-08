# ArduCopter Failsafe_Compass Implementation Plan

## Project Overview

This document outlines the implementation plan for the Failsafe_Compass mode in ArduCopter. This mode enables the aircraft to fly in a predetermined direction using only IMU data (gyroscope and accelerometer) during radio failsafe events.

## Implementation Phases

### Phase 1: POC (Proof of Concept)

**Objective**: Create a basic working implementation with minimal parameters

#### Tasks:

1. **Create the new flight mode class**
   - [x] Create `mode_failsafe_compass.cpp` file in ArduCopter directory
   - [x] Define `ModeFailsafeCompass` class inheriting from `Mode`
   - [x] Implement basic mode structure with required virtual methods

2. **Add mode to flight mode system**
   - [x] Add `FAILSAFE_COMPASS` enum to flight mode definitions
   - [x] Register the mode in the flight mode system
   - [x] Update mode switching logic

3. **Implement basic parameters**
   - [x] Add `FS_COMPASS_HDG` parameter (0-359 degrees) to Parameters.h
   - [x] Add `FS_COMPASS_PITCH` parameter (-45 to +45 degrees) to Parameters.h
   - [x] Add parameter definitions and validation

4. **Implement core flight behavior**
   - [x] Altitude control: Maintain current altitude
   - [x] Heading control: Turn to `FS_COMPASS_HDG` using IMU/AHRS data
   - [x] Pitch control: Apply configurable pitch (FS_COMPASS_PITCH parameter, negative=forward)
   - [x] Use existing attitude control systems

5. **Integrate with failsafe system**
   - [x] Add failsafe_compass as an option in radio failsafe actions
   - [x] Implement mode entry logic during radio failsafe
   - [x] Ensure proper mode exit on manual recovery

6. **Basic testing and validation**
   - [ ] SITL testing for basic functionality
   - [ ] Verify compass heading control
   - [ ] Test altitude climb and hold behavior

### Phase 2: MVP (Minimum Viable Product)

**Objective**: Add essential configurability and safety features

#### Tasks:

1. **Expand parameter system**
   - [ ] Add `FS_COMPASS_MODE` parameter (enable/disable)
   - [x] Add `FS_COMPASS_PITCH` parameter (-45 to +45°, default -5°)

2. **Implement configurable pitch control**
   - [x] Replace fixed pitch with configurable `FS_COMPASS_PITCH`
   - [x] Updated parameter range to -45 to +45 degrees
   - [ ] Add parameter validation and range checking

3. **Enhanced parameter validation**
   - [ ] Add comprehensive parameter range checking
   - [ ] Implement parameter validation on mode entry
   - [ ] Add parameter documentation and help text

4. **Enhanced safety features**
   - [ ] Add parameter validation on mode entry
   - [ ] Implement proper error handling for invalid configurations
   - [ ] Add status reporting to GCS

5. **Extended testing**
   - [ ] Test with various pitch angles
   - [ ] Test parameter validation
   - [ ] Validate heading accuracy over time

### Phase 3: V2 (Enhanced Version)

**Objective**: Add dynamic control and improved safety features

#### Tasks:

1. **RC channel heading adjustment**
   - [x] Add `FS_COMPASS_HDG_CH` parameter for RC channel selection
   - [x] Implement continuous heading monitoring from RC channel
   - [x] Update FS_COMPASS_HDG parameter in real-time during flight
   - [x] Use updated heading value when failsafe activates

2. **OSD heading display**
   - [ ] Add `FS_COMPASS_OSD_ENABLE` parameter to enable/disable OSD display
   - [ ] Add `FS_COMPASS_OSD_ITEM` parameter for OSD item slot configuration
   - [ ] Implement real-time heading display on OSD
   - [ ] Update OSD display when pilot adjusts heading via RC channel
   - [ ] Ensure OSD item can be configured without modifying Mission Planner/QGroundControl
   - [ ] Add heading value to OSD backend data stream

3. **Simplified pitch control**
   - [x] Use single configurable pitch parameter `FS_COMPASS_PITCH`
   - [x] No throttle control - let altitude controller handle throttle
   - [x] No complex pitch adjustment logic needed

4. **Timeout and auto-land**
   - [ ] Add `FS_COMPASS_TIMEOUT` parameter
   - [ ] Implement flight time tracking
   - [ ] Add automatic land mode transition on timeout

5. **Battery monitoring integration**
   - [ ] Integrate with existing battery failsafe system
   - [ ] Implement emergency land on critical battery
   - [ ] Add proper priority handling between different failsafe conditions

6. **Comprehensive testing**
   - [ ] Test RC channel heading adjustment
   - [ ] Test OSD heading display functionality
   - [ ] Validate real-time OSD updates during flight
   - [ ] Test OSD configuration without ground station modification
   - [ ] Validate timeout and auto-land functionality
   - [ ] Test battery failsafe integration
   - [ ] Extended flight duration testing

## Technical Implementation Details

### File Structure
```
ArduCopter/
├── mode_failsafe_compass.cpp    # Main mode implementation
├── mode.h                       # Add mode declaration
├── Copter.h                     # Add mode instance
├── Parameters.h                 # Add parameters
├── Parameters.cpp               # Parameter definitions
└── defines.h                    # Add mode enum
```

### Key Integration Points

1. **Flight Mode System**
   - Add to mode enumeration in `defines.h`
   - Add mode instance in `Copter.h`
   - Register in mode initialization

2. **Failsafe System**
   - Modify radio failsafe handler to support compass failsafe option
   - Add priority logic relative to other failsafe modes

3. **Parameter System**
   - Group all parameters under `FS_COMPASS_*` prefix
   - Implement proper parameter validation
   - Add parameter documentation

4. **Attitude Control Integration**
   - Use existing `AC_AttitudeControl` for heading and pitch
   - Leverage existing altitude control for altitude hold (no climb)
   - Integrate with IMU and AHRS systems (no compass dependency)

### Testing Strategy

1. **SITL Testing**
   - Basic functionality validation
   - Parameter testing
   - Failsafe scenario simulation

2. **Hardware Testing**
   - IMU/AHRS heading accuracy verification
   - Real-world failsafe scenarios
   - Wind resistance testing

3. **Safety Testing**
   - Manual recovery verification
   - Battery failsafe interaction
   - Timeout functionality

## Success Criteria

### POC Success Criteria
- [ ] Mode can be activated during radio failsafe
- [x] Aircraft maintains current altitude (no climbing)
- [ ] Aircraft turns to and maintains configured heading
- [x] Aircraft maintains forward flight with configurable pitch
- [ ] Pilot can regain control and exit mode
- [ ] Code compiles successfully for target board
- [ ] SITL testing validates functionality
- [ ] Real-world testing confirms GPS-free operation

### MVP Success Criteria
- [ ] All parameters are configurable and validated
- [ ] Home direction option works (if feasible)
- [ ] Variable pitch angles function correctly
- [ ] Enhanced safety features are operational

### V2 Success Criteria
- [ ] RC channel heading adjustment works
- [ ] Throttle modification functions properly
- [ ] Timeout and auto-land work correctly
- [ ] Battery monitoring integration is seamless

## Risks and Mitigations

### Technical Risks
- **IMU heading drift**: Accept limitation without compass, document in user guide
- **Wind drift**: Accept limitation, document in user guide
- **Altitude hold accuracy**: Use barometer, accept inherent limitations

### Safety Risks
- **No obstacle avoidance**: Mitigate with high altitude requirement
- **No position feedback**: Document as limitation, require pilot training

### Implementation Risks
- **AHRS integration complexity**: Start with simple implementation, iterate
- **Parameter validation**: Implement comprehensive range checking
- **Mode switching logic**: Careful integration with existing failsafe system