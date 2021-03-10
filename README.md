# Frame1/B0XX layout style open-source digital controller software for the Raspberry Pi Pico (v0.1)

This software implements the Joybus protocol and is meant to be uploaded onto a Raspberry Pi Pico (RP2040, ARM Cortex M0+) to allow it to act as a Gamecube Controller based on GPIO inputs.
The translation from GPIO input to Gamecube controller state mimicks that of the "Frame1" and "B0XX" controllers.

### Current state:

- Full logic bar SDI/Pivot nerfs (i.e functionally equivalent to Frame 1)
- Parasol dashing / slight side B nerfs togglable in the code (releases with/without)
- Compatible with consoles
- Compatible with PC directly from the board USB with 1000Hz reporting, identifies as a Gamecube controller to USB adapter

### LEGAL INFORMATION

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

In particular, when communicating over USB, device using this software will use the 0x057E USB Vendor ID, that is affiliated with Nintendo. By uploading this software onto your board, you assert that you understand what that means and take entire responsability for it.

### Safety information

Don't have this board plugged via USB and via its Gamecube port at the same time.
This may damage the console/adapter (feeding 5V to 3.3V). If you want to prevent this electrically, use a Schottky diode.

### Comparison to atmega32u4 based controllers:

- Up to 25 inputs + a console data line
- No 5V input required (Voltage regulator accepts 3.3V)
- No logic level shifter required (3.3V GPIO by default)
- Extremely simple programming (USB drag and drop)
- 133MHz max (on paper) dual core MCU (clocked at 125MHz as of this commit) allows reacting to console poll rather than having to predict them, hence better latency (i.e matches Frame1 latency give or take a few cycles which is slightly better than that of B0XX/other atmega32u4 based controllers)
- MCU speed (8+ times that of the atmega32u4) allows it to have 1000Hz effective reporting over USB, which some atmega32u4 based controllers (notably the B0XX) don't have, granting better latency & latency stability
- Very cheap ($4)

### PC Mode information

So far two modes are supported: Melee for console and Melee for PC. The mode is selected automatically (based on whether the board is powered through USB).

Upon connecting the Raspberry Pi Pico to your PC after uploading the software at hand onto it, the Raspberry Pi Pico will identify as a "WUP-028" i.e a Gamecube controller to USB adapter. Things will behave as if the Gamecube controller this board emulates was plugged in port 1. You need to use the "GameCube Adapter for Wii U" in the Slippi Controllers tab (which is the default). You don't need to configure anything.

However, in order to use this controller, you must first unplug any Gamecube controller to USB adapter connected to your PC, as softwares will typically only support communicating with one adapter at a time (notably Dolphin/Slippi).

Note that this mode of operation requires the "WinUSB" driver to be associated with this device. If you've used a Gamecube controller to USB adapter before, chances are you've associated WinUSB to the WUP-028 through Zadig. But even if you haven't, you shouldn't need to, as WinUSB will be installed automatically upon plugging the Pico board in.

Note that polling rate enforcements using the HIDUSBF filter driver that apply to "standard" WUP-028s will apply to this board, so enforced rates less than 1000Hz you could have configured would decrease this controller's performance.

Automated WinUSB installation is very experimental. If you encounter any driver related issue, please contact me.

### How to program your board:

- Download the latest release (on the right of the Github page)
- Plug in your Raspberry Pico to your computer via USB while holding the "BOOTSEL" white button on the board.
- The board should appear as an external drive. Put the .uf2 of your choice in there. The board should disconnect and be ready for use.

### Binary notes:

There are multiple .uf2 files in a release (currently 2). Pick one according to the nerfs you want. You currently get to pick between 2 options: with or without the Slight Side B nerf and Peach's Parasol Dashing nerf.

Removing either of these doesn't make your controller non SWT ruleset compliant. Don't use them if you don't want them. Use them if you want true functional equivalence to the Frame1/B0XX layouts.

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
