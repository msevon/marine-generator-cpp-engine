#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "Sensors.h"

/**
 * @brief Marine Generator Simulation Engine
 * 
 * This class models the behavior of a marine generator including:
 * - Engine startup/shutdown sequences
 * - Load management and power generation
 * - Sensor monitoring and alarm management
 * - Fuel consumption and efficiency calculations
 */
class Generator {
public:
    enum class State {
        STOPPED,
        STARTING,
        RUNNING,
        STOPPING,
        FAULT
    };

    enum class AlarmType {
        OVERLOAD,
        HIGH_TEMPERATURE,
        LOW_OIL_PRESSURE,
        LOW_FUEL_LEVEL,
        HIGH_VIBRATION,
        OVERSPEED
    };

    struct Alarm {
        AlarmType type;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        bool active;
    };

    struct GeneratorStatus {
        State state;
        double rpm;
        double voltage;
        double frequency;
        double load_percentage;
        double fuel_level;
        double oil_pressure;
        double cooling_temp;
        std::vector<Alarm> active_alarms;
    };

    Generator();
    ~Generator() = default;

    // Control methods
    bool start();
    bool stop();
    bool emergency_stop();
    void set_load(double percentage);
    
    // Status methods
    GeneratorStatus get_status() const;
    std::vector<Alarm> get_alarms() const;
    
    // Simulation update
    void update(double delta_time);
    
    // Configuration
    void set_parameters(double max_rpm, double max_voltage, double max_frequency);
    
    // Alarm management
    void acknowledge_alarm(AlarmType type);
    void reset_alarms();

private:
    // Generator state
    State current_state_;
    double target_rpm_;
    double current_rpm_;
    double target_voltage_;
    double current_voltage_;
    double target_frequency_;
    double current_frequency_;
    double target_load_;
    double current_load_;
    
    // Physical parameters
    double max_rpm_;
    double max_voltage_;
    double max_frequency_;
    double max_load_;
    
    // Sensor data
    std::unique_ptr<Sensors> sensors_;
    
    // Alarms
    std::vector<Alarm> alarms_;
    
    // Timing
    std::chrono::system_clock::time_point last_update_;
    double startup_time_;
    double shutdown_time_;
    
    // Internal methods
    void update_startup_sequence(double delta_time);
    void update_running_state(double delta_time);
    void update_shutdown_sequence(double delta_time);
    void check_alarm_conditions();
    void add_alarm(AlarmType type, const std::string& message);
    void remove_alarm(AlarmType type);
    
    // Smooth transitions
    double smooth_transition(double current, double target, double rate, double delta_time);
    
    // Constants
    static constexpr double RPM_ACCELERATION_RATE = 100.0;  // RPM per second
    static constexpr double VOLTAGE_RAMP_RATE = 50.0;       // Volts per second
    static constexpr double FREQUENCY_RAMP_RATE = 2.0;      // Hz per second
    static constexpr double LOAD_RAMP_RATE = 10.0;          // % per second
    static constexpr double STARTUP_TIME = 30.0;            // seconds
    static constexpr double SHUTDOWN_TIME = 15.0;           // seconds
};
