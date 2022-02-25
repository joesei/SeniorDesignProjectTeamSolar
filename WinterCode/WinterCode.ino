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
#include <BH1750.h>
#include <SFE_BMP180.h>

//code assumes our LDRs will be attached to either side of the solar panel, with one on the east side and one on the west side
Servo motor;                   // servo object to control the motor

// LDR values
const int LDRpin_topleft = A0;           // green cable 
const int LDRpin_bottomleft = A1;        // yellow cable
const int LDRpin_topright = A2;          // blue cable
const int LDRpin_bottomright = A3;       // white cable
const int eLDRpin = A0;
const int wLDRpin = A1;

// DHT values
const int kDHTpin = 8;
const int kDHTtype = DHT22;
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

// Address 0x23 and 0x77 for light sensor and pressure sensor (I2C Address)
BH1750 lightIntensity(0x23);

SFE_BMP180 atmoPressure;
#define ALTITUDE 17.0688  //Altitude of Irvine CA in meters 


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
    Serial.println("Motor moving left");
  } else {
    Serial.println("Motor moving right");
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

  //BH1750 lightIntensity Code
  Wire.begin();
  lightIntensity.begin();

  //BMP180 atmoPressure
  
  if (atmoPressure.begin()) {
    Serial.println("BMP180 init success");
  } else {
    Serial.println("BMP180 init fail\n\n");
    Serial.println("Check connection");
    while (1);
  }
  
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

  readDHTvalues(dht);
  delay(1000);
  Serial.print("t: ");
  Serial.println(temperature);
  Serial.print("humidity: ");
  Serial.println(humidity);
  
  float luxLevel = lightIntensity.readLightLevel();
  Serial.print("LightIntensity: ");
  Serial.print(luxLevel);
  Serial.println(" lx");
  delay(1000);

  char check;
  double temp, pressure, slpressure, altitude;

  check = atmoPressure.startTemperature();
  if (check != 0){
    delay(check);
    //Get temp from BMP180
    check = atmoPressure.getTemperature(temp);
    if (check != 0) {
      Serial.print("Temperature: ");
      Serial.print(temp, 2);
      Serial.print(" degrees Celsius,");
      Serial.print((9.0/5.0) * temp + 32.0, 2);
      Serial.print(" degree Farenheit");
  
      //Get atmo pressure from BMP180
      check = atmoPressure.startPressure(3);
      if (check != 0) {
        delay(check);
        check = atmoPressure.getPressure(pressure, temp);
        if (check != 0) {
          Serial.print("Absolute Pressure ");
          Serial.print(pressure, 2);
    
          //Getting sea-level pressure
          slpressure = atmoPressure.sealevel(pressure, ALTITUDE);
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(slpressure, 2);
          Serial.println(" millibars");
    
          //Getting altitude
          altitude = atmoPressure.altitude(pressure, slpressure);
          Serial.print("Current altitude: ");
          Serial.print(altitude, 0);
          Serial.println(" meters");
        } else {
          Serial.println("Error getting pressure measurement");
        }
      } else {
        Serial.println("Error starting pressure measurement");
      }
    } else {
      Serial.println("Error getting temp measurement");
    }
  } else {
    Serial.println("Error starting temp measurement");
  }

  Serial.println();
  delay(10000);


}
  


  
  
   /* ---------------------------------------------------------------------------------------------------------
  //delay(300); //determines how often we want to check the values in the LDRs

  //Display lightIntensity values
  float luxLevel = lightIntensity.readLightLevel();
  Serial.print("LightIntensity: ");
  Serial.print(luxLevel);
  Serial.println(" lx");
  delay(1000);

  //Atmospheric Pressure Section
  Serial.println();
  Serial.print("Altitude of Irvine CA ");
  Serial.print(ALTITUDE,0);
  Serial.print(" meters");

  char check;
  double temp, pressure, slpressure, altitude;

  check = atmoPressure.startTemperature():
  if (check != 0){
    delay(check);
    //Get temp from BMP180
    check = atmoPressure.getTemperature(temp);
    if (check != 0) {
      Serial.print("Temperature: ");
      Serial.print(temp, 2);
      Serial.print(" degrees Celsius,");
      Serial.print((9.0/5.0) * temp + 32.0, 2);
      Serial.print(" degree Farenheit");
  
      //Get atmo pressure from BMP180
      check = atmoPressure.startPressure(3);
      if (check != 0) {
        delay(check);
        check = atmoPressure.getPressure(pressure, temp);
        if (check != 0) {
          Serial.print("Absolute Pressure ");
          Serial.print(pressure, 2);
    
          //Getting sea-level pressure
          slpressure = atmoPressure.sealevel(pressure, ALTITUDE);
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(slpressure, 2);
          Serial.println(" millibars");
    
          //Getting altitude
          altitude = atmoPressure.altitude(pressure, slpressure);
          Serial.print("Current altitude: ");
          Serial.print(altitude, 0);
          Serial.println(" meters");
        } else {
          Serial.println("Error getting pressure measurement");
        }
      } else {
        Serial.println("Error starting pressure measurement");
      }
    } else {
      Serial.println("Error getting temp measurement");
    }
  } else {
    Serial.println("Error starting temp measurement");
  }

  Serial.println();
  delay(10000);
  ---------------------------------------------------------------------------------------------------
  */
  









