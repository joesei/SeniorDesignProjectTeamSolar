#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <BH1750.h>
#include <SFE_BMP180.h>
#include <Wire.h>


// LDR values
const int top_left = A0;      // green
const int bottom_left = A1;   // yellow
const int top_right = A2;     // blue
const int bottom_right = A3;  // white


// Servo Motors
const int bottom_servo_pin = 2;
const int top_servo_pin = 3;
const int servo_stop = 90;
const int servo_forward = 78;
const int servo_backward = 98; 
const float servo_speed = 46.66;
float current_angle; 
int motor_pos;
Servo bottom_servo;
Servo top_servo;

// LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Light Intensity
BH1750 light_intensity(0x23);
float lux_level;

// Pressure
SFE_BMP180 atmo_pressure;
const float ALTITUDE = 17.0688;

// DHT values
const int DHT_pin = 8;
const int DHT_type = DHT22;
float temperature;
float humidity;
DHT dht(DHT_pin, DHT_type);


void test_angle();



void setup() {
  // Serial Monitor
  Serial.begin(9600);

  // LDR
  pinMode(top_left, INPUT);
  pinMode(bottom_left, INPUT);
  pinMode(top_right, INPUT);
  pinMode(bottom_right, INPUT);

  // Servo
  bottom_servo.attach(bottom_servo_pin);
  top_servo.attach(top_servo_pin);
  bottom_servo.write(servo_stop);
  current_angle = 0;
  motor_pos = 90;

  // DHT 
  dht.begin();
  temperature = 0;
  humidity = 0;

  // LCD 
  lcd.begin(16, 2);
  lcd.backlight();

  //Light Intensity
  Wire.begin();
  light_intensity.begin();

  // Pressure
  atmo_pressure.begin();
  
}




void loop() {
  /* 
  // Sun Tracker
  int ldr1 = analogRead(top_left);
  int ldr2 = analogRead(bottom_left);
  int ldr3 = analogRead(top_right);
  int ldr4 = analogRead(bottom_right);

  if ((ldr1 - ldr2) > 30) {
    if (motor_pos <= 160) {
      ++motor_pos;
      top_servo.write(motor_pos);
    }
  } else if ((ldr1 - ldr2) < -30) {
    if (motor_pos >= 20) {
      --motor_pos;
      top_servo.write(motor_pos);
    }
  }

  if ((ldr1 - ldr3) > 30) {
    if (current_angle <= 170) {
      bottom_servo.write(servo_forward);
      delay(150);
      bottom_servo.write(servo_stop);
      current_angle += servo_speed * .15;
    }
  } else if ((ldr1 - ldr3) < -30) {
    if (current_angle >= -170) {
      bottom_servo.write(servo_backward);
      delay(150);
      bottom_servo.write(servo_stop);
      current_angle -= servo_speed * 0.15;
    }
  }

  // Weather Station
  temperature = dht.readTemperature(true);
  humidity = dht.readHumidity();
  lux_level = light_intensity.readLightLevel();

  char check;
  double temp, pressure, slpressure, altitude;
  check = atmo_pressure.startTemperature();
  if (check != 0){
    delay(check);
    //Get temp from BMP180
    check = atmo_pressure.getTemperature(temp);
    if (check != 0) {
      //Get atmo pressure from BMP180
      check = atmo_pressure.startPressure(3);
      if (check != 0) {
        delay(check);
        check = atmo_pressure.getPressure(pressure, temp);
        if (check != 0) {
          //Getting sea-level pressure
          slpressure = atmo_pressure.sealevel(pressure, ALTITUDE);
          //Getting altitude
          altitude = atmo_pressure.altitude(pressure, slpressure);
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


  // Monitor Testing
  Serial.print("Temp: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Light Intensity: ");
  Serial.println(lux_level);

  
  // LCD
  char str[16];
  char top_str[16] = "T: ";
  char bottom_str[16] = "H: "; 
  char top_p[16] = " P: ";
  char bottom_l[16] = " L: ";
  
  sprintf(str, "%d", (int) temperature);
  strcat(top_str, str);
  char astr[16] = " F";
  strcat(top_str, astr);
  sprintf(str, "%d", (int) humidity);
  strcat(bottom_str, str);
  char bstr[16] = " %";
  strcat(bottom_str, bstr);

  sprintf(str, "%d", (int) pressure);
  strcat(top_p, str);
  strcat(top_str, top_p);

  sprintf(str, "%d", (int) lux_level);
  strcat(bottom_l, str);
  strcat(bottom_str, bottom_l);
  

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(top_str);
  lcd.setCursor(0, 1);
  lcd.print(bottom_str);
  */
  
  bottom_servo.write(servo_forward);
  delay(1500);
  
}



void test_angle() {
  Serial.print("Current Angle: ");
  Serial.println(current_angle);
  bottom_servo.write(servo_forward);
  delay(100);
  current_angle += .1 * servo_speed;
  bottom_servo.write(servo_stop);
  
  
}
