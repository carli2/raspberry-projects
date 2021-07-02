Workstation setup
=================
```
sudo apt install avrdude gcc-avr avr-libc make
```

- make
- flash main.hex to your attiny85 using fuses L:E1 H:DD
- connect attiny to your I²C bus
- put PWM input on PB1, PB3 and PB4
