#include <EEPROM.h>

#define MAXD 21          //highest digital
#define MAXA 7          //highest analog input
#define MAXC 1          //highest counter
#define MAXE 511        //highest eeprom
#define MAXDEVICE 0    //maximum number of devices

char pwmPins[]={3,5,6,9,10,11};  //valid pwm outputs on 168 & 328
char inputString[20];         // a string to hold incoming data format is "@TargetActionTypeAddress:data"
boolean stringComplete = false;  // whether the string is complete
unsigned char device;
volatile long counters[]={0,0};    //pulse counters for external interrupts
volatile unsigned long period[]={0,0};  //period for external interrupts

void intSr0(){
volatile static unsigned long lastMicros=0,currentMicros;
  currentMicros=micros();
  period[0]=(long) currentMicros-lastMicros;
  lastMicros=currentMicros;
  counters[0]++;
}

void intSr1(){
volatile static unsigned long lastMicros=0,currentMicros;
  currentMicros=micros();
  period[1]=(long) currentMicros-lastMicros;
  lastMicros=currentMicros;
  counters[1]++;
}

void setup() {
  // initialize serial:
  Serial.begin(115200);
  device=EEPROM.read(0);
  if (device > MAXDEVICE)
    device=0;
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  attachInterrupt(0,intSr0,RISING);
  attachInterrupt(1,intSr1,RISING);
}

void handleRequest(){

int result, target=-1, address=-1;
long data=-1,counterTemp;
char i, action='_',type='_',dir='_';
boolean error=false;

//  Serial.print('>');
//  Serial.print(inputString);
  result=sscanf(inputString,"%d%c%c%d:%ld",&target,&action,&type,&address,&data);
//  Serial.print(' ');
//  Serial.println(result);
  if (result>0 && target==device){
    Serial.print('@');
    Serial.print(target);
    switch (tolower(action)){
    case 'r':
      Serial.print(type);
      Serial.print(address);
      Serial.print(':');
      switch (tolower(type)){
      case 'd':
        if (address>=0&&address<=MAXD)
          Serial.print(digitalRead(address));
        else
          error=true;
        break;
      case 'a':
        if (address>=0&&address<=MAXA)
          Serial.print(analogRead(address));
        else
          error=true;
        break;
      case 'c':
        if (address>=0&&address<=MAXC){
          noInterrupts();
          counterTemp=counters[address];
          if (result==5)
            counters[address]=data;
          interrupts();
          Serial.print(counterTemp);
        }
        else
          error=true;
        break;
      case 'e':
        if (address>=0&&address<=MAXE)
          Serial.print(EEPROM.read(address));
        else
          error=true;
        break;
      case 'p':
        if (address>=0&&address<=MAXC){
          Serial.print(period[address]);
        }
        else
          error=true;
        break;
      default:
        error=true;
      }
      break;
    case 'w':
      Serial.print(type);
      Serial.print(address);
      Serial.print(':');
      Serial.print(data);
      switch (tolower(type)){
        case 'd':
          if (address>=0&&address<=MAXD&&(data==0||data==1)){
            digitalWrite(address,data);
          }
          else
            error=true;
          break;
        case 'a':
          error=true;
          for (i=0; i<sizeof(pwmPins);i++)
            if (address==pwmPins[i] && data>=0 && data <=255){
              analogWrite(address,(unsigned char) data);
              error=false;
              break;
            }
          break;
        case 'e':
          if (/*address>0 && */address<=MAXE && data>=0 && data <=255)
            EEPROM.write(address,(unsigned char) data);
          else
            error=true;
          break;
        case 'c':
          if (address>=0 && address<=MAXC)
            counters[address]=data;
          else
            error=true;
          break;
        case 'i':
          result=sscanf(inputString,"%*d%*c%*c%*d:%c",&dir);
          Serial.print(' ');
          Serial.print(dir);
          if (address>=0 && address<=MAXD){
            switch (tolower(dir)){
            case 'o':
              pinMode(address,OUTPUT);
              break;
            case 'i':
              pinMode(address,INPUT);
              break;
            default:
              error=true;
            }
          }
          else
            error=true;
          break;
        default:
          error=true;
      }
      break; 
    default:
      error=true;      
    }
    if (error)
      Serial.print(F("???"));
    Serial.println();
  }
}

void loop() {
  if (stringComplete){
    handleRequest();
    stringComplete=false;
  }
}


void serialEvent() {
static char i=0;
static boolean atReceived=false;

  while (Serial.available()) { // get the new byte:
    char inChar = (char)Serial.read(); // add it to the inputString:
    if (inChar=='@'){
      i=0;
      atReceived=true;
      return;
    }
    if (atReceived){
      if (i<sizeof(inputString)){
        if (inChar == '\n'){
          inChar= '\0';
          stringComplete = true;
          atReceived=false;
        }
      inputString[i++]=inChar;
      }
    } 
  }
}
