#include <digitalWriteFast.h>

// DAC AD7528
// digital pins 0-7 write DAC value (port D)
// chip select is always low (selected)
// digital pin 8 is DAC select (x/y) (port B), low is A
// digital pin 9 is write enable (port B), low is active

const byte dac_select_pin = 8;
const byte write_enable_pin = 9;
const byte adc_x_pin = A0;
const byte adc_y_pin = A1;
const byte DACA = 0;
const byte DACB = 1;

byte x_freq_hz = 127;
byte y_freq_hz = 52;

// sine wavetable
const int wavetable_samples = 256;
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

    // ADC setup
    // http://www.gammon.com.au/adc
    ADCSRA =  bit (ADEN);   // turn ADC on
    ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
    ADCSRA |= bit (ADPS2);                               //  16
    ADMUX  = bit (REFS0) | (adc_x_pin & 0x07);  // AVcc and select input port


    // port B pins 8 and 9 to output
    pinMode(dac_select_pin, OUTPUT);
    pinMode(write_enable_pin, OUTPUT);

    // init pin values
    PORTD = 0;
    digitalWriteFast(dac_select_pin, LOW);
    digitalWriteFast(write_enable_pin, HIGH);

}

void loop() {
    byte x_pos = wave8(x_freq_hz);
    byte y_pos = wave8(y_freq_hz);
    write_dac(DACA, x_pos);
    write_dac(DACB, y_pos);
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
