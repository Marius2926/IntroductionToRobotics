#include <AFMotor.h>
#include <LiquidCrystal.h>
//declaring the motors
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

//initializing the lcd
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

unsigned int xAxis, yAxis;
unsigned int  x = 0;
unsigned int  y = 0;
unsigned int productId, hasOrder = 0;
//hasOrder is different than 0 when the client presses a button.
//productId knows which product is mapped to that value
bool orderInProgress = false;
unsigned long orderStarted = 0, maxOrderWait = 60000

;

unsigned int motorSpeedA = 0;
unsigned int motorSpeedB = 0;

void setup() {
  pinMode(2,OUTPUT);
  digitalWrite(2,LOW);
  lcd.begin(16, 2);
  motor1.run(RELEASE); //sets the speed to 0 and removes power from the motor
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
  Serial.begin(38400);// 38400 is the default communication rate of the Bluetooth module
  lcd.print("You can order");
}

void loop() {
  // Default value - no movement when the Joystick stays in the center
  x = 510 / 4;
  y = 510 / 4;
  productId = 0;
  // Read the incoming data from the Joystick, or the master Bluetooth device
  while (Serial.available() >= 2) {
    x = Serial.read();
    delay(10);
    y = Serial.read();
  }

  if(millis() - orderStarted >= maxOrderWait && orderInProgress){
    orderInProgress = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("You can order");
  }
  if (Serial.available() && orderInProgress == false) {
    hasOrder = Serial.read();
    if (hasOrder > 0 && hasOrder < 5) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Produs: ");
      lcd.print(hasOrder);
      productId = hasOrder;
      orderInProgress = true;
      orderStarted = millis();
    }
  }
    xAxis = x * 4;
    yAxis = y * 4;
  if (xAxis < 470 || xAxis > 550 || yAxis < 470 || yAxis > 550) {
    if (yAxis < 470) {
      //backwards
      // Convert the declining Y-axis readings for going backward from 470 to 0 into 0 to 255 value for the PWM signal for increasing the motor speed
      motorSpeedA = map(yAxis, 470, 0, 0, 255);
      motorSpeedB = map(yAxis, 470, 0, 0, 255);
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }
    else {
      if (yAxis > 550) {
        //forward
        // Convert the increasing Y-axis readings for going forward from 550 to 1023 into 0 to 255 value for the PWM signal for increasing the motor speed
        motorSpeedA = map(yAxis, 550, 1023, 0, 255);
        motorSpeedB = map(yAxis, 550, 1023, 0, 255);
        motor1.run(BACKWARD);
        motor2.run(BACKWARD);
        motor3.run(BACKWARD);
        motor4.run(BACKWARD);
      } else {
        motorSpeedA = 0;
        motorSpeedB = 0;
      }
    }
    // X-axis used for left and right control
    if (xAxis < 470) {
      // Convert the declining X-axis readings from 470 to 0 into increasing 0 to 255 value
      int xMapped = map(xAxis, 470, 0, 0, 255);
      // Move to left - decrease left motor speed, increase right motor speed
      motorSpeedA = motorSpeedA - xMapped;
      motorSpeedB = motorSpeedB + xMapped;
      // Confine the range from 0 to 255
      if (motorSpeedA < 0) {
        motorSpeedA = 0;
      }
      if (motorSpeedB > 255) {
        motorSpeedB = 255;
      }
    }

    if (xAxis > 550) {
      int xMapped = map(xAxis, 550, 1023, 0, 255);
      // Move right - decrease right motor speed, increase left motor speed
      motorSpeedA = motorSpeedA + xMapped;
      motorSpeedB = motorSpeedB - xMapped;
      // Confine the range from 0 to 255
      if (motorSpeedA > 255) {
        motorSpeedA = 255;
      }
      if (motorSpeedB < 0) {
        motorSpeedB = 0;
      }
    }
    if (motorSpeedA < 70) {
      motorSpeedA = 0;
    }
    if (motorSpeedB < 70) {
      motorSpeedB = 0;
    }
    motor1.setSpeed(motorSpeedA);
    motor2.setSpeed(motorSpeedB);
    motor3.setSpeed(motorSpeedB);
    motor4.setSpeed(motorSpeedA);
  }
}
