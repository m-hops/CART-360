//LIBRARY DEPENDENCIES//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Adafruit_BNO08x.h>

//DIGITAL PIN SETUP//
#define JOY_A_BTN 1
#define JOY_B_BTN 2
#define BUTTON_PIN 3
#define PIXEL_PIN 4

//SMART LED COUNT//
#define PIXEL_COUNT 60

//ANALOG PIN SETUP//
#define JOY_A_X A0
#define JOY_A_Y A1
#define JOY_B_X A2
#define JOY_B_Y A3

//9-DOF GLOBAL VARIABLES//
#define BNO08X_CS 10
#define BNO08X_INT 9
#define BNO08X_RESET -1

Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

const int MovingAverageN = 5;
float Gx;
float Gy;
float Gz;
float Ax;
float Ay;
float Az;
float GxA=0;
float GyA=0;
float GzA=0;
float AxA=0;
float AyA=0;
float AzA=0;
float movingAverage(float avg, float v, float n){
  return avg * (n-1)/n + v/n;
}

//NEOPIXEL GLOBAL VARIABLES//
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
#define oldState HIGH
#define ledChargeTimer 10
#define ledPulseTimer 10
#define ledUnloadTimer 8
#define ChargeColor strip.Color(255,0,0)
#define PulseColor strip.Color(255,255,0)
#define ChargeColorR 255
#define ChargeColorG 0
#define ChargeColorB 0
#define PulseColorR 255
#define PulseColorG 0
#define PulseColorB 255
#define PulseLength 20

//LED PROGRAM GLOBAL VARIABLES//
unsigned long LastTime = 0;
int ChargeIndex = 0;
bool LastButtonState = false;
int PulseIndex = 0;

//JOYSTICK GLOBAL VARIABLES//
int range = 12;             
int responseDelay = 5;      
int threshold = range / 4;  
int center = range / 2;    

//ADD ADDITIONAL STATES HERE//
enum StateId{
  State_Uncharged,
  State_Charged,
  State_Unload,
  DOF_Idle,
  DOF_Ready,
  DOF_SwingDown,
  DOF_End
};
StateId CurrentStateId = State_Uncharged;
StateId CurrentDOFID = DOF_Idle;

//DEFINED STATES//
//DEFAULT STATE; SWORD HAS NO OR INCOMPLETE CHARGE//
void loop_Uncharged(){

  if (isButtonLow()){
    if(!LastButtonState){
      LastTime = millis();
      LastButtonState = true;
    }
    unsigned long elapsedTime = millis() - LastTime;
    if(elapsedTime >= ledChargeTimer){
      LastTime = millis();
      if(ChargeIndex < strip.numPixels())
        ++ChargeIndex;
      else
        CurrentStateId = State_Charged;
    }
  } else {
    if(LastButtonState){
      LastTime = millis();
      LastButtonState = false;
    }
    unsigned long elapsedTime = millis() - LastTime;
    if(elapsedTime >= ledChargeTimer){
      LastTime = millis();
      if(ChargeIndex > 0)
        --ChargeIndex;
    }
  }


}
//MAINTAINS SWORD CHARGE ONCE CHARGE HITS MAXIMUM//
void loop_Charged(){
  
  unsigned long elapsedTime = millis() - LastTime;
  if(elapsedTime >= ledPulseTimer){
    LastTime = millis();
    if(PulseIndex < strip.numPixels())
      ++PulseIndex;
    else
      PulseIndex = 0;
  }

  // TEMPORARY CODE TO UNLOAD BY PUSHING THE BUTTON
  if (isButtonLow()){
    if(!LastButtonState){
      //
      LastTime = millis();
      ChargeIndex = 0;
      PulseIndex = 0;
      CurrentStateId = State_Unload;
      //
      LastButtonState = true;
    }
  }
  else{
    if(LastButtonState){
      LastButtonState = false;
    }
  }
}
//DISCHARGES SWORD AND RESETS STATE BACK TO THE BEGINNING//
void loop_Unload(){

    unsigned long elapsedTime = millis() - LastTime;
    if(elapsedTime >= ledUnloadTimer){
      LastTime = millis();
      if(ChargeIndex < strip.numPixels())
        ++ChargeIndex;
      else
      {
        ChargeIndex = 0;
        PulseIndex = 0;
        CurrentStateId = State_Uncharged;
      }
    }
}
//STATE MACHINE FOR SWORD//
void swordStateMachine() {
  
  switch(CurrentStateId) {
    case State_Uncharged:
      loop_Uncharged();
      break;
    case State_Charged:
      loop_Charged();
      break;
    case State_Unload:
      loop_Unload();
      break;
  }
}
void DOFStateMachine() {
  switch(CurrentDOFID) {
    case DOF_Idle:

      if (AyA > 9) {
        Serial.println("Going to DOF_Ready");
        CurrentDOFID = DOF_Ready;
      }
      break;
    case DOF_Ready:

      if (AyA <= 7) {
        Serial.println("Going to DOF_SwingDown");
        CurrentDOFID = DOF_SwingDown;
      }
      break;
    case DOF_SwingDown:
    
      if (AyA >= -3) {
        Serial.println("Going to DOF_End");
        CurrentDOFID = DOF_End;
      }  
      break;
    case DOF_End:

      Serial.println("Going to DOF_Idle");
      CurrentDOFID = DOF_Idle;
      
      break;
  }
}

