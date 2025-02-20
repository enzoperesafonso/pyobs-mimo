#include <Stepper.h>
#include <Servo.h>

#define STEPS_PER_REV 2048  // Stepper motor steps per revolution (27BYJ-28)
#define AZ_MOTOR_SPEED 10   // Stepper motor speed (RPM)
#define SIDEREAL_STEPS_PER_SEC (STEPS_PER_REV / 24 / 60 / 4)  // ~15°/hour
#define SERVO_STEP_DELAY 20  // Delay between each servo step (ms)
#define LED_PIN 13  // LED pin

Stepper azimuthMotor(STEPS_PER_REV, 8, 10, 9, 11);  // Define stepper motor pins
Servo altitudeServo;

bool siderealTracking = false;
bool ledState = false;
float targetAz = 0, targetAlt = 90;  // Default position
float currentAz = 0, currentAlt = 90;
unsigned long lastBlinkTime = 0;
bool blinkState = false;

void setup() {
    Serial.begin(9600);
    azimuthMotor.setSpeed(AZ_MOTOR_SPEED);
    altitudeServo.attach(6);
    altitudeServo.write(targetAlt);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // LED starts OFF
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input == "TRACK_ON") {
            siderealTracking = true;
            Serial.println("Sidereal tracking ON");
        } else if (input == "TRACK_OFF") {
            siderealTracking = false;
            Serial.println("Sidereal tracking OFF");
            digitalWrite(LED_PIN, LOW);  // LED OFF when tracking stops
        } else if (input == "GET_POS") {
            Serial.print("ALT:"); Serial.print(currentAlt);
            Serial.print(" AZ:"); Serial.println(currentAz);
        } else if (input == "LED_ON") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println("LED ON");
        } else if (input == "LED_OFF") {
            digitalWrite(LED_PIN, LOW);
            Serial.println("LED OFF");
        } else if (input.startsWith("ALT:") && input.indexOf("AZ:") > 0) {
            int altIndex = input.indexOf("ALT:") + 4;
            int azIndex = input.indexOf("AZ:") + 3;
            targetAlt = input.substring(altIndex, input.indexOf(" ", altIndex)).toFloat();
            targetAz = input.substring(azIndex).toFloat();
            Serial.print("Moving to ALT: "); Serial.print(targetAlt);
            Serial.print(" AZ: "); Serial.println(targetAz);
            digitalWrite(LED_PIN, HIGH);  // LED ON while moving
        }
    }

    // Move altitude (servo) gradually
    if (abs(targetAlt - currentAlt) > 1) {  // Avoid jitter
        for (int pos = currentAlt; pos != targetAlt; pos += (targetAlt > currentAlt) ? 1 : -1) {
            altitudeServo.write(pos);
            delay(SERVO_STEP_DELAY);
        }
        currentAlt = targetAlt;
    }

    // Move azimuth (stepper)
    int azimuthSteps = (targetAz - currentAz) * (STEPS_PER_REV / 360.0);
    if (abs(azimuthSteps) > 2) {
        azimuthMotor.step(azimuthSteps);
        currentAz = targetAz;
    }

    // Sidereal tracking (blinking LED)
    if (siderealTracking) {
        azimuthMotor.step(SIDEREAL_STEPS_PER_SEC);
        currentAz += 15.0 / 3600.0;  // Update position estimate
        delay(1000);

        // Blink LED every 500ms
        if (millis() - lastBlinkTime >= 500) {
            blinkState = !blinkState;
            digitalWrite(LED_PIN, blinkState);
            lastBlinkTime = millis();
        }
    } else {
        digitalWrite(LED_PIN, LOW);  // Ensure LED is OFF when not tracking
    }
}
