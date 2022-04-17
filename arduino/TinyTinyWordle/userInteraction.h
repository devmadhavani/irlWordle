#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define DISPLAY_ADDRESS 0x3C

#define BUTTON_UP 2
#define BUTTON_LEFT 3
#define BUTTON_DOWN 4
#define BUTTON_RIGHT 5

#define DEBOUNCE_BUTTON_MICROS 100000

long lastPressT=0;
uint8_t getButton(){
  uint8_t ret=0;
  for(;;){
    if(digitalRead(BUTTON_UP)==LOW){
      ret=BUTTON_UP;
    }else if(digitalRead(BUTTON_LEFT)==LOW){
      ret=BUTTON_LEFT;
    }else if(digitalRead(BUTTON_DOWN)==LOW){
      ret=BUTTON_DOWN;
    }else if(digitalRead(BUTTON_RIGHT)==LOW){
      ret=BUTTON_RIGHT;
    }
    if(ret!=0){
      if(micros()-lastPressT>DEBOUNCE_BUTTON_MICROS){
        lastPressT=micros();
        return ret;
      }else{
        lastPressT=micros();
        ret=0;
      }
    }
    delay(10);
  }
}

Adafruit_SSD1306 display(128, 64, &Wire, -1, 1000000UL, 1000000UL);

void displayLetter(char c, bool inverted, bool border, uint8_t x, uint8_t y){
  display.setTextSize(1);
  if(inverted){
    if(border){
      display.fillRect(x,y,9,11,SSD1306_WHITE);
    }
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  }else{
    if(border){
      display.drawRect(x,y,9,11,SSD1306_WHITE);
    }
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }
  display.setCursor(x+2,y+2);
  display.print(c);
}

void displayWord(char* text, uint8_t* colors, uint8_t x, uint8_t y, uint8_t len){
  for(uint8_t i=0;i<len;i++){
    displayLetter(text[i],colors[i]==2,colors[i]>=1,x,y);
    x+=10;
  }
}

void printCentered(char* text, uint8_t y, uint8_t fontSize, bool inverted){
  display.setTextColor(inverted?SSD1306_BLACK:SSD1306_WHITE,inverted?SSD1306_WHITE:SSD1306_BLACK);
  display.setTextSize(fontSize);
  uint8_t x=(128-strlen(text)*6*fontSize)/2;
  display.setCursor(x,y);
  display.print(text);
}

void clearDisplay(){
  display.clearDisplay();
  display.display();
}

#define SPACE_BETWEEN_COLUMNS 12

void output(char* w, uint8_t* colors, uint8_t attempt, uint8_t attempts, uint8_t wordLength){
  if(attempt<attempts){
    uint8_t attemptsPerColumn=ceil(attempts/2);
    uint8_t x=(128-((wordLength*10)*2+SPACE_BETWEEN_COLUMNS))/2;
    if(attempt>=attemptsPerColumn){
      x+=(wordLength*10)+SPACE_BETWEEN_COLUMNS;
    }
    displayWord(w,colors,x,16+(attempt%attemptsPerColumn)*((64-16)/attemptsPerColumn),wordLength);
  }else{
    display.fillRect(0,0,128,16,SSD1306_BLACK);
    displayWord(w,colors,(128-(wordLength*10))/2,0,wordLength);
  }
  display.display();
}

