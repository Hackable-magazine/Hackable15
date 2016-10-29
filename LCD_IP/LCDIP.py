#!/usr/bin/python
# -*- coding: utf-8 -*-

import RPi.GPIO as GPIO
import netifaces
from time import sleep

class HD44780:

    def __init__(self, pin_rs=40, pin_e=38, pins_db=[37, 35, 33, 31]):

        self.pin_rs = pin_rs
        self.pin_e = pin_e
        self.pins_db = pins_db

        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(self.pin_e, GPIO.OUT)
        GPIO.setup(self.pin_rs, GPIO.OUT)
        for pin in self.pins_db:
            GPIO.setup(pin, GPIO.OUT)

        self.clear()

    def clear(self):
        self.cmd(0x33) # init 8 bits
	self.cmd(0x32) # init 8 bits confirmation
	self.cmd(0x28) # 4 bits - 2 lignes
	self.cmd(0x0C) # Pas de curseur
	self.cmd(0x06) # Incrémentation curseur
	self.cmd(0x01) # Efface écran
	
    def cmd(self, bits, char_mode=False):
        sleep(0.005)
        bits=bin(bits)[2:].zfill(8)

        GPIO.output(self.pin_rs, char_mode)

        for pin in self.pins_db:
            GPIO.output(pin, False)

        for i in range(4):
            if bits[i] == "1":
                GPIO.output(self.pins_db[::-1][i], True)

        GPIO.output(self.pin_e, True)
        GPIO.output(self.pin_e, False)

        for pin in self.pins_db:
            GPIO.output(pin, False)

        for i in range(4,8):
            if bits[i] == "1":
                GPIO.output(self.pins_db[::-1][i-4], True)

        GPIO.output(self.pin_e, True)
        GPIO.output(self.pin_e, False)

    def message(self, text):
        for char in text:
            if char == '\n':
                self.cmd(0xC0) # 0x80 + 0x40 adresse 2eme ligne
            else:
                self.cmd(ord(char),True)

    def __del__(self):
        GPIO.cleanup()

if __name__ == '__main__':
    lcd = HD44780()
    lcd.clear()
    lcd.message("Mon IP:\n%s" %  netifaces.ifaddresses('eth0')[netifaces.AF_INET][0]['addr'])
