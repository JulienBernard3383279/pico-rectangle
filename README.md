# Frame1/B0XX layout style open-source digital controller software for the Raspberry Pi Pico

This software implements the Joybus protocol and is meant to be uploaded onto a Raspberry Pi Pico (RP2040, ARM Cortex M0+) to allow it to act as a Gamecube Controller based on GPIO inputs.
The translation from GPIO input to Gamecube controller state mimicks that of the "Frame1" and "B0XX" controllers.

### Current state:
- Full logic bar SDI/Pivot nerfs (i.e functionally equivalent to Frame 1)
- Parasol dashing / slight side B nerfs togglable in the code (releases with/without)
- Compatible with console / PC through adapter (no dedicated PC mode yet)

### Comparison to atmega32u4 based controllers:
- Up to 25 inputs + a console data line
- No logic level shifter required (native 3.3V board)
- Extremely simple programming
- 133MHz max (on paper) dual core MCU (clocked at 125MHz as of this commit) allows reacting to console poll rather than having to predict them, hence better latency (i.e matches Frame1 latency give or take a few cycles which is slightly better than that of B0XX/other atmega32u4 based controllers)
- MCU speed (8+ times that of the atmega32u4) means it will have 1000Hz effective reporting over USB when I do the PC mode (instead of 125Hz/500Hz) and opens the door for further improvements.
- Very cheap ($4)

### Safety information
Don't have this board plugged via USB and via its Gamecube port at the same time.
This may damage the console/adapter (feeding 5V to 3.3V). If you want to prevent this electrically, use a Schottky diode.

### How to program your board:
- Download the latest release (on the right of the Github page)
- Plug in your Raspberry Pico to your computer via USB while holding the "BOOTSEL" white button on the board.
- The board should appear as an external drive. Put the .uf2 of your choice in there. The board should disconnect and be ready for use.

### Binary notes:

There are multiple .uf2 files in a release (currently 2). Pick one according to the nerfs you want. You currently get to pick between 2 options: with or without the Slight Side B nerf and Peach's Parasol Dashing nerf.

Removing either of these doesn't make your controller illegal compliant as per the SWT ruleset. Don't use them if you don't want them. Use them if you want true functional equivalence to the Frame1/B0XX layouts.

### How to wire the board:
- Start to GP0 (pin 0)
- Right to GP2 (pin 4)
- Down to GP3 (pin 5)
- Left to GP4 (pin 6)
- L to GP5 (pin 7)
- MX to GP6 (pin 9)
- MY to GP7 (pin 10)
- CStick Up to GP12 (pin 16)
- CStick Left to GP13 (pin 17)
- A to GP14 (pin 19)
- CStick Down to GP15 (pin 20)
- CStick Right to GP16 (pin 21)
- Up to GP17 (pin 22)
- MS to GP18 (pin 24)
- Z to GP19 (pin 25)
- LS to GP20 (pin 26)
- X to GP21 (pin 27)
- Y to GP22 (pin 29)
- B to GP26 (pin 31)
- R to GP27 (pin 32)
- Ground to pins 33 and 38
- Gamecube data line to GP 28 (pin 34)
- 3.3V to VSYS (pin 39)
- Don't connect your 5V input

### Troubleshooting

B/R don't work -> Did you connect pin 33 to ground ?
Everything but B/R doesn't work -> Did you connect pin 38 to ground ?

### Contact

Discord: Arte#2458
Twitter: https://twitter.com/SSBM_Arte

This didn't undergo much testing yet. Please let me know if anything is off.
