# Failsafe Compass Mode - User Guide

## What is Failsafe Compass Mode?

Failsafe Compass Mode is an emergency feature for ArduCopter that helps your drone escape from its current location when radio contact is lost. Unlike RTL (Return to Launch) which requires GPS, this mode only needs a compass and barometer - no GPS or velocity sensors required. It can work completely indoors or in areas with no GPS reception.

When activated, your drone will:
1. Climb to a safe altitude
2. Turn to face a pre-set compass direction or toward home
3. Fly forward in that direction until you regain control

## When to Use This Mode

This mode is ideal for:
- Flying in GPS-denied environments (indoors, under bridges, in canyons)
- Areas with GPS interference
- Situations where you want the drone to fly AWAY from obstacles rather than return home
- Racing or freestyle flying where RTL might fly into obstacles

## How to Set It Up

### Step 1: Choose Your Escape Direction

You have two options for the escape direction:

**Option 1: Fixed Compass Direction**
- **0° = North**
- **90° = East**
- **180° = South**
- **270° = West**

**Option 2: Home Direction**
- The drone will calculate the direction back to home using its internal navigation system
- Works without GPS by using the stored home position from when it was armed

Think about your flying area and choose the option that leads to the safest escape route.

### Step 2: Configure Parameters

Using Mission Planner, QGroundControl, or another ground station:

1. **Set the escape heading:**
   - Find parameter: `FS_COMPASS_HDG`
   - Enter your chosen heading (0-359 degrees)
   - Example: Set to 180 to fly South

2. **Choose heading source:**
   - Find parameter: `FS_COMPASS_HDG_SRC`
   - Set to `0` for fixed heading (uses FS_COMPASS_HDG value)
   - Set to `1` for home direction (if available, falls back to fixed heading)

3. **Set forward pitch angle:**
   - Find parameter: `FS_COMPASS_PITCH`
   - Range: 5-20 degrees (default: 5)
   - Higher values = more aggressive forward flight
   - Lower values = gentler, more stable flight

4. **Enable the failsafe mode:**
   - Find parameter: `FS_THR_ENABLE`
   - Set value to: `8`
   - Write parameters to save

5. **Verify RTL altitude (optional):**
   - Find parameter: `RTL_ALT`
   - This sets how high the drone climbs (in centimeters)
   - Default is usually 1500 (15 meters)
   - Adjust if needed for your flying area

## How It Works

When your transmitter signal is lost:

1. **Immediate Response**: The drone detects radio failsafe
2. **Direction Calculation**: 
   - If HDG_SRC=1: Tries to calculate direction to home
   - If HDG_SRC=0 or home unavailable: Uses fixed compass heading
3. **Climb Phase**: Climbs to RTL altitude to clear obstacles
4. **Turn Phase**: Rotates to face the calculated heading
5. **Escape Phase**: Pitches forward at the configured angle in that direction
6. **Recovery**: Continues until you regain radio control

## Parameter Reference

| Parameter | Description | Range | Default |
|-----------|-------------|-------|---------|
| `FS_COMPASS_HDG` | Fixed compass heading in degrees | 0-359 | 0 (North) |
| `FS_COMPASS_PITCH` | Forward pitch angle in degrees | 5-20 | 5 |
| `FS_COMPASS_HDG_SRC` | Heading source: 0=fixed, 1=home | 0-1 | 0 |
| `FS_THR_ENABLE` | Radio failsafe action | 0-8 | Set to 8 for compass mode |

## Important Safety Information

### Limitations
- **No obstacle avoidance** - The drone will fly straight into anything in its path
- **No position holding** - Wind will push the drone off course
- **No velocity control** - Speed depends on pitch angle and environmental conditions
- **No return home** - It keeps flying in one direction
- **Battery limits** - Will fly until battery dies if you don't regain control

### Pre-flight Checks
1. **Test your compass**: Ensure compass is calibrated and working properly
2. **Check heading**: Verify FS_COMPASS_HDG points to a safe direction
3. **Test home direction**: If using HDG_SRC=1, verify home position is set correctly
4. **Know your area**: Be aware of obstacles in your escape direction
5. **Battery planning**: Ensure you have enough battery for the escape flight

