// main.c – Automatic Cleaning Bot
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "TMPL3edLrYLJZ"
#define BLYNK_TEMPLATE_NAME "HOME AID"
#define BLYNK_AUTH_TOKEN "1H9sAr7YTR-DBc4tKIlayA_0fdfBAPrs"

#define TRIG_PIN 18
#define ECHO_PIN 2

#define LEFT_MOTOR_FORWARD 25
#define LEFT_MOTOR_BACKWARD 33
#define RIGHT_MOTOR_FORWARD 16
#define RIGHT_MOTOR_BACKWARD 32

#define ANALOG_IN_PIN 34

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "home_aid";
char pass[] = "12345678";

Servo servo;
BlynkTimer timer;

bool goesForward = false;
float duration;
float distance = 100;
int flag = 0;
int counter = 0;

void moveStop() {
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
}

void moveForward() {
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
}

void moveBackward() {
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
}

void turnRight() {
    digitalWrite(RIGHT_MOTOR_FORWARD, LOW);
    digitalWrite(RIGHT_MOTOR_BACKWARD, HIGH);
    digitalWrite(LEFT_MOTOR_FORWARD, HIGH);
    digitalWrite(LEFT_MOTOR_BACKWARD, LOW);
}

void turnLeft() {
    digitalWrite(RIGHT_MOTOR_FORWARD, HIGH);
    digitalWrite(RIGHT_MOTOR_BACKWARD, LOW);
    digitalWrite(LEFT_MOTOR_FORWARD, LOW);
    digitalWrite(LEFT_MOTOR_BACKWARD, HIGH);
}

int readUltrasonic() {
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    return (duration / 2) * 0.034;
}

int lookRight() {
    servo.write(50);
    delay(500);
    int dist = readUltrasonic();
    delay(100);
    servo.write(120);
    return dist;
}

int lookLeft() {
    servo.write(180);
    delay(500);
    int dist = readUltrasonic();
    delay(100);
    servo.write(120);
    return dist;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
    pinMode(ANALOG_IN_PIN, INPUT);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
    pinMode(RIGHT_MOTOR_BACKWARD, OUTPUT);
    pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
    pinMode(LEFT_MOTOR_BACKWARD, OUTPUT);

    Serial.begin(9600);
    servo.attach(26);
    servo.write(120);

    Blynk.begin(auth, ssid, pass);
    timer.setInterval(1000L, []() {
        Blynk.virtualWrite(V5, millis() / 1000);
    });
}

void loop() {
    Blynk.run();
    timer.run();

    distance = readUltrasonic();

    if (distance <= 35) {
        moveStop();
        delay(250);
        moveBackward();
        delay(350);
        moveStop();
        delay(250);

        if (flag == 0) {
            turnRight();
            delay(1300);
            moveForward();
            delay(1500);
            moveStop();

            int leftDistance = lookLeft();
            if (leftDistance > 50) {
                turnLeft();
                delay(1300);
                moveForward();
                delay(4000);
                turnLeft();
                delay(1300);
                moveForward();
                delay(1500);
                turnRight();
                delay(1300);
            } else {
                turnRight();
                delay(1500);
                flag = 1;
                counter++;
            }
        } else {
            turnLeft();
            delay(1300);
            moveForward();
            delay(1500);
            int rightDistance = lookRight();
            if (rightDistance > 50) {
                turnRight();
                delay(1300);
                moveForward();
                delay(4000);
                turnRight();
                delay(1300);
                moveForward();
                delay(1500);
                turnLeft();
                delay(1300);
            } else {
                turnLeft();
                delay(1400);
                flag = 0;
                counter++;
            }
        }
    } else {
        moveForward();
    }

    int sensorValue = analogRead(ANALOG_IN_PIN);
    float voltage = ((sensorValue * 5.0 / 1024.0) * 2.0) + 0.4;
    float battery = mapfloat(voltage, 0, 12, 0, 100);
    battery = constrain(battery, 1, 100);
    Blynk.virtualWrite(V0, battery);

    if (battery <= 80) {
        Blynk.logEvent("battery");
    }

    if (counter >= 10) {
        Blynk.logEvent("cleaning_completed");
    }
}
