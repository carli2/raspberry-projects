#!/bin/bash

# add isolcpus=3 to /boot/cmdline.txt

echo 25 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio25/direction
echo 22 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio22/direction
echo 23 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio23/direction
echo 24 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio24/direction
taskset -c 3 php stepper.php
