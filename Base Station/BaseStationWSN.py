import serial
import time
from xbee import ZigBee

PORT = "/dev/tty.usbserial-A90316CX"
BAUD_RATE = 9600
serialPort = serial.Serial(PORT, BAUD_RATE)

# Create API object
xbee = ZigBee(serialPort)

# Bypass Mode of XBEE module
xbee.send('tx', dest_addr_long=b'\x00\x00\x00\x00\x00\x00\xFF\xFF', dest_addr=b'\xFF\xFF', data=b'B')

# Continuously read packets
while True:
    try:
        response = xbee.wait_read_frame()
        dat = response["rf_data"]
        dato = []
        for n in dat:
            dato.append(hex(ord(n)))

        node = int(dato[0],16)
        intTemp = int(dato[1],16)
        decTemp = int(dato[2],16)
        intCH4 = int(dato[3],16)
        decCH4 = int(dato[4],16)
        oxygen = int(dato[5],16)
        carbonMon1 = int(dato[6], 16)
        carbonMon2 = int(dato[7], 16)

        methane = intCH4+decCH4/100.0
        temperature = intTemp + decTemp / 100.0
        carbonMon = carbonMon2 * 100 + carbonMon1

        print "Hora " + time.strftime("%X")
        print("Node %i - Temperature: %.2f C - Methane: %.2f - Oxygen: %i - Carbon Monoxide: %i ppm" %(node, temperature, methane, oxygen, carbonMon))
    except KeyboardInterrupt:
        serial.close()
        break