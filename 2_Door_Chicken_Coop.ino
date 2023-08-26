// Libraries
#include <Wire.h>
#include <OneWire.h>
#include <RTClib.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <OneButton.h>
#include <EEPROM.h>


// Constants
#define TEMP_SENSOR_PIN 13
#define ONE_WIRE_BUS TEMP_SENSOR_PIN

#define MOTOR_PIN1 9
#define MOTOR_PIN2 6
#define MOTOR_PIN3 11
#define MOTOR_PIN4 10

#define OPEN_DOOR_1_SWITCH_PIN 2
#define CLOSE_DOOR_1_SWITCH_PIN 3
#define OPEN_DOOR_2_SWITCH_PIN 4
#define CLOSE_DOOR_2_SWITCH_PIN 5

//Limit Switch Activation States
const int SWITCH_OPEN1_STATE = LOW;   // change to LOW if necessary
const int SWITCH_CLOSE1_STATE = LOW;  // change to LOW if necessary
const int SWITCH_OPEN2_STATE = LOW;   // change to LOW if necessary
const int SWITCH_CLOSE2_STATE = LOW;  // change to LOW if necessary

#define BUTTON_OPEN_PIN 7
#define BUTTON_CLOSE_PIN 12
#define BACKLIGHT_BUTTON_PIN 8

#define ERROR_COUNT_ADDRESS 0

#define DEBOUNCE_DELAY 50  // delay in ms for button debounce


#define MIN_TEMP 5
#define BACKLIGHT_TIMEOUT 20000  // Backlight timeout in ms (1 minutes)

// Define separate motor timeouts for each door (in ms)
#define MOTOR_TIMEOUT_DOOR1 3200
#define MOTOR_TIMEOUT_DOOR2 5200

#define DOOR_INTERVAL 10  // Universal door interval time in milliseconds

// Add Time Constants for Season Open and Close Times
#define SUMMER_OPEN_HOUR 7
#define SUMMER_OPEN_MINUTE 15
#define SUMMER_CLOSE_HOUR 20
#define SUMMER_CLOSE_MINUTE 01

#define WINTER_OPEN_HOUR 8
#define WINTER_OPEN_MINUTE 30
#define WINTER_CLOSE_HOUR 18
#define WINTER_CLOSE_MINUTE 1

#define SUMMER_START_MONTH 4
#define SUMMER_END_MONTH 10



bool door1Enabled = true;  // Enable/disable doors.
bool door2Enabled = false;

bool manualControl = false;  // Add the manualControl flag here. This is triggered automatically with pushing Buttons 1 or 2 until midnight.

bool canOpenDoors = true;   // Declare and initialize canOpenDoors variable
bool canCloseDoors = true;  // Declare and initialize canCloseDoors variable

bool triedToOpenDoors = false;  // Flag to know if we tried to open doors when it was too cold

int lastMinuteChecked = -1;
bool hasResetControl = false;

unsigned long startMillisDoor1 = 0;
unsigned long startMillisDoor2 = 0;

// Global Variables
OneWire oneWire(ONE_WIRE_BUS);
RTC_DS3231 rtc;
DeviceAddress tempDeviceAddress;
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// create an instance for each button
OneButton openButton(BUTTON_OPEN_PIN, true, true);
OneButton closeButton(BUTTON_CLOSE_PIN, true, true);
OneButton backlightButton(BACKLIGHT_BUTTON_PIN, true, true);

char prevDateString[20] = "";
char prevNextEvent[20] = "";
float prevTemp = -1000;


unsigned long lastDebounceTimeBacklight = 0;
int lastButtonStateBacklight = LOW;
bool backlightOn = false;
unsigned long backlightOnTime = 0;
int currentMinutes, openMinutes, closeMinutes;


// Debouncing variables
unsigned long lastDebounceTimeOpen = 0;
unsigned long lastDebounceTimeClose = 0;
int lastButtonStateOpen = LOW;
int lastButtonStateClose = LOW;

int openHour = 0;     // or some appropriate initial value
int openMinute = 0;   // or some appropriate initial value
int closeHour = 0;    // or some appropriate initial value
int closeMinute = 0;  // or some appropriate initial value
bool hasOpenedDoors = false;
bool hasClosedDoors = false;