/*
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h> 

// DHT values
const int kDHTpin = 8;
const int kDHTtype = DHT22;
float temperature;
float humidity;
DHT dht(kDHTpin, kDHTtype);
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

BH1750 lightIntensity(0x23);

SFE_BMP180 atmoPressure;
#define ALTITUDE 17.0688  //Altitude of Irvine CA in meters 

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

Servo servo;
const int servoPin = 2;
const int servoStop = 90;
const int servoForward = 84;
const int servoBackward = 98; 
float servoSpeed = 46.66;
int currentAngle = 0;
// 53 61
// Add safety so it doesnt pass angle 360 or -360
void rotateServo(Servo servo, int angle) {
  int changeAngle = currentAngle - angle;
  int changeTime = abs(changeAngle / servoSpeed);
  if (changeAngle < 0) {
    servo.write(servoBackward);
    delay(changeTime);
  } else if (changeAngle > 0) {
    servo.write(servoForward);
    delay(changeTime);
  }
  servo.write(servoStop);
  currentAngle = angle;
}



const int topleft = A0;      // green
const int bottomleft = A1;   // yellow
const int topright = A2;     // blue
const int bottomright = A3;  // white
void setup() {
  Serial.begin(9600);
  pinMode(topleft, INPUT);
  pinMode(bottomleft, INPUT);
  pinMode(topright, INPUT);
  pinMode(bottomright, INPUT);

  // Servo Motor Code
  servo.attach(servoPin);
  servo.write(servoStop);
  */
  /*
  // DHT Code
  dht.begin();
  temperature = 0;
  humidity = 0;

  // LCD Code
  lcd.begin(16, 2);
  lcd.backlight();

  //BH1750 lightIntensity Code
  Wire.begin();
  lightIntensity.begin();

  //BMP180 atmoPressure
  
  if (atmoPressure.begin()) {
    Serial.println("BMP180 init success");
  } else {
    Serial.println("BMP180 init fail\n\n");
    Serial.println("Check connection");
    while (1);
  }
  */
//}

//void loop() {
  //int ldrStatus1 = analogRead(topleft);
  //int ldrStatus2 = analogRead(bottomleft);
  //int ldrStatus3 = analogRead(topright);
  //int ldrStatus4 = analogRead(bottomright);
/*
  int angle1 = servo.read();
  Serial.print("Servo Angle: ");
  Serial.println(angle1);
  servo.write(servoForward);
  */
  /*
  Serial.print("Servo Going Forward: ");
  Serial.println(servoForward);
  delay(150);
  servo.write(servoStop);
  int angle2 = servo.read();
  Serial.print("Servo Angle: ");
  Serial.println(angle2);
  delay(3000);
  servo.write(servoBackward);
  Serial.print("Servo Going Backward: ");
  Serial.println(servoBackward);
  delay(150);
  servo.write(servoStop);
  int angle3 = servo.read();
  Serial.print("Servo Angle: ");
  Serial.println(angle3);
  delay(3000);
  //servoSpeed = (angle2 - angle1) / 0.1; // angle / second
  Serial.print("Servo Speed: ");
  Serial.println(servoSpeed);
  */
  
  


  

  /*
  // 30 is offset so no constant movement
  if ((ldrStatus1 + ldrStatus2) - (ldrStatus3 + ldrStatus4) > 45) {
    Serial.println("Stepper motor moved left");
  } else if ((ldrStatus1 + ldrStatus2) - (ldrStatus3 + ldrStatus4) < -45){
    Serial.println("Stepper motor moved right");
  } 

  if ((ldrStatus1 + ldrStatus3) - (ldrStatus2 + ldrStatus4) > 45) {
    Serial.println("Servo motor moved up");
  } else if ((ldrStatus1 + ldrStatus3) - (ldrStatus2 + ldrStatus4) < -45){
    Serial.println("Servo motor moved down");
  } 
  
  readDHTvalues(dht);
  float luxLevel = lightIntensity.readLightLevel();
  Serial.print("Lux Level: ");
  Serial.println(luxLevel);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  char check;
  double temp, pressure, slpressure, altitude;

  check = atmoPressure.startTemperature();
  if (check != 0){
    delay(check);
    //Get temp from BMP180
    check = atmoPressure.getTemperature(temp);
    if (check != 0) {  
      //Get atmo pressure from BMP180
      check = atmoPressure.startPressure(3);
      if (check != 0) {
        delay(check);
        check = atmoPressure.getPressure(pressure, temp);
        if (check != 0) {
          Serial.print("Absolute Pressure: ");
          Serial.println(pressure);
    
          //Getting sea-level pressure
          slpressure = atmoPressure.sealevel(pressure, ALTITUDE);
          Serial.print("Relative (Sea-Level) Pressure: ");
          Serial.print(slpressure, 2);
          Serial.println(" millibars");
    
          //Getting altitude
          altitude = atmoPressure.altitude(pressure, slpressure);
          Serial.print("Current Altitude: ");
          Serial.print(altitude);
          Serial.println(" meters");
        } else {
          Serial.println("Error getting pressure measurement");
        }
      } else {
        Serial.println("Error starting pressure measurement");
      }
    } else {
      Serial.println("Error getting temp measurement");
    }
  } else {
    Serial.println("Error starting temp measurement");
  }
  
  
  Serial.print("ldr1: ");
  Serial.println(ldrStatus1);
  Serial.print("ldr2: ");
  Serial.println(ldrStatus2);
  Serial.print("ldr3: ");
  Serial.println(ldrStatus3);
  Serial.print("ldr4: ");
  Serial.println(ldrStatus4);
  Serial.println("------------------");
  
  delay(3500);
  */
  
//}





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
