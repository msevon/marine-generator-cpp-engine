#include "Sensors.h"
#include <cmath>
#include <random>
#include <chrono>

// Static random number generator for sensor noise
static std::random_device rd;
static std::mt19937 gen(rd());
static std::normal_distribution<double> noise_dist(0.0, 1.0);

Sensors::Sensors()
    : fuel_sensor_failed_(false)
    , oil_sensor_failed_(false)
    , temp_sensor_failed_(false)
    , fuel_calibration_drift_(0.0)
    , oil_calibration_drift_(0.0)
    , temp_calibration_drift_(0.0)
{
    // Initialize sensor readings to realistic values
    current_readings_.fuel_level = 100.0;  // Start at 100%
    current_readings_.oil_pressure = 3.0;
    current_readings_.cooling_temp = 25.0;
    current_readings_.vibration = 0.0;
    current_readings_.exhaust_temp = 25.0;
    current_readings_.ambient_temp = 25.0;
    current_readings_.humidity = 60.0;
}

Sensors::SensorReadings Sensors::get_readings() const {
    return current_readings_;
}

void Sensors::update(double delta_time, bool generator_running, double load_percentage) {
    if (generator_running) {
        update_fuel_sensor(delta_time, generator_running, load_percentage);
        update_oil_pressure_sensor(delta_time, generator_running, load_percentage);
        update_temperature_sensors(delta_time, generator_running, load_percentage);
        update_vibration_sensor(delta_time, generator_running, load_percentage);
    } else {
        // Generator stopped - sensors return to ambient values
        current_readings_.oil_pressure = 0.0;
        current_readings_.cooling_temp = current_readings_.ambient_temp;
        current_readings_.vibration = 0.0;
        current_readings_.exhaust_temp = current_readings_.ambient_temp;
    }
}

void Sensors::set_sensor_failure(bool fuel_failed, bool oil_failed, bool temp_failed) {
    fuel_sensor_failed_ = fuel_failed;
    oil_sensor_failed_ = oil_failed;
    temp_sensor_failed_ = temp_failed;
}

void Sensors::set_calibration_drift(double fuel_drift, double oil_drift, double temp_drift) {
    fuel_calibration_drift_ = fuel_drift;
    oil_calibration_drift_ = oil_drift;
    temp_calibration_drift_ = temp_drift;
}

void Sensors::reset_sensors() {
    fuel_sensor_failed_ = false;
    oil_sensor_failed_ = false;
    temp_sensor_failed_ = false;
    fuel_calibration_drift_ = 0.0;
    oil_calibration_drift_ = 0.0;
    temp_calibration_drift_ = 0.0;
}

void Sensors::update_fuel_sensor(double delta_time, bool generator_running, double load_percentage) {
    if (fuel_sensor_failed_) {
        // Failed sensor returns random values
        current_readings_.fuel_level = 50.0 + (noise_dist(gen) * 20.0);
        return;
    }
    
    if (generator_running) {
        // Very slow fuel consumption - only 0.001% per second regardless of load
        double consumption_rate = 0.001;  // 0.001% per second
        current_readings_.fuel_level -= consumption_rate * delta_time;
        
        // Prevent fuel from going below 0
        if (current_readings_.fuel_level < 0.0) {
            current_readings_.fuel_level = 0.0;
        }
    }
    
    // Add calibration drift
    current_readings_.fuel_level += fuel_calibration_drift_ * delta_time;
    
    // Add noise
    current_readings_.fuel_level = add_noise(current_readings_.fuel_level, SENSOR_NOISE_LEVEL);
    
    // Clamp to valid range (0-100%)
    if (current_readings_.fuel_level > 100.0) current_readings_.fuel_level = 100.0;
    if (current_readings_.fuel_level < 0.0) current_readings_.fuel_level = 0.0;
}

