

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <stdlib.h>
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

#define PLAYER_SIZE 8
#define PLAYER_WIDTH 2
#define BALL_SIZE 2
#define GAME_X 128
#define GAME_Y 32
#define BALL_START_X 64
#define BALL_START_Y 16
#define BALL_START_DX 2
#define BALL_START_DY 1
#define MAX_NAME_LENGTH 16

struct Player {
  int pos, score;
};

struct highscorePlayer {
  char name[MAX_NAME_LENGTH];
};

void readHighscore( void );
void mainMenu( void );
void browseHighscore( void );

int lostGame = 0;

// Spelare och boll
int movePlayer = 1;

struct Player p1 = {0, 0};
struct Player p2 = {0, 0};

int ball_x = BALL_START_X;
int ball_y = BALL_START_Y;
// Bollens hastighet
int ball_dx = BALL_START_DX;
int ball_dy = BALL_START_DY;

int tempdx = 0;
int tempdy = 0;

// AI
int AI_MODE = 0;
int AI_dy = 1;

// board
int board[512] = {0};


void reset_ball() {
  ball_x = BALL_START_X;
  ball_y = BALL_START_Y;
  ball_dx = BALL_START_DX;
  ball_dy = BALL_START_DY;
}

void ball_player_collision(struct Player hit_player, struct Player* other) {
  // om bollen träffar någon spelare
  if (ball_y >= hit_player.pos && ball_y <= hit_player.pos + PLAYER_SIZE) {
    ball_dx = -ball_dx;
    tempdx = 0;
    tempdy = 0;
  } else {
    (*other).score++;
    if (AI_MODE != 0 && p2.score > 0)
      lostGame = 1;
    if (p1.score > 4 || p2.score > 4)
      lostGame = 1;
    reset_ball();
  }
}

void check_collision( void ) {
  // om bollen träffar toppen eller botten
  if (ball_y >= GAME_Y - BALL_SIZE || ball_y <= 0) {
    ball_dy = -ball_dy;
    tempdx = 0;
    tempdy = 0;
  } 
  if (ball_x <= PLAYER_WIDTH) {
    ball_player_collision(p1, &p2);
  } else if (ball_x >= GAME_X-PLAYER_WIDTH-BALL_SIZE) {
    ball_player_collision(p2, &p1);
  }
}

void move_player(int sw, struct Player* player) {
  if (sw && (*player).pos > 0) {
    (*player).pos--;
  } else if (!sw && (*player).pos + PLAYER_SIZE < GAME_Y) {
    (*player).pos++;
  }
}

void move_ai(int mode, struct Player* player) {
  // Mode 1: Går upp tills den inte kan mer, går då ner tills den inte kan mer osv
  // Mode 2. God Mode, följer bollens position
  if (mode == 1 && (*player).pos == 0){
    AI_dy = 1;
  } else if (mode == 1 && (*player).pos + PLAYER_SIZE == GAME_Y) {
    AI_dy = -1;
  } else if (mode == 2) {
    if (ball_y < GAME_Y-PLAYER_SIZE)
      (*player).pos = ball_y-BALL_SIZE;
    return;
  }
  (*player).pos += AI_dy;

}

void enter_score(struct Player winner) {
  // winner will always be the p1 struct
  int position = 0;
  int counter = 64; // 65 i ASCII = 'A'
  char name[MAX_NAME_LENGTH];
  int temp;
  for (temp = 0; temp < MAX_NAME_LENGTH; temp++) // OBS, initialization cannot be 0 (display_string will ignore everything right of a 0 char!!)
    name[temp] = 1;
  
  int button;
 
  // Obs: Antar att score är mellan 0 och 99
  if (winner.score < 10) {
    // 48-57
    name[MAX_NAME_LENGTH-1] = winner.score+48; // 48 i ASCII = '0'
  }
  else {
    name[MAX_NAME_LENGTH-2] = (winner.score/10)+48; // Tiotalet
    name[MAX_NAME_LENGTH-1] = (winner.score%10)+48; // Entalet
  }
  while (1) {
    button = getbtns();

    if (button & 0x1 || position > 12 ){ // Namn = 12, space = 1, score = 3. Totalt: 16 characters
      int i;
      for (i = 0; i < MAX_NAME_LENGTH; i++) {
        writeEEPROM(name[i]);
      }
      // TODO: Choice to read through highscore
      browseHighscore();
      p1.score = 0;
      p2.score = 0;
      mainMenu();
      lostGame = 0;
      return;
    }

    if (button & 0x2){
      quicksleep(1000000);
      name[position] = counter;
      counter = 64; // Reset
      position++;
    }

    if (button & 0x4){
      quicksleep(1000000);
      if (counter == 90) // Z is 90
        counter = 64;
      counter++;
      name[position] = counter;

      display_string(1, name);
      display_update();
    }

  }
}

