#ifndef USB__LOGIC_H
#define USB__LOGIC_H

/**
 * Algorithm
 * 
 * Remember the 10 last completion timings
 * Rolling avg over them after modulo 1us-ing them
 * Release at that point minus 50us (for now)
 * Clear report buffer if not fired yet
 * Build USB report
 * Send new report
 */

/* Pass the us cycles number for time manipulation purposes, should be called before any other function */
void initUsbLogic(uint32_t us);

/* Read inputs, translate to GC state, translate to USB report */
uint8_t* build_usb_report(void);

/* Informs an in transfer just completed for timing recording purposes */
void inform_in_transfer_completed(void);

/* Blocks execution until n us before the next in transfer should happen */
void wait_until_n_us_before_in_transfer(uint32_t n);

#endif