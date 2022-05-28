
//////////////////////////
//  NOCTUA FAN
//  
//  pin color function
//    1   BLK      GND
//    2   YEL   +12vcd
//    3   GRN     Tach
//    4   BLU      PWM
//
//  PWM range
//  off slow   fast
//  0-8,   9 -  255 PWM
//    0, 420 - 5000 rpm
//
//////////////////////////

// Interrups are being used for fan tach reads,
// so we can't use them for the encoders.
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>    // 1.4.2
Encoder x_encoder(9, 10);
Encoder y_encoder(12, 11);

// fan outputs 5 and 6 are controled by timer 0
const byte FAN_1_PIN = 5;
const byte FAN_2_PIN = 6;

// interrup pins for tac signal
// may be used for PID loop in future
const byte TAC_1_PIN = 2;
const byte TAC_2_PIN = 3;

// you'll have to move this if you want pwm
const byte LASER_PIN = 7;


void setup() {
    pinMode(FAN_1_PIN, OUTPUT);
    pinMode(FAN_2_PIN, OUTPUT);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(TAC_1_PIN, INPUT_PULLUP);
    pinMode(TAC_2_PIN, INPUT_PULLUP);

    digitalWrite(LASER_PIN, HIGH);
}

void loop() {
    analogWrite(FAN_1_PIN, read_encoder(x_encoder));
    analogWrite(FAN_2_PIN, read_encoder(y_encoder));
}


byte read_encoder(Encoder &the_encoder) {
    int state = the_encoder.read();
    if (state > 255) {
        state = 255;
        the_encoder.write(255);
    } else if (state < 7) {
        state = 7;
        the_encoder.write(7);
    }
    return (byte)state;
}
