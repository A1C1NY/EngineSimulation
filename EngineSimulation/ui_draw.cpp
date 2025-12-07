#include "ui_draw.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;


void drawGauges(const vector<Gauge>& gauges, const Engine& engine) {
    // gauges 顺序为 N1_L, N1_R, EGT_L, EGT_R
    if (gauges.size() >= 4) {
        // 阈值（原始值而非百分比）
        double n1_caution = 42000.0;  // ≈105% of rated
        double n1_warning = 48000.0;  // ≈120% of rated
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
    RECT start_rect = { 150, 380, 250, 410 };
    COLORREF start_color = (engine.getState() == EngineState::OFF) ? COLOR_GREEN: COLOR_GREY;
    setfillcolor(start_color);
    solidrectangle(start_rect.left, start_rect.top, start_rect.right, start_rect.bottom);

    settextcolor(COLOR_BLACK);
    settextstyle(18, 0, L"Arial"); 
    setbkmode(TRANSPARENT);
    outtextxy(start_rect.left + 20, start_rect.top + 5, L"START");

    RECT stop_rect = { 260, 380, 360, 410 };
    COLORREF stop_color = (engine.getState() == EngineState::STABLE || engine.getState() == EngineState::STARTING) ? COLOR_RED : COLOR_GREY;
    setfillcolor(stop_color);
    solidrectangle(stop_rect.left, stop_rect.top, stop_rect.right, stop_rect.bottom);

    settextcolor(COLOR_WHITE);
    settextstyle(18, 0, L"Arial"); 
    setbkmode(TRANSPARENT);
    outtextxy(stop_rect.left + 25, stop_rect.top + 5, L"STOP");

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
    outtextxy(400, 280, L"Fuel Flow:");

    RECT ff_rect = { 400, 300, 550, 330 };
    setlinecolor(COLOR_WHITE);
    rectangle(ff_rect.left, ff_rect.top, ff_rect.right, ff_rect.bottom);

    stringstream ff_ss;
    ff_ss << fixed << setprecision(1) << engine.getFuelFlow();
    string ff_str = ff_ss.str();
    wstring ff_wstr(ff_str.begin(), ff_str.end());

    settextcolor(COLOR_WHITE);
    settextstyle(18, 0, L"Arial"); 
    outtextxy(410, 305, ff_wstr.c_str());

    outtextxy(400, 340, L"Fuel Reserve:");

    RECT fuel_bar_rect = { 400, 360, 550, 390 };
    setlinecolor(COLOR_WHITE);
    rectangle(fuel_bar_rect.left, fuel_bar_rect.top, fuel_bar_rect.right, fuel_bar_rect.bottom);

    double fuel_reserve = engine.getFuelReserve();

    if (!isnan(fuel_reserve)) {
        double fuel_percentage = fuel_reserve / FUEL_CAPACITY;
        if (fuel_percentage < 0.0) fuel_percentage = 0.0;
        if (fuel_percentage > 1.0) fuel_percentage = 1.0;
        COLORREF fuel_color = COLOR_WHITE; // 正常值白色
        if (fuel_reserve < 1000.0 && fuel_reserve > 0.0) {
            fuel_color = COLOR_AMBER; // 警戒值琥珀色
        }
        else if (fuel_reserve <= 0.0) {
            fuel_color = COLOR_RED; // 警告值红色
        }

        // 绘制填充条
        setfillcolor(fuel_color);
        solidrectangle(400, 360, 400 + static_cast<int>(150 * fuel_percentage), 390);

        // 绘制数值
        stringstream fr_ss;
        fr_ss << fixed << setprecision(0) << fuel_reserve;
        string fr_str = fr_ss.str();
        wstring fr_wstr(fr_str.begin(), fr_str.end());

        settextcolor(COLOR_GREY); 
        settextstyle(18, 0, L"Arial"); 
        outtextxy(410, 365, fr_wstr.c_str());
    }
    else {
        // 无效值 (--) 红色
        settextcolor(COLOR_RED);
        settextstyle(18, 0, L"Arial"); 
        outtextxy(410, 365, L"--");
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

    settextcolor(COLOR_WHITE);
    settextstyle(18, 0, L"Arial");
    setbkmode(TRANSPARENT);
    string full_message = "STATUS: " + status_message;
    wstring full_message_wstr(full_message.begin(), full_message.end());
    outtextxy(20, 30, full_message_wstr.c_str());
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
