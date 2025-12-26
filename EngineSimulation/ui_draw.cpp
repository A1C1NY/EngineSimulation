#include "ui_draw.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;


void drawGauges(const vector<Gauge>& gauges, const Engine& engine) {
    // gauges 顺序为 N1_L, N1_R, EGT_L, EGT_R
    if (gauges.size() >= 4) {
        // 阈值（原始值而非百分比）
        double n1_caution = 42000.0;  // ≈105%
        double n1_warning = 48000.0;  // ≈120%
        double egt_caution = 950.0;
        double egt_warning = 1100.0;

        double n1_base = 0.0;
        double egt_base = AMBIENT_TEMP;

        // N1
        gauges[0].draw(engine.getN1Left(), n1_base, n1_caution, n1_warning);
        gauges[1].draw(engine.getN1Right(), n1_base, n1_caution, n1_warning);

        // EGT（使用环境温度为基线）
        gauges[2].draw(engine.getEgtLeft(), egt_base, egt_caution, egt_warning);
        gauges[3].draw(engine.getEgtRight(), egt_base, egt_caution, egt_warning);
    }
}

void drawButtons(const Engine& engine, const map<string, TriangleButton>& thrust_buttons) {
    RECT start_rect = { 820, 50, 950, 110 };
    COLORREF start_color = (engine.getState() == EngineState::OFF) ? COLOR_GREEN : COLOR_GREY;
    setfillcolor(start_color);
    solidrectangle(start_rect.left, start_rect.top, start_rect.right, start_rect.bottom);

    settextcolor(COLOR_BLACK);
    settextstyle(22, 0, L"Arial");
    setbkmode(TRANSPARENT);
    outtextxy(start_rect.left + 35, start_rect.top + 20, L"START");

    RECT stop_rect = { 820, 120, 950, 180 };
    COLORREF stop_color = (engine.getState() == EngineState::STABLE || engine.getState() == EngineState::STARTING) ? COLOR_RED : COLOR_GREY;
    setfillcolor(stop_color);
    solidrectangle(stop_rect.left, stop_rect.top, stop_rect.right, stop_rect.bottom);

    settextcolor(COLOR_WHITE);
    settextstyle(22, 0, L"Arial");
    setbkmode(TRANSPARENT);
    outtextxy(stop_rect.left + 37, stop_rect.top + 20, L"STOP");

    // 绘制推力按钮
    if (thrust_buttons.count("ThrustUp") && thrust_buttons.count("ThrustDown")) {
        thrust_buttons.at("ThrustUp").draw();
        thrust_buttons.at("ThrustDown").draw();
    }
}

