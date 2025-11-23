#include "event.h"
using namespace std;

void commandLoop(bool& cmdThreadRunning, Engine& engine) {
	string line;
	while (cmdThreadRunning) {
		cout << ">";
		if (!getline(cin, line) || !cmdThreadRunning) {
			break;
		}
		istringstream iss(line);
		string cmd;
		if (!(iss >> cmd)) continue;

		if (cmd == "set") {
			string target, type, lvl;
			if (!(iss >> target >> type)) {
				// set后面没有参数
				cout << "[cmdThread]Usage: set <target> <type> [level]\n";
				cout << "       e.g., set N1_LX/N1_RX/EGT_LX/EGT_RX fail(X=1-2)\n";
				cout << "       e.g., set N1_LX/N1_RX/EGT_LX/EGT_RX overspeed amber/red(X=1-2)\n";
				cout << "       e.g., set FUEL_RES low/invalid\n";
				cout << "       e.g., set FUEL_FLOW invalid/value 1000\n";
				continue;
				// 重新输入
			}
			iss >> lvl;
			if (target == "FUEL_RES") {
				if (type == "low") {
					engine.setForcedFuelReserve(1000.0);
					cout << "[cmdThread]Set " << target << " to state 'low'\n";
				}
				else if (type == "invalid") {
					engine.setFuelReserveSensorInvalid(true);
					cout << "[cmdThread]Set " << target << " to state 'invalid'\n";
				}
				else {
					cout << "[cmdThread]Invalid type for FUEL_RES. Use 'low' or 'invalid'.\n";
				}
				continue;
			}
			else if (target == "FUEL_FLOW") {
				if (type == "value" && !lvl.empty()) {
					try {
						double value_to_set = stod(lvl);
						engine.setForcedFuelFlow(value_to_set);
						cout << "[cmdThread]Set " << target << " to value " << value_to_set << "\n";
					}
					catch (...) {
						cout << "[cmdThread]Invalid value for FUEL_FLOW.\n";
					}
				}
				else if (type == "invalid") {
					engine.setFuelFlowSensorInvalid(true);
					cout << "[cmdThread]Set " << target << " to state 'invalid'\n";
				}
				else {
					cout << "[cmdThread]Invalid type for FUEL_FLOW. Use 'value <number>' or 'invalid'.\n";
				}
				continue;
			}
			double value;
			bool is_n1_target = (target.find("N1") != string::npos);
			bool is_egt_target = (target.find("EGT") != string::npos);

			if (type == "fail") {
				value = -50; // 传感器故障时的保持值
			}
			else if (type == "overspeed" && is_n1_target) {
				if (lvl == "amber") value = 43000.0;
				else if (lvl == "red") value = 49000.0;
				else { cout << "[cmdThread]Invalid level for overspeed. Use 'amber' or 'red'.\n"; continue; }
			}
			else if (type == "overtemp" && is_egt_target) {
				if (lvl == "amber") value = 960.0;
				else if (lvl == "red") value = 1110.0;
				else { cout << "[cmdThread]Invalid level for overtemp. Use 'amber' or 'red'.\n"; continue; }
			}
			else {
				std::cout << "[cmdThread]Invalid type '" << type << "' for target '" << target << "'.\n";
				continue;
			}
			if (target == "N1_L1") engine.setForcedN1Sensor(0, 0, value);
			else if (target == "N1_L2") engine.setForcedN1Sensor(0, 1, value);
			else if (target == "N1_R1") engine.setForcedN1Sensor(1, 0, value);
			else if (target == "N1_R2") engine.setForcedN1Sensor(1, 1, value);
			else if (target == "EGT_L1") engine.setForcedEGTSensor(0, 0, value);
			else if (target == "EGT_L2") engine.setForcedEGTSensor(0, 1, value);
			else if (target == "EGT_R1") engine.setForcedEGTSensor(1, 0, value);
			else if (target == "EGT_R2") engine.setForcedEGTSensor(1, 1, value);
			else {
				cout << "[cmdThread]Invalid target sensor: " << target << "\n";
				continue;
			}
		}
		else if (cmd == "reset") {
			string name;
			if (!(iss >> name)) {
				std::cout << "[cmdThread]Usage: reset NAME\n";
				continue;
			}
			if (name == "N1_L1") engine.resetN1SensorOverride(0, 0);
			else if (name == "N1_L2") engine.resetN1SensorOverride(0, 1);
			else if (name == "N1_R1") engine.resetN1SensorOverride(1, 0);
			else if (name == "N1_R2") engine.resetN1SensorOverride(1, 1);
			else if (name == "EGT_L1") engine.resetEGTSensorOverride(0, 0);
			else if (name == "EGT_L2") engine.resetEGTSensorOverride(0, 1);
			else if (name == "EGT_R1") engine.resetEGTSensorOverride(1, 0);
			else if (name == "EGT_R2") engine.resetEGTSensorOverride(1, 1);
			else if (name == "FUEL_RES") {
				engine.resetFuelReserveOverride();
				engine.setFuelReserveSensorInvalid(false);
			}
			else if (name == "FUEL_FLOW") {
				engine.resetForcedFuelFlow();
				engine.setFuelFlowSensorInvalid(false);
			}
			else {
				cout << "[cmdThread]Invalid target to reset: " << name << "\n";
				continue;
			}
			cout << "[cmdThread]Reset " << name << " override\n";
		}
		else {
			std::cout << "[cmdThread]Unknown command. Supported: set, reset\n";
		}
	}
}