#include "Copter.h"

#if MODE_FAILSAFE_COMPASS_ENABLED

// Constructor
ModeFailsafeCompass::ModeFailsafeCompass() : Mode(),
    _pitch_controller(1.0f, 0.1f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 20.0f)
{
    // Constructor body if needed
}

// failsafe_compass_init - initialise failsafe compass mode
bool ModeFailsafeCompass::init(bool ignore_checks)
{
    // Validate parameters
    if (g2.fs_compass_pitch < 1 || g2.fs_compass_pitch > 90) {
        gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: Invalid pitch angle %d", (int)g2.fs_compass_pitch);

        return false;
    }

    // Validate throttle control parameters if enabled
    if (g2.fs_compass_thr_ctrl_enabled) {
        if (g2.fs_compass_pitch_min >= g2.fs_compass_pitch_max) {
            gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: Invalid pitch min/max range");
            return false;
        }
        if (g2.fs_compass_target_thr < 20 || g2.fs_compass_target_thr > 80) {
            gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: Invalid target throttle %.1f", (float)g2.fs_compass_target_thr);
            return false;
        }
    }

    // initialize vertical maximum speeds and acceleration
    pos_control->set_max_speed_accel_U_cm(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);
    pos_control->set_correction_speed_accel_U_cmss(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);

    // initialise altitude controller
    if (!pos_control->is_active_U()) {
        pos_control->init_U_controller();
    }

    // Check if we should use RC channel heading
    if (g2.fs_compass_hdg_ch > 0 && g2.fs_compass_hdg_ch <= RC_Channels::get_valid_channel_count()) {
        // Use the last known heading from the RC channel
        RC_Channel *ch = rc().channel(g2.fs_compass_hdg_ch - 1);
        if (ch != nullptr) {
            // Map RC input to 0-360 degrees
            float rc_input = ch->get_control_in();
            _target_heading_deg = wrap_360(rc_input * 0.036f + 180.0f); // -100% = 0°, 0% = 180°, 100% = 360°
            gcs().send_text(MAV_SEVERITY_INFO, "FailsafeCompass: Using RC heading %.0f deg", _target_heading_deg);
        } else {
            _target_heading_deg = g2.fs_compass_heading;
            gcs().send_text(MAV_SEVERITY_WARNING, "FailsafeCompass: RC channel invalid, using fixed heading");
        }
    } else if (g2.fs_compass_hdg_src == 1) {
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

    // Initialize pitch controller if throttle control is enabled
    if (g2.fs_compass_thr_ctrl_enabled) {
        // Initialize current pitch to the configured default
        _current_pitch_deg = g2.fs_compass_pitch;
        
        // Setup PID controller for pitch adjustment
        // P=1.0 means 1% throttle error = 1 degree pitch change
        // I=0.1 for slow integration, D=0 for simplicity
        _pitch_controller.set_kP(1.0f);
        _pitch_controller.set_kI(0.1f);
        _pitch_controller.set_kD(0.0f);
        _pitch_controller.set_imax(5.0f);  // Limit integral to ±5 degrees
        _pitch_controller.reset_I();
    } else {
        // Use fixed pitch
        _current_pitch_deg = g2.fs_compass_pitch;
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
    float current_alt_cm = pos_control->get_pos_target_U_cm();
    
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
    pos_control->set_pos_target_U_from_climb_rate_cm(target_climb_rate);

    // Handle pitch control
    if (g2.fs_compass_thr_ctrl_enabled) {
        // Advanced throttle control with pitch adjustment
        float current_throttle = motors->get_throttle_out();
        float target_throttle = g2.fs_compass_target_thr * 0.01f; // Convert percentage to 0-1
        float throttle_error = target_throttle - current_throttle;
        
        // Only adjust pitch if we're outside the tolerance band
        if (fabsf(throttle_error) > FAILSAFE_COMPASS_THR_TOLERANCE) {
            // Use PID controller to calculate pitch adjustment
            float pitch_adjustment = _pitch_controller.update_error(throttle_error, 0.01f); // 100Hz update rate
            
            // Update current pitch with limits
            _current_pitch_deg = constrain_float(_current_pitch_deg + pitch_adjustment, 
                                                 g2.fs_compass_pitch_min, 
                                                 g2.fs_compass_pitch_max);
        } else {
            // Within tolerance, reset integrator to prevent windup
            _pitch_controller.reset_I();
        }
    }

    // Open-loop control: Calculate pitch based on target heading
    // Pitch forward in the direction of the target heading
    float target_pitch_cd = _current_pitch_deg * 100;  // Use current pitch (fixed or adjusted)
    float target_roll_cd = 0;  // No roll for straight flight
    
    // Calculate target yaw from target heading
    float target_yaw_cd = _target_heading_deg * 100.0f;
    
    // Call attitude controller with fixed angles (open-loop)
    attitude_control->input_euler_angle_roll_pitch_yaw_cd(target_roll_cd, target_pitch_cd, target_yaw_cd, true);
    
    // Update altitude controller
    pos_control->update_U_controller();
}

float ModeFailsafeCompass::wp_distance_m() const
{
    return 0.0f;
}

int32_t ModeFailsafeCompass::wp_bearing() const
{
    return _target_heading_deg;
}

#endif  // MODE_FAILSAFE_COMPASS_ENABLED