#include "pico/stdlib.h"

#include "comms.hpp"
#include "inputs.hpp"

#ifndef LOGIC_H
#define LOGIC_H

/* Makes a B0XX/F1 report from the provided RectangleInput */
GCReport makeReport(const RectangleInput &ri);

#endif