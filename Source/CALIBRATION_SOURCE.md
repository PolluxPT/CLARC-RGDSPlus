# CLARC 0.1.4 input correction

The user's saved physical calibration (`Controls.txt`, 2 November 2026) identifies
GUID `1900b655010000000100000000010000`, 17 SDL buttons, 4 axes and 1 hat:

| Physical button | Calibrated SDL button | Linux evdev code on the captured device |
|---|---|---|
| A | 0 | 304 |
| B | 1 | 305 |
| X | 3 | 307 |
| Y | 2 | 306 |
| L1 | 4 | 308 |
| R1 | 5 | 309 |
| SELECT | 6 | 310 |
| START | 7 | 311 |
| L2 | 10 | 314 |
| R2 | 11 | 315 |

Sources: owner-provided physical calibration log and RGDS+ device capability report.

The original event5 key bitmap is:
`400000000 1fff000000000000 0 0 c000000000000 2`.

SDL2 Linux enumerates supported EV_KEY codes starting at BTN_JOYSTICK (288),
then appends supported lower key codes. The daemon now uses that enumeration
and the saved physical calibration instead of assigning conventional gamepad
key names to the physical buttons. It reads live EVIOCGBIT capabilities on
startup, with the captured device capabilities as a fallback.

Reference implementation:
https://github.com/libsdl-org/SDL/blob/SDL2/src/joystick/linux/SDL_sysjoystick.c
(`ConfigJoystick`, BTN_JOYSTICK button loops).

The earlier incorrect assumption assigned evdev310/311 to shoulders, although
those are the user's SELECT/START. It assigned evdev315 to START, although that
is R2. The corrected exit chord uses evdev310+311, not 314+315.

The managed controller layer now resolves face buttons and START/SELECT from
physical mailbox state as well, bypassing Unity's generic controller profile.
L1/R1 are reserved for RX=-1/+1. L2/R2 are unassigned. Left-stick values remain
native; D-pad uses the same left-stick movement path as in 0.1.3.

Local fixtures test the actual Python mapping and the real compiled .NET2
helper, including packets exchanged between them. These are host checks;
physical hardware validation is still pending.
