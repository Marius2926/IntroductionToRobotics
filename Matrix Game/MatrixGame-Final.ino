#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <LedControl.h>
char githubLink[79];
byte offsetMenuMessage = 0;
//pins for the led display
const byte V0_PIN = 6; // PWM instead of a potentiometer
const byte RS = 13;
const byte enable = 12;
const byte d4 = 5;
const byte d5 = 4;
const byte d6 = 3;
const byte d7 = 2;
//pins for the joystick
const byte oxPin = A5, oyPin = A4, switchJoystickPin = A3;
//variables for joystick
int axisX, axisY, switchJoystick;
const int minThreshold = 350, maxThreshold = 600;

//displayType -1 means principal menu, 0 means play, 1 means settings and 2 means HighScore
int displayType = -1;
byte whichMessage = 1;
char startMessage[33], playMessage[33], settingsMessage[33], highScoreMessage[33], finishedGameMessage[33], lostGameMessage[33], finishedLevelMessage[49], scrollMessage[33], lostLifeMessage[49]; //strings to keep the message which will be displayed
bool enteredInfoMenuMessage = true;

const byte numberOptionsMenu = 4, arrowSelection[4] = {1, 7, 16, 27}, maxLevel = 5; // number of option in the principal menu and the positions where the arrow should be draw
int selectedOption = 0, selectedOptionSettings = 0;
int selectedOptionInfoMenu = 0;
bool arrowMoved = false, secondaryArrowMoved = false, tertiaryArrowMoved = false, justStartedGame = true, justChangedLevel = true, scrollText = false;
unsigned long gameStartedTime, lastTimeChangedLevel;
int score = 0, highScore = 0;
byte startingLevel, actualLevel, lives = 3, actualLives, githubLinkOffset = 0;

byte playMenuPositions[3] = {6, 14, 25}, settingsMenuPositions[2] = {6, 30}, highScoreMenuPositions[2] = {0, 11}, arrowSelectionSettings[2] = {0, 16}, finishedLevelPosition = 44; //the positions on the lcd where the messages will start
const byte numberOptionsSettings = 2;
char alphabet[38] = "_abcdefghijklmnopqrstuvwxyz0123456789";
const byte sizeAlphabet = 37;
byte playerName[] = {16, 12, 1, 25, 5, 18, 0, 0, 0, 0}; //default player name is : player____
byte playerNameSize = 10;
byte nameIndex = settingsMenuPositions[0];
bool changingParam = false;

const unsigned long debounceTimer = 500;
unsigned long lastTimePressed = 0;

//matrix
LedControl lc = LedControl(11, 10, 9, 1); //DIN,CLK,LOAD,No of drivers
byte levelsMap[maxLevel][8][8] = {  {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 1, 1, 1, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
  },
  { 0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 0, 1, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 1, 1, 1, 0, 1,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 0, 1, 1,
    0, 1, 1, 0, 0, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
  },
  {
    0, 0, 0, 1, 1, 1, 1, 0,
    0, 1, 0, 0, 0, 1, 0, 0,
    0, 1, 1, 0, 1, 0, 0, 1,
    1, 1, 1, 0, 0, 0, 1, 0,
    0, 0, 0, 1, 1, 0, 0, 1,
    0, 1, 0, 0, 0, 1, 0, 1,
    0, 1, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
  },
  {
    0, 0, 0, 0, 0, 1, 0, 0,
    1, 0, 0, 0, 0, 0, 1, 0,
    0, 1, 1, 1, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1
  },
    {
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 0, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1
  }
};
unsigned long startSinking[maxLevel] = {3000, 2500, 3500, 5000, 4000}; //after the specified miliseconds the boat starts sinking
bool specialLevel[maxLevel] = {false, false, false, true, true};
byte specialRow[maxLevel] = {0, 0, 0, 4, 1}, randomColumn = 0, delayBlinkSinkedLevels = 75;
unsigned long delaySinking, timeLastFloorSinked = 0, lastBlinkSinkedLevels = 0, delayRandomEntry = 550, lastTimeRandomEntry = 0;
byte floorNumber = 7, state = 0;
bool sinking = false;
const char helloPlayer[256] PROGMEM =   {0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,  0, 0, 1, 1, 1, 1, 0, 0,  0, 0, 1, 1, 1, 1, 0, 0,
                           0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1, 0,
                           0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  1, 0, 1, 0, 0, 1, 0, 1,  1, 0, 1, 0, 0, 1, 0, 1,
                           0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 0, 0, 0, 1,  1, 0, 0, 0, 0, 0, 0, 1,
                           0, 1, 1, 1, 1, 1, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  1, 0, 1, 0, 0, 1, 0, 1,  1, 0, 0, 1, 1, 0, 0, 1,
                           0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 1, 1, 0, 0, 1,  1, 0, 1, 0, 0, 1, 0, 1,
                           0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  0, 1, 0, 0, 0, 0, 1, 0,  0, 1, 0, 0, 0, 0, 1, 0,
                           0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 1, 1, 1, 1, 0, 0,  0, 0, 1, 1, 1, 1, 0, 0
                          };
