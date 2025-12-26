#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <thread> // 用于输入指令的线程
#include <ctime>
#include <iostream>
#include <chrono>
#include <graphics.h> 
#include "engine.h" 
#include "ui.h"
#include "ui_draw.h"
#include "event.h"
#include "log.h"
using namespace std;

Engine engine;
map<string, Indicator> indicators;
map<string, TriangleButton> thrust_buttons;
AlertInfo alertInfo;
bool startButtonPressed = false;
bool stopButtonPressed = false;

bool cmdThreadRunning = true;
thread cmdThread;
ofstream data_log_file;
ofstream alert_log_file;

bool isLogging = false;


static void updateIndicators(Engine& engine, map<string, Indicator>& indicators) {
    // 更新各个指示灯时间状态
    for (auto& pair : indicators) {
        // 不让 Start 和 Run 指示灯自动熄灭
        if (pair.first != "Start" && pair.first != "Run") {
            pair.second.update();
        }
    }

    // 获取发动机状态
    EngineState state = engine.getState();
    EngineSubState subState = engine.getSubState();

    // 控制 Start 和 Run 指示灯
    if (state == EngineState::STARTING) {
        indicators.at("Start").setActive(COLOR_GREEN);
        indicators.at("Run").deactivate();
    }
    else if (state == EngineState::STABLE) {
        indicators.at("Start").deactivate();
        // N1 低于稳定阈值的95%则熄灭
        if (engine.getN1Left() < N1_STABLE_THRESHOLD * 0.95 || engine.getN1Right() < N1_STABLE_THRESHOLD * 0.95) {
            indicators.at("Run").deactivate();
        }
        else {
            indicators.at("Run").setActive(COLOR_GREEN);
        }
    }
    else { // OFF 或 STOPPING 状态
        indicators.at("Start").deactivate();
        indicators.at("Run").deactivate();
    }

    // 修改：直接触发所有警报，而不是只保留最高优先级
    auto trigger_alert = [&](const std::string& msg, const COLORREF color) {
        if (!msg.empty()) {
            alertInfo.triggerAlert(msg, color);
        }
    };

    // 更换颜色
    if (engine.isN1SensorAnomal(0, 0)) {
        indicators.at("N1_L_S1_Fail").setActive(COLOR_WHITE);
        trigger_alert("N1 SENSOR 1 LEFT ANOMALY", COLOR_WHITE);
    }
    if (engine.isN1SensorAnomal(0, 1)) {
        indicators.at("N1_L_S2_Fail").setActive(COLOR_WHITE);
        trigger_alert("N1 SENSOR 2 LEFT ANOMALY", COLOR_WHITE);
    }
    if (engine.isEGTSensorAnomal(0, 0)) {
        indicators.at("EGT_L_S1_Fail").setActive(COLOR_WHITE);
        trigger_alert("EGT SENSOR 1 LEFT ANOMALY", COLOR_WHITE);
    }
    if (engine.isEGTSensorAnomal(0, 1)) {
        indicators.at("EGT_L_S2_Fail").setActive(COLOR_WHITE);
        trigger_alert("EGT SENSOR 2 LEFT ANOMALY", COLOR_WHITE);
    }
    if (engine.isN1SensorAnomal(1, 0)) {
        indicators.at("N1_R_S1_Fail").setActive(COLOR_WHITE);
        trigger_alert("N1 SENSOR 1 RIGHT ANOMALY", COLOR_WHITE);
    }
    if (engine.isN1SensorAnomal(1, 1)) {
        indicators.at("N1_R_S2_Fail").setActive(COLOR_WHITE);
        trigger_alert("N1 SENSOR 2 RIGHT ANOMALY", COLOR_WHITE);
    }
    if (engine.isEGTSensorAnomal(1, 0)) {
        indicators.at("EGT_R_S1_Fail").setActive(COLOR_WHITE);
        trigger_alert("EGT SENSOR 1 RIGHT ANOMALY", COLOR_WHITE);
    }
    if (engine.isEGTSensorAnomal(1, 1)) {
        indicators.at("EGT_R_S2_Fail").setActive(COLOR_WHITE);
        trigger_alert("EGT SENSOR 2 RIGHT ANOMALY", COLOR_WHITE);
    }

    if (engine.isN1SystemFault(0) || engine.isN1SystemFault(1)) {
        indicators.at("N1SFail").setActive(COLOR_AMBER);
        trigger_alert("N1 SYSTEM FAULT", COLOR_AMBER);
    }
    if (engine.isEGTSystemFault(0) || engine.isEGTSystemFault(1)) {
        indicators.at("EGTSFail").setActive(COLOR_AMBER);
        trigger_alert("EGT SYSTEM FAULT", COLOR_AMBER);
    }
    if (engine.isN1SystemFault(0) && engine.isN1SystemFault(1)) {
        indicators.at("N1SFail").setActive(COLOR_RED);
        trigger_alert("DUAL N1 SYSTEM FAILURE - SHUTDOWN", COLOR_RED);
        if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
    }
    if (engine.isEGTSystemFault(0) && engine.isEGTSystemFault(1)) {
        indicators.at("EGTSFail").setActive(COLOR_RED);
        trigger_alert("DUAL EGT SYSTEM FAILURE - SHUTDOWN", COLOR_RED);
        if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
    }


    double fuelRes = engine.getFuelReserve();
    double fuelFlow = engine.getFuelFlow();
    if (engine.isFuelReserveSensorInvalid()) {
        indicators.at("FuelResFail").setActive(COLOR_RED);
        trigger_alert("FUEL RESERVE SENSOR INVALID", COLOR_RED);
    }
    else if (!std::isnan(fuelRes)) {
        if (fuelRes <= 0.0 && state != EngineState::OFF) {
                indicators.at("LowFuel").setActive(COLOR_RED);
                trigger_alert("FUEL DEPLETED - ENGINE SHUTDOWN", COLOR_RED);
            }
        else if (fuelRes < 1000.0 && state != EngineState::OFF) {
                indicators.at("LowFuel").setActive(COLOR_AMBER);
                trigger_alert("LOW FUEL RESERVE", COLOR_AMBER);
            }
        }
    if (engine.isFuelFlowSensorInvalid()) {
        indicators.at("FuelFlowFail").setActive(COLOR_AMBER);
        trigger_alert("FUEL FLOW SENSOR INVALID", COLOR_AMBER);
    }
    else if (!std::isnan(fuelFlow) && fuelFlow > FUEL_FLOW_MAX) {
        indicators.at("OverFF").setActive(COLOR_AMBER);
        trigger_alert("FUEL FLOW EXCEEDED LIMIT", COLOR_AMBER);
    }

    double n1L_pct = engine.getN1LeftPercentage();
    double n1R_pct = engine.getN1RightPercentage();
    if (!std::isnan(n1L_pct)) {
        if (n1L_pct > 120.0) {
            indicators.at("OverSpd1").setActive(COLOR_RED);
            trigger_alert("N1 LEFT OVERSPEED - SHUTDOWN", COLOR_RED);
            if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
        }
        else if (n1L_pct > 105.0) {
            indicators.at("OverSpd1").setActive(COLOR_AMBER);
            trigger_alert("N1 LEFT OVERSPEED CAUTION", COLOR_AMBER);
        }
    }
    if (!std::isnan(n1R_pct)) {
        if (n1R_pct > 120.0) {
            indicators.at("OverSpd2").setActive(COLOR_RED);
            trigger_alert("N1 RIGHT OVERSPEED - SHUTDOWN", COLOR_RED);
            if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
        }
        else if (n1R_pct > 105.0) {
            indicators.at("OverSpd2").setActive(COLOR_AMBER);
            trigger_alert("N1 RIGHT OVERSPEED CAUTION", COLOR_AMBER);
        }
    }

    bool isStartingPhase = (subState == EngineSubState::LINEAR_START || subState == EngineSubState::LOG_START);
    double egtL = engine.getEgtLeft();
    double egtR = engine.getEgtRight();

    if (isStartingPhase) {
        if ((!std::isnan(egtL) && egtL > 1000.0) || (!std::isnan(egtR) && egtR > 1000.0)) {
            indicators.at("OverTemp2").setActive(COLOR_RED);
            trigger_alert("EGT STARTING OVERTEMP - SHUTDOWN", COLOR_RED);
            if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
        }
        else if ((!std::isnan(egtL) && egtL > 850.0) || (!std::isnan(egtR) && egtR > 850.0)) {
            indicators.at("OverTemp1").setActive(COLOR_AMBER);
            trigger_alert("EGT STARTING OVERTEMP CAUTION", COLOR_AMBER);
        }
    }
    else if (state == EngineState::STABLE) {
        if ((!std::isnan(egtL) && egtL > 1100.0) || (!std::isnan(egtR) && egtR > 1100.0)) {
            indicators.at("OverTemp4").setActive(COLOR_RED);
            trigger_alert("EGT STABLE OVERTEMP - SHUTDOWN", COLOR_RED);
            if (state != EngineState::STOPPING && state != EngineState::OFF) engine.stop();
        }
        else if ((!std::isnan(egtL) && egtL > 950.0) || (!std::isnan(egtR) && egtR > 950.0)) {
            indicators.at("OverTemp3").setActive(COLOR_AMBER);
            trigger_alert("EGT STABLE OVERTEMP CAUTION", COLOR_AMBER);
        }
    }

    bool stable = (engine.getState() == EngineState::STABLE);
    if (thrust_buttons.count("ThrustUp")) thrust_buttons.at("ThrustUp").setEnabled(stable);
    if (thrust_buttons.count("ThrustDown")) thrust_buttons.at("ThrustDown").setEnabled(stable);
}

