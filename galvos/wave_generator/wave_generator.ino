#include <digitalWriteFast.h>

// Encoders used because ADC read and denoise was too slow.
// Interrups are on the only 8-bit port used for the laser DAC
// so we can't use them for the encoders without port-spanning :(
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
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

byte x_freq_hz = 255; 
byte pre_x_freq_hz = 255;
byte y_freq_hz = 255;
byte pre_y_freq_hz = 255;

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

    // port B pins 8 and 9 to output
    pinMode(dac_select_pin, OUTPUT);
    pinMode(write_enable_pin, OUTPUT);

    // init pin values
    PORTD = 0;
    digitalWriteFast(dac_select_pin, LOW);
    digitalWriteFast(write_enable_pin, HIGH);

}

void loop() {
    byte x_pos = wave8(x_freq_hz, pre_x_freq_hz);
    byte y_pos = wave8(y_freq_hz, pre_y_freq_hz);
    write_dac(DACA, x_pos);
    write_dac(DACB, y_pos);
    x_freq_hz = read_encoder(x_encoder);
    //y_freq_hz = read_encoder(y_encoder);
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


uint8_t wave8(uint8_t hz, uint8_t &pre_hz) {
    // micros is a uint32_t
    uint8_t beat = ((micros()) * pre_hz * 4294) >> 24;
    
    if (hz != pre_hz) {
        if (beat == 0){
            pre_hz = hz;
        }
    }
    
    return wavetable[beat];
}
