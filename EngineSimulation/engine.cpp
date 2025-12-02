#include "engine.h"
#include <iostream>
#include <limits>
using namespace std;

Engine::Engine() {
	srand(static_cast<unsigned int>(time(nullptr)));  // 根据当前时间的随机种子
	resetParameters();
}

void Engine::resetParameters() {
	// 重置引擎参数到初始状态
	state = EngineState::OFF;
	subState = EngineSubState::NONE;
	simElapsed = 0.0;
	fuelReserve = FUEL_CAPACITY;
	startPhaseElapsed = 0.0;
	stopPhaseElapsed = 0.0;
	fuelFlow = 0.0;
	fuelReserve = FUEL_CAPACITY;
	fuelFlowBase = 0.0;
	fuelReserveSensorInvalid = false;
	fuelFlowSensorInvalid = false;
	fuelFlowOverridden = false;
	leftEngine = SingleEngine(); // 用构造函数重置左引擎
	rightEngine = SingleEngine(); // 用构造函数重置右引擎
}

void Engine::start() {
	if (state == EngineState::OFF) {
		resetParameters();
		state = EngineState::STARTING;
		subState = EngineSubState::LINEAR_START;
		cout << "[Engine] Starting sequence initiated." << endl;
	}
	else {
		cout << "[Engine] Start command ignored. Engine is not OFF." << endl;
		return;
	}
}

// 解除传感器覆盖
static void releaseCoolingOverride(SingleEngine& engine) {
	for (int s = 0; s < 2; ++s) {
		// N1 覆盖释放条件：已覆盖 + 非强制异常 + 覆盖值在有效范围 (0 <= v <= 1.25 * 40000)
		if (engine.n1SensorOverridden[s]) {
			double value = engine.n1SensorOverrideVal[s];
			double percent = (value / N1_MAX_RATED);
			bool overrideIsFail = std::isnan(value) || percent < 0.0 || percent > 1.25;
			if (!engine.n1SensorForcedAnomal[s] && !overrideIsFail) {
				engine.n1SensorOverridden[s] = false;
			}
		}
		// EGT 覆盖释放条件：已覆盖 + 非强制异常 + 覆盖值在有效范围 (-5 <= v <= 1200)
		if (engine.egtSensorOverridden[s]) {
			double v = engine.egtSensorOverrideVal[s];
			bool overrideIsFail = std::isnan(v) || v < -5.0 || v > 1200.0;
			if (!engine.egtSensorForcedAnomal[s] && !overrideIsFail) {
				engine.egtSensorOverridden[s] = false;
			}
		}
	}
}

void Engine::stop() {
	if (state == EngineState::STOPPING || state == EngineState::OFF) {
		cout << "[Engine] Stop command ignored. Engine is already stopping or off." << endl;
		return;
	}

	// 需要从显示值开始冷却（符合常理）
	double n1_left = getN1Left();
	double n1_right = getN1Right();
	double egt_left = getEgtLeft();
	double egt_right = getEgtRight();

	// 如果不是NaN，则将基准值改为当前显示值，就可以实现从显示值开始冷却
	if (!isnan(n1_left)) leftEngine.n1Base = n1_left;
	if (!isnan(n1_right)) rightEngine.n1Base = n1_right;
	if (!isnan(egt_left)) leftEngine.egtBase = egt_left;
	if (!isnan(egt_right)) rightEngine.egtBase = egt_right;

	releaseCoolingOverride(leftEngine);
	releaseCoolingOverride(rightEngine);

	state = EngineState::STOPPING;
	subState = EngineSubState::SHUTDOWN;
	stopPhaseElapsed = 0.0;
	fuelFlow = 0.0;
	fuelFlowOverridden = false;
	cout << "[Engine] Stopping sequence initiated.\n";
}