const char readySetGo[192] PROGMEM =  {0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 1, 0, 0, 1, 0, 0,  0, 0, 1, 0, 0, 1, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 1, 0, 0,  0, 0, 0, 1, 1, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 1, 0, 0, 0,  0, 0, 1, 0, 1, 0, 0, 0,
                           0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 1, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,
                           0, 0, 1, 0, 0, 0, 1, 0,  0, 1, 1, 1, 1, 1, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,
                           0, 0, 0, 1, 1, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0
                          };
const char* const stringTable[] PROGMEM = {helloPlayer,readySetGo};
int colPosition = 0, infoMessage = 0, jumpPhase = 0, jumpDirection = 0;
bool showInfoMessage = true;
unsigned long pauseStartingMessage = 75, lastTimeEntered = 0, timeBetweenDisplayingPlayer = 130, lastTimeDisplayingPlayer = 0, lastTimeScrollText = 0, delayScrollText = 1400, timeFinishedLevel = 0;

void displayToMatrix(int j, int numberOfColumns, const char* m) {
  for (int i = 0; i < 8; i++)
    for ( int j2 = j; j2 < j + 8; j2++)
      lc.setLed(0, i, j2 - j, pgm_read_byte_near(m + i * numberOfColumns + j2));
//m[i * numberOfColumns + j2]
}

void showMap() {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++)
      lc.setLed(0, i, j, levelsMap[actualLevel][i][j]);
}

struct player {
  int x, y;
  player(): x(0), y(0) {}
  player(int x, int y) : x(x), y(y) {}
  void show() {
    lc.setLed(0, x, y, true);
  }
  void hide() {
    lc.setLed(0, x, y, false);
  }
  bool update(int nx, int ny) {
    if (nx >= 0 && ny >= 0 && nx < 8 && ny < 8 && levelsMap[actualLevel][nx][ny] == 0) {
      hide();
      x = nx;
      y = ny;
      show();
      return true;
    }
    return false;
  }
  bool isOnGround() {
    if (levelsMap[actualLevel][x + 1][y] == 1)
      return true;
    return false;
  }

  bool win() {
    if (x == 0 && isOnGround())
      return true;
    return false;
  }
};
player startingPositions = player(6,0), player1;

LiquidCrystal lcd(RS, enable, d4, d5, d6, d7);

void setScore(int& score) {
  playMessage[playMenuPositions[2]] = ' ';
  playMessage[playMenuPositions[2] + 1] = ' ';
  playMessage[playMenuPositions[2] + 2] = ' ';
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

void setLevel(byte& level) {
  level++;
  if (level >= 10) {
    playMessage[playMenuPositions[1]] = level / 10 + '0';
    playMessage[playMenuPositions[1] + 1] = level % 10 + '0';
  }
  else
    playMessage[playMenuPositions[1]] = level + '0';
  level--;
}

void setLives(int lives) {
  playMessage[playMenuPositions[0]] = lives + '0';
}

void setStartingLevel(byte& level) {
  level++;
  if (level <= 9) {
    settingsMessage[settingsMenuPositions[1]] = level + '0';
    settingsMessage[settingsMenuPositions[1] + 1] = ' ';
  }
  else {
    settingsMessage[settingsMenuPositions[1]] = level / 10 + '0';
    settingsMessage[settingsMenuPositions[1] + 1] = level % 10 + '0';
  }
  level--;
}

//we receive a codification which we will transform to ascii
void setName(byte v[]) {
  for (int i = 0; i < playerNameSize; i++)
    settingsMessage[settingsMenuPositions[0] + i] = alphabet[v[i]];
}

void setNameHighScore(byte v[]) {
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
    else {
      highScoreMessage[highScoreMenuPositions[1]] = sc + '0';
      highScoreMessage[highScoreMenuPositions[1] + 1] = ' ';
      highScoreMessage[highScoreMenuPositions[1] + 2] = ' ';
    }
  }
}

