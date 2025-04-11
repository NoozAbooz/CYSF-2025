int analogPin = A0;
int raw = 0;
int Vin = 3.3;
float Vout = 0;
float R1 = 1000;
float R2 = 0;
float buffer = 0;

void setup(){
Serial.begin(9600);
}

void loop(){
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(analogPin);
    delay(5);  // small delay between samples
  }
  raw = sum / 10.0;

  if(raw){
    buffer = raw * Vin;
    Vout = (buffer)/1024.0;
    buffer = (Vin/Vout) - 1;
    float R2temp = R1 * buffer;

    R2 = (-(R2temp) - 70) / 5;
    
    Serial.print("Vout: ");
    Serial.print(Vout);
    Serial.print(" | R2: ");
    Serial.print(R2);
    Serial.println("kΩ");
    delay(1000);
  }
}
