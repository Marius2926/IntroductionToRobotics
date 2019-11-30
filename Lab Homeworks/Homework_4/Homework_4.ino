#include <LiquidCrystal.h>
#include <EEPROM.h>
//pins for the led display
const int V0_PIN = 9; // PWN in loc de POTENTIOMETRU
const int RS = 12;
const int enable = 11;
const int d4 = 5;
const int d5 = 4;
const int d6 = 3;
const int d7 = 2;
//pins for the joystick
const int oxPin = A5, oyPin = A4, switchJoystickPin = A3;

//displayType -1 means principal menu, 0 means play, 1 means settings and 2 means HighScore
int displayType = -1;
char startMessage[33];
char playMessage[33];
char settingsMessage[33];
char highScoreMessage[33];
char finishedGame[33];

const int numberOptionsMenu = 3;
const int arrowSelection[3] = {1, 7, 19};
int selectedOption = 0;
bool arrowMoved = false;
bool secondaryArrowMoved = false;
bool tertiaryArrowMoved = false;
bool justStartedGame = true;
bool justChangedLevel = true;
const int distanceBetweenLevels = 5000;
const int gameMaxTime = 10300;
unsigned long gameStartedTime;
unsigned long lastTimeChangedLevel;
const int maxLevel = 15;
int startingLevel = 0;
int actualLevel = 0;
byte lives = 3, actualLives;
int score = 0;
int highScore = 0;
int playMenuPositions[3] = {6, 14, 25};
int settingsMenuPositions[2] = {6, 30};
int highScoreMenuPositions[2] = {0, 11};
int arrowSelectionSettings[2] = {0, 16};
const int numberOptionsSettings = 2;
int selectedOptionSettings = 0;
char alphabet[38] = "_abcdefghijklmnopqrstuvwxyz0123456789";
const int sizeAlphabet = 37;
int playerName[] = {16, 12, 1, 25, 5, 18, 0, 0, 0, 0};
int playerNameSize = 10;
int nameIndex = settingsMenuPositions[0];
bool changingParam = false;
//variables for joystick
int axisX, axisY, switchJoystick;
const int minThreshold = 400, maxThreshold = 600;

const unsigned long debounceTimer = 500;
unsigned long lastTimePressed = 0;

LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);

void setScore(int& score) {
  if (score >= 100) {
    playMessage[playMenuPositions[2]] = score / 100 + '0';
    playMessage[playMenuPositions[2] + 1] = score / 10 % 10 + '0';
    playMessage[playMenuPositions[2] + 2] = score % 10 + '0';
  }
  else {
    if (score >= 10) {
      playMessage[playMenuPositions[2]] = score / 10 + '0';
      playMessage[playMenuPositions[2] + 1] = score % 10 + '0';
    }
    else
      playMessage[playMenuPositions[2]] = score + '0';
  }
}

void setLevel(int& level) {
  if (level >= 10) {
    playMessage[playMenuPositions[1]] = level / 10 + '0';
    playMessage[playMenuPositions[1] + 1] = level % 10 + '0';
  }
  else
    playMessage[playMenuPositions[1]] = level + '0';
}

void setLives(int lives) {
  playMessage[playMenuPositions[0]] = lives + '0';
}

void setStartingLevel(int level) {
  if (level <= 9) {
    settingsMessage[settingsMenuPositions[1]] = level + '0';
    settingsMessage[settingsMenuPositions[1] + 1] = ' ';
  }
  else {
    settingsMessage[settingsMenuPositions[1]] = level / 10 + '0';
    settingsMessage[settingsMenuPositions[1] + 1] = level % 10 + '0';
  }
}

//we receive a codification which we will transform to ascii
void setName(int v[]) {
  for (int i = 0; i < playerNameSize; i++)
    settingsMessage[settingsMenuPositions[0] + i] = alphabet[v[i]];
}

void setNameHighScore(int v[]) {
  for (int i = 0; i < playerNameSize; i++)
    highScoreMessage[highScoreMenuPositions[0] + i] = alphabet[v[i]];
}