void setLevelWon(byte& level) {
  level++;
  if (level <= 9) {
    finishedLevelMessage[finishedLevelPosition] = level + '0';
    finishedLevelMessage[finishedLevelPosition + 1] = ' ';
  }
  else {
    finishedLevelMessage[finishedLevelPosition] = level / 10 + '0';
    finishedLevelMessage[finishedLevelPosition + 1] = level % 10 + '0';
  }
  level--;
}

void showingMenu(char *message) {
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++)
    lcd.print(message[i]);
  lcd.setCursor(0, 1);
  for (int i = 16; i < 32; i++)
    lcd.print(message[i]);
}

void setup() {
  lcd.begin(16, 2);
  pinMode(V0_PIN, OUTPUT); // PWN in loc de POTENTIOMETRU
  pinMode(switchJoystickPin, INPUT_PULLUP);
  analogWrite(V0_PIN, 90);
  strcpy(githubLink,"https://github.com/Marius2926/IntroductionToRobotics/tree/master/Matrix%20Game");
  strcpy(startMessage, " >Play  Settings HighScore  Info");
  strcpy(playMessage, "Lives:  Level:     Score:       ");
  strcpy(highScoreMessage, "          :     Press to reset! ");
  strcpy(settingsMessage, ">Name:           Starting lvl:  ");
  strcpy(finishedGameMessage, "Congrats,you wonthe game!       ");
  strcpy(finishedLevelMessage, "Congratulations on making it    past level      ");
  strcpy(lostLifeMessage, "You just lost 1 life. Press to  continue        ");
  strcpy(lostGameMessage, " Game over! You did a great job!");
  setLevel(startingLevel);
  setLives(lives);
  setScore(score);
  setStartingLevel(startingLevel);
  setName(playerName);
  //check if it has one highscore saved
  if (EEPROM.read(0) != 255) {
    int l, p = 1;
    for (l = 0; l < playerNameSize; l++) {
      playerName[l] = EEPROM.read(l);
    }
    while (EEPROM.read(l) != 255) {
      highScore = highScore + EEPROM.read(l) * p;
      p *= 10;
      l++;
    }
  }
  setNameHighScore(playerName);
  setScoreHighScore(highScore);
  lc.shutdown(0, false);
  lc.setIntensity(0, 5); // 0 15
  lc.clearDisplay(0);
}

