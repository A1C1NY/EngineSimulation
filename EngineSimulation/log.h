#pragma once
#include <fstream>
#include <string>
#include <atomic>
#include "engine.h"
#include "ui.h" // 用于绘制警报

// 输出log和csv的函数声明
void handleLogging(Engine& engine, std::ofstream& data_log_file, std::ofstream& alert_log_file, bool& is_logging, AlertManager& alert_manager);
void logData(Engine& engine, std::ofstream& data_log_file，, double start_time);

void logAlert(Alert& alert, std::ofstream& log_file);