void gameOver( void ) {

  display_string(0, "     NAME:");
  display_string(1, "");
  display_string(2, "");
  display_string(3, "");
  display_update();

  enter_score(p1);

}

/* Interrupt Service Routine */
/* Kommer endast att flytta spelare och boll ett steg vid varje iteration */
void user_isr( void ) {

  if (lostGame) {
    IFS(0) = 0x0;
    return;
  }

  int switches = getsw();

  // Flytta spelare
  if (movePlayer) {
    move_player(switches & 0x8, &p1);
    if (AI_MODE == 0)
      move_player(switches & 0x1, &p2);
    else
      move_ai(AI_MODE, &p2);
    movePlayer = 0;
  }
  else
    movePlayer = 1;

  // bollen
  // för att endast flytta en pixel åt gången
  // återställer när båda blivit 0
  if (tempdx == 0 && tempdy == 0) {
    tempdx = ball_dx;
    tempdy = ball_dy;
  }
  if (tempdx > 0) {
    ball_x++;
    tempdx--;
  } else if (tempdx < 0) {
    ball_x--;
    tempdx++;
  }
  if (tempdy > 0) {
    ball_y++;
    tempdy--;
  } else if (tempdy < 0) {
    ball_y--;
    tempdy++;
  }
  check_collision();
  IFS(0) = 0x0;
}

void create_board( void ) {
  int i;
  for (i = 0; i < 512; i++) {
    board[i]= 0;
  }
  int p1page = p1.pos/PLAYER_SIZE;
  int p2page = p2.pos/PLAYER_SIZE;
  int p1pixelsLow = p1.pos % PLAYER_SIZE;
  int p2pixelsLow = p2.pos % PLAYER_SIZE;
  int p1pixelsHigh = PLAYER_SIZE - p1pixelsLow;
  int p2pixelsHigh = PLAYER_SIZE - p2pixelsLow;

  uint8_t playerbyte = 255;
  int k;
  int playerbyte2 = 0;
  // ta bort alla pixlar vi inte vil skriva ut på pagen.
  for (k = 0; k < p1pixelsLow; k++) {
    playerbyte2 += customPow(2,k);
  }
  playerbyte -= (uint8_t) playerbyte2;
  // rita spelare 1
  board[128*p1page] = playerbyte;
  board[128*p1page + 1] = playerbyte;
  if (p1page != 3) {
    board[128*(p1page + 1)] = playerbyte2;
    board[128*(p1page + 1) + 1] = playerbyte2;
  }

  playerbyte = 255;
  playerbyte2 = 0;

  for (k = 0; k < p2pixelsLow; k++) {
    playerbyte2 += customPow(2,k);
  }
  playerbyte -= (uint8_t) playerbyte2;
  // rita spelare 1
  board[128*p2page+126] = playerbyte;
  board[128*p2page + 127] = playerbyte;
  if (p2page != 3) {
    board[128*(p2page + 1) + 126] = playerbyte2;
    board[128*(p2page + 1) + 127] = playerbyte2;
  }

  // Boll
  int ballpage = ball_y/PLAYER_SIZE;
  int ballpixelLow = ball_y % PLAYER_SIZE;

  int ballbyte = 0;
  /*
  * Exempel:
  * pos = 10
  * ballLow = 2
  * vill ha tal: 00001100
  */
  for (k = 0; k < 8; k++){
    if (k == ballpixelLow) {
      ballbyte += customPow(2,k);
      ballbyte += customPow(2,(k+1));

      board[128*ballpage+ball_x] = ballbyte;
      board[128*ballpage+ball_x+1] = ballbyte;

      if (k == 7 && ballpage != 3) {
        board[128*(ballpage+1)+ball_x] = 1;
        board[128*(ballpage+1)+ball_x+1] = 1;
      }

      break;

    }
  }
}