void setupPinModes() {
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(MOTOR_PIN3, OUTPUT);
  pinMode(MOTOR_PIN4, OUTPUT);
  pinMode(OPEN_DOOR_1_SWITCH_PIN, INPUT);
  pinMode(CLOSE_DOOR_1_SWITCH_PIN, INPUT);
  pinMode(OPEN_DOOR_2_SWITCH_PIN, INPUT);
  pinMode(CLOSE_DOOR_2_SWITCH_PIN, INPUT);
}


void stopDoors() {
  if (door1Enabled) {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
  }
  if (door2Enabled) {
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);
  }
}

unsigned long lastCheckDoor1 = 0;
unsigned long lastCheckDoor2 = 0;

int openDoors() {
  // Initialization of values
  unsigned long currentMillis = millis();
  startMillisDoor1 = currentMillis;
  startMillisDoor2 = currentMillis;

  bool door1LimitReached = true;  // Assume doors are initially at their limit
  bool door2LimitReached = true;

  if (door1Enabled) {
    door1LimitReached = false;  // If door1 is enabled, then it's not yet at its limit
  }

  if (door2Enabled) {
    door2LimitReached = false;  // If door2 is enabled, then it's not yet at its limit
  }

  while (true) {
    currentMillis = millis();

    // Check Door 1
    if (door1Enabled && !door1LimitReached) {
      door1LimitReached = handleDoor1Opening();
      if (!door1LimitReached) {
        continue;  // If door 1 is not yet open, don't proceed to door 2.
      }
    }

    // Reset startMillis for Door 2 before its operation begins
    if (door1LimitReached) {
      startMillisDoor2 = millis();
    }

    // Check Door 2
    if (door2Enabled && !door2LimitReached) {
      door2LimitReached = handleDoor2Opening();
      if (!door2LimitReached) {
        continue;  // If door 2 is not yet open, loop again.
      }
    }

    // Exit condition
    if ((!door1Enabled || door1LimitReached) && (!door2Enabled || door2LimitReached)) {
      Serial.println("All motor functions complete, exiting...");
      return 0;  // Success
    }
  }
}

bool handleDoor1Opening() {
  lastCheckDoor1 = millis();

  if (millis() - startMillisDoor1 > MOTOR_TIMEOUT_DOOR1) {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
    Serial.println("Door 1 motor timeout");
    return true;  // Consider the operation as done to prevent further attempts
  }

  if (digitalRead(OPEN_DOOR_1_SWITCH_PIN) == SWITCH_OPEN1_STATE) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(OPEN_DOOR_1_SWITCH_PIN) == SWITCH_OPEN1_STATE) {
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("Door 1 Opened");
      delay(10000);
      Serial.println("Performing 10-second check for Door 1...");
      if (digitalRead(OPEN_DOOR_1_SWITCH_PIN) == SWITCH_OPEN1_STATE) {
        return true;  // Success
      } else {
        return false;  // Failure
      }
    }
  } else {
    digitalWrite(MOTOR_PIN1, HIGH);
    digitalWrite(MOTOR_PIN2, LOW);
    return false;  // Failure
  }
}

bool handleDoor2Opening() {
  lastCheckDoor2 = millis();

  if (millis() - startMillisDoor2 > MOTOR_TIMEOUT_DOOR2) {
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);
    Serial.println("Door 2 motor timeout");
    return true;  // Consider the operation as done to prevent further attempts
  }

  if (digitalRead(OPEN_DOOR_2_SWITCH_PIN) == SWITCH_OPEN2_STATE) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(OPEN_DOOR_2_SWITCH_PIN) == SWITCH_OPEN2_STATE) {
      digitalWrite(MOTOR_PIN3, LOW);
      digitalWrite(MOTOR_PIN4, LOW);
      Serial.println("Door 2 Opened");
      delay(10000);
      Serial.println("Performing 10-second check for Door 2...");
      if (digitalRead(OPEN_DOOR_2_SWITCH_PIN) == SWITCH_OPEN2_STATE) {
        return true;  // Success
      } else {
        return false;  // Failure
      }
    }
  } else {
    digitalWrite(MOTOR_PIN3, HIGH);
    digitalWrite(MOTOR_PIN4, LOW);
    return false;  // Failure
  }
}

