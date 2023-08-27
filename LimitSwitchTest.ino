#define OPEN_DOOR_1_SWITCH_PIN 2
#define CLOSE_DOOR_1_SWITCH_PIN 3
#define OPEN_DOOR_2_SWITCH_PIN 4 
#define CLOSE_DOOR_2_SWITCH_PIN 5 

void setup() {
  Serial.begin(9600);
  pinMode(OPEN_DOOR_1_SWITCH_PIN, INPUT);
  pinMode(CLOSE_DOOR_1_SWITCH_PIN, INPUT);
  pinMode(OPEN_DOOR_2_SWITCH_PIN, INPUT);
  pinMode(CLOSE_DOOR_2_SWITCH_PIN, INPUT);
}

void loop() {
  checkLimitSwitches();
  delay(1000);  // Delay for 1 second
}

void checkLimitSwitches() {

  if (digitalRead(OPEN_DOOR_1_SWITCH_PIN) == LOW) {
    Serial.println("Door 1 Open");
  }
  if (digitalRead(CLOSE_DOOR_1_SWITCH_PIN) == LOW) {
    Serial.println("Door 1 Closed");
  }
  if (digitalRead(OPEN_DOOR_2_SWITCH_PIN) == LOW) {
    Serial.println("Door 2 Open");
  }
  if (digitalRead(CLOSE_DOOR_2_SWITCH_PIN) == LOW) {
    Serial.println("Door 2 Closed");
  }
}
