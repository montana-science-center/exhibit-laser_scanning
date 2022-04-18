// PWM outputs 5 and 6 are controled by timer 0
const byte PWM = 6;
const byte AIN1 = 7;
const byte AIN2 = 8;
const byte TAC = 3;

volatile unsigned long rpmtime;
float rpmfloat;
unsigned int rpm;
volatile bool tooslow = 1;

void setup() {

    Serial.begin(9600);

    // put your setup code here, to run once:
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(PWM, OUTPUT);
    
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << CS12); //Prescaler 256
    TIMSK1 |= (1 << TOIE1); //enable timer overflow
    pinMode(TAC, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TAC), RPM, FALLING);

    digitalWrite(AIN1, LOW);
    digitalWrite(AIN1, HIGH);
    analogWrite(PWM, 40);
    // for intel fan
    // 0 = 1500
    // 255 = 3120

}

ISR(TIMER1_OVF_vect) {
    tooslow = 1;
}

void loop() {

    delay(1000);
//    if (tooslow == 1) {
//        Serial.println("Overflow");
//    } else {
//        rpmfloat = 120 / (rpmtime / 31250.00);
//        rpm = round(rpmfloat);
//        Serial.println(rpm);
//    }

}

void RPM () {
    rpmtime = TCNT1;
    TCNT1 = 0;
    tooslow = 0;
    digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);   // toggle LED pin
}
