#!/usr/bin/env python
# _*_ coding: utf-8 _*_

import time
try:
    import RPi.GPIO as GPIO
except RuntimeError:
    print "Error importing RPi.GPIO!"

class dht11(object):
    """
    Getting dunidity and temperature by one-wire. 
    """
    ONE_WIRE_DATA = 12

    def __init__(self):
        self.data = []
        
        # Set up the GPIO channels
        GPIO.setmode(GPIO.BOARD)
        GPIO.setwarnings(False)

    def start(self):
        self.data = []
        GPIO.setup(self.ONE_WIRE_DATA, GPIO.OUT, initial = GPIO.HIGH)
        GPIO.output(self.ONE_WIRE_DATA, GPIO.LOW)
        time.sleep(0.018)
        GPIO.output(self.ONE_WIRE_DATA, GPIO.HIGH)
        GPIO.setup(self.ONE_WIRE_DATA, GPIO.IN, pull_up_down = GPIO.PUD_UP)

    def ack(self):
        while GPIO.input(self.ONE_WIRE_DATA) == GPIO.HIGH:
            continue
        while GPIO.input(self.ONE_WIRE_DATA) == GPIO.LOW:
            continue
        while GPIO.input(self.ONE_WIRE_DATA) == GPIO.HIGH:
            continue

    def read(self):
        while GPIO.input(self.ONE_WIRE_DATA) == GPIO.HIGH:
            continue
        
        i = 0
        while i < 40:
            j = 0
            k = 0
            while GPIO.input(self.ONE_WIRE_DATA) == GPIO.LOW:
                j += 1
                continue
            while GPIO.input(self.ONE_WIRE_DATA) == GPIO.HIGH:
                k += 1
                if k > 10000:
                    return -1
            if k < j:
                self.data.append(0)
            else:
                self.data.append(1)
            i += 1
        return 0
    
    def stop(self):
        GPIO.cleanup()

    def show(self):
        humidity_bit = self.data[0:8]
        humidity_point_bit = self.data[8:16]
        temperature_bit = self.data[16:24]
        temperature_point_bit = self.data[24:32]
        check_bit = self.data[32:40]
        
        humidity = 0
        humidity_point = 0
        temperature = 0
        temperature_point = 0
        check = 0
        
        for i in range(8):
            humidity += humidity_bit[i] * 2 ** (7 - i)
            humidity_point += humidity_point_bit[i] * 2 ** (7 - i)
            temperature += temperature_bit[i] * 2 ** (7 - i)
            temperature_point += temperature_point_bit[i] * 2 ** (7 - i)
            check += check_bit[i] * 2 ** (7 - i)
            
        tmp = humidity +  humidity_point + temperature + temperature_point
        if check == (0xff & tmp):
            print "temperature is " + str(temperature) + "." + str(temperature_point)
            print "wet is " + str(humidity) +  "." + str(humidity_point) + "%"
        else:
            return -1

        return 0


if __name__ == '__main__':
    read_flag = 1
    dht = dht11()
    while read_flag:
        dht.start()
        dht.ack()
        if dht.read() == 0:
            if dht.show() == 0:
                read_flag = 0
                continue
        time.sleep(2)

    time.sleep(0.050)
    dht.stop()
