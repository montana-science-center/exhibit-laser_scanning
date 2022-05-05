#include <digitalWriteFast.h>

// Encoders used because ADC read and denoise was too slow.
// Interrups are on the only 8-bit port used for the laser DAC
// so we can't use them for the encoders without port-spanning :(
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>    // 1.4.2
Encoder x_encoder(10, 11);
Encoder y_encoder(12, 13);

// DAC AD7528
// digital pins 0-7 write DAC value (port D)
// chip select is always low (selected)
// digital pin 8 is DAC select (x/y) (port B), low is A
// digital pin 9 is write enable (port B), low is active

const byte dac_select_pin = 8;
const byte write_enable_pin = 9;
const byte DACA = 0;
const byte DACB = 1;

// sine wavetable
const int wavetable_samples = 256; // must be same as uint8_t size
byte wavetable[wavetable_samples];


void setup() {

    // init wavetable
    float half_res = wavetable_samples / 2.0;
    for (int i = 0; i < wavetable_samples; i++) {
        wavetable[i] = (byte)(sin( (i / half_res) * M_PI) * 127) + 128;
    }

    // Port D to all output
    //      MSB..LSB
    // pins 9      0
    DDRD = B11111111;

    // port B pins 8 and 9 to output
    pinMode(dac_select_pin, OUTPUT);
    pinMode(write_enable_pin, OUTPUT);

    // init pin values
    PORTD = 0;
    digitalWriteFast(dac_select_pin, LOW);
    digitalWriteFast(write_enable_pin, HIGH);

}


void loop() {
    write_dac(DACA, wave8(read_encoder(x_encoder)));
    write_dac(DACB, wave8(read_encoder(y_encoder)));
}


byte read_encoder(Encoder &the_encoder) {
    int state = the_encoder.read() >> 2;
    if (state > 255) {
        state = 255;
        the_encoder.write(255 << 2);
    }else if (state < 0) {
        state = 0;
        the_encoder.write(0);
    }
    return (byte)state;
}


void write_dac(bool ab_select, byte value) {
    digitalWriteFast(dac_select_pin, ab_select);
    digitalWriteFast(write_enable_pin, LOW);
    PORTD = value;
    digitalWriteFast(write_enable_pin, HIGH);
}


uint8_t wave8(uint8_t hz) {
    // micros is a uint32_t
    uint8_t beat = ((micros()) * hz * 4294) >> 24;
    return wavetable[beat];
}
