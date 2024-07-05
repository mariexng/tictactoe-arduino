// install Adafruit TouchScreen & Adafruit tftldc & mcufriend_kbv library 
#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
MCUFRIEND_kbv tft;
#include <stdio.h>

#define MINPRESSURE 100
#define MAXPRESSURE 1000
#define BLACK     0x0000
#define WHITE     0xFFFF
#define PINK      0xFC95
#define LIGHTPINK 0xFDB8
#define PURPLE    0x6990
#define ORCHID    0xDB9A
#define BLUE      0x1D58
#define LIGHTBLUE 0xBF3A

// COPY-PASTE from Serial Terminal: (Screen Values)
const int XP=6,XM=A2,YP=A1,YM=7; //240x320 ID=0x9341
const int TS_LEFT=959,TS_RT=216,TS_TOP=924,TS_BOT=190;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

const char PLAYER_X = 'x';
const char PLAYER_O = 'o';
const int HOMESCREEN_MODE = 0;
const int ONE_PLAYER_MODE = 1;
const int TWO_PLAYER_MODE = 2;

int pixel_x, pixel_y;
Adafruit_GFX_Button btnOnePlayer, btnTwoPlayer;
char currentPlayerSign;

char gameBoard[3][3];
struct Position {
  int row;
  int col;
};
const Position dictionary[9] = {
  {0, 0}, {0, 1}, {0, 2},
  {1, 0}, {1, 1}, {1, 2},
  {2, 0}, {2, 1}, {2, 2}
};

void setup() {
  Serial.begin(9600);
  int ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1);
  drawHomeScreen("Tic Tac Toe");
  initializeBoard();
}

void loop() {
  int gamemode = HOMESCREEN_MODE;
  while (gamemode == HOMESCREEN_MODE){
    bool btnPressed = touchGetXY();
    btnOnePlayer.press(btnPressed && btnOnePlayer.contains(pixel_x,pixel_y));
    btnTwoPlayer.press(btnPressed && btnTwoPlayer.contains(pixel_x,pixel_y));

    if (btnOnePlayer.justPressed()){
      Serial.println("1 Player Mode");
      gamemode = ONE_PLAYER_MODE;
    }
    else if (btnTwoPlayer.justPressed()){
      Serial.println("2 Player Mode");
      gamemode = TWO_PLAYER_MODE;
    }
  }
  if (gamemode == ONE_PLAYER_MODE){
    runGame(false);
  }
  else if (gamemode == TWO_PLAYER_MODE){
    runGame(true);
  }
}

void runGame(bool twoPlayer){
  initializeBoard();
  drawGrid();
  delay(500); // wait so player doesnt sets x to field on accident

  currentPlayerSign = PLAYER_X;
  int turnCount = 0;

  while (turnCount < 9 && !isWin()){
    if (!twoPlayer && currentPlayerSign == PLAYER_O){
      delay(800);
      moveAI();
    }
    else{
      movePlayer();
      delay(550); //to avoid accidently setting two fields
    }
    printBoard();
    turnCount++;
    if (!isWin()){
      currentPlayerSign = (currentPlayerSign == 'x') ? 'o' : 'x';
    }
  }
  delay(2000);
  if (isWin()){
    drawHomeScreen("  " + String(currentPlayerSign) + " wins!");
  }
  else{
    drawHomeScreen("its a draw!");
  }
}

void initializeBoard() {  // set an empty gamefield
  memset(gameBoard, ' ', sizeof(gameBoard));
}

bool touchGetXY(void){
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed)
    { //Update mapping from calibration
      pixel_x = map(p.y, 959, 216, 0, 320);
      pixel_y = map(p.x, 924, 190, 0, 240);
    }
    return pressed;
}

void drawHomeScreen(String caption){
  tft.fillScreen(BLACK);
  tft.setCursor(60,30);
  tft.setTextSize(3);
  tft.print(caption);
  btnOnePlayer.initButton(&tft, 160, 120, 100, 40, PINK, PINK, WHITE, "1 Player", 2);
  btnOnePlayer.drawButton();
  btnTwoPlayer.initButton(&tft, 160, 200, 100, 40, PURPLE, PURPLE, WHITE, "2 Player", 2);
  btnTwoPlayer.drawButton();
}

void drawGrid(){
  // 320x 240y -> 80x80 Boxes
  tft.fillScreen(BLACK);
  for (int i=0; i < 5; i++){
    // horizontal
    tft.drawLine (40, 77.5+i, 280, 77.5+i, ORCHID);
    tft.drawLine (40, 157.5+i, 280, 157.5+i, ORCHID);
    // vertical
    tft.drawLine(117.5+i, 5, 117.5+i, 235, LIGHTPINK);
    tft.drawLine(197.5+i, 5, 197.5+i, 235, LIGHTPINK);
  }
}

void drawXO(int x, int y, char sign){
  if (sign == PLAYER_O){
    tft.drawCircle(x, y, 20, BLUE);
  }
  else{
    tft.drawLine(x - 10, y - 10, x + 10, y + 10, LIGHTBLUE);
    tft.drawLine(x + 10, y - 10, x - 10, y + 10, LIGHTBLUE);
  }
}

void drawMove(int pos){
  switch(pos){
    case 0: drawXO(75, 40, currentPlayerSign); break;
    case 1: drawXO(160, 40, currentPlayerSign); break;
    case 2: drawXO(245, 40, currentPlayerSign); break;
    case 3: drawXO(75, 120, currentPlayerSign); break;
    case 4: drawXO(160, 120, currentPlayerSign); break;
    case 5: drawXO(245, 120, currentPlayerSign); break;
    case 6: drawXO(75, 200, currentPlayerSign); break;
    case 7: drawXO(160, 200, currentPlayerSign); break;
    case 8: drawXO(245, 200, currentPlayerSign); break;
  }
}

