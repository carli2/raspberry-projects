/****************************************************/
/* The USI as Slave                                 */
/* Author: Axel Gartner (I2C logic)                 */
/* Author: Carl-Philip Hänsch (registerbank)        */
/****************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

#define USI_DATA   			USIDR
#define USI_STATUS  		USISR
#define USI_CONTROL 		USICR
#define USI_ADDRESS			0x20

#define NONE					0
#define ACK_PR_RX			1
#define BYTE_RX				2
#define ACK_PR_TX			3
#define PR_ACK_TX			4
#define BYTE_TX				5

// Device dependant defines
#if defined(__at90tiny26__) | defined(__attiny26__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB0
    #define PORT_USI_SCL        PORTB2
#endif

#if defined(__at90Tiny2313__) | defined(__attiny2313__)
    #define DDR_USI             DDRB
    #define PORT_USI            PORTB
    #define PIN_USI             PINB
    #define PORT_USI_SDA        PORTB5
    #define PORT_USI_SCL        PORTB7
#endif

#define DDR_USI             DDRB
#define PORT_USI            PORTB
#define PIN_USI             PINB
#define PORT_USI_SDA        PORTB0
#define PORT_USI_SCL        PORTB2

volatile uint8_t COMM_STATUS = NONE;

void USI_init(void) {
	// 2-wire mode; Hold SCL on start and overflow; ext. clock
	USI_CONTROL |= (1<<USIWM1) | (1<<USICS1);
	USI_STATUS = 0xf0;  // write 1 to clear flags, clear counter
	DDR_USI  &= ~(1<<PORT_USI_SDA);
	PORT_USI &= ~(1<<PORT_USI_SDA);
	DDR_USI  |=  (1<<PORT_USI_SCL);
	PORT_USI |=  (1<<PORT_USI_SCL);
	// startcondition interrupt enable
	USI_CONTROL |= (1<<USISIE);
}

/* slave implementation: simple register bank */
volatile int read_state = -1;
volatile uint8_t read_addr = 0;
volatile uint8_t registerbank[0x40]; // 0x40 bytes registerbank
volatile uint32_t clk_overflow = 0; // clock register
volatile uint32_t pb_old = 0;

volatile uint32_t pwm1_time = 0;
volatile uint32_t pwm2_time = 0;
volatile uint32_t pwm3_time = 0;

inline void i2c_start() {
	// neue übertragung beginnt
	read_state = 0;
}
inline void i2c_rcv(uint8_t byte) {
	if (read_state == 0) {
		read_addr = byte; // first read byte is address
		read_state = 1; // next bytes are data
	} else {
		registerbank[read_addr++] = byte; // write operation
		// TODO: react to some values??
	}
}
inline uint8_t i2c_send() {
	if (read_addr >= 0x40) return 0; // overflow
	return registerbank[read_addr++]; // read from registerbank
}

int main(void) {

	registerbank[0] = 0xC0;
	registerbank[1] = 0xFF;
	registerbank[2] = 0xEE;

	// Timer: Takte zählen (1 MHz)
	TIMSK = (1 << TOIE0); // enable overflow interrupt
	TCCR0B = 1; // no prescaling, clock enable
	OCR0B = 0xff; // count to 0xff
	TCNT0 = 0;

	// change PB1, PB3 and PB4 to input
	DDRB  &= 0x11100101b; // input
	PORTB |= 0x00011010b; // pull up
	PCMSK = (1 << PCINT4) | (1 << PCINT3) | (1 << PCINT1); // enable interrupt on pins 

USI_init();
sei();

for(;;)	{
	// capture current time
	uint8_t pb = PINB & 0x00011010b;
	// fast spin for change (interrupts do not fire for some reason)
	if (pb == pb_old) {
		continue;
	}
	cli(); // prevent clk_overflow and TCNT0 from getting out of sync
	uint32_t time_us = (clk_overflow << 4) | (TCNT0 >> 4); // hardcoded 16MHz
	sei();

	// debug registers
	/*registerbank[0x08] = time_us >> 0;
	registerbank[0x09] = time_us >> 8;
	registerbank[0x0a] = time_us >> 16;
	registerbank[0x0b] = time_us >> 24;*/
	registerbank[0x0f] = pb;

	// PB1 PWM input
	if ((pb & 2) != (pb_old & 2)) {
		if (pb & 2) {
			// init PWM state
			pwm1_time = time_us;
		} else {
			// got pwm value
			pwm1_time = (time_us - pwm1_time);
			if (pwm1_time > 700 && pwm1_time < 2300) {
				registerbank[0x10] = pwm1_time >> 0;
				registerbank[0x11] = pwm1_time >> 8;
			}
		}
	}

	// PB3 PWM input
	if ((pb & 8) != (pb_old & 8)) {
		if (pb & 8) {
			// init PWM state
			pwm2_time = time_us;
		} else {
			// got pwm value
			pwm2_time = (time_us - pwm2_time);
			if (pwm2_time > 700 && pwm2_time < 2300) {
				registerbank[0x20] = pwm2_time >> 0;
				registerbank[0x21] = pwm2_time >> 8;
			}
		}
	}

	// PB4 PWM input
	if ((pb & 16) != (pb_old & 16)) {
		if (pb & 16) {
			// init PWM state
			pwm3_time = time_us;
		} else {
			// got pwm value
			pwm3_time = (time_us - pwm3_time);
			if (pwm3_time > 700 && pwm3_time < 2300) {
				registerbank[0x30] = pwm3_time >> 0;
				registerbank[0x31] = pwm3_time >> 8;
			}
		}
	}

	// remember previous pb value
	pb_old = pb;
}

}

