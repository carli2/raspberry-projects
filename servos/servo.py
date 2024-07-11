import RPi.GPIO as GPIO
import time
import math

GPIO.setmode(GPIO.BCM)

# setup pin
GPIO.setup(11, GPIO.OUT)

# create PWM handler
p = GPIO.PWM(11, 50)

# start with neutral position
p.start(7.5)
# use this function to adjust position
def position(x):
	global p
	p.ChangeDutyCycle(7.5 + x * 2.5)

# some movement that shows how stable it is
for n in range(400, 1100):
	p.ChangeDutyCycle(n / 100.0)
	if n == 750:
		time.sleep(4)
	time.sleep(0.01)

p.stop()
GPIO.cleanup()