void drawGameOverScreen(String winner){
  tft.fillScreen(BLACK);
  tft.setCursor(60,30);
  tft.setTextSize(3);
  tft.print("Game Over. "+ winner + "wins.");
}

void setMove(int pos){
  gameBoard[dictionary[pos].row][dictionary[pos].col] = currentPlayerSign;
  drawMove(pos);
}

bool isFieldEmpty(int pos){
  return gameBoard[dictionary[pos].row][dictionary[pos].col] == ' ';
}

bool isEquals(char a, char b, char c){ // check if 3 fields are equal and not empty
    if(a == b && b == c && a != ' '){
        return true;
    }
    return false;
}

bool isWin(){ // check if one player has won
  // check horizontal and vertical lines
  for(int i = 0; i < 3; i++){
    if (isEquals(gameBoard[i][0], gameBoard[i][1], gameBoard[i][2]) ||
        isEquals(gameBoard[0][i], gameBoard[1][i], gameBoard[2][i])) {
        return true;
    }
  }
  // check diagonal lines
  return (isEquals(gameBoard[0][0], gameBoard[1][1],gameBoard[2][2]) ||
          isEquals(gameBoard[0][2], gameBoard[1][1],gameBoard[2][0]));
}

void printBoard(){
  for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++){
        Serial.print(gameBoard[i][j]);
        Serial.print("|");
      }
      Serial.println();
  }
}

void movePlayer(){
  bool validMove = false;
  Serial.println(String(currentPlayerSign) + " turn");
  do{
    bool screenPressed = touchGetXY();
    if (screenPressed){
      Serial.println("X: " + String(pixel_x) + " Y: "  + String(pixel_y));
      if ((pixel_x < 120) && (pixel_y < 80) && isFieldEmpty(0)){
          Serial.print("f0");
          setMove(0);
          validMove = true;
        }
      else if ((pixel_x > 120) && (pixel_x < 200) && (pixel_y < 80) && isFieldEmpty(1)){
        setMove(1);
        validMove = true;
      }
      else if ((pixel_x > 200) && (pixel_y < 80) && isFieldEmpty(2)){
        setMove(2);
        validMove = true;
      }
      else if ((pixel_x < 120) && (pixel_y < 160) && (pixel_y > 80) && isFieldEmpty(3)){
        setMove(3);
        validMove = true;
      }
      else if ((pixel_x > 120) && (pixel_x < 200) && (pixel_y < 160) && (pixel_y > 80) && isFieldEmpty(4)){
        setMove(4);
        validMove = true;
      }
      else if ((pixel_x > 200) && (pixel_y < 160) && (pixel_y > 80) && isFieldEmpty(5)){
        setMove(5);
        validMove = true;
      }
      else if ((pixel_x < 120) && (pixel_y > 160) && isFieldEmpty(6)){
        setMove(6);
        validMove = true;
      }
      else if ((pixel_x > 120) && (pixel_x < 200) && (pixel_y > 160) && isFieldEmpty(7)){
        setMove(7);
        validMove = true;
      }
      else if ((pixel_x > 200) && (pixel_y > 160) && isFieldEmpty(8)){
        setMove(8);
        validMove = true;
      }
    }
  }while(!validMove);
}

void moveAI(){
  int winAI = ableToWin(PLAYER_O);
  int winPlayer = ableToWin(PLAYER_X);

  if (winAI != -1){
    setMove(winAI);
  }
  else if(winPlayer != -1){
    setMove(winPlayer);
  }
  else{
    bool validMove = false;
    while (!validMove){
      int move = random(0, 9);
      if (isFieldEmpty(move)){
        setMove(move);
        validMove = true;
      }
    }
  }
}

int ableToWin(char sign){ //checks if any sign is able to win in the next turn
  for (int i = 0; i < 3; i++){
    // check rows
    if (gameBoard[i][1] == sign && gameBoard[i][2] == sign && isFieldEmpty(i * 3)) return i * 3;
    else if (gameBoard[i][0] == sign && gameBoard[i][2] == sign && isFieldEmpty(i * 3 + 1)) return i * 3 + 1;
    else if (gameBoard[i][0] == sign && gameBoard[i][1] == sign && isFieldEmpty(i * 3 + 2)) return i * 3 + 2; 
    // check columns
    else if (gameBoard[1][i] == sign && gameBoard[2][i] == sign && isFieldEmpty(i)) return i;
    else if (gameBoard[0][i] == sign && gameBoard[2][i] == sign && isFieldEmpty(3 + i)) return i + 3;
    else if (gameBoard[0][i] == sign && gameBoard[1][i] == sign && isFieldEmpty(6 + i)) return i + 6;
  }
  // check diagonals
  if (gameBoard[0][0] == sign && gameBoard[1][1] == sign && isFieldEmpty(8)) return 8;
  else if (gameBoard[2][2] == sign && gameBoard[1][1] == sign && isFieldEmpty(0)) return 0;
  else if (gameBoard[0][2] == sign && gameBoard[1][1] == sign && isFieldEmpty(6)) return 6;
  else if (gameBoard[2][0] == sign && gameBoard[1][1] == sign && isFieldEmpty(2)) return 2;
  else if (gameBoard[0][0] == sign && gameBoard[2][2] == sign && isFieldEmpty(4)) return 4;
  else if (gameBoard[0][2] == sign && gameBoard[2][0] == sign && isFieldEmpty(4)) return 4;
  else{
    return -1;
  }
}

