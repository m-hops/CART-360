//LIBRARY DEPENDENCIES//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Adafruit_BNO08x.h>

//DIGITAL PIN SETUP//
#define BUTTON_PIN 7
#define PIXEL_PIN 6
#define PIXEL_COUNT 60

//ANALOG PIN SETUP//

//I2C PIN SETUP//
#define BNO08X_SCL 12
#define BNO08X_SDA 11

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

//ADD ADDITIONAL STATES HERE//
enum StateId{
  State_Uncharged,
  State_Charged,
  State_Unload,
};
StateId CurrentStateId = State_Uncharged;

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

void setup() {
  
  //SET SERIAL.BEGIN TO 9600//
  Serial.begin(9600);

  //USER SETUP FUNCTIONS//
  buttonSetup();
  LEDStripSetup();

  //DEFAULT STARTING VALUES//
  LastTime = millis();
  ChargeIndex = 0;
  LastButtonState = false;
  CurrentStateId = State_Uncharged;
  PulseIndex = 0;
}
void loop() {
  
  switch(CurrentStateId)
  {
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
  strip.show();

  //Serial.println("Works");

}

