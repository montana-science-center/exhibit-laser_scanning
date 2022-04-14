#include <FastLED.h> // 3.5.0
#include <digitalWriteFast.h>

// DAC AD7528
// digital pins 0-7 write DAC value (port D)
// chip select is always low (selected)
// digital pin 8 is DAC select (x/y) (port B), low is A
// digital pin 9 is write enable (port B), low is active

#define dac_select_pin 8
#define write_enable_pin 9
#define DACA 0
#define DACB 1

// needs fine frequency control.
// try scaling 12 bit form adc to 0-100hz
byte x_freq_hz = 10;
byte y_freq_hz = 11;


void setup() {

    // Port D to all output
    //      MSB..LSB
    // pins 9      0
    DDRD = B11111111;

    // port B pins 8 and 9 to output
    pinModeFast(dac_select_pin, OUTPUT);
    pinModeFast(write_enable_pin, OUTPUT);

    // init pin values
    PORTD = 0;
    digitalWriteFast(dac_select_pin, LOW);
    digitalWriteFast(write_enable_pin, HIGH);

}

void loop() {
    byte x_pos = wave8(x_freq_hz);
    byte y_pos = wave8(y_freq_hz);

    // X
    write_dac(DACA, x_pos);
    // Y
    write_dac(DACB, y_pos);
}


void write_dac(bool ab_select, byte value) {
    // select, DAC,
    // pull write low,
    // dump data to port D,
    // pull write high.

    digitalWriteFast(dac_select_pin, ab_select);
    digitalWriteFast(write_enable_pin, LOW);
    PORTD = value;
    digitalWriteFast(write_enable_pin, HIGH);
}


uint8_t wave8(uint8_t hz) {
    // GET_MILLIS is uint32_t

    // Hz is 'beats per second', or 'cycles per 1000ms'.
    // To avoid using the (slower) division operator, we
    // want to convert 'cycles per 1000ms' to 'cycles per 65536ms',
    // and then use a simple, fast bit-shift to divide by 65536.
    //
    // The ratio 65536:1000 is 4294967:256

    uint8_t beat = ((GET_MILLIS()) * hz * 4294967) >> 24;
    return sin8(beat);
}
