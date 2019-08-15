#!/usr/bin/python

import io
import sys
import RPi.GPIO as GPIO
import time
import math
import random
from datetime import datetime
import signal
import os
from signal import SIGTERM
import subprocess
import re
from threading import Thread
from threading import Lock
import threading
import contextlib


def GetTimeMs():
    	d = datetime.now()
    	return d.minute * 60000 + d.second * 1000 + d.microsecond / 1000.0


def RunProcess(exe):
    p = subprocess.Popen(exe, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    while(True):
        # returns None while subprocess is running
        retcode = p.poll()
        line = p.stdout.readline()
        yield line
        if retcode is not None:
            break


newlines = ['\n', '\r\n', '\r']
def Unbuffered(proc, stream='stdout'):
    stream = getattr(proc, stream)
    with contextlib.closing(stream):
        while True:
            out = []
            last = stream.read(1)
            # Don't loop forever
            if last == '' and proc.poll() is not None:
                break
            while last not in newlines:
                # Don't loop forever
                if last == '' and proc.poll() is not None:
                    break
                out.append(last)
                last = stream.read(1)
            out = ''.join(out)
            yield out


class LightsLine(object):
	def __init__(self):
		print('Init LightsLine.')
                self.led_pins = [5, 6, 13, 19, 26, 12, 16, 20, 21]
                GPIO.setup(self.led_pins[0], GPIO.OUT)
                GPIO.setup(self.led_pins[1], GPIO.OUT)
                GPIO.setup(self.led_pins[2], GPIO.OUT)
                GPIO.setup(self.led_pins[3], GPIO.OUT)
                GPIO.setup(self.led_pins[4], GPIO.OUT)
                GPIO.setup(self.led_pins[5], GPIO.OUT)
                GPIO.setup(self.led_pins[6], GPIO.OUT)
                GPIO.setup(self.led_pins[7], GPIO.OUT)
                GPIO.setup(self.led_pins[8], GPIO.OUT)
                self.clear()

	def __del__(self):
		self.clear()
		print('Del LightsLine')

	def getPins(self):
		return self.led_pins

	def setAllTo(self, val):
                GPIO.output(self.led_pins[0], val)
                GPIO.output(self.led_pins[1], val)
                GPIO.output(self.led_pins[2], val)
                GPIO.output(self.led_pins[3], val)
                GPIO.output(self.led_pins[4], val)
                GPIO.output(self.led_pins[5], val)
                GPIO.output(self.led_pins[6], val)
                GPIO.output(self.led_pins[7], val)
                GPIO.output(self.led_pins[8], val)

	def clear(self):
		self.setAllTo(GPIO.LOW)


class ProgressViewer(object):
	def __init__(self, lights_line, max_val = 100):
		self.max_val = max_val
		self.is_light = False
		self.lights_line = lights_line
		self.clear()

	def __del__(self):
		self.lights_line.clear()

	def clear(self):
		self.time_mls = GetTimeMs()
		self.step = 0
		self.lights_line.clear()

	def stepTo(self):
		self.is_light = not(self.is_light)
		light = GPIO.LOW
		if self.is_light:
			light = GPIO.HIGH
		if self.step == 9:
			self.lights_line.setAllTo(light)
		else:
			led_pins = self.lights_line.getPins()
			GPIO.output(led_pins[self.step], light)

	def setVal(self, val):
		if val <= self.max_val:
			i = int(math.floor(9.0 * (float(val) / float(self.max_val)) + 0.05))
			#print('# {} {}'.format(i, val))
			led_pins = self.lights_line.getPins()
			while self.step < i:
				GPIO.output(led_pins[self.step], GPIO.HIGH)
				self.step += 1

	def update(self):
		smls = GetTimeMs() - self.time_mls
#		print('update [{}]: {}'.format(smls, self.time_mls))
		if 200 < smls:
			self.time_mls = GetTimeMs()
			self.stepTo()

class Waiting(object):
	def __init__(self, lights_line, timeout = 100):
		print('Init Waiting.')
		self.timeout = timeout
		self.mls = GetTimeMs()
		self.pos = 0;
                self.is_forward = True
		self.lights_line = lights_line
                self.lights_line.clear()

	def __del__(self):
		self.lights_line.clear()
		print('Del Waiting.')

        def update(self):
		mls = GetTimeMs()
		if self.timeout < mls - self.mls:
			self.mls = mls
			led_pins = self.lights_line.getPins()
			GPIO.output(led_pins[self.pos], GPIO.LOW)
			if self.is_forward:
				if self.pos < 8:
					self.pos += 1
				else:
					self.is_forward = False
					self.pos -= 1
			else:
				if self.pos > 0:
					self.pos -= 1
				else:
					self.is_forward = True
					self.pos += 1
			GPIO.output(led_pins[self.pos], GPIO.HIGH)

	def clear(self):
		self.lights_line.clear()


class Error(object):
	def __init__(self, lights_line, timeout = 5):
		print('Init Error')
		self.timeout = timeout
		self.mls = GetTimeMs()
		self.lights_line = lights_line
		self.lights_line.clear()
		self.old_pos = 0

	def __del__(self):
		self.lights_line.clear()
		print('Del Error')

	def update(self):
		mls = GetTimeMs()
		if self.timeout < mls - self.mls:
			self.mls = mls
			led_pins = self.lights_line.getPins()
			GPIO.output(led_pins[self.old_pos], GPIO.LOW)
			self.old_pos = random.randint(0, 8)
			GPIO.output(led_pins[self.old_pos], GPIO.HIGH)
			#GPIO.output(led_pins[random.randint(0, 8)], GPIO.LOW)
			#GPIO.output(led_pins[random.randint(0, 8)], GPIO.HIGH)

	def clear(self):
		self.lights_line.clear()


def PushCallback():
	print('PUSH BT')

class Flasher(object):
	def __init__(self, lights_line, path_to_parts, path_to_bin):
		self.path_to_parts = path_to_parts
		self.path_to_bin = path_to_bin
		self.mutex = Lock()
		self.state = 'BUTTON'
		self.lights_line = lights_line
		self.waiting = Waiting(self.lights_line)
		self.wait_timeout = 3000
		self.progress = ProgressViewer(self.lights_line)
		self.error = Error(self.lights_line)
		self.err_timeout = 3000
		self.mls = GetTimeMs()
		self.bt_pin = 24
		GPIO.setup(self.bt_pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
		#GPIO.add_event_callback(self.bt_pin, PushCallback)

	def __del__(self):
		pass

	def findDevice(self):
		print('Finding...')
		self.lights_line.clear()
		result = False
		self.tty_usb = ''
		p1 = subprocess.Popen('dmesg', stdout=subprocess.PIPE)
		proc = subprocess.Popen('grep ttyUSB'.split(), stdin=p1.stdout, stdout=subprocess.PIPE, universal_newlines = True)
                for line in Unbuffered(proc):
			line = line.replace('\n', '').replace('\r', '')
			time.sleep(0.001)
			reg_res = re.search(r'.+(cp210x).+(attached) +(to) +(ttyUSB\d)', line)
			if reg_res is not None:
				self.tty_usb = reg_res.group(4)
			#else:
				#print('# {}'.format(line))
		for line in RunProcess('esptool.py --port /dev/{} flash_id'.format(self.tty_usb).split()):
			time.sleep(0.001)
			reg_res = re.search(r'(Hard) +(resetting) +(via) +(RTS) +(pin).{3}', line)
			if reg_res is not None:
				print('Reset')
				result = True
			self.waiting.update()
		self.mutex.acquire()
		if result:
			self.state = 'FOUND'
			print('Found: {}'.format(self.tty_usb))
		else:
			self.state = 'ERROR'
		self.mls = GetTimeMs()
		self.mutex.release()
		print('Complete. State is {}'.format(self.state))
		self.lights_line.clear()

	def flashDevice(self):
		print('Flashing...')
		self.lights_line.clear()
                result = False
		self.mutex.acquire()
                self.progress.setVal(1)
                self.mutex.release()
		print('Erase')
                for line in RunProcess('esptool.py --port /dev/{} erase_flash'.format(self.tty_usb).split()):
                        time.sleep(0.001)
			reg_res = re.search(r'(Hard) +(resetting) +(via) +(RTS) +(pin).{3}', line)
                        if reg_res is not None:
                                print('Reset.')
                                result = True
		if result:
			result = False
			print('Try flash: {} {}'.format(self.tty_usb, self.path_to_bin))
                        exe = 'esptool.py --port /dev/{} write_flash 0x8000 {} 0x10000 {}'.format(self.tty_usb, self.path_to_parts, self.path_to_bin).split()
                        proc = subprocess.Popen(exe, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, universal_newlines = True)
			for line in Unbuffered(proc):
			        line = line.replace('\r', '\n')
	                       	time.sleep(0.001)
                        	reg_res = re.search(r'(Writing).+(at).+\.{3}.+\( *(\d{1,3}) *% *\)', line)
                        	if reg_res is not None:
					pval = int(reg_res.group(3))
					print('Progress: {} %'.format(pval))
					self.mutex.acquire()
					self.progress.setVal(pval)
		                        self.mutex.release()
				reg_res = re.search(r'(Hard) +(resetting) +(via) +(RTS) +(pin).{3}', line)
	                        if reg_res is not None:
        	                        print('Reset.')
					self.mutex.acquire()
                                        self.progress.setVal(100)
					self.state = 'FLASHING'
					self.mls = GetTimeMs()
                                        self.mutex.release()
					result = True
		if not result:
			self.mutex.acquire()
                        self.state = 'ERROR'
			self.mls = GetTimeMs()
                        self.mutex.release()
		self.tty_usb = 'ttyUSB0'

	def update(self):
		mls = GetTimeMs()
		sub_mls = 0
		state = ''
		self.mutex.acquire()
		sub_mls = mls - self.mls;
		state = self.state
		self.mutex.release()
		if state == 'BUTTON':
                        if 10 < sub_mls:
                                self.mls = mls
                                if GPIO.input(self.bt_pin) == GPIO.HIGH:
                                        print('PUSH BT')
					self.state = 'FIND'
					self.find_th = Thread(target = self.findDevice)
					self.find_th.start()
		elif state == 'FIND':
			self.mutex.acquire()
			self.waiting.update()
			self.mutex.release()
		elif state == 'FOUND':
			self.state = 'FLASH'
			self.flash_th = Thread(target = self.flashDevice)
                        self.flash_th.start()
		elif state == 'FLASH':
			self.mutex.acquire()
                        self.progress.update()
                        self.mutex.release()
		elif state == 'FLASHING':
			self.progress.update()
			if 1000 < sub_mls:
                                self.mutex.acquire()
                                self.mls = mls
                                self.state = 'WAITING'

		elif state == 'WAITING':
			self.waiting.update()
			if self.wait_timeout < sub_mls:
				self.mutex.acquire()
				self.mls = mls
				self.state = 'BUTTON'
				self.lights_line.clear()
				self.mutex.release()
		elif state == 'ERROR':
			self.error.update()
			if self.err_timeout < sub_mls:
				self.mutex.acquire()
				self.mls = mls
				self.state = 'BUTTON'
				self.lights_line.clear()
				self.mutex.release()
				print('ERR stop')

#########################################################################################################


is_loop = True


class Application(object):
	def __init__(self):
	        GPIO.setwarnings(False)
        	GPIO.setmode(GPIO.BCM)
		self.lights_line = LightsLine()

	def clear(self):
		print('Glogals clear.')
		global is_loop
		is_loop = False
		time.sleep(1)
		del self.lights_line

	def __del__(self):
	        GPIO.cleanup()

	def execute(self):
		path_to_parts = '{}/{}'.format(os.getcwd(), 'data/partitions.elf')
        	path_to_bin = '{}/{}'.format(os.getcwd(), 'data/firmware.elf')
	        f = Flasher(lights_line, path_to_parts, path_to_bin)
        	print('Start Application')
	        while True:
        	        f.update()
                	time.sleep(0.001)
	                global is_loop
        	        if not is_loop:
                	        del f
                        	print('Stop Application.')
                        	break


def ProgressTest(lights_line):
        mv = 2555
        pv = ProgressViewer(lights_line, mv)
        v = 0
        while v < mv:
                #print('sleep {}'.format(v))
                v += random.randint(0, 32)
                pv.setVal(v)
                pv.update()
                time.sleep(random.uniform(0.0, 0.2))
        print('is complette.')
        pv.setVal(mv)
        v = 50
        while 0 < v:
                pv.update()
                v -= 1
                time.sleep(random.uniform(0.0, 0.1))
        del pv


def WaitingTest(lights_line):
	w = Waiting(lights_line)
	print('Start loop')
	while True:
		w.update()
		time.sleep(0.001)
		global is_loop
		if not is_loop:
			del w
			print('Stop loop.')
			break


def ErrorTest(lights_line):
	err = Error(lights_line)
	print('Start error period')
	mls = GetTimeMs()
	while GetTimeMs() < mls + 3000:
		err.update()
	del err
	print('Stop error loop')


def PushButtonTest(lights_line):
	path_to_parts = '{}/{}'.format(os.getcwd(), 'data/partitions.elf')
	path_to_bin = '{}/{}'.format(os.getcwd(), 'data/firmware.elf')
	f = Flasher(lights_line, path_to_parts, path_to_bin)
	print('Start BT test')
        while True:
                f.update()
                time.sleep(0.001)
                global is_loop
                if not is_loop:
                        del f
                        print('Stop BT test.')
                        break

#########################################################################################################


app = Application()



def SigHandler(signum, frame):
	print('SIGTERM: GPIO cleanup!')
	global app
	app.clear()


def main(argv = sys.argv):
        signal.signal(signal.SIGTERM, SigHandler)
	signal.signal(signal.SIGINT, SigHandler)
	global app
	#ProgressTest(app.lights_line)
	#ErrorTest(app.lights_line)
	#WaitingTest(app.lights_line)
	#PushButtonTest(app.lights_line)
	#app.clear()
	app.execute()
	del app


if __name__ == '__main__':
	sys.exit(main())