int customPow(base, exp) {
  int i;
  int returnValue = 1;
  for (i = 0; i < exp; i++) {
    returnValue *= base;
  }
  return returnValue;
}

void setAIMode( void ) {
  while ( 1 )
  {
    int btns = getbtns();
  
    if (btns & 0x4){
      AI_MODE = 0;
      return;
    }
    if (btns & 0x2){
      AI_MODE = 1;
      return;
    }
    if (btns & 0x1){
      AI_MODE = 2;
      return;
    }
  }
}


void readHighscore( void ) {

  clearScreen();

  int adressCount = readEEPROM(0);
  struct highscorePlayer highscore[(adressCount-1)/MAX_NAME_LENGTH]; // Minus 1 because "clean" highscore has adressC = 1
  int tempiter;
  struct highscorePlayer tempplayer = {0};
  for (tempiter = 0; tempiter < (adressCount-1)/MAX_NAME_LENGTH; tempiter++) {
    highscore[tempiter] = tempplayer;
  }
  int i, j, l;
  char player[MAX_NAME_LENGTH];


  for (i = 1; i <= (adressCount-1)/MAX_NAME_LENGTH; i++) {
    for (j = 1; j <= MAX_NAME_LENGTH; j++) {
      player[j-1] = readEEPROM((i-1)*MAX_NAME_LENGTH+j);
    }
    struct highscorePlayer p;
    for (l = 0; l < MAX_NAME_LENGTH; l++) {
      p.name[l] = player[l];
    }
    highscore[i-1] = p;
  }

  int btn;
  int iter = 0;
  while (1) {
    display_string(0, highscore[iter].name);
    display_string(1, highscore[iter+1].name);
    display_string(2, highscore[iter+2].name);
    display_string(3, highscore[iter+3].name);
    display_update();

    while (1) {
      btn = getbtns();

      if (btn & 0x4) {
        if (iter+4 < (adressCount-1)/MAX_NAME_LENGTH)
          iter++;
        quicksleep(1000000);
        break;
      }
      if (btn & 0x2) {
        quicksleep(1000000);
        return;

      }

    }
  }


}

void browseHighscore( void ) {
  quicksleep(1000000);

  display_string(0, "HIGHSCORE: 1");
  display_string(1, "RESET HS : 2");
  display_string(2, "MAIN MENU: 3");
  display_string(3, "");
  display_update();

  int btn;

  while (1) {
    btn = getbtns();

    if (btn & 0x4) {
      quicksleep(1000000);
      readHighscore();
      display_string(0, "HIGHSCORE: 1");
      display_string(1, "RESET HS : 2");
      display_string(2, "MAIN MENU: 3");
      display_string(3, "");
      display_update();
    }
    if (btn & 0x2) {
      quicksleep(1000000);
      resetHighscore();
      display_string(3, "HIGHSCORE RESET");
      display_update();

    }
    if (btn & 0x1) {
      quicksleep(1000000);
      return;
    }

  }
}

void mainMenu( void ) {
  quicksleep(1000000);

  display_string(0, "   PONG GAME");
	display_string(1, "  MULTIPLAY: 1");
	display_string(2, "  AI (EASY): 2");
	display_string(3, "  AI (HARD): 3");
	display_update();

  setAIMode();

  reset_ball();
}

/* Initialize pong */
void pong_init( void ) {

  TRISD = (((TRISD >> 5) | 0x3F) << 5) | (TRISD & 0x1F);
  PR2 = 0x2000; // Timeout frequency
  TMR2 = 0x0;
  // sätter prescale till 256
  T2CON = (((T2CON >> 4) | 0x7) << 4) | (T2CON & 0xF);
  // startar timer
  T2CONSET = 0x8000;
  // sätt på interrupts för timer 2
  IEC(0) = 0x900;
  // sätt priority
  IPC(2) = 0x1F;
  IPCSET(2) = 0x1F000000;

  setAIMode();

  enable_interrupt();

}

/* This function is called repetitively from the main program */
void pong( void ) {
  if (lostGame) {
    gameOver();
  }
  create_board();
  display_update_pong(board);
}