void loop() {
  if (showInfoMessage && millis() - lastTimeEntered >= pauseStartingMessage) {
    switch (infoMessage) {
      case 0:
        displayToMatrix(colPosition, 32, stringTable[0]);
        colPosition++;
        lastTimeEntered = millis();
        if (colPosition == 17)
          showInfoMessage = false;
        break;
      case 1:
        displayToMatrix(colPosition, 24, stringTable[1]);
        colPosition++;
        lastTimeEntered = millis();
        if (colPosition == 17)
          showInfoMessage = false;
        break;
      case 2:
        displayToMatrix(colPosition, 32, stringTable[0]);
        lastTimeEntered = millis();
        showInfoMessage = false;
        break;
    }
  }
  else if (!showInfoMessage) {
    int offset = 0;
    axisX = analogRead(oxPin);
    axisY = analogRead(oyPin);
    switchJoystick = digitalRead(switchJoystickPin);
    //swtich to find out in which menu we are right now
    switch (displayType) {
      case -1: {
          showingMenu(startMessage);
          if (!arrowMoved) {
            if (axisX > maxThreshold)
              offset = 1;
            else {
              if (axisX < minThreshold)
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
          if (axisX >= minThreshold and axisX <= maxThreshold)
            arrowMoved = false;
          //if we select one menu
          if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
            lastTimePressed = millis();
            displayType = selectedOption;
            if (displayType == 0) {
              showInfoMessage = true;
              infoMessage = 1;
              colPosition = 0;
            }
          }
          break;
        }
      case 0:
        if (justStartedGame) {
          gameStartedTime = millis();
          actualLevel = startingLevel;
          score = 0;
          actualLives = lives;
          justStartedGame = false;
          justChangedLevel = true;
          whichMessage = 1;
          scrollText = false;
        }
        if (justChangedLevel) {
            delaySinking = 800 - (actualLevel + 1) * 50;
            floorNumber = 7;
            lastTimeChangedLevel = millis();
            justChangedLevel = false;
            sinking = false;
            showMap(); //draw the map of the actual level on the matrix
            player1 = startingPositions;
            player1.show(); //show the player
            whichMessage = 1;
            scrollText = false;
            if(specialLevel[actualLevel] == 1){
            lc.setLed(0,specialRow[actualLevel],randomColumn,true);
            for(int i = 0; i < 8; i++)
              levelsMap[actualLevel][specialRow[actualLevel]][i] = 1;
            }
          }
        if (player1.win() || player1.x > floorNumber) {
          if (player1.win()) { //if the player wins the level
            if (whichMessage == 1) {
              if (!scrollText) {
                timeFinishedLevel = millis();
                setLevelWon(actualLevel);
                showingMenu(finishedLevelMessage);
                scrollText = true;
                lastTimeScrollText = millis();
              }
              if (scrollText && millis() - lastTimeScrollText >= delayScrollText) {
                strcpy(scrollMessage, finishedLevelMessage + 16);
                showingMenu(scrollMessage);
                whichMessage = 2;
                lastTimeScrollText = millis();
              }
            }
            else {
              if (whichMessage == 2 && millis() - lastTimeScrollText >= delayScrollText) {
                strcpy(scrollMessage, "Press to go to  the next level. ");
                showingMenu(scrollMessage);
                whichMessage = 3;
                lastTimeScrollText = millis();
              } else if (whichMessage == 3 && switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
                lastTimePressed = millis();
                justChangedLevel = true;
                byte actualLevelScore = startSinking[actualLevel] / 1000 + 10 - (timeFinishedLevel - lastTimeChangedLevel) / 1000;
                score += actualLevelScore;
                if(actualLevelScore >= 10){
                  if(actualLives < 3)
                    actualLives++;
                }
                actualLevel++;
                setScore(score);
                floorNumber = 7;//to prevent entering again here
                player1.x = 8;//to prevent entering again here
                whichMessage = 1;
                scrollText = false;
                if(actualLevel == maxLevel){
                  showingMenu(finishedGameMessage);
                  showInfoMessage = true;
                  infoMessage = 0;
                  colPosition = 16;
                  lastTimeScrollText = millis();
                  scrollText = true;
                  justChangedLevel = false; //because the game is finished and we don't have another levels
                  actualLives = 1;
                  player1.x = 10;
                  whichMessage = 1;
                }
              }
            }
          }
          else { //it means that the player is dead
            if (actualLives == 1) { //if he has no more lifes. check his score, update to highscore...
              if (whichMessage == 1) {
                if (!scrollText) {
                  showInfoMessage = true;
                  infoMessage = 2;
                  colPosition = 24;
                  showingMenu(lostGameMessage);
                  scrollText = true;
                  lastTimeScrollText = millis();
                }
                if (scrollText && millis() - lastTimeScrollText >= delayScrollText) {
                  strcpy(scrollMessage,"       ");
                  strcpy(scrollMessage + 7,playMessage + 7);
                  showingMenu(scrollMessage);
                  lastTimeScrollText = millis();
                }
                if (scrollText == true && switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
                          lastTimePressed = millis();
                          whichMessage = 3;
                         }
              }
              else {
                if (score > highScore) {
                  highScore = score;
                  int l;
                  for (l = 0; l < playerNameSize; l++)
                    EEPROM.write(l, playerName[l]);
                  while (score) {
                    EEPROM.write(l, score % 10);
                    score /= 10;
                    l++;
                  }
                  setNameHighScore(playerName);
                  setScoreHighScore(highScore);
                  whichMessage = 2;
                }
                if (whichMessage == 2 && millis() - lastTimeScrollText >= delayScrollText) {
                  strcpy(scrollMessage, "  You made a      highscore!    ");
                  showingMenu(scrollMessage);
                  whichMessage = 4;
                  lastTimeScrollText = millis();
                }
                else {
                  if (whichMessage == 3 && millis() - lastTimeScrollText >= delayScrollText) {
                    strcpy(scrollMessage, "Nice run!You can do better!     ");
                    showingMenu(scrollMessage);
                    whichMessage = 4;
                    lastTimeScrollText = millis();
                  }
                  else {
                    if (whichMessage == 4 && millis() - lastTimeScrollText >= delayScrollText) {
                             strcpy(scrollMessage, "Move JS to left to go home menu.");
                             showingMenu(scrollMessage);
                             whichMessage = 5;
                             lastTimeScrollText = millis();
                    }
                    else {
                      if (axisX < minThreshold) {
                        displayType = -1;
                        justStartedGame = true;
                        showInfoMessage = true;
                        infoMessage = 0;
                        colPosition = 16;
                      }
                      else {
                        if (whichMessage == 5 && millis() - lastTimeScrollText >= delayScrollText) {
                          strcpy(scrollMessage, "Press to restartthe game to play");
                          showingMenu(scrollMessage);
                          whichMessage = 6;
                          lastTimeScrollText = millis();
                        }
                        else if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
                          lastTimePressed = millis();
                          whichMessage = 1;
                          justStartedGame = true; // to force not to enter in this if
                        }
                      }
                    }
                  }
                }
              }
            }
            else {
              if (whichMessage == 1) {
                if (!scrollText) {
                  showInfoMessage = true;
                  infoMessage = 2;
                  colPosition = 24;
                  showingMenu(lostLifeMessage);
                  scrollText = true;
                  lastTimeScrollText = millis();
                }
                if (scrollText && millis() - lastTimeScrollText >= delayScrollText) {
                  lastTimePressed = millis();
                  strcpy(scrollMessage, lostLifeMessage + 16);
                  showingMenu(scrollMessage);
                  whichMessage = 2;
                }
              }
              else if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
                lastTimePressed = millis();
                whichMessage = 1;
                floorNumber = 7; // to force not to enter in this if
                actualLives--;
                justChangedLevel = true;
              }
            }
          }
        }//the player is still playing
        else {
          if(specialLevel[actualLevel] && millis() - lastTimeRandomEntry >= delayRandomEntry && player1.x >= specialRow[actualLevel]){
            levelsMap[actualLevel][specialRow[actualLevel]][randomColumn] = 1;
            lc.setLed(0,specialRow[actualLevel],randomColumn,true);
            randomColumn = random(8);
            lc.setLed(0,specialRow[actualLevel],randomColumn,false);
            levelsMap[actualLevel][specialRow[actualLevel]][randomColumn] = 0;
            lastTimeRandomEntry = millis();
          }
          setLevel(actualLevel);
          setLives(actualLives);
          setScore(score);
          showingMenu(playMessage);
          if (!sinking && millis() - lastTimeChangedLevel >= startSinking[actualLevel]) {
            sinking = true;
            timeLastFloorSinked = millis();
            floorNumber--;
          }
          if (sinking && millis() - timeLastFloorSinked >= delaySinking) {
            timeLastFloorSinked = millis();
            floorNumber--;
          }
          if (millis() - lastBlinkSinkedLevels >= delayBlinkSinkedLevels) {
            lastBlinkSinkedLevels = millis();
            state = 0;
          }
          else state = 255;
          if (sinking)
            lc.setRow(0, floorNumber + 1, state);
          //if the player wants to jump
          if(jumpPhase == 0 && player1.isOnGround()){
            if (axisY < minThreshold){
              jumpPhase = 1;
              jumpDirection = 0;
            }
            else if(axisY > maxThreshold){
              jumpPhase = 2;
              jumpDirection = 0;
            }
          }
         
          if(jumpPhase == 2 || jumpPhase == 1){
            if (axisX > maxThreshold)
              jumpDirection = 1; //if the player wants to jump and then go one cell to the right
            else
              if (axisX < minThreshold)
                jumpDirection = -1; //jump and one cell to the left
          }
          if (jumpPhase == 0) { //moving left,right or going down to the floor if the player is in the air
            if (!player1.isOnGround()) {
              jumpPhase = 3;
              jumpDirection = 0;
            }
            else if (millis() - lastTimeDisplayingPlayer >= timeBetweenDisplayingPlayer) {
              lastTimeDisplayingPlayer = millis();
              if (axisX > maxThreshold)
                player1.update(player1.x, player1.y + 1);
              else if (axisX < minThreshold)
                player1.update(player1.x, player1.y - 1);
            }
          }

          if (jumpPhase != 0 && millis() - lastTimeDisplayingPlayer >= timeBetweenDisplayingPlayer) {
            lastTimeDisplayingPlayer = millis();
            switch (jumpPhase) {
              case 1: //go up a unit
                if (!player1.update(player1.x - 1, player1.y))
                  jumpPhase = 0; //set the jumpPhase to 0, which means the jump finished because there is a block
                else
                  jumpPhase = 2;
                break;
              case 2: //go up another unit
                if (!player1.update(player1.x - 1, player1.y))
                  jumpPhase = 0;
                else
                  jumpPhase = 3; //now we have to go back to the floor
                break;
              case 3: //go back to the floor
                if (jumpDirection == 1)
                  player1.update(player1.x, player1.y + 1);
                else if (jumpDirection == -1)
                  player1.update(player1.x, player1.y - 1);
                jumpDirection = 0;
                if (!player1.update(player1.x + 1, player1.y))
                  jumpPhase = 0;
            }
          }
        }
        break;
      case 1: {
          showingMenu(settingsMessage);
          if (!arrowMoved && !changingParam) {
            if (axisX < minThreshold)
              displayType = -1;
            else {
              if (axisY > maxThreshold)
                offset = -1;
              else {
                if (axisY < minThreshold)
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
          if (axisY >= minThreshold && axisY <= maxThreshold)
            arrowMoved = false;
          if (changingParam) {
            if (selectedOptionSettings == 0) {
              lcd.setCursor(nameIndex, selectedOptionSettings);
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
                  nameIndex += offset;
                  if (nameIndex == settingsMenuPositions[0] - 1)
                    nameIndex ++;
                  if (nameIndex == 16)
                    nameIndex--;
                  secondaryArrowMoved = true;
                }
              }
              if (axisX >= minThreshold && axisX <= maxThreshold)
                secondaryArrowMoved = false;
              offset = 0;
              if (!tertiaryArrowMoved) {
                if (axisY > maxThreshold)
                  offset = -1;
                else {
                  if (axisY < minThreshold)
                    offset = 1;
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
              if (axisY >= minThreshold && axisY <= maxThreshold)
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
                if (axisY > maxThreshold)
                  offset = -1;
                else {
                  if (axisY < minThreshold)
                    offset = 1;
                }
                if (offset != 0) {
                  startingLevel += offset;
                  if (startingLevel == -1)
                    startingLevel = 0;
                  if (startingLevel == maxLevel)
                    startingLevel = maxLevel - 1;
                  setStartingLevel(startingLevel);
                  secondaryArrowMoved = true;
                }
              }
              if (axisY >= minThreshold && axisY <= maxThreshold) {
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
            if (axisX < minThreshold)
              displayType = -1;
          if (axisX >= minThreshold && axisX <= maxThreshold)
            arrowMoved = false;
          if (switchJoystick == LOW and millis() - lastTimePressed >= debounceTimer) {
            lastTimePressed = millis();
            for (int i = 0; i < EEPROM.length(); i++)
              EEPROM.update(i, 255);
            highScore = 0;
            setNameHighScore(playerName);
            setName(playerName);
            setScoreHighScore(highScore);
          }
          break;
        }
     case 3:{
        if(axisX < minThreshold)
          displayType = -1;
        if(enteredInfoMenuMessage){
          lcd.clear();
          enteredInfoMenuMessage = false;
          lcd.setCursor(0,0);
          lcd.print(" Marius Chitac   ");
        }
        if(millis() % 401 == 0 && selectedOptionInfoMenu == 0){
          lcd.setCursor(1,1);
          for(int j = offsetMenuMessage; j < offsetMenuMessage + 16; j++)
            lcd.print(githubLink[j % 78]);
          offsetMenuMessage++;
        }
        if(axisY > maxThreshold && selectedOptionInfoMenu == 0)
          offset = 1;
        else  if(axisY < minThreshold && selectedOptionInfoMenu ==1)
                offset = -1;
        if(offset != 0){
          selectedOptionInfoMenu += offset;
          if(selectedOptionInfoMenu == 1){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" Escape Titanic ");
            lcd.setCursor(0,1);
            lcd.print(" @UnibucRobotics");
          }
          else{
            offsetMenuMessage = 0;
            enteredInfoMenuMessage = true;
          }
          offset = 0;
        }
        break;
     }
      default: break;
    }
  }
}