### Best Practices
- Start with conservative pitch angles (5-8 degrees) and adjust based on experience
- Choose an escape heading that leads to open areas
- Set RTL altitude high enough to clear all obstacles
- Practice regaining control in a safe area
- Monitor battery levels closely
- Have a spotter who can track the drone visually

## Advanced Configuration

### Choosing the Right Pitch Angle

The pitch angle determines how aggressively your drone flies forward:

- **5-8 degrees**: Gentle, stable flight - good for beginners or windy conditions
- **9-12 degrees**: Moderate speed - balanced approach for most situations
- **13-20 degrees**: Aggressive flight - faster escape but less stable

### Home Direction vs Fixed Heading

**Use Home Direction (HDG_SRC=1) when:**
- Flying in familiar areas where home is in a safe direction
- You want the drone to try to return toward the launch point
- The escape route varies depending on where you're flying

**Use Fixed Heading (HDG_SRC=0) when:**
- You always want the same escape direction regardless of location
- Home direction might lead toward obstacles
- Flying in areas where the safest escape is always the same direction

## Regaining Control

When your radio link is restored:
1. Your transmitter will reconnect automatically
2. Move your throttle stick to regain control
3. Switch to your preferred flight mode
4. Fly back manually

The drone will show status messages on your ground station indicating which heading source it's using.

## Comparison with Other Failsafe Options

| Failsafe Mode | Requires GPS | Returns Home | Configurable | Good For |
|---------------|--------------|--------------|--------------|----------|
| RTL | Yes | Yes | Limited | Outdoor flying with good GPS |
| Land | No | No (lands in place) | Limited | Safe landing zones |
| Compass | No | No (escapes) | Highly | GPS-denied areas, obstacle avoidance |

## Troubleshooting

**Drone doesn't enter Compass failsafe:**
- Verify FS_THR_ENABLE = 8
- Check radio failsafe is properly configured
- Ensure MODE_FAILSAFE_COMPASS_ENABLED in firmware

**Wrong direction:**
- Recalibrate compass
- Verify FS_COMPASS_HDG value
- Check for magnetic interference
- If using home direction, verify home position was set correctly

**Not climbing:**
- Check RTL_ALT parameter
- Verify battery has enough power
- Check if already above RTL altitude

**Flying too fast/slow:**
- Adjust FS_COMPASS_PITCH parameter
- Lower values for gentler flight
- Higher values for more aggressive escape

**Home direction not working:**
- Check FS_COMPASS_HDG_SRC = 1
- Verify drone was armed with good position lock
- Mode will fall back to fixed heading if home unavailable

## Example Scenarios

### Indoor Flying
- Set HDG_SRC=0 (fixed heading) toward the largest exit
- Set FS_COMPASS_HDG toward exit door/window
- Lower RTL_ALT to avoid ceiling
- Use low pitch angle (5-7 degrees) for control
- Have spotter at exit to catch drone

### Canyon/Urban Flying  
- Consider using HDG_SRC=1 (home direction) if launch point is safe
- If using fixed heading, set toward open area
- Increase RTL_ALT above buildings/canyon walls
- Consider wind direction when choosing heading
- Use moderate pitch angle (8-12 degrees)

### Racing Course
- Set HDG_SRC=0 (fixed heading) away from course
- Choose direction with fewest obstacles
- Moderate RTL_ALT to clear gates but not too high
- Brief other pilots about escape direction
- Use higher pitch angle (10-15 degrees) for quick escape

### GPS-Denied Indoor Space
- Set HDG_SRC=0 for predictable behavior
- Choose heading toward largest opening
- Lower RTL_ALT significantly (2-5 meters)
- Use gentle pitch angle (5-6 degrees)
- Practice in the space first with manual control

## Summary

Failsafe Compass Mode provides a configurable, GPS-free emergency escape option with multiple heading sources and adjustable aggressiveness. While it has limitations, it can be invaluable in environments where traditional RTL won't work. The new MVP features allow you to fine-tune the behavior for your specific flying environment and use cases. Always plan your escape route before flying and test the system in a safe environment first.