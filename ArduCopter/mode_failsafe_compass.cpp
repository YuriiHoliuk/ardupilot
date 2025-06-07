#include "Copter.h"

#if MODE_FAILSAFE_COMPASS_ENABLED

// failsafe_compass_init - initialise failsafe compass mode
bool ModeFailsafeCompass::init(bool ignore_checks)
{
    // Validate parameters
    if (g2.fs_compass_pitch < 1 || g2.fs_compass_pitch > 90) {
        gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: Invalid pitch angle %d", (int)g2.fs_compass_pitch);

        return false;
    }

    // initialize vertical maximum speeds and acceleration
    pos_control->set_max_speed_accel_z(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);
    pos_control->set_correction_speed_accel_z(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);

    // initialise altitude controller
    if (!pos_control->is_active_z()) {
        pos_control->init_z_controller();
    }

    // set initial target heading based on configured source
    if (g2.fs_compass_hdg_src == 1) {
        // Try to use home direction
        Vector2f home_offset;

        if (ahrs.get_relative_position_NE_home(home_offset)) {
            // Calculate bearing to home from relative position
            float home_bearing_rad = atan2f(-home_offset.y, -home_offset.x);
            _target_heading_deg = wrap_360(degrees(home_bearing_rad));
            gcs().send_text(MAV_SEVERITY_INFO, "FailsafeCompass: Using home direction %.0f deg", _target_heading_deg);
        } else {
            // Fallback to configured heading if home direction unavailable
            _target_heading_deg = g2.fs_compass_heading;
            gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: Home unavailable, using fixed heading %.0f deg", _target_heading_deg);
        }
    } else {
        // Use configured fixed heading
        _target_heading_deg = g2.fs_compass_heading;
        gcs().send_text(MAV_SEVERITY_INFO, "FailsafeCompass: Using fixed heading %.0f deg", _target_heading_deg);
    }

    return true;
}

// failsafe_compass_run - runs the failsafe compass mode
// should be called at 100hz or more
void ModeFailsafeCompass::run()
{
    // if not armed set throttle to zero and exit immediately
    if (is_disarmed_or_landed()) {
        make_safe_ground_handling();
        return;
    }

    // set motors to full range
    motors->set_desired_spool_state(AP_Motors::DesiredSpoolState::THROTTLE_UNLIMITED);

    // Get target altitude from RTL altitude parameter
    float target_alt_cm = g.rtl_altitude * 100.0f;
    
    // Get current altitude (cm above home)
    float current_alt_cm = pos_control->get_pos_target_z_cm();
    
    // Calculate climb rate based on altitude difference
    float alt_diff_cm = target_alt_cm - current_alt_cm;
    float target_climb_rate = 0.0f;
    
    if (alt_diff_cm > FAILSAFE_COMPASS_ALT_TOLERANCE_CM) {
        // Need to climb
        target_climb_rate = g.pilot_speed_up;
    } else if (alt_diff_cm < -FAILSAFE_COMPASS_ALT_TOLERANCE_CM) {
        // Do nothing, altitude should be >= target_alt_cm
    }
    
    // Send the commanded climb rate to the position controller
    pos_control->set_pos_target_z_from_climb_rate_cm(target_climb_rate);

    // Open-loop control: Calculate fixed pitch based on target heading
    // Pitch forward in the direction of the target heading
    float target_pitch_cd = g2.fs_compass_pitch * 100;  // Use configurable pitch angle
    float target_roll_cd = 0;  // No roll for straight flight
    
    // Calculate target yaw from target heading
    float target_yaw_cd = _target_heading_deg * 100.0f;
    
    // Call attitude controller with fixed angles (open-loop)
    attitude_control->input_euler_angle_roll_pitch_yaw(target_roll_cd, target_pitch_cd, target_yaw_cd, true);
    
    // Update altitude controller
    pos_control->update_z_controller();
}

#endif  // MODE_FAILSAFE_COMPASS_ENABLED