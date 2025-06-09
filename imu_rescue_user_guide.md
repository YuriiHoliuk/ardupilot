# IMU Rescue Mode - Quick Setup

## How It Works
When radio contact is lost, your drone maintains altitude, turns to a preset heading, and flies straight in that direction until you regain control (Need to manually change mode to another). Uses only IMU sensors - no GPS or compass required.

## Setup Steps

**1. Set escape direction:**
- Change `IMU_RESCUE_HDG` to your chosen heading (0-359°) relative to the starting position.
- 180° by default. Will be updated from RC if enabled.

**2. Set flight angle:**
- Change `IMU_RESCUE_PITCH` to control speed
- Default: -30° (negative = forward flight)
- No need to change this unless you want to change the speed of the flight.

**3. Enable failsafe:**
- Change `FS_THR_ENABLE` to `8`

**4. Optional - RC heading control:**
- Change `IMU_RESCUE_CH` to 5-16 to use CH5-CH16
- Allows real-time heading adjustment during flight
- Full left=0°, Center=180°, Full right=360°

**5. Optional - Set up mode switch (for testing without failsafe):**
- Set one of your mode switch positions to mode ID 29 (IMU_RESCUE)
- Example: `FLTMODE6 = 29` to assign IMU Rescue to switch position 6
- This allows manual testing of the mode without triggering a failsafe

## Notes
- Drone turns using yaw only, we always set -30deg pitch and 0deg roll. So, don't expect sharp turns.
- If copter cannot maintain altitude with given pitch (-30deg by default), it will descend and fall. No smart controller available.