#!/usr/bin/env python3
import board
import neopixel
import time

# sudo adduser pi kmem
# pip3 install rpi_ws281x adafruit-circuitpython-adapixel

total_pixels = 144
pixels = neopixel.NeoPixel(board.D18, total_pixels, auto_write = False)

pixels[0] = (0, 0, 0)

def fillRatio(x, w, stride, ratio):
	for i in range(w):
		ratio2 = i / (w-1)
		if ratio2 <= ratio:
			pixels[x + stride * i] = (int(255*ratio2), int(255-255*ratio2), 0)
		else:
			pixels[x + stride * i] = (0, 0, 0)



w = total_pixels // 4
lastx = dict()
lasty = dict()
while True:
	f = open('/proc/stat', 'r')
	for line in f.readlines():
		line = line.split(' ')
		if line[0].startswith('cpu') and line[1] != '':
			x = int(line[1]) + int(line[2]) + int(line[3])
			y = int(line[1]) + int(line[2]) + int(line[3]) + int(line[4])
			cpuid = int(line[0][3:])
			if cpuid in lastx and y > lasty[cpuid]:
				ratio = (x - lastx[cpuid]) / (y - lasty[cpuid])
				fillRatio(cpuid * w, w, 1, ratio)
				#print(cpuid,"=",ratio)
			lastx[cpuid] = x
			lasty[cpuid] = y
	pixels.show()
	time.sleep(0.5)
