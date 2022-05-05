// Interrups are being used for fan tach reads,
// so we can't use them for the encoders.
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>    // 1.4.2
Encoder x_encoder(10, 11);
Encoder y_encoder(12, 13);

// fan_1_pwm fan pinout
// pin  color function
// 1 BLK GND
// 2 YEL +12vcd
// 3 GRN Tach
// 4 BLU PWM

// PWM range for noctua fan
// 0-8,   9 -  255 PWM
//   0, 420 - 5000 rpm

// fan_1_pwm outputs 5 and 6 are controled by timer 0
const byte fan_1_pwm = 6;
const byte TAC = 3;

volatile unsigned long rpmtime;
float rpmfloat;
unsigned int rpm;
volatile bool tooslow = 1;

void setup() {

    Serial.begin(9600);
    
    pinMode(fan_1_pwm, OUTPUT);

    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << CS12); //Prescaler 256
    TIMSK1 |= (1 << TOIE1); //enable timer overflow
    pinMode(TAC, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TAC), RPM, FALLING);

    analogWrite(fan_1_pwm, 9);


}

// if the timer overlfows, spinning too slow
ISR(TIMER1_OVF_vect) {
    tooslow = 1;
}


void RPM () {
    // called every tach pulse from an interrup pin
    rpmtime = TCNT1;    // records time since last pulse
    TCNT1 = 0;  // re-zero
    tooslow = 0;    // comfirm we are not stopped
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);   // toggle LED pin
}

void loop() {

    //delay(1000);
    
    // might want to read encoder every loop but only update pwm every 100ms or so.
    analogWrite(fan_1_pwm, read_encoder(x_encoder));

// may want a PI loop
    
//    if (tooslow == 1) {
//        Serial.println("Overflow");
//    } else {
//        rpmfloat = 60 / (rpmtime / 31250.00);
//        rpm = round(rpmfloat);
//        Serial.println(rpm);
//    }

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
