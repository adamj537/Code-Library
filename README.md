# Code-Library
Low-level drivers, utilities, examples, and project outlines for microcontrollers.  The goal is that applications written using these files should work across multiple platforms.

Organization is by folder:
* External Peripherals - code to control hardware other than a microcontroller (i.e. external chips).
* Processor Peripherals - drivers for microcontroller peripherals.
* Projects - examples using the library, used for demo and testing, as well as generic top-level files for new projects.
* Utilities - files to calculate, store, or otherwise manipulate data.

TODO:  This code should be guaranteed to work at a minimum on these devices:
At this point, almost nothing is tested, so this is more like a wish list.
* PICkit 3 Debug Express (PIC18F45K20)
* Arduino Uno (ATmega328)
* Arduino Leonardo (ATmega32u4)
* TI EZ430-Chronos (CC430F6137)
* Arduino Zero (ATSAMD21G18)
* Coming late 2017:  Kinetis & LPC