SIGNAL(SIG_OVERFLOW0) {
	clk_overflow++;
}

SIGNAL(SIG_USI_START) {
	COMM_STATUS = NONE;
	// Wait for SCL to go low to ensure the "Start Condition" has completed.
	// otherwise the counter will count the transition
	while ( (PIN_USI & (1<<PORT_USI_SCL)) );
	USI_STATUS = 0xf0; // write 1 to clear flags; clear counter
	// enable USI interrupt on overflow; SCL goes low on overflow
	USI_CONTROL |= (1<<USIOIE) | (1<<USIWM0);
}




SIGNAL(SIG_USI_OVERFLOW) {
  uint8_t BUF_USI_DATA = USI_DATA;
	switch(COMM_STATUS) {
	case NONE:
		if (((BUF_USI_DATA & 0xfe) >> 1) != USI_ADDRESS) {	// if not receiving my address
			// disable USI interrupt on overflow; disable SCL low on overflow
			USI_CONTROL &= ~((1<<USIOIE) | (1<<USIWM0));
		}
		else { // else address is mine
			i2c_start();
			DDR_USI  |=  (1<<PORT_USI_SDA);
			USI_STATUS = 0x0e;	// reload counter for ACK, (SCL) high and back low
			if (BUF_USI_DATA & 0x01) {
				COMM_STATUS = ACK_PR_TX;
			} else {
				COMM_STATUS = ACK_PR_RX;
			}
		}
		break;
	case ACK_PR_RX:
		DDR_USI  &= ~(1<<PORT_USI_SDA);
		COMM_STATUS = BYTE_RX;
		break;
	case BYTE_RX:
		/* Save received byte here! ... = USI_DATA*/
		i2c_rcv(USI_DATA);
		DDR_USI  |=  (1<<PORT_USI_SDA);
		USI_STATUS = 0x0e;	// reload counter for ACK, (SCL) high and back low
		COMM_STATUS = ACK_PR_RX;
		break;
	case ACK_PR_TX:
		/* Put first byte to transmit in buffer here! USI_DATA = ... */
		USI_DATA = i2c_send();
		PORT_USI |=  (1<<PORT_USI_SDA); // transparent for shifting data out
		COMM_STATUS = BYTE_TX;
		break;
	case PR_ACK_TX:
		if(BUF_USI_DATA & 0x01) {
			COMM_STATUS = NONE; // no ACK from master --> no more bytes to send
		}
		else {
			/* Put next byte to transmit in buffer here! USI_DATA = ... */
			USI_DATA = i2c_send();
			PORT_USI |=  (1<<PORT_USI_SDA); // transparent for shifting data out
			DDR_USI  |=  (1<<PORT_USI_SDA);
			COMM_STATUS = BYTE_TX;
		}
		break;
	case BYTE_TX:
		DDR_USI  &= ~(1<<PORT_USI_SDA);
		PORT_USI &= ~(1<<PORT_USI_SDA);
		USI_STATUS = 0x0e;	// reload counter for ACK, (SCL) high and back low
		COMM_STATUS = PR_ACK_TX;
		break;
	}
	USI_STATUS |= (1<<USIOIF); // clear overflowinterruptflag, this also releases SCL
}