int main() {
    const std::string WINDOW_NAME = "Virtual Engine Monitor (EICAS)";

    // EasyX 初始化
    initializeUI(WINDOW_NAME, &engine, &startButtonPressed, &stopButtonPressed, &thrust_buttons);

    // 初始化 indicators 和 thrust_buttons
    initializeIndicators(indicators);
    initializeButtons(thrust_buttons);

    cmdThread = std::thread(commandLoop, std::ref(cmdThreadRunning), std::ref(engine));

    std::vector<Gauge> gauges;
    gauges.emplace_back(POINT{ 180, 120 }, 80, "N1_L", N1_MAX_RATED);
    gauges.emplace_back(POINT{ 390, 120 }, 80, "N1_R", N1_MAX_RATED);
    gauges.emplace_back(POINT{ 180, 300 }, 80, "EGT_L", EGT_MAX * 0.8);
    gauges.emplace_back(POINT{ 390, 300 }, 80, "EGT_R", EGT_MAX * 0.8);

    double lastWall = getCurrenTimeSeconds();
    double accum = 0.0;
    const double STEP = 0.005;

    // 开启双缓冲绘图
    BeginBatchDraw();

    while (cmdThreadRunning) {
        double now = getCurrenTimeSeconds();
        double frameDt = now - lastWall;
        if (frameDt < 0) frameDt = 0;
        if (frameDt > 0.05) frameDt = 0.05;
        lastWall = now;
        accum += frameDt;

        while (accum >= STEP) {
            engine.advance(STEP);
            alertInfo.update();
            logging(engine, data_log_file, alert_log_file, isLogging, alertInfo);
            accum -= STEP;
        }

        if (startButtonPressed) {
            engine.start();
            startButtonPressed = false;
        }
        if (stopButtonPressed) {
            engine.stop();
            stopButtonPressed = false;
        }

        updateIndicators(engine, indicators);

        // EasyX 绘图
        drawUI(gauges, indicators, thrust_buttons, engine, alertInfo);

        // 处理鼠标消息
        ExMessage msg;
        while (peekmessage(&msg, EM_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                handleMouseClick(msg.x, msg.y, &engine, &startButtonPressed, &stopButtonPressed, &thrust_buttons);
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            cmdThreadRunning = false;
        }

        Sleep(1); 
    }

    EndBatchDraw();
    closegraph(); // 关闭窗口

    cmdThreadRunning = false;
    if (cmdThread.joinable()) {
        cmdThread.detach();  // 让线程自行终止，不等待
    }

    if (isLogging) {
        if (data_log_file.is_open()) data_log_file.close();
        if (alert_log_file.is_open()) alert_log_file.close();
    }

    return 0;
}
