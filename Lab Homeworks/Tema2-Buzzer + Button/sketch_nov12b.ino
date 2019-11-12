#include "pitches.h"
const int knockDetectPin = A0;
const int singPin = 9;
const int threshold = 1;
int speakerValue = 0;
unsigned long lastKnockMillis = 0,timeToSing;
const unsigned long diff = 5000;
bool haveToSing;

const int buttonPin = 7;
int stateButton = 0;

/*song*/
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,
 
  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,
 
  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,
 
  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,
 
  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};
//Mario main them tempo
int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
 
  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

void setup() {
  // put your setup code here, to run once:
  pinMode(knockDetectPin,INPUT);
  pinMode(buttonPin,INPUT_PULLUP);
  Serial.begin(9600);
}

void playSong(bool& haveToSing){
 unsigned long lastNotePlayedMillis = millis(),actualNotePlayedMillis;
 int numberNotes = sizeof(melody)/sizeof(int);
 int i,durationNote, pauseNoteMillis;
 for(i = 0; i < numberNotes; i++){
   if(digitalRead(buttonPin) == LOW){
    i = numberNotes;
    haveToSing = false;
   }
   else{
   durationNote = 1000 / tempo[i];
   pauseNoteMillis = durationNote * 1.30;
   actualNotePlayedMillis = millis();
   if(actualNotePlayedMillis - lastNotePlayedMillis < pauseNoteMillis)
      i--;
   else{
        tone(singPin,melody[i],durationNote); 
        lastNotePlayedMillis = actualNotePlayedMillis;
      }
   }
 }
}

void loop() {
  // put your main code here, to run repeatedly:
  speakerValue = analogRead(knockDetectPin);
  if(speakerValue >= 1 ){
      lastKnockMillis = millis();
      haveToSing = true;
      Serial.println(speakerValue);
  }
  timeToSing = millis();
  if(timeToSing - lastKnockMillis >= diff and haveToSing){
    playSong(haveToSing);
  }
}