void Sensors::update_oil_pressure_sensor(double delta_time, bool generator_running, double load_percentage) {
    if (oil_sensor_failed_) {
        // Failed sensor returns random values
        current_readings_.oil_pressure = 2.0 + (noise_dist(gen) * 1.0);
        return;
    }
    
    if (generator_running) {
        // Oil pressure increases with load
        double target_pressure = OIL_PRESSURE_BASE + (load_percentage * OIL_PRESSURE_LOAD_FACTOR);
        current_readings_.oil_pressure = smooth_transition(current_readings_.oil_pressure, target_pressure, 2.0, delta_time);
    } else {
        current_readings_.oil_pressure = 0.0;
    }
    
    // Add calibration drift
    current_readings_.oil_pressure += oil_calibration_drift_ * delta_time;
    
    // Add noise
    current_readings_.oil_pressure = add_noise(current_readings_.oil_pressure, SENSOR_NOISE_LEVEL);
    
    // Clamp to valid range
    if (current_readings_.oil_pressure < 0.0) current_readings_.oil_pressure = 0.0;
    if (current_readings_.oil_pressure > 10.0) current_readings_.oil_pressure = 10.0;
}

void Sensors::update_temperature_sensors(double delta_time, bool generator_running, double load_percentage) {
    if (temp_sensor_failed_) {
        // Failed sensor returns random values
        current_readings_.cooling_temp = 80.0 + (noise_dist(gen) * 20.0);
        return;
    }
    
    if (generator_running) {
        // Cooling temperature increases with load
        double target_temp = COOLING_TEMP_BASE + (load_percentage * COOLING_TEMP_LOAD_FACTOR);
        current_readings_.cooling_temp = smooth_transition(current_readings_.cooling_temp, target_temp, 5.0, delta_time);
        
        // Exhaust temperature follows cooling temp with some lag
        double target_exhaust = target_temp + 200.0; // Exhaust is much hotter
        current_readings_.exhaust_temp = smooth_transition(current_readings_.exhaust_temp, target_exhaust, 10.0, delta_time);
    } else {
        // Cool down when stopped
        current_readings_.cooling_temp = smooth_transition(current_readings_.cooling_temp, current_readings_.ambient_temp, 2.0, delta_time);
        current_readings_.exhaust_temp = smooth_transition(current_readings_.exhaust_temp, current_readings_.ambient_temp, 5.0, delta_time);
    }
    
    // Add calibration drift
    current_readings_.cooling_temp += temp_calibration_drift_ * delta_time;
    current_readings_.exhaust_temp += temp_calibration_drift_ * delta_time;
    
    // Add noise
    current_readings_.cooling_temp = add_noise(current_readings_.cooling_temp, SENSOR_NOISE_LEVEL);
    current_readings_.exhaust_temp = add_noise(current_readings_.exhaust_temp, SENSOR_NOISE_LEVEL);
    
    // Clamp to valid ranges
    if (current_readings_.cooling_temp < -20.0) current_readings_.cooling_temp = -20.0;
    if (current_readings_.cooling_temp > 150.0) current_readings_.cooling_temp = 150.0;
    if (current_readings_.exhaust_temp < -20.0) current_readings_.exhaust_temp = -20.0;
    if (current_readings_.exhaust_temp > 600.0) current_readings_.exhaust_temp = 600.0;
}

void Sensors::update_vibration_sensor(double delta_time, bool generator_running, double load_percentage) {
    if (generator_running) {
        // Vibration increases with load
        double target_vibration = VIBRATION_BASE + (load_percentage * VIBRATION_LOAD_FACTOR);
        current_readings_.vibration = smooth_transition(current_readings_.vibration, target_vibration, 1.0, delta_time);
    } else {
        current_readings_.vibration = 0.0;
    }
    
    // Add noise
    current_readings_.vibration = add_noise(current_readings_.vibration, SENSOR_NOISE_LEVEL);
    
    // Clamp to valid range
    if (current_readings_.vibration < 0.0) current_readings_.vibration = 0.0;
    if (current_readings_.vibration > 50.0) current_readings_.vibration = 50.0;
}

double Sensors::add_noise(double value, double noise_level) const {
    return value + (noise_dist(gen) * value * noise_level);
}

double Sensors::add_drift(double value, double drift_rate, double delta_time) {
    return value + (drift_rate * delta_time);
}

double Sensors::smooth_transition(double current, double target, double rate, double delta_time) const {
    double difference = target - current;
    double max_change = rate * delta_time;
    
    if (std::abs(difference) <= max_change) {
        return target;
    } else {
        return current + (difference > 0 ? max_change : -max_change);
    }
}