int closeDoors() {
  // Initialization of values
  unsigned long currentMillis = millis();
  startMillisDoor1 = currentMillis;
  startMillisDoor2 = currentMillis;

  bool door1LimitReached = true;  // Assume doors are initially at their limit
  bool door2LimitReached = true;

  if (door1Enabled) {
    door1LimitReached = false;  // If door1 is enabled, then it's not yet at its limit
  }

  if (door2Enabled) {
    door2LimitReached = false;  // If door2 is enabled, then it's not yet at its limit
  }

  while (true) {
    currentMillis = millis();

    // Check Door 1
    if (door1Enabled && !door1LimitReached) {
      door1LimitReached = handleDoor1Closing();  // Use the closing method
      if (!door1LimitReached) {
        continue;  // If door 1 is not yet closed, don't proceed to door 2.
      }
    }

    // Reset startMillis for Door 2 before its operation begins
    if (door1LimitReached) {
      startMillisDoor2 = millis();
    }

    // Check Door 2
    if (door2Enabled && !door2LimitReached) {
      door2LimitReached = handleDoor2Closing();  // Use the closing method
      if (!door2LimitReached) {
        continue;  // If door 2 is not yet closed, loop again.
      }
    }
    // Exit condition
    if ((!door1Enabled || door1LimitReached) && (!door2Enabled || door2LimitReached)) {
      Serial.println("All motor functions complete, exiting...");
      return 0;  // Success
    }
  }
}


bool handleDoor1Closing() {
  lastCheckDoor1 = millis();

  if (millis() - startMillisDoor1 > MOTOR_TIMEOUT_DOOR1) {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, LOW);
    Serial.println("Door 1 motor timeout");
    return true;  // Consider the operation as done to prevent further attempts
  }

  if (digitalRead(CLOSE_DOOR_1_SWITCH_PIN) == SWITCH_CLOSE1_STATE) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(CLOSE_DOOR_1_SWITCH_PIN) == SWITCH_CLOSE1_STATE) {
      digitalWrite(MOTOR_PIN1, LOW);
      digitalWrite(MOTOR_PIN2, LOW);
      Serial.println("Door 1 Closed");
      delay(10000);
      Serial.println("Performing 10-second check for Door 1...");
      if (digitalRead(CLOSE_DOOR_1_SWITCH_PIN) == SWITCH_CLOSE1_STATE) {
        return true;  // Success
      } else {
        return false;  // Failure
      }
    }
  } else {
    digitalWrite(MOTOR_PIN1, LOW);
    digitalWrite(MOTOR_PIN2, HIGH);
    return false;  // Failure
  }
}

bool handleDoor2Closing() {
  lastCheckDoor2 = millis();

  if (millis() - startMillisDoor2 > MOTOR_TIMEOUT_DOOR2) {
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, LOW);
    Serial.println("Door 2 motor timeout");
    return true;  // Consider the operation as done to prevent further attempts
  }

  if (digitalRead(CLOSE_DOOR_2_SWITCH_PIN) == SWITCH_CLOSE2_STATE) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(CLOSE_DOOR_2_SWITCH_PIN) == SWITCH_CLOSE2_STATE) {
      digitalWrite(MOTOR_PIN3, LOW);
      digitalWrite(MOTOR_PIN4, LOW);
      Serial.println("Door 2 Closed");
      delay(10000);
      Serial.println("Performing 10-second check for Door 2...");
      if (digitalRead(CLOSE_DOOR_2_SWITCH_PIN) == SWITCH_CLOSE2_STATE) {
        return true;  // Success
      } else {
        return false;  // Failure
      }
    }
  } else {
    digitalWrite(MOTOR_PIN3, LOW);
    digitalWrite(MOTOR_PIN4, HIGH);
    return false;  // Failure
  }
}



void handleBacklight() {
  // Turn off backlight after the timeout
  if (backlightOn && millis() - backlightOnTime > BACKLIGHT_TIMEOUT) {  // 2 minutes
    lcd.noBacklight();
    backlightOn = false;
  }
}

void handleError(String error) {
  // Read error count from EEPROM
  int errorCount = EEPROM.read(ERROR_COUNT_ADDRESS);

  if (errorCount < 3) {
    // Increment error count
    EEPROM.update(ERROR_COUNT_ADDRESS, ++errorCount);

    // Restart the system
    asm volatile("jmp 0");
  } else {
    // If there have been 3 errors already, blink the LCD indefinitely with error message

    lcd.setCursor(0, 0);  // Move the cursor to the start of the first line
    lcd.print(error);
    // Blink the LCD indefinitely
    while (true) {
      lcd.noBacklight();  // turn off the backlight
      delay(500);         // delay for 500ms
      lcd.backlight();    // turn on the backlight
      delay(500);         // delay for 500ms
    }
  }
}


