// Laser Scanning exhibit 2022
// Monogon module
//
// NOTES:
//  Arduino IDE 1.8.19
//  Arduino Boards 1.8.3
//  Encoders
//      Interrups are being used for fan tach reads,
//      so we can't use them for the encoders.
//
//  NOCTUA FAN
//      pin color function
//        1   BLK      GND
//        2   YEL   +12vcd
//        3   GRN     Tach
//        4   BLU      PWM
//
//      PWM range
//      off slow   fast
//      0-8,   9 -  255 PWM
//        0, 420 - 5000 rpm
//

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>    // 1.4.2


// SOFTWARE CONFIG
const byte sleep_delay_m = 2;     // minutes to wait before turning off the exhibit
const byte knob_deadband = 2;           // encoder ticks before exhibit is considered "awake"


// HARDWARE CONFIG
Encoder x_encoder(9, 10);
Encoder y_encoder(12, 11);
const byte FAN_1_PIN = 5;   //both on timer 0
const byte FAN_2_PIN = 6;
const byte TAC_1_PIN = 2;
const byte TAC_2_PIN = 3;
const byte LASER_PIN = 7;
const byte zero_fan_pwm = 7;


// TRACKING VARIABLES
byte fan_1_speed = 0;
byte fan_2_speed = 0;
byte previous_fan_1_speed = 0;
byte previous_fan_2_speed = 0;
bool awake = 1;                         // exhibit motor power state
long last_touch_ms = 0;                 // time in milliseconds since last touch


void set_awake(bool state) {
    // Turn the laser on or off.
    digitalWrite(LASER_PIN, state);
    if (!state) {
        analogWrite(FAN_1_PIN, zero_fan_pwm);
        analogWrite(FAN_2_PIN, zero_fan_pwm);
    }
    awake = state;
}


void setup() {
    pinMode(FAN_1_PIN, OUTPUT);
    pinMode(FAN_2_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(TAC_1_PIN, INPUT_PULLUP);   //here for the future
    pinMode(TAC_2_PIN, INPUT_PULLUP);
    set_awake(1);
}


void loop() {

    fan_1_speed = read_encoder(x_encoder);
    fan_2_speed = read_encoder(y_encoder);

    update_state();

    if (awake) {
        analogWrite(FAN_1_PIN, fan_1_speed);
        analogWrite(FAN_2_PIN, fan_2_speed);
    }
}


// Updates the power state of the exhibit.
// Sleeps after no one has touched it in sleep_delay_s
void update_state() {
    // compare current knob setting to last recorded value
    // and see if it's outside the deadband, if so, wake up
    if (abs(fan_1_speed -  previous_fan_1_speed) > knob_deadband ||
        abs(fan_2_speed -  previous_fan_2_speed) > knob_deadband) {

        last_touch_ms = millis();
        previous_fan_1_speed = fan_1_speed;
        previous_fan_2_speed = fan_2_speed;

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
    } else if (state < zero_fan_pwm) {
        state = zero_fan_pwm;
        the_encoder.write(zero_fan_pwm);
    }
    return (byte)state;
}
