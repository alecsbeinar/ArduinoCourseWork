#include <stdlib.h>

#include <OLED_I2C.h> 
#include <AnalogKeypad.h>
#include <DHT.h>

#define ZUMMER 3
#define BUTTON 2
#define LEDRED 8
#define LEDGREEN 7
#define LEDBLUE 6
#define MOVE 5
#define VIBR 12

OLED  myOLED(A2, A3, A2);      
DHT dht(11, DHT11);  

const int KeypadMap[] = {0, 32, 88, 166, 352, 1022};
const uint16_t KeypadHoldTimeMs = 5000;
const uint8_t KeypadAnalogPin = A0;
AnalogKeypad keypad(KeypadAnalogPin, KeypadMap, countof(KeypadMap), KeypadHoldTimeMs);

extern uint8_t RusFont[]; 
extern uint8_t SmallFont[]; 
unsigned long timing1; 
unsigned long timing2;

int buttonState = 0;
int movecheck = 0;
int vibrcheck = 0;

int prev_temp = 0;
int prev_hum = 0;

bool is_submenu = false;
bool is_settings_changed = false;
int current_setting = 0;
int min_temp = 18;
int max_temp = 22;
int min_hum = 40;
int max_hum = 70;
int is_sound = 0;
int min_temp_limit = -10;
int max_temp_limit = 50;
int min_hum_limit = 0;
int max_hum_limit = 100;
int sound_limit_left = 0;
int sound_limit_right = 1;


enum Stages {
  TEMP_HUM,
  VIBRATION,
  MOVEMENT
};
typedef struct {
  enum Stages stage;
  const char *str;
} menu_t;
menu_t menu[] = {
  {TEMP_HUM, "Temp and Humidity"},
  {VIBRATION, "Vibration"},
  {MOVEMENT, "Movement"},
};

typedef struct {
  int index;
  const char* str;
  int* value;
  int* limit_left;
  int* limit_right;
} settings_t;
settings_t settings[] = {
  {1, "Min temp", &min_temp, &min_temp_limit, &max_temp},
  {2, "Max temp", &max_temp, &min_temp, &max_temp_limit},
  {3, "Min humidity", &min_hum, &min_hum_limit, &max_hum},
  {4, "Max humidity", &max_hum, &min_hum, &max_hum_limit},
  {5, "Sound", &is_sound, &sound_limit_left, &sound_limit_right},
};


void turnOffBulb(){
  digitalWrite(LEDRED, HIGH); 
  digitalWrite(LEDGREEN, HIGH);
  digitalWrite(LEDBLUE, HIGH);
}

void setup() {
  Serial.begin(9600);  
  pinMode(BUTTON, INPUT);
  pinMode(LEDRED, OUTPUT); 
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  pinMode(MOVE, INPUT); 
  pinMode(VIBR, INPUT);
  pinMode(ZUMMER, OUTPUT); 

  turnOffBulb();
  for(int i=0; i<3; i++){
    digitalWrite(LEDGREEN, LOW);
    delay(250);
    turnOffBulb();
    delay(250);
  }

  myOLED.begin(); 
  myOLED.setFont(SmallFont); 
  dht.begin(); 
}

void TempHumInfo(int i){
  float h = dht.readHumidity(); 
  float t = dht.readTemperature();
  int intTemp = static_cast<int>(t);
  int intHum = static_cast<int>(h);
  if (intTemp != prev_temp || intHum != prev_hum) {
    myOLED.clrScr();
    myOLED.print(menu[i].str, 0, 10); 
    myOLED.print("Temperature: " + String(t) + " C", 0, 30); 
    myOLED.print("Humidity: " + String(h) + " %", 0, 50);
    myOLED.update();

    prev_temp = intTemp;
    prev_hum = intHum;
  }

  if (is_sound){
    if (intTemp < min_temp || intTemp > max_temp || intHum < min_hum || intHum > max_hum) {
      tone(ZUMMER, 1000); 
    } else {
      noTone(ZUMMER); 
    }
  }
  
}

void VibrInfo(int i){
  vibrcheck = digitalRead(VIBR);
  if(vibrcheck){
    digitalWrite(LEDRED, LOW); 
    digitalWrite(LEDGREEN, LOW);
  }
  else{
    turnOffBulb();
  }
}

void MoveInfo(int i){
  // данные получаются раз в 8 секунд
  movecheck = digitalRead(MOVE);
  if(movecheck){
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDBLUE, LOW);
    if (is_sound){
      tone(ZUMMER, 1000); 
    }
  }
  else{
    noTone(ZUMMER); 
    turnOffBulb();
  }
}

void screenStageSetup(int i){
  myOLED.clrScr();
  myOLED.print(menu[i].str, 0, 10); 
  myOLED.update();
}

void closeStage(){
  myOLED.clrScr();
  myOLED.update();
  noTone(ZUMMER);
  turnOffBulb();
  movecheck = 0;
  vibrcheck = 0;
  prev_temp = 0;
  prev_hum = 0;
}

void ButtonHandler(const ButtonParam& param) {
  // param.button
  // 4 separate
  // 3 right
  // 2 down
  // 1 up
  // 0 left
  if (param.state == ButtonState_Click) {
    if (is_submenu){
      switch(param.button) {
        case 0:
          if (*(settings[current_setting].value) > *(settings[current_setting].limit_left))
            *(settings[current_setting].value) -= 1;
          is_settings_changed = true;
          break;
        case 1:
          current_setting = (current_setting == 0 ? 0 : current_setting -= 1);
          is_settings_changed = true;
          break;
        case 2:
          current_setting = (current_setting == sizeof(settings)/sizeof(settings_t) - 1 ? sizeof(settings)/sizeof(settings_t) - 1 : current_setting += 1);
          is_settings_changed = true;
          break;
        case 3:
          if (*(settings[current_setting].value) < *(settings[current_setting].limit_right))
            *(settings[current_setting].value) += 1;
          is_settings_changed = true;
          break;
        case 4:
          is_submenu = false;
          break;
      }
    } else {
      if (param.button == 4) {
        closeStage();
        is_submenu = true;
        is_settings_changed = true;
      }
    }
   
  }

}

int i = 0;
void main_menu(){
  buttonState = digitalRead(BUTTON); 
  if(buttonState) 
  {
    digitalWrite(LEDBLUE, LOW);
    while(buttonState)
        buttonState = digitalRead(BUTTON); 
    turnOffBulb();

    closeStage();

    i += 1;
    if(i == (sizeof(menu) / sizeof(menu[0]))) i = 0;
    screenStageSetup(i);
  }

  switch(menu[i].stage){
    case TEMP_HUM:
      TempHumInfo(i);
      break;
    case VIBRATION:
      VibrInfo(i);
      break;
    case MOVEMENT:
      MoveInfo(i);
      break;
  }
}


void print_submenu(){
  myOLED.clrScr();
  char value[7];
  char text[50];
  for(int i = 0; i < sizeof(settings)/sizeof(settings_t); i++) {
    if(i == current_setting) {
      strcpy(text, "> "); 
    } else {
      strcpy(text, "  ");
    }
    itoa(*(settings[i].value), value, 10);
    strcat(text, settings[i].str);
    strcat(text, ": ");
    strcat(text, value);
    myOLED.print(text, 3, i * 10); 
    memset(text, '\0', sizeof(text));
  }
  myOLED.update();
}

void submenu(){
  if (is_settings_changed){
    print_submenu();
    is_settings_changed = false;
  }
}


void loop()
{
  keypad.loop(ButtonHandler);

  if (is_submenu) submenu();
  else main_menu();
}
