#include "SR04.h"

// declare constant variables for ultrasonic sensor and relay
const int trigPin = 3;
const int echoPin = 2;

const int relayPin = 8;

// declare variables for ultrasonic sensor
long val;
SR04 sr04 = SR04(echoPin, trigPin);

// declare constant variables for motor control pins
const int leftMotorForwardPin = 53;
const int leftMotorBackwardPin = 51;

const int rightMotorForwardPin = 47;
const int rightMotorBackwardPin = 49;

// constant variables for microphone pins
const int mic1pin = A8;
const int mic2pin = A9;

// other variables
const int sample_window = 50;  // amount of time to sample sound in milliseconds
int mic1;                      // microphone 1 value from analogRead (0-1023)
int mic2;                      // microphone 2 value from analogRead (0-1023)
int mic1_max;                  // maximum value recorded by mic1 during sample window
int mic1_min;                  // minimum value recorded by mic1 during sample window
int mic2_max;                  // maximum value recorded by mic2 during sample window
int mic2_min;                  // minimum value recorded by mic2 during sample window
int amp1;                      // largest amplitude of mic1 during sample window (max-min)
int amp2;                      // largest amplitude of mic2 during sample window (max-min)
int difference;                // difference between the mic amplitudes
unsigned long start_time;      // time in milliseconds at start of sample window (since program started)
unsigned long current_time;    // current time in milliseconds (since program started)

void setup() {
  // set motor control pins as outputs
  pinMode(leftMotorForwardPin, OUTPUT);
  pinMode(leftMotorBackwardPin, OUTPUT);
  pinMode(rightMotorForwardPin, OUTPUT);
  pinMode(rightMotorBackwardPin, OUTPUT);

  // set relay pin for output
  pinMode(relayPin, OUTPUT);

  // initialize serial communication
  Serial.begin(31250);
}

void loop() {  // code that loops forever

  getSample();

  // assign value distance from ultrasonic sensor to variable
  val = sr04.Distance();

  if ((abs(difference) <= 25) && (val > 15))  // default - drive forward
  {
    while ((abs(difference) <= 25) && (val > 15)) {
      Serial.print("Forward: ");
      driveForward();

      getSample();
      val = sr04.Distance();

      digitalWrite(relayPin, LOW);
    }
  } else if ((abs(difference) <= 25) && (val < 15)) {
    while ((abs(difference) <= 25) && (val < 15)) {
      Serial.print("Stopping: ");
      stopDriving();

      getSample();
      val = sr04.Distance();

      // acivate taser when stopped
      digitalWrite(relayPin, HIGH);
    }
  } else if ((amp1 > amp2))  // turn right
  {
    while ((amp1 > amp2) && (abs(difference) > 25)) {
      turnRight();
      Serial.print("Right: ");

      getSample();
      Serial.print(amp2);

      digitalWrite(relayPin, LOW);
    }
  } else if ((amp2 > amp1))  // turn left
  {
    while ((amp2 > amp1) && (abs(difference) > 15))  // difference adjusted for defective sensor
    {
      turnLeft();
      Serial.print("Left: ");

      getSample();

      digitalWrite(relayPin, LOW);
    }
  }
}

void driveForward() {  // set pins to make robot drive forward
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
}

void turnLeft() {  // set pins to make robot turn left
  digitalWrite(leftMotorForwardPin, HIGH);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, HIGH);
}

void turnRight() {  // set pins to make robot turn right
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, HIGH);
  digitalWrite(rightMotorForwardPin, HIGH);
  digitalWrite(rightMotorBackwardPin, LOW);
}

void stopDriving() {  // set pins to make robot stop
  digitalWrite(leftMotorForwardPin, LOW);
  digitalWrite(leftMotorBackwardPin, LOW);
  digitalWrite(rightMotorForwardPin, LOW);
  digitalWrite(rightMotorBackwardPin, LOW);
}

// sample the microphones for sample_window milliseconds and calculate maximum amplitude for each mic.
void getSample() {

  // reset max and min values
  mic1_min = 1023;
  mic1_max = 0;
  mic2_min = 1023;
  mic2_max = 0;

  start_time = millis();  // start time for this sample window
  current_time = millis();

  while (millis() - start_time < sample_window) {
    // read mic values
    mic1 = analogRead(mic1pin);
    mic2 = analogRead(mic2pin);

    // assign new min and max values
    mic1_min = min(mic1, mic1_min);
    mic1_max = max(mic1, mic1_max);
    mic2_min = min(mic2, mic2_min);
    mic2_max = max(mic2, mic2_max);

    current_time = millis();  // update current time
  }

  // done sampling - calculate max amplitude for each mic
  amp1 = mic1_max - mic1_min;
  amp2 = mic2_max - mic2_min;
  difference = amp1 - amp2;

  // Print amplitudes to serial monitor

  Serial.print("Mic 1 amplitude: ");
  Serial.print(amp1);
  Serial.print(" | Mic 2 amplitude: ");
  Serial.print(amp2);
  Serial.print(" | Difference = ");
  Serial.println(difference);
}