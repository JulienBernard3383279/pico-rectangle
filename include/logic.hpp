#include "pico/stdlib.h"

#include "comms.hpp"
#include "inputs.hpp"

#ifndef LOGIC_H
#define LOGIC_H

enum class ParasolDashing { BAN, DONT_BAN };
enum class SlightSideB { BAN, DONT_BAN };

/* Pick whether to apply optional B0XX/F1 bans */
void initLogic(ParasolDashing banParasolDashing, SlightSideB banSlightSideB);

/* Makes a B0XX/F1 report from the provided RectangleInput */
GCReport makeReport(const RectangleInput &ri);

#endif