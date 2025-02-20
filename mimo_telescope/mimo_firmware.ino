#include <Stepper.h>

#define STEPS_PER_REV 2048  // Steps per revolution for 27BYJ-28
#define DEFAULT_SPEED 10    // Default stepper speed (RPM)
#define LED_PIN 13          // General status LED
#define CCD_LED_PIN 12      // CCD LED

Stepper azimuthMotor(STEPS_PER_REV, 8, 10, 9, 11);
Stepper altitudeMotor(STEPS_PER_REV, 4, 6, 5, 7);

float targetAz = 0, targetAlt = 0;  // Target Alt/Az
float currentAz = 0, currentAlt = 0; // Current Alt/Az (relative to zero)
float zeroAz = 0, zeroAlt = 0; // Zero position offsets

bool blinkLed = false;
unsigned long lastBlinkTime = 0;
bool blinkState = false;
int motorSpeed = DEFAULT_SPEED;

void setup() {
    Serial.begin(9600);
    azimuthMotor.setSpeed(motorSpeed);
    altitudeMotor.setSpeed(motorSpeed);

    pinMode(LED_PIN, OUTPUT);
    pinMode(CCD_LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(CCD_LED_PIN, LOW);


    Serial.println("microMONET is ready for some stargazing!");
}

void moveBothSteppers(int stepsAz, int stepsAlt) {
    int maxSteps = max(abs(stepsAz), abs(stepsAlt));
    for (int i = 0; i < maxSteps; i++) {
        if (i < abs(stepsAz)) azimuthMotor.step(stepsAz > 0 ? 1 : -1);
        if (i < abs(stepsAlt)) altitudeMotor.step(stepsAlt > 0 ? 1 : -1);
    }
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input == "GET_POS") {
            Serial.print("ALT: "); Serial.print(currentAlt);
            Serial.print(" AZ: "); Serial.println(currentAz);
        } 
        else if (input == "SET_ZERO") {
            zeroAlt = currentAlt;
            zeroAz = currentAz;
            Serial.println("Zero position set!");
        } 
        else if (input == "LED_ON") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ON");
        } 
        else if (input == "LED_OFF") {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED OFF");
            blinkLed = false;
        }
        else if (input == "BLINK_LED") {
            blinkLed = true;
            Serial.println("LED Blinking...");
        }
        else if (input == "CCD_ON") {
            digitalWrite(CCD_LED_PIN, HIGH);
            Serial.println("CCD LED ON");
        }
        else if (input == "CCD_OFF") {
            digitalWrite(CCD_LED_PIN, LOW);
            Serial.println("CCD LED OFF");
        }
        else if (input.startsWith("SET_SPEED ")) {
            int newSpeed = input.substring(10).toInt();
            if (newSpeed >= 1 && newSpeed <= 15) {
                motorSpeed = newSpeed;
                azimuthMotor.setSpeed(motorSpeed);
                altitudeMotor.setSpeed(motorSpeed);
                Serial.print("Speed set to "); Serial.print(motorSpeed); Serial.println(" RPM");
            } else {
                Serial.println("Invalid speed! Use 1-15 RPM.");
            }
        }
        else if (input.startsWith("ALT:") && input.indexOf("AZ:") > 0) {
            int altIndex = input.indexOf("ALT:") + 4;
            int azIndex = input.indexOf("AZ:") + 3;
            targetAlt = input.substring(altIndex, input.indexOf(" ", altIndex)).toFloat();
            targetAz = input.substring(azIndex).toFloat();

            Serial.print("Moving to ALT: "); Serial.print(targetAlt);
            Serial.print(" AZ: "); Serial.println(targetAz);

            int stepsAlt = (targetAlt - (currentAlt - zeroAlt)) * (STEPS_PER_REV / 360.0);
            int stepsAz = (targetAz - (currentAz - zeroAz)) * (STEPS_PER_REV / 360.0);

            digitalWrite(LED_PIN, HIGH);
            moveBothSteppers(stepsAz, stepsAlt);
            digitalWrite(LED_PIN, LOW);

            currentAlt = targetAlt + zeroAlt;
            currentAz = targetAz + zeroAz;
        }
    }

    if (blinkLed && millis() - lastBlinkTime >= 500) {
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
        lastBlinkTime = millis();
    }
}
