#include "usb/logic.hpp"

#include "logic.hpp"

static AdapterReport adapterReport = defaultAdapterReport;

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