void input(char* w, uint8_t wordLength, uint8_t attempt, uint8_t attempts){
  for(uint8_t i=0;i<wordLength;i++){
    w[i]='_';
  }
  uint8_t currentPos=0;
  while(true){
    display.fillRect(0,0,128,16,SSD1306_BLACK);
    uint8_t colors[wordLength];
    for(uint8_t i=0;i<wordLength;i++){
      colors[i]=i==currentPos?2:1;
    }
    displayWord(w,colors,(128-wordLength*10)/2,0,wordLength);
    displayLetter('>',currentPos==wordLength,currentPos==wordLength,128-10,0);
    char temp[8];
    sprintf(temp,"%d/%d",attempt,attempts);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0,2);
    display.print(temp);
    display.display();
    bool confirm=false;
    switch(getButton()){
      case BUTTON_UP:{
        if(currentPos<wordLength){
          if(w[currentPos]=='_'){
            w[currentPos]='Z';
          }else{
            w[currentPos]--;
            if(w[currentPos]<'A') w[currentPos]='_';
          }
        }
      }break;
      case BUTTON_DOWN:{
        if(currentPos<wordLength){
          if(w[currentPos]=='_'){
            w[currentPos]='A';
          }else{
            w[currentPos]++;
            if(w[currentPos]>'Z') w[currentPos]='_';
          }
        }
      }break;
      case BUTTON_LEFT:{
        if(currentPos>0) currentPos--;
      }break;
      case BUTTON_RIGHT:{
        if(currentPos<wordLength) currentPos++; else confirm=true;
      }break;
    }
    if(confirm){
      for(uint8_t i=0;i<wordLength;i++){
        if(w[i]=='_') confirm=false;
      }
    }
    if(confirm){
      w[wordLength]='\0';
      return;
    }
  }
}

void notInDictionary(){
  display.fillRect(0,0,128,16,SSD1306_WHITE);
  printCentered(NOT_IN_DICTIONARY,4,1,true);
  display.display();
  delay(1000);
}

#define TRANSITION_TIME 200000
void transition(uint8_t color){
  long ts=micros();
  float f=0;
  do{
    f=(float)(micros()-ts)/(float)(TRANSITION_TIME);
    uint8_t w=f*128, h=f*64;
    display.fillRect(64-w/2,32-h/2,w,h,color);
    display.display();
  }while(f<1);
}

//specialTranistion removed because it used too much program memory
/*void specialTransition(uint8_t color){
  long ts=micros();
  float f=0;
  do{
    f=(float)(micros()-ts)/(float)(4*TRANSITION_TIME);
    uint16_t x1=cos(4*f)*128*f+64, y1=sin(4*f)*64*f+32,
            x2=cos(2*PI/3+4*f)*128*f+64, y2=sin(2*PI/3+4*f)*64*f+32,
            x3=cos(-2*PI/3+4*f)*128*f+64, y3=sin(-2*PI/3+4*f)*64*f+32;
    display.fillTriangle(x1,y1,x2,y2,x3,y3,color);
    display.display();
  }while(f<1);
  display.fillRect(0,0,128,64,color);
}*/
#define specialTransition(color) transition(color)

void endgame(char* w, uint8_t wordLength, bool failed){
  delay(200);
  transition(failed?SSD1306_BLACK:SSD1306_WHITE);
  printCentered(w,26,3,!failed);
  printCentered(failed?THE_WORD_WAS:WELL_DONE,5,1,!failed);
  display.display();
  delay(1000);
  printCentered(PRESS_A_BUTTON,55,1,!failed);
  display.display();
  getButton();
  transition(SSD1306_BLACK);
}

#define win(w,len) endgame(w,len,false)
#define lose(w,len) endgame(w,len,true)

void combo(uint8_t n){
  specialTransition(SSD1306_WHITE);
  specialTransition(SSD1306_BLACK);
  printCentered(COMBO,0,2,false);
  char temp[8];
  sprintf(temp,"x%d",n);
  printCentered(temp,22,3,false);
  display.display();
  delay(1000);
  printCentered(PRESS_A_BUTTON,55,1,false);
  display.display();
  getButton();
  transition(SSD1306_BLACK);
}

void splashScreen(){
  clearDisplay();
  printCentered(TINY_TINY,6,1,false);
  printCentered(WORDLE,22,3,false);
  display.display();
  delay(600);
  printCentered(PRESS_A_BUTTON,52,1,false);
  display.display();
  getButton();
  transition(SSD1306_BLACK);
}

void ioInit(){
  display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);
  clearDisplay();
  pinMode(BUTTON_UP,INPUT_PULLUP);
  pinMode(BUTTON_LEFT,INPUT_PULLUP);
  pinMode(BUTTON_DOWN,INPUT_PULLUP);
  pinMode(BUTTON_RIGHT,INPUT_PULLUP);
}
