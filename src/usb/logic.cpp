#include "usb/logic.hpp"

#include "hardware/structs/systick.h"

#include "logic.hpp"

static AdapterReport adapterReport = defaultAdapterReport;

static uint32_t us_;
void initUsbLogic(uint32_t us) {
    us_ = us;
}

void updateAdapterReportFromGcReport(GCReport gcReport) {
    adapterReport.a = gcReport.a;
    adapterReport.b = gcReport.b;
    adapterReport.x = gcReport.x;
    adapterReport.y = gcReport.y;
    adapterReport.dLeft = gcReport.dLeft;
    adapterReport.dRight = gcReport.dRight;
    adapterReport.dUp = gcReport.dUp;
    adapterReport.dDown = gcReport.dDown;
    adapterReport.l = gcReport.l;
    adapterReport.r = gcReport.r;
    adapterReport.z = gcReport.z;
    adapterReport.start = gcReport.start;
    adapterReport.xStick = gcReport.xStick;
    adapterReport.yStick = gcReport.yStick;
    adapterReport.cxStick = gcReport.cxStick;
    adapterReport.cyStick = gcReport.cyStick;
    adapterReport.analogL = gcReport.analogL; //TODO It's not a 1:1 translation
    adapterReport.analogR = gcReport.analogR;
}

uint8_t* build_usb_report(void) {
    RectangleInput ri = getRectangleInput();
    GCReport gcReport = makeReport(ri);
    updateAdapterReportFromGcReport(gcReport);

    return (uint8_t*) &adapterReport;
}


// 2**24 / (125*1000) = ~134
// 134ms max full cycle
// 2**32 / (125*1000) = ~34360

static uint32_t lastCvr = 0;

void inform_in_transfer_completed(void) {
    uint32_t reading = systick_hw->cvr;
    lastCvr = reading;
}

// CVR MOVES BACKWARDS WHY DO I KEEP FORGETTING THIS ZDNIGIONUEFGOINUGFDINODFG
void wait_until_n_us_before_in_transfer(uint32_t n) {
    uint32_t target = (lastCvr - (1000-n)*us_) & 0x00FFFFFF;

    while ( (target - systick_hw->cvr) & 0x00800000); // while (current - target) < 0 i.e while current < target, hold
}