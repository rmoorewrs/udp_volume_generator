# udp_volume_generator
Arduino-based project that listens to UDP messages and sets a corresponding volume to drive an LED VU meter. The intended usecase is to implement a visual heartbeat indicator to show whether a system or process that continually updates the display, has stopped.

The hardware required is:

- Arduino Uno or equivalent
- Arduino Ethernet Shield (Wiznet w5100, but w5200 or others can be supported with the right library)
- Some kind of DAC shield. Code here assumes a self-made 12-bit DAC using an R-2R ladder on a prototype shield. For a different DAC, change the dac_write() function.
- LED VU meter such as the American Audio db-Display (http://www.adj.com/db-display)

Brief demo video here: https://www.youtube.com/watch?v=v0rQAT5kwGw