void setScoreHighScore(int sc) {
  if (sc >= 100) {
    highScoreMessage[highScoreMenuPositions[1]] = sc / 100 + '0';
    highScoreMessage[highScoreMenuPositions[1] + 1] = sc / 10 % 10 + '0';
    highScoreMessage[highScoreMenuPositions[1] + 2] = sc % 10 + '0';
  }
  else {
    if (sc >= 10) {
      highScoreMessage[highScoreMenuPositions[1]] = sc / 10 + '0';
      highScoreMessage[highScoreMenuPositions[1] + 1] = sc % 10 + '0';
      highScoreMessage[highScoreMenuPositions[1] + 2] = ' ';
    }
    else{
      highScoreMessage[highScoreMenuPositions[1]] = sc + '0';
      highScoreMessage[highScoreMenuPositions[1] + 1] = ' ';
      highScoreMessage[highScoreMenuPositions[1] + 2] = ' ';
    }
  }
}

void setup() {
  lcd.begin(16, 2);
  pinMode(V0_PIN, OUTPUT); // PWN in loc de POTENTIOMETRU
  pinMode(switchJoystickPin, INPUT_PULLUP);
  analogWrite(V0_PIN, 90);
  strcpy(startMessage, " >Play  Settings    HighScore   ");
  strcpy(playMessage, "Lives:  Level:     Score:       ");
  strcpy(highScoreMessage, "          :     Press to reset! ");
  strcpy(settingsMessage, ">Name:           Starting lvl:  ");
  strcpy(finishedGame, "Congrats,you won Press to EXIT. ");
  setLevel(startingLevel);
  setLives(lives);
  setScore(score);
  setStartingLevel(startingLevel);
  setName(playerName);
  //check if it has one highscore saved
  if(EEPROM.read(0) != 255){
    int l, p = 1;
  for(l = 0; l < playerNameSize; l++){
    playerName[l] = EEPROM.read(l);
  }
    while(EEPROM.read(l) != 255){
      highScore = highScore + EEPROM.read(l) * p;
      p *= 10;
      l++;
    }
  }
  setNameHighScore(playerName);
  setScoreHighScore(highScore);
}

void showingMenu(char *message) {
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++)
    lcd.print(message[i]);
  lcd.setCursor(0, 1);
  for (int i = 16; i < 32; i++)
    lcd.print(message[i]);
}

