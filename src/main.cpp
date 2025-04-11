#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "deep_sleep.h"
#include "pwm_pulse.h"

/* Define pins */
const int electrodePin = 4; // gpio4/a2/d2

/* Global vars */
float pwmFreq = 0;         // Hz
int pwmDutyCycle = 0;      // 0–255, keep at 50%
String status = "idle";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;

/*  */
// Commands:
// ping (returns uptime)
// calibrate <seconds> (calibrates the device for x seconds)
// pulse <frequency> <intensity> (sends AC at x Hz, y/255 duty cycle)
// status (returns the current variables of the device)
// stop (cancels all tasks)

void parseCommand(String value) {
  value.toLowerCase();
  String response = "[INFO] ";

  if (value.startsWith("ping")) {
    response += "Pong! Uptime: ";
    response += millis();
    pCharacteristic->setValue(response.c_str());
  }
  
  if (value.startsWith("calibrate")) {
    status = "calibrating";
    parseCommand("stop"); // cancel current tasks

    String numberPart = value.substring(10);
    numberPart.trim();
    if (numberPart.length() > 0 && numberPart.toInt() != 0) {
      response += "Calibrating for " + numberPart + " sec...";
      pCharacteristic->setValue(response.c_str());
      Serial.println(response);

      for (int i = 0; i < numberPart.toInt()*1000;) {
        response = "[INFO] Calibrating... " + String(i + 1) + "/" + numberPart;
        pCharacteristic->setValue(response.c_str());
        Serial.println(response);

        i += 100;
        delay(100);
      }
    }
  }

  if (value.startsWith("pulse ")) {
    status = "pulsing";
    // grab everything after "pulse "
    String args = value.substring(6);
    args.trim();

    // find the space between the two numbers
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex > 0) {
      // first token
      String freqPart = args.substring(0, spaceIndex);
      String intensityPart = args.substring(spaceIndex + 1);
      freqPart.trim();
      intensityPart.trim();

      // make sure both parsed to something non‐zero
      if (freqPart.toInt() > 0.0 && intensityPart.toInt() > 0.0) {
        pinMode(electrodePin, OUTPUT); // !! make sure to output
        // convert to numeric
        pwmFreq = freqPart.toFloat();
        pwmDutyCycle = intensityPart.toInt();

        // build and send the response
        response  = "Sending AC at ";
        response += pwmFreq;
        response += "Hz, ";
        response += (int) (intensityPart.toFloat()/255*100);
        response += "% duty cycle...";
        pCharacteristic->setValue(response.c_str());
        Serial.println(response);
      }
    }
  }

  if (value.startsWith("status")) {
    response += "Status: ";
    response += "Frequency: ";
    response += pwmFreq;
    response += "Hz, Duty Cycle: ";
    response += pwmDutyCycle;
    pCharacteristic->setValue(response.c_str());
  }

  if (value.startsWith("stop")) {
    status = "idle";
    pwmFreq = 0;
    pwmDutyCycle = 0;

    response += "CANCELLED all tasks";
    pCharacteristic->setValue(response.c_str());
    Serial.println(response);
  }
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
      String value = String(pChar->getValue().c_str());

      if (value.length() > 0) {
        Serial.print("[INCOMING] (BT): ");
        Serial.println(value);
      }

      parseCommand(value);
    }
};

/* INIT */
void setup() {
  Serial.begin(9600);

  BLEDevice::init("XIAO-ESP32C3");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World! Initializing device...");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();


  // setup electrode pin
}

void loop() {
  // run pulse func
  if (pwmFreq != 0) {
    outputPWM(electrodePin, pwmFreq, pwmDutyCycle);
  }

  // take serial input (BROKEN ATM)
  // if (Serial.available() > 0) {
  //   String value = Serial.readStringUntil('\n');
  //   parseCommand(value);
  // }
}

