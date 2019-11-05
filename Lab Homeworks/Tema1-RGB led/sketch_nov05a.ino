const int redPin=9,greenPin=10,bluePin=11;
const int redPotentiometer=A0,greenPotentiometer=A1,bluePotentiometer=A2;

void setup() {
pinMode(redPin,OUTPUT);
pinMode(greenPin,OUTPUT);
pinMode(bluePin,OUTPUT);
}

void loop() {
  int redValue,greenValue,blueValue;
  redValue=analogRead(redPotentiometer); //e o valoare intre 0 si 1023
  greenValue=analogRead(greenPotentiometer);
  blueValue=analogRead(bluePotentiometer);
  redValue=map(redValue,0,1023,0,255);
  greenValue=map(greenValue,0,1023,0,255);
  blueValue=map(blueValue,0,1023,0,255);
  analogWrite(redPin,redValue);
  analogWrite(greenPin,greenValue);
  analogWrite(bluePin,blueValue);
  delay(1);
}
