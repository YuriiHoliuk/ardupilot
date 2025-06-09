#include "Copter.h"

#if MODE_IMU_RESCUE_ENABLED

// imu_rescue_init - initialise IMU Rescue mode
bool ModeIMURescue::init(bool ignore_checks)
{
    // initialize vertical maximum speeds and acceleration
    pos_control->set_max_speed_accel_z(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);
    pos_control->set_correction_speed_accel_z(-get_pilot_speed_dn(), g.pilot_speed_up, g.pilot_accel_z);

    // initialise altitude controller
    if (!pos_control->is_active_z()) {
        pos_control->init_z_controller();
    }

    // set initial target heading to configured rescue heading
    _target_heading_deg = g2.imu_rescue_heading;

    return true;
}

// imu_rescue_run - runs the IMU Rescue mode
// should be called at 100hz or more
void ModeIMURescue::run()
{
    // if not armed set throttle to zero and exit immediately
    if (is_disarmed_or_landed()) {
        make_safe_ground_handling();
        return;
    }

    // set motors to full range
    motors->set_desired_spool_state(AP_Motors::DesiredSpoolState::THROTTLE_UNLIMITED);

    float target_climb_rate = 0.0f;
    
    // Send the commanded climb rate to the position controller (We always want to stay at the same altitude)
    pos_control->set_pos_target_z_from_climb_rate_cm(target_climb_rate);

    // Pitch forward in the direction of the target heading
    float target_pitch_cd = g2.imu_rescue_pitch * 100;  // Use configured pitch angle from parameter
    float target_roll_cd = 0;  // No roll for straight flight
    
    // Calculate target yaw from target heading
    float target_yaw_cd = _target_heading_deg * 100.0f;
    
    // Call attitude controller with fixed angles (open-loop)
    attitude_control->input_euler_angle_roll_pitch_yaw(target_roll_cd, target_pitch_cd, target_yaw_cd, true);
    
    // Update altitude controller
    pos_control->update_z_controller();
}

#endif  // MODE_IMU_RESCUE_ENABLED
