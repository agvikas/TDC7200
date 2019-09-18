# TDC7200
Arduino code to configure and read the registers in TDC7200 to evaluate time-of-flight (ToF). The datasheet of the IC for reference can be found [here](http://www.ti.com/lit/ds/symlink/tdc7200.pdf)

## What this code does?

This code was developed for arduino to work with Texas Instrument's TDC7200 evaluation module. This code can be used to:

- Configure the registers
- Read register and evaluate ToF
- Generate clock using arduino if not using an external clock
- Generate START and STOP signals (for testing)

## How to run the code?

The IC has two measurement modes. Measurement mode 1 is recommended for measuring shorter time durations of <500 ns. Measurement mode 2 is recommended for >500 ns durations and provides better accuracy.
The ToF calculations differ for both modes and are implemented accordingly. Comment/Uncomment lines pertaining to two modes as required in the code.
All communication with the IC is over SPI. Pin numbers used in the codea are with reference to arduino UNO. Uncomment the lines generating
START and STOP signals from arduino if not required and connect the appropriate pins on IC with external signals.