void drawFuelInfo(const Engine& engine) {
    settextcolor(COLOR_WHITE);
    settextstyle(18, 0, L"Arial");
    setbkmode(TRANSPARENT);
    outtextxy(600, 50, L"Fuel Flow:");

    RECT ff_rect = { 600, 70, 730, 100 };
    setlinecolor(COLOR_WHITE);
    rectangle(ff_rect.left, ff_rect.top, ff_rect.right, ff_rect.bottom);

    double fuelFlow = engine.getFuelFlow();
    if (std::isnan(fuelFlow)) {
        // 值无效时用红色显示 "--"
        settextcolor(COLOR_RED);
        settextstyle(18, 0, L"Arial");
        setbkmode(TRANSPARENT);
        outtextxy(ff_rect.left + 10, ff_rect.top + 5, L"--");
    }
    else {
        settextcolor(COLOR_WHITE);
        stringstream ff_ss;
        ff_ss << fixed << setprecision(1) << fuelFlow;
        string ff_str = ff_ss.str();
        wstring ff_wstr(ff_str.begin(), ff_str.end());

		if (fuelFlow > 50) settextcolor(COLOR_AMBER);
        settextstyle(18, 0, L"Arial");
        setbkmode(TRANSPARENT);
        outtextxy(ff_rect.left + 10, ff_rect.top + 5, ff_wstr.c_str());
    }

	settextcolor(COLOR_WHITE);
    outtextxy(600, 120, L"Fuel Reserve:");

    RECT fuel_bar_rect = { 600, 140, 730, 170 };
    setlinecolor(COLOR_WHITE);
    rectangle(fuel_bar_rect.left, fuel_bar_rect.top, fuel_bar_rect.right, fuel_bar_rect.bottom);

    double fuel_reserve = engine.getFuelReserve();

    if (!isnan(fuel_reserve)) {
        double fuel_percentage = fuel_reserve / FUEL_CAPACITY;
        settextcolor(COLOR_GREY);
        if (fuel_percentage < 0.0) fuel_percentage = 0.0;
        if (fuel_percentage > 1.0) fuel_percentage = 1.0;
        COLORREF fuel_color = COLOR_WHITE; // 正常值白色
        if (fuel_reserve < 1000.0 && fuel_reserve > 0.0) {
            fuel_color = COLOR_AMBER; // 警戒值琥珀色
			settextcolor(COLOR_AMBER);
        }
        else if (fuel_reserve <= 0.0) {
            fuel_color = COLOR_RED; // 警告值红色
			settextcolor(COLOR_RED);
        }

        // 绘制填充条
        setfillcolor(fuel_color);
        int bar_width = fuel_bar_rect.right - fuel_bar_rect.left;
        solidrectangle(fuel_bar_rect.left, fuel_bar_rect.top, fuel_bar_rect.left + static_cast<int>(bar_width * fuel_percentage), fuel_bar_rect.bottom);

        // 绘制数值
        stringstream fr_ss;
        fr_ss << fixed << setprecision(0) << fuel_reserve;
        string fr_str = fr_ss.str();
        wstring fr_wstr(fr_str.begin(), fr_str.end());

        settextstyle(18, 0, L"Arial");
        outtextxy(fuel_bar_rect.left + 10, fuel_bar_rect.top + 5, fr_wstr.c_str());
    }
    else {
        // 无效值 (--) 红色
        settextcolor(COLOR_RED);
        settextstyle(18, 0, L"Arial");
        outtextxy(fuel_bar_rect.left + 10, fuel_bar_rect.top + 5, L"--");
    }
}

void drawAllIndicators(const map<string, Indicator>& indicators) {
    for (const auto& pair : indicators) {
        pair.second.draw();
    }
}

void drawStatusMessage(const Engine& engine) {
    string status_message;
    switch (engine.getState()) {
    case EngineState::OFF: status_message = "OFFLINE"; break;
    case EngineState::STARTING: status_message = "STARTING"; break;
    case EngineState::STABLE: status_message = "STABLE RUN"; break;
    case EngineState::STOPPING: status_message = "SHUTDOWN"; break;
    }

    // 定义与指示灯对齐的方框
    RECT status_box = { 300, 425, 565, 485 };
    setlinecolor(COLOR_WHITE);
    rectangle(status_box.left, status_box.top, status_box.right, status_box.bottom);

    settextcolor(COLOR_WHITE);
    settextstyle(30, 0, L"Consolas");
    setbkmode(TRANSPARENT);
    string full_message = "STATUS: " + status_message;
    wstring full_message_wstr(full_message.begin(), full_message.end());

    // 计算文本居中位置
    int text_width = textwidth(full_message_wstr.c_str());
    int text_height = textheight(full_message_wstr.c_str());
    int x_pos = status_box.left + (status_box.right - status_box.left - text_width) / 2;
    int y_pos = status_box.top + (status_box.bottom - status_box.top - text_height) / 2;

    outtextxy(x_pos, y_pos, full_message_wstr.c_str());
}

void drawUI(const vector<Gauge>& gauges, const map<string, Indicator>& indicators, const map<string, TriangleButton>& thrust_buttons, const Engine& engine, const AlertInfo& alertInfo) {
    // 1. 清屏
    cleardevice();

    // 2. 绘制所有元素
    drawGauges(gauges, engine);
    drawButtons(engine, thrust_buttons);
    drawFuelInfo(engine);
    drawAllIndicators(indicators);
    drawStatusMessage(engine);
    alertInfo.drawHistory();

    // 3. 刷新屏幕
    FlushBatchDraw();
}
