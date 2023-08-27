
# Simple 2-Door Chicken Coop Doors Control System (Arduino-based)

## Overview
Generated with the assistance of GPT-4.

This Arduino-based system allows you to control two separate doors for a chicken coop. The system is designed to provide an automated and manual way to control the opening and closing of the doors, keeping your chickens safe and making your life easier.

## Features

- **Two-Door Operation**: Control two separate doors independently.
- **Real-Time Clock (RTC)**: Uses a real-time clock to perform operations at specific times.
- **Temperature Sensing**: Monitors the temperature outside the coop. Limits door opening on days when colder than preset temp, default is 5 Celsius.
- **LCD Interface**: Provides a user-friendly LCD interface to display information.
- **Manual Override**: Includes physical switches for manual operation.

## Two-Door Operation
![ChickenCoop](\2 Door Chicken COop.jpg)

## Code Description

### Libraries Used
The code makes use of several libraries:
- Wire
- OneWire
- RTClib
- DallasTemperature
- LiquidCrystal_I2C
- OneButton
- EEPROM

### Pin Configuration
Here are the pins used for different components:

- **Motor Pins**: 
  - Door 1: `MOTOR_PIN1`, `MOTOR_PIN2`
  - Door 2: `MOTOR_PIN3`, `MOTOR_PIN4`

- **Switch Pins**:
  - Open Door 1: `OPEN_DOOR_1_SWITCH_PIN`
  - Close Door 1: `CLOSE_DOOR_1_SWITCH_PIN`
  - Open Door 2: `OPEN_DOOR_2_SWITCH_PIN`
  - Close Door 2: `CLOSE_DOOR_2_SWITCH_PIN`

### Two-Door Operation
The code for the two-door operation is the core of the system. The Arduino listens for signals from the switches or the real-time clock to decide whether to open or close each door. The doors can be operated independently of each other, giving you greater flexibility in managing your coop.

## Installation
1. Upload the Arduino code to your board.
2. Connect the hardware as specified in the pin configuration.
3. Power up the system.

## Future Improvements
- currently bugs for timekeeping errors. Unsure if software or hardware related. 
- Implement IoT features to control the doors remotely.
- Add a weather sensor to adapt to environmental conditions.

## Contributing
Feel free to submit issues or pull requests to improve the code or add new features.

## License
MIT

There are two additional files MotorTest and LimitSwitchTest which allow for testing of components before integration. 
