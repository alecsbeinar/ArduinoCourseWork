#include <OLED_I2C.h> 
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

extern uint8_t RusFont[]; 
extern uint8_t SmallFont[]; 
unsigned long timing1; 
unsigned long timing2;

int buttonState = 0;
int movecheck = 0;
int vibrcheck = 0;

int prev_temp = 0;
int prev_hum = 0;

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

  tone(ZUMMER, 1000); 
  delay(1000);   
  noTone(ZUMMER); 

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
  }
  else{
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
  turnOffBulb();
  movecheck = 0;
  vibrcheck = 0;
  prev_temp = 0;
  prev_hum = 0;
}

int i = 0;
void loop()
{
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



  // delay(2000);
  // // белый
  // digitalWrite(LEDRED, LOW); 
  // digitalWrite(LEDGREEN, LOW);
  // digitalWrite(LEDBLUE, LOW);
  // delay(2000);
  // // красный
  // digitalWrite(LEDRED, LOW); 
  // digitalWrite(LEDGREEN, HIGH);
  // digitalWrite(LEDBLUE, HIGH);
  // delay(2000);
  // // зеленый
  // digitalWrite(LEDRED, HIGH);
  // digitalWrite(LEDGREEN, LOW);
  // digitalWrite(LEDBLUE, HIGH);
  // delay(2000);
  // // синий
  // digitalWrite(LEDRED, HIGH);
  // digitalWrite(LEDGREEN, HIGH);
  // digitalWrite(LEDBLUE, LOW);
  // delay(2000);
  // // желтый
  // digitalWrite(LEDRED, LOW); 
  // digitalWrite(LEDGREEN, LOW);
  // digitalWrite(LEDBLUE, HIGH);
  // delay(2000);
  // // голубой
  // digitalWrite(LEDRED, HIGH);
  // digitalWrite(LEDGREEN, LOW);
  // digitalWrite(LEDBLUE, LOW);
  // delay(2000);
  // розовый
  // digitalWrite(LEDRED, LOW);
  // digitalWrite(LEDGREEN, HIGH);
  // digitalWrite(LEDBLUE, LOW);
  // delay(2000);