void loop() {
  int offset = 0;
  axisX = analogRead(oxPin);
  axisY = analogRead(oyPin);
  switchJoystick = digitalRead(switchJoystickPin);
  //swtich to find out in which menu we are right now
  switch (displayType) {
    case -1: {
        showingMenu(startMessage);
        if (!arrowMoved) {
          if (axisY > maxThreshold)
            offset = 1;
          else {
            if (axisY < minThreshold)
              offset = -1;
          }
          if (offset != 0) {
            startMessage[arrowSelection[selectedOption]] = ' ';
            selectedOption += offset;
            if (selectedOption == -1)
              selectedOption = numberOptionsMenu - 1;
            if (selectedOption == numberOptionsMenu)
              selectedOption = 0;
            startMessage[arrowSelection[selectedOption]] = '>';
            arrowMoved = true;
          }
        }
        if (axisY >= minThreshold and axisY <= maxThreshold)
          arrowMoved = false;
        //if we select one menu
        if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
          lastTimePressed = millis();
          displayType = selectedOption;
        }
        break;
      }
    case 0: {
        if (justStartedGame) {
          for (int i = playMenuPositions[1]; i < 16; i++)
            playMessage[i] = ' ';
          for (int i = playMenuPositions[2]; i < 32; i++)
            playMessage[i] = ' ';
          gameStartedTime = millis();
          actualLevel = startingLevel;
          score = (startingLevel * (startingLevel + 1)) / 2 * 3;
          actualLives = lives;
          justStartedGame = false;
          justChangedLevel = true;
        }
        else {}
        if (millis() - gameStartedTime > gameMaxTime || actualLevel == maxLevel) {
          showingMenu(finishedGame);
          if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
            lastTimePressed = millis();
            displayType = -1;
            justStartedGame = true;
            //set a new highscore
            if (score > highScore) {
              highScore = score;
              int l;
              for(l = 0; l < playerNameSize; l++)
                EEPROM.write(l,playerName[l]);
              while(score){
                EEPROM.write(l,score%10);
                score /= 10;
                l++;
              }
              setNameHighScore(playerName);
              setScoreHighScore(highScore);
            }
          }
        }
        else {
          setLevel(actualLevel);
          setLives(actualLives);
          setScore(score);
          showingMenu(playMessage);
          if (justChangedLevel) {
            lastTimeChangedLevel = millis();
            justChangedLevel = false;
          }
          if (millis() - lastTimeChangedLevel >= distanceBetweenLevels) {
            actualLevel++;
            score += actualLevel * 3;
            justChangedLevel = true;
          }
        }
        break;
      }
    case 1: {
        showingMenu(settingsMessage);
        if (!arrowMoved && !changingParam) {
          if (axisY < minThreshold)
            displayType = -1;
          else {
            if (axisX > maxThreshold)
              offset = -1;
            else {
              if (axisX < minThreshold)
                offset = 1;
            }
            if (offset != 0) {
              settingsMessage[arrowSelectionSettings[selectedOptionSettings]] = ' ';
              selectedOptionSettings += offset;
              if (selectedOptionSettings == -1)
                selectedOptionSettings = numberOptionsSettings - 1;
              if (selectedOptionSettings == numberOptionsSettings)
                selectedOptionSettings = 0;
              settingsMessage[arrowSelectionSettings[selectedOptionSettings]] = '>';
              arrowMoved = true;
            }
            if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
              lastTimePressed = millis();
              changingParam = true;
              nameIndex = settingsMenuPositions[0];
            }
          }
        }
        if (axisX >= minThreshold && axisX <= maxThreshold)
          arrowMoved = false;
        if (changingParam) {
          if (selectedOptionSettings == 0) {
            lcd.setCursor(nameIndex, 0);
            lcd.print(' ');
            offset = 0;
            if (!secondaryArrowMoved) {
              if (axisY > maxThreshold)
                offset = 1;
              else {
                if (axisY < minThreshold)
                  offset = -1;
              }
              if (offset != 0) {
                nameIndex += offset;
                if (nameIndex == settingsMenuPositions[0] - 1)
                  nameIndex ++;
                if (nameIndex == 16)
                  nameIndex--;
                secondaryArrowMoved = true;
              }
            }
            if (axisY >= minThreshold && axisY <= maxThreshold)
              secondaryArrowMoved = false;
            offset = 0;
            if (!tertiaryArrowMoved) {
              if (axisX > maxThreshold)
                offset = 1;
              else {
                if (axisX < minThreshold)
                  offset = -1;
              }
              if (offset != 0) {
                playerName[nameIndex - settingsMenuPositions[0]] = playerName[nameIndex - settingsMenuPositions[0]] + offset;
                if (playerName[nameIndex - settingsMenuPositions[0]] == -1)
                  playerName[nameIndex - settingsMenuPositions[0]] = sizeAlphabet - 1;
                if (playerName[nameIndex - settingsMenuPositions[0]] == sizeAlphabet)
                  playerName[nameIndex - settingsMenuPositions[0]] = 0;
                setName(playerName);
                tertiaryArrowMoved = true;
              }
            }
            if (axisX >= minThreshold && axisX <= maxThreshold)
              tertiaryArrowMoved = false;
            if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
              lastTimePressed = millis();
              changingParam = false;
            }
          }
          //the selected option is starting level
          else {
            lcd.setCursor(0, selectedOptionSettings);
            lcd.print(' ');
            offset = 0;
            if (!secondaryArrowMoved) {
              if (axisX > maxThreshold)
                offset = 1;
              else {
                if (axisX < minThreshold)
                  offset = -1;
              }
              if (offset != 0) {
                startingLevel += offset;
                if (startingLevel == -1)
                  startingLevel = 0;
                if (startingLevel == maxLevel + 1)
                  startingLevel = maxLevel;
                setStartingLevel(startingLevel);
                secondaryArrowMoved = true;
              }
            }
            if (axisX >= minThreshold && axisX <= maxThreshold) {
              secondaryArrowMoved = false;
            }
            if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
              lastTimePressed = millis();
              changingParam = false;
            }
          }
        }
        break;
      }
    case 2: {
        showingMenu(highScoreMessage);
        if (!arrowMoved)
          if (axisY < minThreshold)
            displayType = -1;
        if (axisY >= minThreshold && axisY <= maxThreshold)
          arrowMoved = false;
        if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
              lastTimePressed = millis();
              for(int i = 0; i < EEPROM.length(); i++)
                EEPROM.update(i,255);
              highScore = 0;
              setNameHighScore(playerName);
              setName(playerName);
              setScoreHighScore(highScore);
        }
        break;
      }
    default: break;
  }
}
