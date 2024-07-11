#!/bin/bash

# add isolcpus=3 to /boot/cmdline.txt

# export pin, adjust pin number to your needs
echo 11 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio11/direction
# run on CPU 3 exclusively
taskset -c 3 php servo.php
