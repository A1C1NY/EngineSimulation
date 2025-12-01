#include "log.h"
#include "engine.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <cmath>
#include <unordered_map>
using namespace std;	

// 输出double，NaN特殊处理
static ostream& outDouble(ostream& os, double val) { 
	if (isnan(val)) {
		os << "NaN";
	} else {
		os << val;
	}
	return os;
}

// 获取从程序启动到现在的时间，单位秒
double getCurrenTimeSeconds() {
	static auto start_time = std::chrono::high_resolution_clock::now();
	auto current_time = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<double>(current_time - start_time).count();
}

void logData(Engine& engine, std::ofstream& f, double start_time) {
	if (!f.is_open()) return;
	int total_ms = static_cast<int>(start_time * 1000 + 0.5);
	int seconds = total_ms / 1000;
	int milliseconds = total_ms % 1000;
	f << seconds << "." << setfill('0') << setw(3) << milliseconds << ",";
	f << std::fixed << std::setprecision(1);
	// 依次记录各项数据
	outDouble(f, engine.getSensorValue(0, 0, 0)) << ",";
	outDouble(f, engine.getSensorValue(0, 0, 1)) << ",";
	outDouble(f, engine.getN1Left()) << ",";
	outDouble(f, engine.getSensorValue(0, 1, 0)) << ",";
	outDouble(f, engine.getSensorValue(0, 1, 1)) << ",";
	outDouble(f, engine.getEgtLeft()) << ",";
	outDouble(f, engine.getSensorValue(1, 0, 0)) << ",";
	outDouble(f, engine.getSensorValue(1, 0, 1)) << ",";
	outDouble(f, engine.getN1Right()) << ",";
	outDouble(f, engine.getSensorValue(1, 1, 0)) << ",";
	outDouble(f, engine.getSensorValue(1, 1, 1)) << ",";
	outDouble(f, engine.getEgtRight()) << ",";
	outDouble(f, engine.getFuelFlow()) << ",";
	outDouble(f, engine.getFuelReserve()) << ",";
	switch (engine.getState()) {
	case EngineState::OFF:      f << "OFF\n"; break;
	case EngineState::STARTING: f << "STARTING\n"; break;
	case EngineState::STABLE:   f << "STABLE\n"; break;
	case EngineState::STOPPING: f << "STOPPING\n"; break;
	}
}

void logAlert(Alert& alert, std::ofstream& os) {
	if (!os.is_open()) return;

	static unordered_map<string, double> lastMsg; // 每条消息上次记录时间

	double current_time = getCurrenTimeSeconds();
	auto it = lastMsg.find(alert.message);
	if (it != lastMsg.end()) {
		if (current_time - it->second < 5.0) {
			// 相同消息5秒内不重复记录
			return;
		}
	}

	auto now = chrono::system_clock::now();
	auto time = chrono::system_clock::to_time_t(now);
	tm buf;
	localtime_s(&buf, &time);
	os << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << " - ALERT: " << alert.message << "\n";
	lastMsg[alert.message] = time;
	alert.lastTime = time; // 保持原字段更新（如果其它地方用到）

}

void handleLogging(Engine& engine, ofstream& datafile, ofstream& alertfile, bool& logging, AlertManager& alert_manager) {
	if (engine.getState() != EngineState::OFF && !logging) {
		auto sysNow = chrono::system_clock::now(); // 获取系统当前时间
		auto time = chrono::system_clock::to_time_t(sysNow); // 转换为time_t格式
		tm buf;
		localtime_s(&buf, &time);

		ostringstream oss;
		oss << "engine_data_" << put_time(&buf, "%Y%m%d_%H%M%S") << ".csv";
		datafile.open(oss.str());
		datafile << "Timestamp,"
			"N1_L_S1,N1_L_S2,N1_L_Disp,"
			"EGT_L_S1,EGT_L_S2,EGT_L_Disp,"
			"N1_R_S1,N1_R_S2,N1_R_Disp,"
			"EGT_R_S1,EGT_R_S2,EGT_R_Disp,"
			"FuelFlow,FuelReserve,State\n";
		alertfile.open("engine_alerts.log", ios::app);
		logging = true;
		cout << "[Logging] Started logging to " << oss.str() << " and engine_alerts.log\n";
	}
	else if (engine.getState() == EngineState::OFF && logging) {
		datafile.close();
		alertfile.close();
		logging = false;
		cout << "[Logging] Stopped logging.\n";
	}

	if (!logging) return;

	logData(engine, datafile, engine.getSimTime());

	Alert& a = alert_manager.getCurrentAlert(); // 获取当前警报
	logAlert(a, alertfile);// 记录警报

}



