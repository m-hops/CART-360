#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <SFE_LSM9DS0.h>
#include <Wire.h>
#include <SPI.h>

//DIGITAL PIN SETUP//
#define BUTTON_PIN 21
#define PIXEL_PIN 14

//SMART LED COUNT//
#define PIXEL_COUNT 59

//NEOPIXEL GLOBAL VARIABLES//
Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
#define oldState HIGH
#define ledChargeTimer 10
#define ledPulseTimer 5
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

//DOF GLOBAL VARIABLES//
#define LSM9DS0_XM 0x1D
#define LSM9DS0_G 0x6B
#define PRINT_CALCULATED
#define PRINT_SPEED 500

const byte INT1XM = 25;

LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

//LED PROGRAM GLOBAL VARIABLES//
unsigned long LastTime = 0;
int ChargeIndex = 0;
bool LastButtonState = false;
int PulseIndex = 0;

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
bool lightAttackReady = false;
bool heavyAttackReady = false;
bool isButtonLow(){
  boolean currentButtonState = digitalRead(BUTTON_PIN);
  Serial.println(currentButtonState);
  return currentButtonState == LOW;
}

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
        heavyAttackReady = true;
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
void LEDStateMachine() {
  
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
void buttonSetup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}
//SETUP COMMANDS FOR LED STRIP//
void LEDStripSetup() {
  
  strip.begin();
  strip.show();
}
//CODE TO GENERATE COLOR PULSE THROUGH SMART LEDS//
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

void DOFConfig() {
  uint16_t status = dof.begin();

  pinMode(INT1XM, INPUT);

  dof.setAccelScale(dof.A_SCALE_4G);
}
void DOFLoop() {

  printAccel();
}
void printAccel() {

    dof.readAccel();
    
    Serial.print("A: ");
  #ifdef PRINT_CALCULATED
    Serial.print(dof.calcAccel(dof.ax), 2);
    Serial.print(", ");
    Serial.print(dof.calcAccel(dof.ay), 2);
    Serial.print(", ");
    Serial.println(dof.calcAccel(dof.az), 2);
  #elif defined PRINT_RAW 
    Serial.print(dof.ax);
    Serial.print(", ");
    Serial.print(dof.ay);
    Serial.print(", ");
    Serial.println(dof.az);
  #endif

}
//STATE MACHINE FOR DOF//
void DOFStateMachine() {

  switch(CurrentDOFID) {
    case DOF_Idle:

      if (dof.calcAccel(dof.ax) >= 0) {
        //Serial.println("Going to DOF_Ready");
        CurrentDOFID = DOF_Ready;
      }
      break;
    case DOF_Ready:

      if (dof.calcAccel(dof.ax) <= -0.4) {
        //Serial.println("Going to DOF_SwingDown");
        CurrentDOFID = DOF_SwingDown;
      }
      break;
    case DOF_SwingDown:
    
      CurrentDOFID = DOF_End;
      break;
    case DOF_End:

        if (heavyAttackReady) {
        
        LastTime = millis();
        ChargeIndex = 0;
        PulseIndex = 0;
        
        LastButtonState = true;
        heavyAttackReady = false;
        
        CurrentStateId = State_Unload;
        //Serial.println("Going to DOF_Idle");
        CurrentDOFID = DOF_Idle;
        
      } else { 

        //Serial.println("Going to DOF_Idle");
        CurrentDOFID = DOF_Idle;
       }
      break;
  }
}

void setup() {
  Serial.begin(115200);

  buttonSetup();
  LEDStripSetup();
  DOFConfig();


  //DEFAULT STARTING VALUES//
  LastTime = millis();
  ChargeIndex = 0;
  LastButtonState = false;
  CurrentStateId = State_Uncharged;
  PulseIndex = 0;

  delay(10);

}

void loop() {
  LEDStateMachine();
  LEDStripGradientGenerator();
  strip.show();
  isButtonLow();
  DOFLoop();
  DOFStateMachine();

}
