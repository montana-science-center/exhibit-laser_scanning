// Laser Scanning exhibit 2022
// Galvanometer module
//
// NOTES:
//  Arduino IDE 1.8.19
//  Arduino Boards 1.8.3
//  DAC AD7528
//      Digital pins 0-7 write DAC value (port D)
//      Chip select is always low (selected)
//      Digital pin 8 is DAC select (x/y) (port B), low is A
//      Digital pin 9 is write enable (port B), low is active
//  Encoders
//      Ued because ADC read and denoise was too slow.
//      Interrups are on the only 8-bit port used for the laser DAC
//      so we can't use them for the encoders without port-spanning :(

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>    // 1.4.2
#include <digitalWriteFast.h>   // local


// SOFTWARE CONFIG
const byte sleep_delay_m = 2;     // minutes to wait before turning off the exhibit
const byte knob_deadband = 2;           // encoder ticks before exhibit is considered "awake"


// HARDWARE CONFIG
Encoder x_encoder(10, 11);
Encoder y_encoder(12, 13);
const byte dac_select_pin = 8;
const byte write_enable_pin = 9;
const byte laser_pin = A1;
const int wavetable_samples = 256;      // must be same as uint8_t size
byte wavetable[wavetable_samples];


// TRACKING VARIABLES
byte x_hz = 0;
byte y_hz = 0;
byte previous_x_hz = 0;
byte previous_y_hz = 0;
bool awake = 1;                         // exhibit motor power state
long last_touch_ms = 0;                 // time in milliseconds since last touch


void set_awake(bool state) {
    // Turn the laser on or off.
    digitalWriteFast(laser_pin, state);
    // Stop and center the galvos
    if (!state) {
        write_dac(128, 128);
    }
    awake = state;
}


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

    pinMode(dac_select_pin, OUTPUT);
    pinMode(write_enable_pin, OUTPUT);
    pinMode(laser_pin, OUTPUT);

    // init pin values
    PORTD = 0;
    digitalWriteFast(dac_select_pin, LOW);
    digitalWriteFast(write_enable_pin, HIGH);
    set_awake(1);

}


void loop() {

    x_hz = read_encoder(x_encoder);
    y_hz = read_encoder(y_encoder);

    update_state();

    if (awake) {
        write_dac(wave8(x_hz), wave8(y_hz));
    }

}


// Updates the power state of the exhibit.
// Sleeps after no one has touched it in sleep_delay_s
void update_state() {
    // compare current knob setting to last recorded value
    // and see if it's outside the deadband, if so, wake up
    if (abs(x_hz -  previous_x_hz) > knob_deadband ||
        abs(y_hz -  previous_y_hz) > knob_deadband) {

        last_touch_ms = millis();
        previous_x_hz = x_hz;
        previous_y_hz = y_hz;

        if (!awake) {
            set_awake(1);
        }
        return;
    }

    // Check the timer to see if it's time to sleep.
    if (awake) {
        if (millis() - last_touch_ms > sleep_delay_m * 60000) {
            set_awake(0);
        }
    }
}


byte read_encoder(Encoder &the_encoder) {
    int state = the_encoder.read();
    if (state > 255) {
        state = 255;
        the_encoder.write(255);
    } else if (state < 0) {
        state = 0;
        the_encoder.write(0);
    }
    return (byte)state;
}


void write_dac(byte x_pos, byte y_pos) {
    digitalWriteFast(dac_select_pin, HIGH);
    digitalWriteFast(write_enable_pin, LOW);
    PORTD = x_pos;
    digitalWriteFast(dac_select_pin, LOW);
    PORTD = y_pos;
    digitalWriteFast(write_enable_pin, HIGH);
}


uint8_t wave8(uint8_t hz) {
    // converts micros' uint32_t [0, 4294967296) to hertz by scaling it so it overflows.
    // 1e6 us is 1 second, so micros() loops every 4294 seconds.
    // We mult by 4294 so that it overflows every second.
    // We then keep the top 8 bits so we get a 0-255 saw-wave.
    // That goes into the wavetable lookup.

    uint8_t beat = ((micros()) * hz * 4294) >> 24;
    return wavetable[beat];
}
