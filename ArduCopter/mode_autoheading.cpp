#include "Copter.h"

/*
 * Init and run calls for autoheading flight mode
 * This mode works without GPS positioning (like althold)
 * Maintains altitude automatically
 * Rotates to a hardcoded angle using compass/IMU
 * Flies forward in that direction with 50% throttle (controlled by adjusting pitch)
 * Uses throttle-to-pitch PID feedback to maintain target throttle
 */

// autoheading_init - initialise autoheading controller
bool ModeAutoHeading::init(bool ignore_checks)
{
    // initialise the vertical position controller
    if (!pos_control->is_active_U()) {
        pos_control->init_U_controller();
    }

    // set vertical speed and acceleration limits
    pos_control->set_max_speed_accel_U_cm(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);
    pos_control->set_correction_speed_accel_U_cmss(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);

    // reset target heading
    target_heading_set = false;
    target_heading_cd = HARDCODED_HEADING_DEG * 100.0f; // convert to centidegrees
    mode_start_time_ms = AP_HAL::millis();
    
    // reset the throttle-to-pitch PID controller
    _throttle_to_pitch_pid.reset_I();
    _throttle_to_pitch_pid.reset_filter();

    return true;
}

// autoheading_run - runs the autoheading controller
// should be called at 100hz or more
void ModeAutoHeading::run()
{
    // set vertical speed and acceleration limits
    pos_control->set_max_speed_accel_U_cm(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);

    // get current heading in centidegrees
    float current_heading_cd = ahrs.yaw_sensor;
    
    // calculate heading error
    float heading_error_cd = target_heading_cd - current_heading_cd;
    
    // wrap heading error to [-18000, 18000] centidegrees
    if (heading_error_cd > 18000.0f) {
        heading_error_cd -= 36000.0f;
    } else if (heading_error_cd < -18000.0f) {
        heading_error_cd += 36000.0f;
    }

    // determine desired yaw rate based on heading error
    float target_yaw_rate = 0.0f;
    const float heading_deadband_cd = 500.0f; // 5 degrees deadband
    
    if (fabsf(heading_error_cd) > heading_deadband_cd) {
        // proportional yaw rate control to reach target heading
        const float yaw_rate_gain = 0.5f; // tunable gain
        target_yaw_rate = constrain_float(heading_error_cd * yaw_rate_gain, -45.0f, 45.0f); // limit to 45 deg/s
    }

    // calculate target roll and pitch angles
    float target_roll = 0.0f;
    float target_pitch = 0.0f;
    
    // once we're close to target heading, start forward flight with throttle control
    if (fabsf(heading_error_cd) < 2000.0f) { // within 20 degrees
        target_heading_set = true;
        
        // Get current throttle output from attitude controller
        float current_throttle = attitude_control->get_throttle_in();
        
        // Calculate throttle error (target is 50%)
        float throttle_error = TARGET_THROTTLE - current_throttle;
        
        // Use PID controller to determine pitch adjustment based on throttle error
        // If throttle > 50%, we need more negative pitch (nose down) to reduce lift
        // If throttle < 50%, we need more positive pitch (nose up) to increase lift
        float pitch_adjustment_deg = _throttle_to_pitch_pid.update_all(throttle_error, 0.0f, G_Dt);
        
        // Constrain pitch adjustment to reasonable limits
        pitch_adjustment_deg = constrain_float(pitch_adjustment_deg, -15.0f, 15.0f);
        
        // Start with baseline forward pitch and add PID adjustment
        target_pitch = (-FORWARD_PITCH_DEG + pitch_adjustment_deg) * 100.0f; // negative for forward, convert to centidegrees
    }

    // get pilot desired climb rate (allow altitude adjustment)
    float target_climb_rate = get_pilot_desired_climb_rate();
    target_climb_rate = constrain_float(target_climb_rate, -get_pilot_speed_dn(), g.pilot_speed_up);

    // Alt Hold State Machine Determination
    AltHoldModeState autoheading_state = get_alt_hold_state(target_climb_rate);

    // Auto Heading State Machine (based on AltHold pattern)
    switch (autoheading_state) {

    case AltHoldModeState::MotorStopped:
        attitude_control->reset_rate_controller_I_terms();
        attitude_control->reset_yaw_target_and_rate(false);
        pos_control->relax_U_controller(0.0f);   // forces throttle output to decay to zero
        target_roll = 0.0f;
        target_pitch = 0.0f;
        target_yaw_rate = 0.0f;
        // Reset PID when motors stopped
        _throttle_to_pitch_pid.reset_I();
        break;

    case AltHoldModeState::Landed_Ground_Idle:
        attitude_control->reset_yaw_target_and_rate();
        FALLTHROUGH;

    case AltHoldModeState::Landed_Pre_Takeoff:
        attitude_control->reset_rate_controller_I_terms_smoothly();
        pos_control->relax_U_controller(0.0f);   // forces throttle output to decay to zero
        target_roll = 0.0f;
        target_pitch = 0.0f;
        target_yaw_rate = 0.0f;
        // Reset PID when landed
        _throttle_to_pitch_pid.reset_I();
        break;

    case AltHoldModeState::Takeoff:
        // initiate take-off
        if (!takeoff.running()) {
            takeoff.start(constrain_float(g.pilot_takeoff_alt,0.0f,1000.0f));
        }

        // get avoidance adjusted climb rate
        target_climb_rate = get_avoidance_adjusted_climbrate(target_climb_rate);

        // set position controller targets adjusted for pilot input
        takeoff.do_pilot_takeoff(target_climb_rate);
        
        // no movement during takeoff
        target_roll = 0.0f;
        target_pitch = 0.0f;
        target_yaw_rate = 0.0f;
        break;

    case AltHoldModeState::Flying:
        motors->set_desired_spool_state(AP_Motors::DesiredSpoolState::THROTTLE_UNLIMITED);

        // get avoidance adjusted climb rate
        target_climb_rate = get_avoidance_adjusted_climbrate(target_climb_rate);

#if AP_RANGEFINDER_ENABLED
        // update the vertical offset based on the surface measurement
        copter.surface_tracking.update_surface_offset();
#endif

        // Send the commanded climb rate to the position controller
        pos_control->set_pos_target_U_from_climb_rate_cm(target_climb_rate);
        break;
    }

    // call attitude controller with our computed targets
    attitude_control->input_euler_angle_roll_pitch_euler_rate_yaw_cd(target_roll, target_pitch, target_yaw_rate);

    // run the vertical position controller and set output throttle
    pos_control->update_U_controller();
} 
