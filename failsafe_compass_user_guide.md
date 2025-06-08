# Failsafe Compass Mode - User Guide

## What is Failsafe Compass Mode?

Failsafe Compass Mode is a new emergency feature for ArduCopter that helps your drone escape from its current location when radio contact is lost. Unlike RTL (Return to Launch) which requires GPS, this mode only needs IMU sensors (gyroscope and accelerometer) - no GPS, compass, or velocity sensors required. It can work completely indoors or in areas with no GPS reception.

When activated, your drone will:
1. Maintain its current altitude
2. Turn to face a pre-set heading direction
3. Fly forward in that direction until you regain control

## When to Use This Mode

This mode is ideal for:
- Flying in GPS-denied environments (indoors, under bridges, in canyons)
- Areas with GPS interference
- Situations where you want the drone to fly AWAY from obstacles in a predetermined direction
- Racing or freestyle flying where RTL might fly into obstacles

## How to Set It Up

### Step 1: Choose Your Escape Direction

Decide which heading direction you want your drone to fly when signal is lost:
- **0° = North** (relative to IMU/AHRS heading)
- **90° = East** 
- **180° = South**
- **270° = West**

Think about your flying area and choose a direction that leads to open space. Note: Without compass, heading may drift over time.

### Step 2: Configure Parameters

Using Mission Planner, QGroundControl, or another ground station:

1. **Set the escape heading:**
   - Find parameter: `FS_COMPASS_HDG`
   - Enter your chosen heading (0-359 degrees)
   - Example: Set to 180 to fly South

2. **Set the pitch angle (optional):**
   - Find parameter: `FS_COMPASS_PITCH`
   - Default is -5 degrees (forward flight)
   - Range: -45 to +45 degrees
   - Negative values = forward flight (faster with more negative values)
   - Positive values = backward flight
   - Example: Set to -15 for aggressive forward flight

3. **Configure RC channel for heading adjustment (optional):**
   - Find parameter: `FS_COMPASS_HDG_CH`
   - Set to 0 to disable (default)
   - Set to 5-16 to use CH5-CH16 for heading control
   - When enabled, you can adjust escape heading during flight:
     - Full left (-100%): 0 degrees
     - Center (0%): 180 degrees
     - Full right (+100%): 360 degrees

4. **Enable the failsafe mode:**
   - Find parameter: `FS_THR_ENABLE`
   - Set value to: `8`
   - Write parameters to save

5. **Note about altitude:**
   - The drone will maintain its current altitude when failsafe activates
   - No altitude climbing or changes occur
   - Plan your flights accordingly for obstacle clearance

## How It Works

When your transmitter signal is lost:

1. **Immediate Response**: The drone detects radio failsafe
2. **Altitude Hold**: Maintains current altitude (no climbing)
3. **Turn Phase**: Rotates to face your preset heading using IMU/AHRS data
4. **Escape Phase**: Pitches at configured angle (FS_COMPASS_PITCH, negative=forward) in that direction
5. **Recovery**: Continues until you regain radio control

## Important Safety Information

### Limitations
- **No obstacle avoidance** - The drone will fly straight into anything in its path
- **No position holding** - Wind will push the drone off course
- **No velocity control** - Speed depends on pitch angle and environmental conditions
- **No navigation home** - It keeps flying in one direction
- **Battery limits** - Will fly until battery dies if you don't regain control
- **Heading drift** - Without compass, heading may drift over time due to IMU integration errors
- **No altitude changes** - Maintains current altitude which may not clear obstacles

### Pre-flight Checks
1. **Test your IMU**: Ensure IMU is calibrated and AHRS is working properly
2. **Check heading**: Verify FS_COMPASS_HDG points to a safe direction
3. **Check pitch**: Verify FS_COMPASS_PITCH is appropriate for your aircraft (negative=forward, positive=backward)
4. **Know your area**: Be aware of obstacles in your escape direction at current altitude
5. **Battery planning**: Ensure you have enough battery for the escape flight

### Best Practices
- Choose an escape heading that leads to open areas
- Fly at sufficient altitude to clear obstacles (no auto-climb in this mode)
- Practice regaining control in a safe area
- Monitor battery levels closely
- Have a spotter who can track the drone visually
- Be aware that heading may drift without compass reference
- If using RC channel control, practice adjusting heading during normal flight
- Remember that RC heading adjustments are not saved permanently

## Regaining Control

When your radio link is restored:
1. Your transmitter will reconnect automatically
2. Move your throttle stick to regain control
3. Switch to your preferred flight mode
4. Fly back manually

## Comparison with Other Failsafe Options

| Failsafe Mode | Requires GPS | Requires Compass | Returns Home | Good For |
|--------------|--------------|------------------|--------------|----------|
| RTL | Yes | Usually | Yes | Outdoor flying with good GPS |
| Land | No | No | No (lands in place) | Safe landing zones |
| Compass | No | No | No (escapes) | GPS/compass-denied areas |

## Troubleshooting

**Drone doesn't enter Compass failsafe:**
- Verify FS_THR_ENABLE = 8
- Check radio failsafe is properly configured
- Ensure MODE_FAILSAFE_COMPASS_ENABLED in firmware

**Wrong direction:**
- Recalibrate IMU/AHRS system
- Verify FS_COMPASS_HDG value
- Check IMU mounting and orientation

**Unexpected altitude behavior:**
- Note: This mode maintains current altitude and does not climb
- Verify battery has enough power for altitude hold
- Check barometer functionality

## Example Scenarios

### Indoor Flying
- Set heading toward the largest exit
- Ensure current flying altitude clears obstacles
- Have spotter at exit to catch drone
- Be extra aware of heading drift without compass

### Canyon/Urban Flying  
- Set heading toward open area
- Fly at sufficient altitude to clear buildings (no auto-climb)
- Consider wind direction and potential heading drift

### Racing Course
- Set heading away from course
- Ensure flying altitude clears gates and obstacles
- Brief other pilots about escape direction
- Consider more negative pitch angle for quick escape (e.g., -20 degrees)

## Summary

Failsafe Compass Mode provides a simple, GPS-free and compass-free emergency escape option using only IMU sensors. While it has limitations including potential heading drift, it can be invaluable in environments where traditional RTL won't work. Always plan your escape route before flying and test the system in a safe environment first.