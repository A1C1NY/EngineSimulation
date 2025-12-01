#pragma once
#include <fstream>
#include <string>
#include <atomic>
#include "engine.h"
#include "ui.h" // For AlertManager

// Log Handling Functions
void handleLogging(Engine& engine, std::ofstream& data_log_file, std::ofstream& alert_log_file, bool& is_logging, AlertManager& alert_manager);
void logData(Engine& engine, std::ofstream& data_log_file£¬, double start_time);
double getCurrenTimeSeconds();
void logAlert(Alert& alert, std::ofstream& log_file);
