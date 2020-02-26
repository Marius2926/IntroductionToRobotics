#include <SoftwareSerial.h>
unsigned int xAxis, yAxis;
const int xPin = A0, yPin = A1;
const int numberOrders = 4, ordersPin[numberOrders] = {2, 3, 4, 5};
char commandNumber;
bool orderInProgress = false;
unsigned long orderStarted = 0, maxOrderWait = 60000;

void setup() {
  Serial.begin(38400); // 38400 is the default communication rate of the Serial module
  pinMode(ordersPin[0], INPUT_PULLUP);
  pinMode(ordersPin[1], INPUT_PULLUP);
  pinMode(ordersPin[2], INPUT_PULLUP);
  pinMode(ordersPin[3], INPUT_PULLUP);
}

void loop() {
  commandNumber = 0;
  if (digitalRead(ordersPin[0]) == LOW)
    commandNumber = 1;

  else {
    if (digitalRead(ordersPin[1]) == LOW)
      commandNumber = 2;
    else {
      if (digitalRead(ordersPin[2]) == LOW)
        commandNumber = 3;
      else if (digitalRead(ordersPin[3]) == LOW)
        commandNumber = 4;
    }
  }

  if(millis() - orderStarted >= maxOrderWait && orderInProgress)
    orderInProgress = false;
  
  xAxis = analogRead(xPin);
  yAxis = analogRead(yPin);
  if(!orderInProgress && commandNumber > 0){
    Serial.write(commandNumber);
    orderInProgress = true;
    orderStarted = millis();
  }
  if ((xAxis < 470 || xAxis > 550 || yAxis < 470 || yAxis > 550) && orderInProgress == true) {
    // Send the values divided by 4 because only 1 byte can be sent at one moment via the serial port to the slave HC-05 Serial devicexAxis = analogRead(xPin) / 4;
    yAxis /= 4;
    xAxis /= 4;
    Serial.write(xAxis);
    Serial.write(yAxis);
    delay(20);
    //Serial.flush();
  }
}
