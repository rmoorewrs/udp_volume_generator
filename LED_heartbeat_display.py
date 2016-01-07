import socket
import time

# created 1/17/16 -- rmoore
#
# Send a sequence of values to a custom network interface box that drives an LED VU meter up and down the scale.
#
# The amplitude value sent to the box changes the amplitude of a 100Hz square wave which in turn sets volume unit display.
# The box expects a 4-byte ascii hex representation of an integer between 0x0 and 0x0FFF (12-bit DAC).
#
#	"0000" = off
#	"0FFF" = max volume
#
# Since the VU meter is meant for audio monitoring, it uses log scale and averaging (to simulate the inertia
# of a traditional VU meter needle) so there's not an easy way to get a specific LED position due to the dynamics.
#
# Note that the box replies to the sender with the message "ack", in case you want to check that the box is receiving
# packets correctly


# The following sequence must be tuned to get linear looking response from the LED display
amplitude_sequence=[
0x00,
0x020,
0x060,
0x0b0,
0x0d0,
0x0100,
0x0120,
0x0130,
0x0140,
0x01a0,
0x0200,
0x0300,
0x0400,
0x0600,
0x0a00,
0x0fff,
0x0fff,
0x0a00,
0x0600,
0x0200,
0x0100,
0x08f,
0x07f,
0x00
]

UDP_IP = "192.168.11.21"
UDP_PORT = 8888

print "UDP target IP:", UDP_IP
print "UDP target port:", UDP_PORT

# loop forever sending out the amplitude sequence
while 1:
    for i in (amplitude_sequence):
        msg = "{0:0{1}x}".format(i,4)
        sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
        sock.sendto(msg, (UDP_IP, UDP_PORT))
        print msg
        time.sleep(0.1)
