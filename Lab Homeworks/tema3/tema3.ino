const int pinA = 12;
const int pinB = 8;
const int pinC = 5;
const int pinD = 3;
const int pinE = 2;
const int pinF = 11;
const int pinG = 6;
const int pinDP = 4;
const int pinD1 = 7;
const int pinD2 = 9;
const int pinD3 = 10;
const int pinD4 = 13;

const int segSize = 8;
const int noOfDisplays = 4;
const int noOfDigits = 10;

unsigned long lastMillisPressedSwitch = 0, actualMillisPressedSwitch;
const int pausePress = 1000;
int switchState = HIGH;

// segments array, similar to before
int segments[segSize] = {pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP};

// digits array, to switch between them easily
int digits[noOfDisplays] = {
pinD1, pinD2, pinD3, pinD4
};

byte digitMatrix[noOfDigits][segSize - 1] = {
// a b c d e f g
{1, 1, 1, 1, 1, 1, 0}, // 0
{0, 1, 1, 0, 0, 0, 0}, // 1
{1, 1, 0, 1, 1, 0, 1}, // 2
{1, 1, 1, 1, 0, 0, 1}, // 3
{0, 1, 1, 0, 0, 1, 1}, // 4
{1, 0, 1, 1, 0, 1, 1}, // 5
{1, 0, 1, 1, 1, 1, 1}, // 6
{1, 1, 1, 0, 0, 0, 0}, // 7
{1, 1, 1, 1, 1, 1, 1}, // 8
{1, 1, 1, 1, 0, 1, 1} // 9
};

const int xAxisPin = A0;
const int yAxisPin = A1;
const int switchPin = A2;
const int minThreshold = 400,maxThreshold = 600;

byte actualNumber[4] = {0,0,0,0};
byte selectedDigit[4] = {1,0,0,0};
byte currentDisplay = 0;
bool locked = false;

bool joyMovedAxisX = false, joyMovedAxisY = false;

void displayNumber(byte digit, byte decimalPoint) {
  for (int i = 0; i < segSize - 1; i++) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }
  // write the decimalPoint to DP pin
  digitalWrite(segments[segSize - 1], decimalPoint);
}

// activate the display no. received as param
void showDigit(int num) {
  for (int i = 0; i < noOfDisplays; i++) {
    digitalWrite(digits[i], HIGH);
  }
  digitalWrite(digits[num], LOW);
}

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < segSize - 1; i++){
  pinMode(segments[i], OUTPUT);
  }
for (int i = 0; i < noOfDisplays; i++){
  pinMode(digits[i], OUTPUT);
  }
  pinMode(switchPin , INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
   for(int i = 0; i < 4; i++){
    showDigit(i);
    displayNumber(actualNumber[i],selectedDigit[i]);
    Serial.println(actualNumber[i]);
   }
   for(int i = 0; i < 4; i++)
    digitalWrite(digits[i],LOW);
   int yValue,xValue;
   xValue = analogRead(xAxisPin);
   if(xValue < minThreshold and locked == false and joyMovedAxisX == false){
    selectedDigit[currentDisplay] = 0;
    if(currentDisplay != 3)
      currentDisplay++;
    selectedDigit[currentDisplay] = 1;
    joyMovedAxisX = true;
   }
   else{
    if(xValue > maxThreshold and locked == false and joyMovedAxisX == false){
      selectedDigit[currentDisplay] = 0;
      if(currentDisplay != 0)
        currentDisplay--;
      selectedDigit[currentDisplay] = 1;
      joyMovedAxisX = true;
    }
   }
   switchState = digitalRead(switchPin);
   //it means that the switch is pressed
   if(switchState == LOW){
    actualMillisPressedSwitch = millis();
    if(actualMillisPressedSwitch - lastMillisPressedSwitch > pausePress){
        if(locked == false){
          lastMillisPressedSwitch = actualMillisPressedSwitch;
          locked = true;
        }
        else{
          lastMillisPressedSwitch = actualMillisPressedSwitch;
          locked = false;
        }
   }
  }
  yValue = analogRead(yAxisPin);
//if the button was pressed, lock that digit and allow the user to change the number on it using Y axis of the joystick
   if(locked == true){
      if(yValue < minThreshold and joyMovedAxisY == false){
        joyMovedAxisY = true;
        if(actualNumber[currentDisplay] != 9)
          actualNumber[currentDisplay]++;
      }
      else if(yValue > maxThreshold and joyMovedAxisY == false){
        joyMovedAxisY = true;
        if(actualNumber[currentDisplay] != 0)
          actualNumber[currentDisplay]--;
      }
   }
   if(xValue >= minThreshold and xValue <= maxThreshold)
      joyMovedAxisX = false;
   if(yValue >= minThreshold and yValue <= maxThreshold)
      joyMovedAxisY = false;
   }
