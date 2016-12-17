#!/bin/bash

# working together with PCA9685 16-channel 12-bit PWM Fm + I2C-bus LED controller

# parameters to set the bus id and gadget address
bus=1
gadget=0x40

# setup 50 Hz
i2cset -y $bus $gadget 0 0x10
i2cset -y $bus $gadget 0xFE 0x79
i2cset -y $bus $gadget 0 0x21

function set { # id, val
	i2cset -y $bus $gadget $((8+4*$1)) $2 w
	i2cset -y $bus $gadget $((6+4*$1)) 4095 w
}

set 0 204
set 1 204
sleep 1
set 0 307
set 1 307
sleep 1
set 0 409
set 1 409
sleep 1
set 0 307
set 1 307

