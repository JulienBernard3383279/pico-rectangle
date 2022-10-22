#include "dac_algorithms/xbox_360.hpp"

namespace DACAlgorithms {
namespace Xbox360 {

// Back is inaccessible, idk whether that's a problem, is it *ever* mandatory in place of B ?

void actuateXbox360Report(GpioToButtonSets::F1::ButtonSet buttonSet) {
    bool left = buttonSet.left && !(buttonSet.ms);
    bool right = buttonSet.right && !(buttonSet.ms);
    bool up = buttonSet.up && !(buttonSet.ms);
    bool down = buttonSet.down && !(buttonSet.ms);

    bool dLeft = buttonSet.left && buttonSet.ms;
    bool dRight = buttonSet.right && buttonSet.ms;
    bool dUp = buttonSet.up && buttonSet.ms;
    bool dDown = buttonSet.down && buttonSet.ms;

    USBConfigurations::Xbox360::ControllerReport &xInputReport = USBConfigurations::Xbox360::xInputReport;
    xInputReport.reportId = 0;
    xInputReport.rightStickPress = buttonSet.my;
    xInputReport.leftStickPress = buttonSet.mx;
    xInputReport.back = 0;
    xInputReport.start = buttonSet.start;
    xInputReport.dRight = dRight;
    xInputReport.dLeft = dLeft;
    xInputReport.dDown = dDown;
    xInputReport.dUp = dUp;
    xInputReport.zl = buttonSet.ls;
    xInputReport.zr = buttonSet.z;
    xInputReport.home = 0;
    xInputReport.pad1 = 0;
    xInputReport.a = buttonSet.a;
    xInputReport.b = buttonSet.b;
    xInputReport.x = buttonSet.x;
    xInputReport.y = buttonSet.y;
	xInputReport.leftTrigger = buttonSet.l ? 255 : 0;
	xInputReport.rightTrigger = buttonSet.r ? 255 : 0;
	xInputReport.leftStickX = left && right ? 0 : left ? 0x8000 : right ? 0x7FFF : 0;
	xInputReport.leftStickY = down && up ? 0 : down ? 0x8000 : up ? 0x7FFF : 0;
	xInputReport.rightStickX = buttonSet.cLeft && buttonSet.cRight ? 0 : buttonSet.cLeft ? 0x8000 : buttonSet.cRight ? 0x7FFF : 0;
	xInputReport.rightStickY = buttonSet.cDown && buttonSet.cUp ? 0 : buttonSet.cDown ? 0x8000 : buttonSet.cRight ? buttonSet.cUp : 0;
};

}
}