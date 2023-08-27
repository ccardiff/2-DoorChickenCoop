#define MOTOR_PIN1 9
#define MOTOR_PIN2 6
#define MOTOR_PIN3 11
#define MOTOR_PIN4 10

#define MOTOR_TIMEOUT 6000      // Motor timeout in ms (10 seconds)

void setup() {
  Serial.begin(9600);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(MOTOR_PIN3, OUTPUT);
  pinMode(MOTOR_PIN4, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any leading/trailing white space
    if (command.equals("DOOR 1 OPEN")) {
      Serial.println("Opening Door 1...");
      openDoor(1);
    }
    else if (command.equals("DOOR 1 CLOSE")) {
      Serial.println("Closing Door 1...");
      closeDoor(1);
    }
    else if (command.equals("DOOR 2 OPEN")) {
      Serial.println("Opening Door 2...");
      openDoor(2);
    }
    else if (command.equals("DOOR 2 CLOSE")) {
      Serial.println("Closing Door 2...");
      closeDoor(2);
    }
    else if (command.equals("STOP")) {
      Serial.println("Stopping motor...");
      stopMotor();
    }
    else {
      Serial.println("Unknown command");
    }
  }
}

void openDoor(int doorNum) {
  if (doorNum == 1) {
    digitalWrite(MOTOR_PIN1, HIGH);
    digitalWrite(MOTOR_PIN2, LOW);
    delay(MOTOR_TIMEOUT);
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
  }
  else if (doorNum == 2) {
    digitalWrite(MOTOR_PIN3, HIGH);
    digitalWrite(MOTOR_PIN4, LOW);
    delay(MOTOR_TIMEOUT);
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);
  }
}

void closeDoor(int doorNum) {
  if (doorNum == 1) {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, HIGH);
    delay(MOTOR_TIMEOUT);
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
  }
  else if (doorNum == 2) {
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, HIGH);
    delay(MOTOR_TIMEOUT);
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);
  }
}

void stopMotor() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
  digitalWrite(MOTOR_PIN3, LOW);
  digitalWrite(MOTOR_PIN4, LOW);
}
