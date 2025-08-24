#pragma once

#include <chrono>

/**
 * @brief Sensor monitoring system for the marine generator
 * 
 * This class simulates various sensors and provides realistic
 * sensor readings with noise and drift characteristics
 */
class Sensors {
public:
    struct SensorReadings {
        double fuel_level;      // Percentage (0-100)
        double oil_pressure;    // Bar
        double cooling_temp;    // Celsius
        double vibration;       // mm/s RMS
        double exhaust_temp;    // Celsius
        double ambient_temp;    // Celsius
        double humidity;        // Percentage
    };

    Sensors();
    ~Sensors() = default;

    // Get current sensor readings
    SensorReadings get_readings() const;
    
    // Update sensor values based on generator state
    void update(double delta_time, bool generator_running, double load_percentage);
    
    // Simulate sensor failures or calibration drift
    void set_sensor_failure(bool fuel_failed, bool oil_failed, bool temp_failed);
    void set_calibration_drift(double fuel_drift, double oil_drift, double temp_drift);
    
    // Reset sensors to normal operation
    void reset_sensors();

private:
    // Current sensor values
    SensorReadings current_readings_;
    
    // Sensor failure flags
    bool fuel_sensor_failed_;
    bool oil_sensor_failed_;
    bool temp_sensor_failed_;
    
    // Calibration drift values
    double fuel_calibration_drift_;
    double oil_calibration_drift_;
    double temp_calibration_drift_;
    
    // Noise and drift simulation
    double add_noise(double value, double noise_level) const;
    double add_drift(double value, double drift_rate, double delta_time);
    
    // Utility function for smooth value transitions
    double smooth_transition(double current, double target, double rate, double delta_time) const;
    
    // Sensor-specific update methods
    void update_fuel_sensor(double delta_time, bool generator_running, double load_percentage);
    void update_oil_pressure_sensor(double delta_time, bool generator_running, double load_percentage);
    void update_temperature_sensors(double delta_time, bool generator_running, double load_percentage);
    void update_vibration_sensor(double delta_time, bool generator_running, double load_percentage);
    
    // Constants for realistic sensor behavior
    static constexpr double FUEL_CONSUMPTION_RATE = 0.1;      // % per second at full load
    static constexpr double OIL_PRESSURE_BASE = 3.0;          // Bar at idle
    static constexpr double OIL_PRESSURE_LOAD_FACTOR = 0.02;  // Bar per % load
    static constexpr double COOLING_TEMP_BASE = 85.0;         // Celsius at normal operation
    static constexpr double COOLING_TEMP_LOAD_FACTOR = 0.3;   // Celsius per % load
    static constexpr double VIBRATION_BASE = 2.0;             // mm/s at idle
    static constexpr double VIBRATION_LOAD_FACTOR = 0.05;     // mm/s per % load
    static constexpr double SENSOR_NOISE_LEVEL = 0.02;        // 2% noise
    static constexpr double DRIFT_RATE = 0.001;               // Slow drift over time
};