void displayStatus(DateTime now, float temp) {
  char dateString[20];
  sprintf(dateString, "%02d/%02d/%04d %02d:%02d", now.month(), now.day(), now.year(), now.hour(), now.minute());

  if (strcmp(dateString, prevDateString) != 0) {
    lcd.setCursor(0, 0);
    lcd.print(dateString);
    strncpy(prevDateString, dateString, sizeof(prevDateString));
  }

  if (abs(temp - prevTemp) > 0.9) {
    lcd.setCursor(0, 1);
    lcd.print(temp);
    lcd.print("C ");
    prevTemp = temp;
  }

  // Display the door status
  lcd.setCursor(5, 1);  // Move cursor to the bottom line of the LCD
  if (door1Enabled && door2Enabled) {
    lcd.print(" 1+2 ");
  } else if (door1Enabled) {
    lcd.print(" 1 ");
  } else if (door2Enabled) {
    lcd.print(" 2 ");
  } else {
    lcd.print(" 0 ");
  }

  // Display the manualControl status
  lcd.setCursor(9, 1);  // Move cursor to the second line of the LCD
  if (manualControl) {
    lcd.print("M:On ");
  } else {
    lcd.print("M:Off");
  }
}

void openClick() {
  Serial.println("Open button pressed");
  manualControl = true;
  openDoors();
}

void closeClick() {
  Serial.println("Close button pressed");
  manualControl = true;
  closeDoors();
}

