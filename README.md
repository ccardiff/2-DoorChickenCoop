Project Description: Simple Chicken Coop Two Door Controller

Overview
This code was generated with the assistance of ChatGPT-4. 

The 2-Door Simple Chicken Coop program is designed to automate the opening and closing of two doors in a chicken coop. The script incorporates various hardware components such as temperature sensors, real-time clocks, LCD displays, and motors. The code is modular and makes use of several libraries for seamless integration of these components. Please use the builds of other creators found on this site. This code is unique in that it: 

-allows for control of two doors independently. One button can cycle through Door 1 only, Door 2 only, Door 1+Door 2, and neither Door 1+2 
-uses a minimum temperature to keep chickens indoors when cold. This variable can be easily changed as a universal variable at the top of the code. 
-uses a simple Time Open and Time Closed time-based control that can be user set. Currently two season mode settings. 


Features
Temperature Sensing: Measures the temperature outside the chicken coop using a DallasTemperature sensor to prevent chickens from exiting coop if too cold. Can modify universal value at top of code. 

Real-Time Clock: Utilizes an RTClib library to keep track of time for scheduled door operations. 

LCD Display: Displays relevant information on an LCD screen using the LiquidCrystal_I2C library. 

Motor Control: Operates motors to open and close doors using GPIO pins. 

User Input: Allows manual control of doors via push-buttons using the OneButton library. 

Persistent Storage: Saves settings and other data in EEPROM for retrieval after resets. 

Limitations and Future Work
The code works as intended but there are some errors that occur intermittently. Currently investigating whether hardware or software related. 

The current code does not handle edge cases like sensor failure or motor jamming. 

Future iterations may include additional features like remote control via Wi-Fi, or integration with a home automation system. 

Additional Files:

LimitSwitches: Allows you to test limit switches required to inform program if door has reached open or closed position without full program. 
MotorTest: Allows you to test the funcionality and polarity of the motors before integrating into main program

