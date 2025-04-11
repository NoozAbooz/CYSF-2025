// Internal timing
unsigned long previousMicros = 0;
bool pinState = false;

// output PWM 
void outputPWM(int pin, float freq, int dutyCycle) {
  // Clamp duty cycle
  dutyCycle = constrain(dutyCycle, 0, 255);

  // Calculate period in microseconds
  float period = 1000000.0 / freq;
  float onTime = period * (dutyCycle / 255.0);
  float offTime = period - onTime;

  unsigned long currentMicros = micros();

  if (pinState && (currentMicros - previousMicros >= onTime)) {
    digitalWrite(pin, LOW);
    pinState = false;
    previousMicros = currentMicros;
  } else if (!pinState && (currentMicros - previousMicros >= offTime)) {
    digitalWrite(pin, HIGH);
    pinState = true;
    previousMicros = currentMicros;
  }
}