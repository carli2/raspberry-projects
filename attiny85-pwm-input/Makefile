MCU=attiny85
SRCS=i2cslave.c
PARAMS=
CLOCK=1000000

main.hex: main.bin
	avr-objcopy -j .text -j .data -O ihex main.bin main.hex
main.bin: $(SRCS)
	avr-gcc -Wall -g -Os -mmcu=${MCU} -o main.bin ${SRCS} ${PARAMS}
	avr-size -C main.bin

install: main.hex
	avrdude -p ${MCU} -c usbasp -U flash:w:main.hex:i -F -P usb
