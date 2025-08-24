#include "Generator.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Generator::Generator()
    : current_state_(State::STOPPED)
    , target_rpm_(0.0)
    , current_rpm_(0.0)
    , target_voltage_(0.0)
    , current_voltage_(0.0)
    , target_frequency_(0.0)
    , current_frequency_(0.0)
    , target_load_(0.0)
    , current_load_(0.0)
    , max_rpm_(1800.0)
    , max_voltage_(440.0)
    , max_frequency_(60.0)
    , max_load_(100.0)
    , sensors_(std::make_unique<Sensors>())
    , startup_time_(0.0)
    , shutdown_time_(0.0)
{
    last_update_ = std::chrono::system_clock::now();
}

bool Generator::start() {
    if (current_state_ == State::STOPPED || current_state_ == State::FAULT) {
        current_state_ = State::STARTING;
        startup_time_ = 0.0;
        target_rpm_ = max_rpm_;
        target_voltage_ = max_voltage_;
        target_frequency_ = max_frequency_;
        std::cout << "Generator starting..." << std::endl;
        return true;
    }
    return false;
}

bool Generator::stop() {
    if (current_state_ == State::RUNNING || current_state_ == State::STARTING) {
        current_state_ = State::STOPPING;
        shutdown_time_ = 0.0;
        target_rpm_ = 0.0;
        target_voltage_ = 0.0;
        target_frequency_ = 0.0;
        target_load_ = 0.0;
        std::cout << "Generator stopping..." << std::endl;
        return true;
    }
    return false;
}

bool Generator::emergency_stop() {
    if (current_state_ != State::STOPPED && current_state_ != State::FAULT) {
        current_state_ = State::STOPPED;
        current_rpm_ = 0.0;
        current_voltage_ = 0.0;
        current_frequency_ = 0.0;
        current_load_ = 0.0;
        target_load_ = 0.0;
        std::cout << "EMERGENCY STOP ACTIVATED!" << std::endl;
        return true;
    }
    return false;
}

void Generator::set_load(double percentage) {
    // Cannot change load when generator is stopped
    if (current_state_ == State::STOPPED || current_state_ == State::FAULT) {
        std::cout << "Cannot change load - generator is stopped" << std::endl;
        return;
    }
    
    // Enforce minimum 20% load when running
    if (current_state_ == State::RUNNING && percentage < 20.0) {
        percentage = 20.0;
        std::cout << "Load adjusted to minimum 20%" << std::endl;
    }
    
    if (percentage < 0.0) percentage = 0.0;
    if (percentage > max_load_) percentage = max_load_;
    
    target_load_ = percentage;
    if (current_state_ == State::RUNNING) {
        std::cout << "Load set to " << percentage << "%" << std::endl;
    }
}

Generator::GeneratorStatus Generator::get_status() const {
    GeneratorStatus status;
    status.state = current_state_;
    status.rpm = current_rpm_;
    status.voltage = current_voltage_;
    status.frequency = current_frequency_;
    status.load_percentage = current_load_;
    
    auto sensor_readings = sensors_->get_readings();
    status.fuel_level = sensor_readings.fuel_level;
    status.oil_pressure = sensor_readings.oil_pressure;
    status.cooling_temp = sensor_readings.cooling_temp;
    
    // Get only active alarms
    status.active_alarms.clear();
    for (const auto& alarm : alarms_) {
        if (alarm.active) {
            status.active_alarms.push_back(alarm);
        }
    }
    
    return status;
}

std::vector<Generator::Alarm> Generator::get_alarms() const {
    return alarms_;
}

void Generator::update(double delta_time) {
    auto now = std::chrono::system_clock::now();
    
    switch (current_state_) {
        case State::STARTING:
            update_startup_sequence(delta_time);
            break;
        case State::RUNNING:
            update_running_state(delta_time);
            break;
        case State::STOPPING:
            update_shutdown_sequence(delta_time);
            break;
        case State::STOPPED:
        case State::FAULT:
            // No updates needed
            break;
    }
    
    // Update sensors
    sensors_->update(delta_time, current_state_ == State::RUNNING, current_load_);
    
    // Check for alarm conditions
    check_alarm_conditions();
    
    last_update_ = now;
}

void Generator::set_parameters(double max_rpm, double max_voltage, double max_frequency) {
    max_rpm_ = max_rpm;
    max_voltage_ = max_voltage;
    max_frequency_ = max_frequency;
}

void Generator::acknowledge_alarm(AlarmType type) {
    for (auto& alarm : alarms_) {
        if (alarm.type == type && alarm.active) {
            alarm.active = false;
            std::cout << "Alarm acknowledged: " << alarm.message << std::endl;
        }
    }
}

void Generator::reset_alarms() {
    for (auto& alarm : alarms_) {
        alarm.active = false;
    }
    std::cout << "All alarms reset" << std::endl;
}

void Generator::update_startup_sequence(double delta_time) {
    startup_time_ += delta_time;
    
    // Smooth transitions during startup
    current_rpm_ = smooth_transition(current_rpm_, target_rpm_, RPM_ACCELERATION_RATE, delta_time);
    current_voltage_ = smooth_transition(current_voltage_, target_voltage_, VOLTAGE_RAMP_RATE, delta_time);
    current_frequency_ = smooth_transition(current_frequency_, target_frequency_, FREQUENCY_RAMP_RATE, delta_time);
    
    // Check if startup is complete
    if (startup_time_ >= STARTUP_TIME && 
        std::abs(current_rpm_ - target_rpm_) < 10.0 &&
        std::abs(current_voltage_ - target_voltage_) < 5.0 &&
        std::abs(current_frequency_ - target_frequency_) < 0.5) {
        
        current_state_ = State::RUNNING;
        std::cout << "Generator startup complete - now running" << std::endl;
    }
}