void Engine::advance(double dt) {
	if (state == EngineState::OFF) {
		simElapsed += dt;
		return;
	}

	// 传感器无效检查
	if (fuelReserve < 0.0 || fuelReserve > FUEL_CAPACITY) {
		fuelReserveSensorInvalid = true;
	}
	if (fuelReserveSensorInvalid && state != EngineState::STOPPING) {
		stop();
	}

	simElapsed += dt;

	// 更新引擎状态
	switch (state) {
		case EngineState::STARTING:
			// 按照题目要求的分段函数进行启动
			subState = (dt <= 2.0) ? EngineSubState::LINEAR_START : EngineSubState::LOG_START;
			double nVal = (dt <= 2.0) ? (10000.0 * dt) : (23000.0 * log10(dt - 1.0) + 20000.0);
			double vVal = (dt <= 2.0) ? (5.0 * dt) : (42.0 * log10(dt - 1.0) + 10.0);
			double tVal = (dt <= 2.0) ? AMBIENT_TEMP : (900.0 * log10(dt - 1.0) + AMBIENT_TEMP);

			leftEngine.n1True = rightEngine.n1True = min(nVal, N1_MAX_RATED);
			leftEngine.egtTrue = rightEngine.egtTrue = min(tVal, EGT_MAX);
			fuelFlow = min(vVal, FUEL_FLOW_MAX);
			break;
		case EngineState::STABLE:
			// 在稳定状态下，引擎参数围绕基准值小幅波动
			leftEngine.n1True = leftEngine.n1Base * (1.0 + ((rand() % 601) / 10000.0) - 0.03);
			leftEngine.egtTrue = leftEngine.egtBase * (1.0 + ((rand() % 601) / 10000.0) - 0.03);
			rightEngine.n1True = rightEngine.n1Base * (1.0 + ((rand() % 601) / 10000.0) - 0.03);
			rightEngine.egtTrue = rightEngine.egtBase * (1.0 + ((rand() % 601) / 10000.0) - 0.03);
			fuelFlow = min(fuelFlowBase * (1.0 + ((rand() % 601) / 10000.0) - 0.03), FUEL_FLOW_MAX);
			break;
		case EngineState::STOPPING:
			// 按照题目要求的对数函数进行冷却及停止
			double factor = (dt < 8.0) ? (1.0 - log10(dt + 1.0) / log10(9.0)) : 0.0;
			leftEngine.n1True = leftEngine.n1Base * factor;
			leftEngine.egtTrue = AMBIENT_TEMP + (leftEngine.egtBase - AMBIENT_TEMP) * factor;
			rightEngine.n1True = rightEngine.n1Base * factor;
			rightEngine.egtTrue = AMBIENT_TEMP + (rightEngine.egtBase - AMBIENT_TEMP) * factor;
			break;
		default:
			break;
	}

	updateSensor(leftEngine);
	updateSensor(rightEngine);

	// 模拟燃油消耗
	if (!fuelReserveSensorInvalid) {
		fuelReserve -= fuelFlow * dt;
		if (fuelReserve <= 0) {
			fuelReserve = 0;
			if (state != EngineState::STOPPING && state != EngineState::OFF) {
				std::cout << "[Engine] Fuel used-up. Shutting down engine.\n";
				stop();
			}
		}
	}
}

// 增加推力
void Engine::increaseThrust() {
	if (state != EngineState::STABLE) {
		return;
	}
	fuelFlowBase = min(fuelFlowBase + 1.0, FUEL_FLOW_MAX);
	double increase = 0.03 + (rand() % 21) / 1000.0; // 3% - 5%
	leftEngine.n1Base = min(leftEngine.n1Base * (1.0 + increase), N1_MAX_RATED);
	rightEngine.n1Base = min(rightEngine.n1Base * (1.0 + increase), N1_MAX_RATED);
	leftEngine.egtBase = min(leftEngine.egtBase * (1.0 + increase), EGT_MAX);
	rightEngine.egtBase = min(rightEngine.egtBase * (1.0 + increase), EGT_MAX);
	cout << "[Engine] Thrust increased.\n";
}

// 减少推力
void Engine::decreaseThrust() {
	if (state != EngineState::STABLE) {
		return;
	}
	fuelFlowBase = max(fuelFlowBase - 1.0, 0.0);
	double decrease = 0.03 + (rand() % 21) / 1000.0; // 3% - 5%
	leftEngine.n1Base = max(leftEngine.n1Base * (1.0 - decrease), 0.0);
	rightEngine.n1Base = max(rightEngine.n1Base * (1.0 - decrease), 0.0);
	leftEngine.egtBase = max(leftEngine.egtBase * (1.0 - decrease), AMBIENT_TEMP);
	rightEngine.egtBase = max(rightEngine.egtBase * (1.0 - decrease), AMBIENT_TEMP);
	cout << "[Engine] Thrust decreased.\n";
}

void Engine::updateSensor(SingleEngine& engine) {
	// 更新N1传感器读数
	for (int s = 0; s < 2; ++s) {
		if (engine.n1SensorOverridden[s]) {
			engine.n1Sensor[s] = engine.n1SensorOverrideVal[s];
		}
		else if (engine.n1SensorForcedAnomal[s]) {
			// 强制异常时，保持上次读数不变
		}
		else if (engine.n1SensorAnomal[s]) {
			// 异常时，输出NaN
			engine.n1Sensor[s] = numeric_limits<double>::quiet_NaN();
		}
		else {
			// 正常情况下，读数围绕真实值小幅波动
			double noise = engine.n1True * (((rand() % 201) / 10000.0) - 0.01); // ±1%
			engine.n1Sensor[s] = engine.n1True + noise;
		}
	}
}

