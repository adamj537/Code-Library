# CC3200-I2CSample
This is a sample project used for correspondence between TI support staff and
myself, as we attempt to develop a working interrupt-driven I2C driver for the
CC3200.

##Hardware
- CC3200 LaunchXL development PCB
- Custom I2C slave device (capable of returning 32 bytes of data)

##Status
Not done.  This code will work for awhile, but then hang after a few hours (one
or both of the I2C lines will be stuck low).