void Generator::update_running_state(double delta_time) {
    // Smooth load transitions
    current_load_ = smooth_transition(current_load_, target_load_, LOAD_RAMP_RATE, delta_time);
    
    // Adjust RPM based on load (governor behavior)
    double load_factor = current_load_ / max_load_;
    double target_rpm_at_load = max_rpm_ - (load_factor * 50.0); // Slight droop
    current_rpm_ = smooth_transition(current_rpm_, target_rpm_at_load, RPM_ACCELERATION_RATE, delta_time);
    
    // Adjust voltage based on load (AVR behavior)
    double target_voltage_at_load = max_voltage_ - (load_factor * 10.0); // Slight droop
    current_voltage_ = smooth_transition(current_voltage_, target_voltage_at_load, VOLTAGE_RAMP_RATE, delta_time);
    
    // Frequency follows RPM
    current_frequency_ = (current_rpm_ / max_rpm_) * max_frequency_;
}

void Generator::update_shutdown_sequence(double delta_time) {
    shutdown_time_ += delta_time;
    
    // Smooth shutdown
    current_rpm_ = smooth_transition(current_rpm_, 0.0, RPM_ACCELERATION_RATE, delta_time);
    current_voltage_ = smooth_transition(current_voltage_, 0.0, VOLTAGE_RAMP_RATE, delta_time);
    current_frequency_ = smooth_transition(current_frequency_, 0.0, FREQUENCY_RAMP_RATE, delta_time);
    current_load_ = smooth_transition(current_load_, 0.0, LOAD_RAMP_RATE, delta_time);
    
    // Check if shutdown is complete
    if (shutdown_time_ >= SHUTDOWN_TIME || 
        (current_rpm_ < 50.0 && current_voltage_ < 10.0)) {
        
        current_state_ = State::STOPPED;
        current_rpm_ = 0.0;
        current_voltage_ = 0.0;
        current_frequency_ = 0.0;
        current_load_ = 0.0;
        std::cout << "Generator shutdown complete" << std::endl;
    }
}

void Generator::check_alarm_conditions() {
    auto sensor_readings = sensors_->get_readings();
    
    // Check fuel level
    if (sensor_readings.fuel_level < 10.0) {
        add_alarm(AlarmType::LOW_FUEL_LEVEL, "Low fuel level: " + std::to_string(sensor_readings.fuel_level) + "%");
    } else {
        remove_alarm(AlarmType::LOW_FUEL_LEVEL);
    }
    
    // Check oil pressure
    if (sensor_readings.oil_pressure < 1.5) {
        add_alarm(AlarmType::LOW_OIL_PRESSURE, "Low oil pressure: " + std::to_string(sensor_readings.oil_pressure) + " bar");
    } else {
        remove_alarm(AlarmType::LOW_OIL_PRESSURE);
    }
    
    // Check temperature
    if (sensor_readings.cooling_temp > 110.0) {
        add_alarm(AlarmType::HIGH_TEMPERATURE, "High temperature: " + std::to_string(sensor_readings.cooling_temp) + "°C");
    } else {
        remove_alarm(AlarmType::HIGH_TEMPERATURE);
    }
    
    // Check overload
    if (current_load_ > max_load_ * 0.95) {
        add_alarm(AlarmType::OVERLOAD, "Generator overload: " + std::to_string(current_load_) + "%");
    } else {
        remove_alarm(AlarmType::OVERLOAD);
    }
    
    // Check overspeed
    if (current_rpm_ > max_rpm_ * 1.1) {
        add_alarm(AlarmType::OVERSPEED, "Generator overspeed: " + std::to_string(current_rpm_) + " RPM");
        emergency_stop(); // Critical fault
    }
    
    // Check vibration
    if (sensor_readings.vibration > 15.0) {
        add_alarm(AlarmType::HIGH_VIBRATION, "High vibration: " + std::to_string(sensor_readings.vibration) + " mm/s");
    }
}

void Generator::add_alarm(AlarmType type, const std::string& message) {
    // Check if alarm already exists and is active
    for (auto& alarm : alarms_) {
        if (alarm.type == type && alarm.active) {
            return; // Already active
        }
    }
    
    // Create new alarm
    Alarm alarm;
    alarm.type = type;
    alarm.message = message;
    alarm.timestamp = std::chrono::system_clock::now();
    alarm.active = true;
    
    alarms_.push_back(alarm);
    std::cout << "ALARM: " << message << std::endl;
}

void Generator::remove_alarm(AlarmType type) {
    for (auto& alarm : alarms_) {
        if (alarm.type == type && alarm.active) {
            alarm.active = false;
        }
    }
}

double Generator::smooth_transition(double current, double target, double rate, double delta_time) {
    double difference = target - current;
    double max_change = rate * delta_time;
    
    if (std::abs(difference) <= max_change) {
        return target;
    } else {
        return current + (difference > 0 ? max_change : -max_change);
    }
}