//DEFINE BUTTONS PINMODE//
void buttonSetup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}
//CHECKS TO SEE WHILE BUTTON IS DOWN//
bool isButtonLow(){
  boolean currentButtonState = digitalRead(BUTTON_PIN);
  return currentButtonState == LOW;
}
//SETUP COMMANDS FOR LED STRIP//
void LEDStripSetup() {

  //INITIATE LEDS WITH ADAFRUIT NEOPIXEL LIBRARY//
  strip.begin();
  strip.show();
}
void LEDStripGradientGenerator() {
  for(int i=0; i < strip.numPixels(); ++i) {
    int r = 0;
    int g = 0;
    int b = 0;
    if(CurrentStateId == State_Unload){
      if(i >= ChargeIndex){
        r = ChargeColorR;
        g = ChargeColorG;
        b = ChargeColorB;
      } else{
        r = 0;
        g = 0;
        b = 0;
      }
    }
    else{
      if(i < ChargeIndex){
        r = ChargeColorR;
        g = ChargeColorG;
        b = ChargeColorB;
      } else{
        r = 0;
        g = 0;
        b = 0;
      }
      int pulseDistance = PulseIndex - i - 1;
      if(pulseDistance >= 0 && pulseDistance < PulseLength){
        int chargeNom = pulseDistance;
        int chargeDem = PulseLength;
        int pulseNom = PulseLength - pulseDistance;
        int pulseDem = PulseLength;
        r = r * chargeNom / chargeDem + PulseColorR * pulseNom / pulseDem;
        g = g * chargeNom / chargeDem + PulseColorG * pulseNom / pulseDem;
        b = b * chargeNom / chargeDem + PulseColorB * pulseNom / pulseDem;
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;
      }
    }
    strip.setPixelColor(i, strip.Color((uint8_t)r,(uint8_t)g,(uint8_t)b));   
  }
}

//SETUP FOR 9-DOF//
void DOFSetup() {
  if (!bno08x.begin_I2C()) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found!");

  DOFReports();
  delay(100);
}
//CHECKS 9 DOF EVERY ROTATION AND RETURNS SPECIFIED SENSOR VALUES//
void DOFLoop() {

  if (bno08x.wasReset()) {
    Serial.print("sensor was reset ");
    DOFReports();
  }
  
  if (! bno08x.getSensorEvent(&sensorValue)) {
    return;
  }
  
  switch (sensorValue.sensorId) {
    
    case SH2_ACCELEROMETER:
    
      Ax = sensorValue.un.accelerometer.x;
      Ay = sensorValue.un.accelerometer.y;
      Az = sensorValue.un.accelerometer.z;
      AxA = movingAverage(AxA, Ax, MovingAverageN);
      AyA = movingAverage(AyA, Ay, MovingAverageN);
      AzA = movingAverage(AzA, Az, MovingAverageN);
      break;
      
    case SH2_GYROSCOPE_CALIBRATED:
    
      Gx = sensorValue.un.gyroscope.x;
      Gy = sensorValue.un.gyroscope.y;
      Gz = sensorValue.un.gyroscope.z;
      GxA = movingAverage(GxA, Gx, MovingAverageN);
      GyA = movingAverage(GyA, Gy, MovingAverageN);
      GzA = movingAverage(GzA, Gz, MovingAverageN);
      break;
  }
}

//CALLS SPECIFIC DATA FROM THE 9 DOF//
void DOFReports (void) {
    if (!bno08x.enableReport(SH2_ACCELEROMETER)) {
    Serial.println("Could not enable accelerometer");
  }
  if (!bno08x.enableReport(SH2_GYROSCOPE_CALIBRATED)) {
    Serial.println("Could not enable gyroscope");
  }
}

//DEBUG VALUES FOR 9DOF//
void DOFDebug() {
    Serial.print("Accelerometer - x:");
    printFormated(AxA);
    Serial.print(" y:");
    printFormated(AyA);
    Serial.print(" z:");
    printFormated(AzA);
    Serial.print(" Gyro - x:");
    printFormated(GxA);
    Serial.print(" y:");
    printFormated(GyA);
    Serial.print(" z:");
    printFormated(GzA);
    Serial.println();
}

//FORMATS 9DOF VALUES FOR DEBUG//
void printFormated(float value){
  if(value < 10 && value > -10)
    Serial.print(" ");
  if(value >= 0)
    Serial.print(" ");
  Serial.print(value, 2);
}

void setup() {
  
  //SET SERIAL.BEGIN TO 115200//
  Serial.begin(115200);

  //USER SETUP FUNCTIONS//
  buttonSetup();
  LEDStripSetup();
  DOFSetup();

  //DEFAULT STARTING VALUES//
  LastTime = millis();
  ChargeIndex = 0;
  LastButtonState = false;
  CurrentStateId = State_Uncharged;
  PulseIndex = 0;
}

void loop() {
  
  //DOFDebug();
  
  swordStateMachine();
  LEDStripGradientGenerator();
  strip.show();
  DOFStateMachine();
  DOFLoop();

}
