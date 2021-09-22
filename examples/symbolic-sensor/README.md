# symbolic-sensor

Example application introducing symbolic values through a peripheral.

## Usage

*Refer to the `assertion-failure` example application.*

## Peripheral

This application uses the exemplary symbolic sensor peripheral defined
in `vp/src/platform/symex/symex_sensor.h`. This peripheral provides
three 32 Bit registers: Two control registers which allow defining an
upper/lower value bound and a third registers which returns a random
value in this range. This third register utilizes the provided TLM 2.0
extension for symbolic values to return a symbolic register value. The
executed software is then explored based on this value.

The error illustrated in this code is that the application assumes that
the configured lower bound is exclusive though it is not and thereby
allows `symex-vp` to discover an assertion failure in this application.
