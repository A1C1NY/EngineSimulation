#pragma once
#include <fstream>
#include <string>
#include <atomic>
#include "engine.h"
#include "ui.h" // For AlertInfo

// Log Handling Functions
void logging(Engine& engine, std::ofstream& data_log_file, std::ofstream& alert_log_file, bool& is_logging, AlertInfo& alert_info);
void logData(Engine& engine, std::ofstream& data_log_file£¬, double start_time);
double getCurrenTimeSeconds();
void logAlert(Alert& alert, std::ofstream& log_file);