// 仪表显示函数，两个都正常时取平均值，一个异常时取另一个，两个都异常时返回NaN
double Engine::getDisplayedValue(const SingleEngine& engine, bool isN1) const {
	double sum = 0.0;
	int count = 0;
	for (int s = 0; s < 2; ++s) {
		bool bad = isN1 ? engine.n1SensorAnomal[s] : engine.egtSensorAnomal[s];
		if (!bad) {
			double value = isN1 ? engine.n1Sensor[s] : engine.egtSensor[s];
			if (!isnan(value)) {
				sum += value;
				++count;
			}
		}
	}

	return (count > 0) ? (sum / count) : numeric_limits<double>::quiet_NaN();
}

// 仅供 UI / 日志读取
double Engine::getSimTime() const { return simElapsed; }

// 传感器与显示值
double Engine::getN1Left() const {return getDisplayedValue(leftEngine, true);}
double Engine::getN1Right() const {return getDisplayedValue(rightEngine, true);}
double Engine::getEgtLeft() const {return getDisplayedValue(leftEngine, false);}
double Engine::getEgtRight() const {return getDisplayedValue(rightEngine, false);}

double Engine::getN1LeftPercentage() const {
	double n1 = getN1Left();
	return isnan(n1) ? numeric_limits<double>::quiet_NaN() : (n1 / N1_MAX_RATED) * 100.0;
}

double Engine::getN1RightPercentage() const {
	double n1 = getN1Right();
	return isnan(n1) ? numeric_limits<double>::quiet_NaN() : (n1 / N1_MAX_RATED) * 100.0;
}

double Engine::getFuelFlow() const {
	return fuelFlowSensorInvalid ? numeric_limits<double>::quiet_NaN() : fuelFlow;
}

double Engine::getFuelReserve() const {
	return fuelReserveSensorInvalid ? numeric_limits<double>::quiet_NaN() : fuelReserve;
}

EngineState Engine::getState() const {return state;}
EngineSubState Engine::getSubState() const {return subState;}



// 控制接口，用于指令行传感器覆盖和异常
double Engine::getSensorValue(int engine_id, int sensor_type, int sensor_id) const {
	const SingleEngine& engine = (engine_id == 0) ? leftEngine : rightEngine;
	if (sensor_type == 0) { // N1
		if (engine.n1SensorAnomal[sensor_id])
			return numeric_limits<double>::quiet_NaN();
		return engine.n1Sensor[sensor_id];
	}
	else {
		if (engine.egtSensorAnomal[sensor_id])
			return numeric_limits<double>::quiet_NaN();
		return engine.egtSensor[sensor_id];
	}
}

void Engine::setForcedN1Sensor(int engine_id, int sensor_id, double value) {
	SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	if (sensor_id < 0 || sensor_id>1) return;
	eng.n1SensorOverridden[sensor_id] = true;
	eng.n1SensorOverrideVal[sensor_id] = value;
}

void Engine::resetN1SensorOverride(int engine_id, int sensor_id) {
	SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	if (sensor_id < 0 || sensor_id>1) return;
	eng.n1SensorOverridden[sensor_id] = false;
}

void Engine::setForcedEGTSensor(int engine_id, int sensor_id, double value) {
	SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	if (sensor_id < 0 || sensor_id>1) return;
	eng.egtSensorOverridden[sensor_id] = true;
	eng.egtSensorOverrideVal[sensor_id] = value;
}

void Engine::resetEGTSensorOverride(int engine_id, int sensor_id) {
	SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	if (sensor_id < 0 || sensor_id>1) return;
	eng.egtSensorOverridden[sensor_id] = false;
}

void Engine::setForcedFuelReserve(double value) {fuelReserve = value;}
void Engine::resetFuelReserveOverride() { fuelReserve = FUEL_CAPACITY; }
void Engine::setFuelReserveSensorInvalid(bool invalid) { fuelReserveSensorInvalid = invalid; }
bool Engine::isFuelReserveSensorInvalid() const { return fuelReserveSensorInvalid; }
void Engine::setFuelFlowSensorInvalid(bool invalid) { fuelFlowSensorInvalid = invalid; }
bool Engine::isFuelFlowSensorInvalid() const { return fuelFlowSensorInvalid; }
void Engine::setForcedFuelFlow(double value) { fuelFlow = value; fuelFlowOverridden = true; }
void Engine::resetForcedFuelFlow() { fuelFlowOverridden = false; }

// 是否两个传感器均异常
bool Engine::isN1SystemFault(int engine_id) const {
	const SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	return eng.n1SensorAnomal[0] && eng.n1SensorAnomal[1];
}
bool Engine::isEGTSystemFault(int engine_id) const {
	const SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	return eng.egtSensorAnomal[0] && eng.egtSensorAnomal[1];
}
bool Engine::isN1SensorAnomal(int engine_id, int s) const {
	const SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	return (s >= 0 && s < 2) ? eng.n1SensorAnomal[s] : false;
}
bool Engine::isEGTSensorAnomal(int engine_id, int s) const {
	const SingleEngine& eng = (engine_id == 0) ? leftEngine : rightEngine;
	return (s >= 0 && s < 2) ? eng.egtSensorAnomal[s] : false;
}