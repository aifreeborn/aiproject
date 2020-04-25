#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time
try:
    import RPi.GPIO as GPIO
except RuntimeError:
    print "Error importing RPi.GPIO!"

class aibeef(object):
    """
    Getting dunidity and temperature by one-wire. 
    Voltage: 3.3V
    """
    ONE_WIRE_DATA = 37

    def __init__(self):
        # Set up the GPIO channels
        GPIO.setmode(GPIO.BOARD)
        GPIO.setwarnings(False)
        GPIO.setup(self.ONE_WIRE_DATA, GPIO.OUT, initial = GPIO.HIGH)

    def open(self):
        GPIO.output(self.ONE_WIRE_DATA, GPIO.LOW)

    def close(self):
        GPIO.output(self.ONE_WIRE_DATA, GPIO.HIGH)
    
    def stop(self):
        GPIO.cleanup()

if __name__ == '__main__':
    beef = aibeef()
    while 1:
        beef.open()
        time.sleep(0.001)
        beef.close()
        time.sleep(0.001)
    beef.stop()