void backlightClick() {
  backlightOn = !backlightOn;
  if (backlightOn) {
    backlightOnTime = millis();
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

void backlightLongClick() {
  // Turn on the backlight
  lcd.backlight();

  // Change door mode
  if (door1Enabled && !door2Enabled) {
    // Transition from state 1 to state 2
    door1Enabled = false;
    door2Enabled = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door 2 Enabled");
    Serial.println("Door mode changed: door 2 enabled");
  } else if (!door1Enabled && door2Enabled) {
    // Transition from state 2 to state 3
    door1Enabled = true;
    door2Enabled = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door 1 and Door 2 Enabled");
    Serial.println("Door mode changed: both doors enabled");
  } else if (door1Enabled && door2Enabled) {
    // Transition from state 3 to state 4
    door1Enabled = false;
    door2Enabled = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Both doors disabled");
    Serial.println("Door mode changed: no doors enabled");
  } else if (!door1Enabled && !door2Enabled) {
    // Transition from state 4 to state 1
    door1Enabled = true;
    door2Enabled = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door 1 Enabled");
    Serial.println("Door mode changed: door 1 enabled");
  }
  delay(1500);  // 1.5 second pause
}


// Setup
void setup() {
  // Serial
  Serial.begin(9600);
  Serial.println("Serial setup complete");


  // Wire
  Wire.begin();
  Serial.println("Wire setup complete");

  // RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;  // hang if RTC not found
  }
  Serial.println("RTC setup complete");

  // LCD
  lcd.init();
  lcd.backlight();
  Serial.println("LCD setup complete");

  // Sensors
  sensors.begin();
  Serial.println("Sensors setup complete");

  // Get temperature sensor address
  sensors.getAddress(tempDeviceAddress, 0);
  Serial.println("Temperature sensor address obtained");

  // Adjust RTC
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  Serial.println("RTC adjusted");

  // Setup Pin modes
  setupPinModes();
  Serial.println("Pin modes set");


  // Attach functions to button press events
  openButton.attachLongPressStart(openClick);
  closeButton.attachLongPressStart(closeClick);
  backlightButton.attachDoubleClick(backlightClick);
  backlightButton.attachLongPressStop(backlightLongClick);
  Serial.println("Buttons set up");
}


void loop() {
  DateTime now = rtc.now();

  // Reset manualControl at midnight
  if (now.minute() != lastMinuteChecked) {
    lastMinuteChecked = now.minute();

    if (now.hour() == 0 && !hasResetControl) {
      manualControl = false;
      hasResetControl = true;
    }
  } else if (hasResetControl && now.hour() != 0) {
    hasResetControl = false;
  }

  // Determine if it is currently summer or winter
  bool isSummer = now.month() > SUMMER_START_MONTH && now.month() < SUMMER_END_MONTH;
  // Determine the door open and close times based on the season
  int openHour = isSummer ? SUMMER_OPEN_HOUR : WINTER_OPEN_HOUR;
  int openMinute = isSummer ? SUMMER_OPEN_MINUTE : WINTER_OPEN_MINUTE;
  int closeHour = isSummer ? SUMMER_CLOSE_HOUR : WINTER_CLOSE_HOUR;
  int closeMinute = isSummer ? SUMMER_CLOSE_MINUTE : WINTER_CLOSE_MINUTE;

  // Convert the current time and the opening/closing times into minutes past midnight
  int currentMinutes = now.hour() * 60 + now.minute();
  int openMinutes = openHour * 60 + openMinute;
  int closeMinutes = closeHour * 60 + closeMinute;

  handleBacklight();  // Call the backlight function in loop

  sensors.requestTemperatures();
  float tempC = sensors.getTempC(tempDeviceAddress);
  tempC = roundf(tempC * 10) / 10;



  bool isDoor1Open = door1Enabled && digitalRead(OPEN_DOOR_1_SWITCH_PIN) == SWITCH_OPEN1_STATE;
  bool isDoor1Closed = door1Enabled && digitalRead(CLOSE_DOOR_1_SWITCH_PIN) == SWITCH_CLOSE1_STATE;
  bool isDoor2Open = door2Enabled && digitalRead(OPEN_DOOR_2_SWITCH_PIN) == SWITCH_OPEN2_STATE;
  bool isDoor2Closed = door2Enabled && digitalRead(CLOSE_DOOR_2_SWITCH_PIN) == SWITCH_CLOSE2_STATE;
  bool canOpenDoors = (!door1Enabled || isDoor1Closed) && (!door2Enabled || isDoor2Closed);
  bool canCloseDoors = (!door1Enabled || isDoor1Open) && (!door2Enabled || isDoor2Open);



  openButton.tick();
  closeButton.tick();
  backlightButton.tick();



  displayStatus(now, tempC);

  if (!manualControl) {
    // Check if it's time to open or close the doors and if the operation hasn't been performed yet
    // Also, don't open the doors if the temperature is less than MIN_TEMP
    if (canOpenDoors && !hasOpenedDoors) {
      // If it's the correct time but the temperature is too low, note that we tried to open the doors
      if (now.hour() == openHour && now.minute() == openMinute) {
        if (tempC < MIN_TEMP) {
          triedToOpenDoors = true;
        } else {
          Serial.println("Attempt to open doors");
          int status = openDoors();
          switch (status) {
            case 0:
              Serial.println("Doors opened successfully");
              hasOpenedDoors = true;
              hasClosedDoors = false;  // Reset this flag so doors can be closed later
              triedToOpenDoors = false;
              EEPROM.update(ERROR_COUNT_ADDRESS, 0);
              break;
            case -1:
              handleError("Door 1 open error");
              break;
            case -2:
              handleError("Door 2 open error");
              break;
          }
        }
      } else if (triedToOpenDoors && tempC >= MIN_TEMP) {
        Serial.println("Attempt to open doors");
        int status = openDoors();
        switch (status) {
          case 0:
            Serial.println("Doors opened successfully");
            hasOpenedDoors = true;
            hasClosedDoors = false;  // Reset this flag so doors can be closed later
            triedToOpenDoors = false;
            EEPROM.update(ERROR_COUNT_ADDRESS, 0);
            break;
          case -1:
            handleError("Door 1 open error");
            break;
          case -2:
            handleError("Door 2 open error");
            break;
        }
      }
    } else if (canCloseDoors && now.hour() == closeHour && now.minute() == closeMinute && !hasClosedDoors) {
      Serial.println("Attempt to close doors");
      int status = closeDoors();
      switch (status) {
        case 0:
          Serial.println("Doors closed successfully");
          hasClosedDoors = true;
          hasOpenedDoors = false;  // Reset this flag so doors can be opened later
          EEPROM.update(ERROR_COUNT_ADDRESS, 0);
          break;
        case -1:
          handleError("Door 1 close error");
          break;
        case -2:
          handleError("Door 2 close error");
          break;
      }
    }
  }
}
