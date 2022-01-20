/*
 * TODO:
 *    Modes:
 *        Testing: LDRs and Motor used every 200ms
 *        Real: LDRs and Motor used every > 5 mins (placed in sleep mode or low power mode)
 * 
 * 
 * 
 */
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <DHT.h>

//code assumes our LDRs will be attached to either side of the solar panel, with one on the east side and one on the west side
Servo motor;                   // servo object to control the motor

// LDR values
const int LDRpin_topleft = A0;              // assign pins to LDRs and motor
const int LDRpin_bottomleft = A1;
const int LDRpin_topright = A2;
const int LDRpin_bottomright = A3;
const int eLDRpin = A0;
const int wLDRpin = A1;

// DHT values
const int kDHTpin = 8;
const int kDHTtype = DHT11;
float temperature;
float humidity;


// Servo values
int servoPin = 3;
int eLDRvalue = 0;
int wLDRvalue = 0;
int calibration = 0;           // LDRs don't come with the same inital value, so we have to figure out what the difference between the inital values are and save it here
int offset = 0;                // difference between the gathered values of the LDRs
int motorPos = 90;             // starting angle of the servo motor


// Stepper Motor values
const int kSteps = 64;         // Possible max = 64 * 48
const int kStepperSpeed = 5;   // RPM

DHT dht(kDHTpin, kDHTtype);

Stepper stepper_motor(kSteps, 4, 5, 6, 7);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);


// Rotate the servo (top/down) in small increments to avoid fast changes that may impact the hardware
void rotateServo(Servo motor, int angle) {
  int current_angle = motor.read();
  int difference = current_angle - angle;
  int change = (difference > 0) ? -1 : 1;
  for (int i = 0; i < abs(difference); i++) {
    motor.write(current_angle + change + (i * change));
    delay(50);
  }
}

// Rotate stepper motor (left/right) in small increment
void rotateStepper(Stepper motor) {

  int topleft = analogRead(LDRpin_topleft);
  int bottomleft = analogRead(LDRpin_bottomleft);
  int topright = analogRead(LDRpin_topright);
  int bottomright = analogRead(LDRpin_bottomright);

  int steps = 0;
  
  if (topleft + bottomleft > topright + bottomright) {
    steps++;
  } else {
    steps--;
  }
  
  motor.step(kStepperSpeed);
}


void readDHTvalues(DHT dht) {
  
  temperature = dht.readTemperature(true);
  humidity = dht.readHumidity();
  
  if (isnan(temperature)) {
    Serial.println("Failed to read temperature");
  }

  if (isnan(humidity)) {
    Serial.println("Failed to read humidity");
  }
  
}


void setup() {

  // LCD Code
  lcd.begin(16, 2);
  lcd.backlight();
  
  Serial.begin(9600);

  // Servo Motor Code
  motor.attach(servoPin);
  pinMode(eLDRpin, INPUT);
  pinMode(wLDRpin, INPUT);
  
  //motor.write(motorPos);
  rotateServo(motor, motorPos);

  // Stepper Motor Code
  stepper_motor.setSpeed(5);

  // DHT Sensor Code
  dht.begin();
  temperature = 0;
  humidity = 0;
  
  
}

void LDRtrack() { //function for tracking the sun using values gathered from the LDR and moving the motor
  eLDRvalue = analogRead(eLDRpin) + calibration;   
  eLDRvalue = (int)(138 + 0.9 * eLDRvalue);  // new calibration equation
  wLDRvalue = analogRead(wLDRpin);
  offset = eLDRvalue - wLDRvalue;

  // Print LDR values into LCD -----------
  char str[16];
  char estr[16] = "East LDR: ";
  char wstr[16] = "West LDR: "; 
  sprintf(str, "%d", eLDRvalue);
  strcat(estr, str);
  sprintf(str, "%d", wLDRvalue);
  strcat(wstr, str);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(estr);
  lcd.setCursor(0, 1);
  lcd.print(wstr);
  // -------------------------------------

  Serial.print("eldr: ");
  Serial.println(eLDRvalue);
  Serial.print("wldr: ");
  Serial.println(wLDRvalue);
  
  if (offset > 30) { //if error is positive, east LDR's value is too high, so angle the solar panel to the east (value is 20 so the motor isn't constantly moving around)
    Serial.print("Motor Pos: ");
    Serial.println(motorPos);
    if (motorPos <= 160) { //check if motor is at the furthest angle east
      Serial.println("Motor moved positive");
      Serial.println(motorPos);
      //motorPos++;
      //motor.write(motorPos); //angle motor towards east
      rotateServo(motor, ++motorPos);
      
    }
  }
  else if (offset < -30) { //if error is negative, west LDR's value is too high, so angle the solar panel to the west
    if (motorPos >= 20) { //check if motor is at the furthest angle west
      Serial.println("Motor moved negative");
      Serial.println(motorPos);
      //motorPos--;
      //motor.write(motorPos); //angle motor towards west
      rotateServo(motor, --motorPos);
    }
  }
  
}

void loop() {
  LDRtrack();
  delay(300); //determines how often we want to check the values in the LDRs
}















/*
const int ldrPin1 = A0;
const int ldrPin2 = A1;
const int ldrPin3 = A2;
const int ldrPin4 = A3;

void setup() {

  Serial.begin(9600);

  pinMode(ldrPin1, INPUT);
  pinMode(ldrPin2, INPUT);
  pinMode(ldrPin3, INPUT);
  pinMode(ldrPin4, INPUT);
}

void loop() {

  int ldrStatus1 = analogRead(ldrPin1);
  int ldrStatus2 = analogRead(ldrPin2);
  int ldrStatus3 = analogRead(ldrPin3);
  int ldrStatus4 = analogRead(ldrPin4);

  Serial.print("ldr1: ");
  Serial.println(ldrStatus1);
  Serial.print("ldr2: ");
  Serial.println(ldrStatus2);
  Serial.print("ldr3: ");
  Serial.println(ldrStatus3);
  Serial.print("ldr4: ");
  Serial.println(ldrStatus4);
  Serial.println("------------------");
  delay(2000);
  
}
*/




/*
// Include the Servo library 
#include <Servo.h> 
// Declare the Servo pin 
int servoPin = 3; 
// Create a servo object 
Servo Servo1; 
void setup() { 
   // We need to attach the servo to the used pin number 
   Servo1.attach(servoPin); 
   Serial.begin(9600);
}
void loop(){ 
   // Make servo go to 0 degrees 
   delay(2000); 
   Servo1.write(0); 
   Serial.println("0 sweep");
   delay(2000); 
   // Make servo go to 90 degrees 
   Servo1.write(90);
   Serial.println("90 sweep");
   delay(2000); 
   // Make servo go to 180 degrees 
   Servo1.write(160); 
   Serial.println("160 sweep");
   
}
*/
