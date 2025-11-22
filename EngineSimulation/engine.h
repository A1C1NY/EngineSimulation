#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <limits>

// -----CONSTANTS-----
const double FUEL_CAPACITY = 20000.0; // 燃油容量
const double AMBIENT_TEMP = 20.0; // 环境温度
const double N1_MAX_RATED = 40000.0; // 最大额定转速
const double N1_MAX = N1_MAX_RATED * 1.25; // 最大有效转速
const double EGT_MAX = 1200.0; // 最高有效排气温度
const double FUEL_FLOW_MAX = 50.0; // 最大燃油流量
const double N1_STABLE_THRESHOLD = 0.95 * N1_MAX_RATED; // 稳定运行阈值

enum class EngineState { OFF, STARTING, STABLE, STOPPING }; 
enum class EngineSubState { NONE, START, SHUTDOWN, STABLE_RUN};

// 单个引擎的结构体
struct Engine {

};