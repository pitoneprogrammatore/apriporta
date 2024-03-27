#include <Servo.h>

// Define the knock sequence password (1 for knock, 0 for pause)
const int knockSequence[] = {1, 1, 1, 0, 1, 1, 0, 1};  // Must start and end with 1
const unsigned long knockInterval = 500;  // Time between knocks
const unsigned long tolerancePercent = 20;  // Tolerance for knock interval

// Servo control parameters
const int closedPosition = 0;   // Servo position when door is closed
const int openPosition = 104;   // Servo position when door is open
const int openDuration = 3000;  // Time door stays open

// Pin assignments
const int microphonePin = 2;
const int speakerPin = 3;
const int servoPin = 4;

unsigned int knockIndex;
unsigned long beginKnockTime;
bool knocking;
bool error;
bool knocked;
Servo doorServo;

const int knockSequenceLength = sizeof(knockSequence) / sizeof(knockSequence[0]);
const unsigned long tolerance = knockInterval / 100 * tolerancePercent;

void setup() {
  // Set up pin modes
  pinMode(microphonePin, INPUT);
  pinMode(speakerPin, OUTPUT);
  doorServo.attach(servoPin);
  // Begin with door closed
  doorServo.write(closedPosition);
}

void loop() {
  // Check if knocking is in progress
  if (knocking) {
    // Check the time since the last knock
    unsigned long knockDelta = millis() - beginKnockTime;

    // Play a tone at the expected knock interval
    if (knockDelta == knockInterval) {
      tone(speakerPin, 247, 50);
    }

    bool isWithinToleranceWindow = knockDelta >= knockInterval-tolerance && knockDelta <= knockInterval+tolerance;
    // Check if the knock occurred within the acceptable tolerance
    if (isWithinToleranceWindow) {
      if (digitalRead(microphonePin) == HIGH) {
        knocked = true;
      }
    }

    // Check if it's time for the next knock
    if (knockDelta > knockInterval + tolerance) {
      knockIndex++;

      // Check if the user's knock matches the expected knock
      if (knocked != knockSequence[knockIndex]) {
        error = true;
      }
      knocked = false;

      // Check if there are more knocks in the array
      if (knockIndex < knockSequenceLength - 1) {
        // Reset the knock timer for the next knock
        beginKnockTime = millis() - tolerance;
      } else {
        // Wait for a moment before checking the sequence
        delay(1000);

        // Check if the sequence is correct perform the corresponding actions
        if (error) {
          wrongSequence();
        } else {
          correctSequence();
        }

        // Reset flags
        knocking = false;
        error = false;
      }
    }
  } else {
    // Check if a knock is detected
    if (digitalRead(microphonePin) == HIGH) {
      // Record the time of the first knock
      beginKnockTime = millis();
      // Play a tone indicating the detection of a knock
      tone(speakerPin, 247, 50);
      // Reset password index for a new attempt
      knockIndex = 0;
      // Set the knocking flag, to start knocking
      knocking = true;
    }
  }
}


void wrongSequence() {
  // Play wrong sequence sound
  tone(speakerPin, 1976, 200);
  delay(100);
  tone(speakerPin, 1480, 200);
}

void correctSequence() {
  // Play correct sequence music
  tone(speakerPin, 1480, 80);
  delay(240);
  tone(speakerPin, 1480, 40);
  delay(120);
  tone(speakerPin, 1480, 40);
  delay(120);
  tone(speakerPin, 1976, 1000);

  // Open the door
  doorServo.write(openPosition);
  delay(openDuration);
  // Close the door
  doorServo.write(closedPosition);
}
