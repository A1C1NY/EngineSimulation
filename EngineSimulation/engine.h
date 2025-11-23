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
enum class EngineSubState { NONE, LINEAR_START, LOG_START, SHUTDOWN, STABLE_RUN};

// 单个引擎的结构体
struct SingleEngine {
	double n1_true = 0.0;  // 真实的N1转速
	double egt_true = AMBIENT_TEMP;  // 真实的EGT温度
	double n1_sensor[2] = { 0.0, 0.0 }; // N1传感器读数x2
	double egt_sensor[2] = { AMBIENT_TEMP, AMBIENT_TEMP }; // EGT传感器读数x2
	bool n1_sensor_anomalous[2] = { false, false }; // N1传感器异常标志x2
	bool egt_sensor_anomalous[2] = { false, false }; // EGT传感器异常标志x2
	bool n1_sensor_overridden[2] = { false, false }; // N1传感器覆盖标志x2(fail时是否需要屏蔽)
	bool egt_sensor_overridden[2] = { false, false }; // EGT传感器覆盖标志x2(fail时是否需要屏蔽)
	bool n1_sensor_forced_anomalous[2] = { false, false }; // N1传感器强制异常标志x2（输出保持）
	bool egt_sensor_forced_anomalous[2] = { false, false }; // EGT传感器强制异常标志x2（输出保持）
	double n1_sensor_override_value[2] = { 0.0, 0.0 }; // N1传感器覆盖值x2
	double egt_sensor_override_value[2] = { 0.0, 0.0 }; // EGT传感器覆盖值x2
    double n1_base = 0.0;
    double egt_base = AMBIENT_TEMP;
};

class Engine {
public:
    Engine();

	// 启动与停止
    void start();
    void stop();

    // 固定步长推进（取代原来的依赖墙钟的 update）
    void advance(double dt);

	// 推力控制
    void increaseThrust();
    void decreaseThrust();

    // 仅供 UI / 日志读取
    double getSimTime() const;

    // 传感器与显示值
    double getN1Left() const;
    double getN1Right() const;
    double getEgtLeft() const;
    double getEgtRight() const;
    double getN1LeftPercentage() const;
    double getN1RightPercentage() const;
    double getFuelFlow() const;
    double getFuelReserve() const;
    EngineState getState() const;
    EngineSubState getSubState() const;

    double getSensorValue(int engine_idx, int sensor_type, int sensor_idx) const;

    // 控制接口（保留）
    void setForcedN1Sensor(int e, int s, double v);
    void resetN1SensorOverride(int e, int s);
    void setForcedEGTSensor(int e, int s, double v);
    void resetEGTSensorOverride(int e, int s);
    void setForcedFuelReserve(double value);
    void resetFuelReserveOverride();
    void setFuelReserveSensorInvalid(bool invalid);
    bool isFuelReserveSensorInvalid() const;
    void setFuelFlowSensorInvalid(bool invalid);
    bool isFuelFlowSensorInvalid() const;
    void setForcedFuelFlow(double value);
    void resetForcedFuelFlow();

    bool isN1SensorAnomalous(int e, int s) const;
    bool isEGTSensorAnomalous(int e, int s) const;
    bool isN1SystemFault(int e) const;
    bool isEGTSystemFault(int e) const;



private:
	// 状态更新函数
    void resetParameters();
    void updateSensor(SingleEngine& eng);
    double getDisplayedValue(const SingleEngine& eng, bool isN1) const;

	// 引擎状态初始化
    EngineState state = EngineState::OFF;
    EngineSubState subState = EngineSubState::NONE;

	// 时间跟踪
    double simElapsed = 0.0;
    double startPhaseElapsed = 0.0;
    double stopPhaseElapsed = 0.0;

	// 燃油参数
    double fuelFlow = 0.0;
    double fuelReserve = 0.0;
    double fuelFlowBase = 0.0;
    bool fuelReserveSensorInvalid = false;
    bool fuelFlowSensorInvalid = false;
    bool fuelFlowOverridden = false;

	// 2台引擎
    SingleEngine leftEngine;
    SingleEngine rightEngine;
};