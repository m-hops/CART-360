//NOTATION IN ALL CAPS ARE DONE IN FULFILLMENT OF ETUDE 2//

//CONNECTS CORROSPONDING PIN SLOT ON ARDUINO BOARD TO A DEFINED, GLOBAL VARIABLE//
//THESE ARE THE LEDS//
#define PIN_LED_BUTTON_1  3   
#define PIN_LED_BUTTON_2  5   
#define PIN_LED_BUTTON_3  9 
#define PIN_LED_BUTTON_4  10  

//ASSIGNS SPEAKER PIN FROM SLOT 11//
#define PIN_PIEZO         11  

//DEFINES TOTAL NUMBER OF ONBOARD LEDS THAT WILL BE UTILIZED BY OUR PROGRAM
#define LED_BUTTON_COUNT  4 

//LED PINS ASSIGNED TO AN INT ARRAY//
static int buttonLookup[LED_BUTTON_COUNT] = { PIN_LED_BUTTON_1, PIN_LED_BUTTON_2, PIN_LED_BUTTON_3, PIN_LED_BUTTON_4 };

//USED FOR GAME RANDOMIZATION//
static long gameSeed;

//VARIABLE STORING A SWITCH STATEMENT TO ASSIGN TONES TO EACH ANALOG BUTTON//
//IN THE EVENT NO BUTTON TONE IS USED, AN ERROR TONE IS PLAYED INSTEAD//
int ledButtonHalfPeriod(int button) {
  switch (button) {
    case PIN_LED_BUTTON_1:
      return 1911;
    case PIN_LED_BUTTON_2:
      return 1703;
    case PIN_LED_BUTTON_3:
      return 1517;
    case PIN_LED_BUTTON_4:
      return 1432;
  }
  return 3822;
}

//RUNS AT TIME OF PUSHING AN ANALOG BUTTON//
bool checkButtonPush(int button) {
  bool result = false;

  //AN ARDUINO FUNCTION THAT CONFIGURES SPECIFIC PINS//
  //IN THIS CASE, WE ARE PASSING IN A BUTTON FROM THE ARRAY AND SETTING IT TO AN INPUT//
  pinMode(button, INPUT);   

  //AN ARDUINO FUNCTION THAT ASSIGNS A HIGH OR LOW VALUE TO A DIGITAL PIN//
  //IN THIS CASE, WE ARE PASSING IN A BUTTON FROM THE ARRAY AND SETTING IT TO HIGH//
  //HIGH = GROUNDED & LOW = 5V (OR 3.3V)//
  digitalWrite(button, HIGH);
  
  //PULLS THE APPROPRIATE TONE FOR THE DEFINED BUTTON VALUE//
  int halfPeriod = ledButtonHalfPeriod(button);
  
  //DIGITALREAD() IS AN ARDUNIO FUNCTION THAT CHECKS ON THE CURRENT DIGITAL WRITE VALUE OF VARIABLE//
  //IN THIS CASE, THIS WHILE LOOP WILL WORK AS LONG AS THE BUTTON ON THE BOARD IS POWERED//
  while (digitalRead(button) == LOW) {
    
    //SWITCHES BUTTON BOOL TO TRUE IF THE CORRECT BUTTON IS PRESSED//
    result = true;

    //PLAYS TONE FOR ASSOCIATED BUTTON OUT OF THE SPEAKER AND THEN STOPS THE SAME TONE SHORTLY AFTER//
    digitalWrite(PIN_PIEZO, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PIN_PIEZO, LOW);
    delayMicroseconds(halfPeriod);
  }

  //RETURNS RESULT AS EITHER TRUE OR FALSE//
  return result;
}

//A FOR LOOP SETUP TO RETURN WHICH BUTTON WAS PRESSED//
//WILL RETURN A VALUE OF PIN_LED_BUTTON 1-4 FOR ANALOG BUTTONS AND -1 IF THE CHECK FAILS//
int getButtonPush(void) {
  for (int i = 0; i < LED_BUTTON_COUNT; ++i) {
    if (checkButtonPush(buttonLookup[i])) {
      return i;
    }
  }
  return -1;
}

//FUNCTION THAT TAKES IN TWO VALUES WHICH DETERMINE WHICH LED IS TO BE IMPACTED AND FOR HOW LONG//
void displayLightAndSound(int led, int duration) {

  if (led >= 0) {

     //TRANSFORMS PINMODE OF A SPECIFICED LED PIN TO OUTPUT//
    pinMode(led, OUTPUT); 

    //POWERS ON THE PREVIOUSLY SPECIFIED LED//
    digitalWrite(led, LOW); 
  }
  
  //PREPS DURATION OF ASSOCIATED LED & TONE AND STORES THEM INTO VARIABLES// 
  int halfPeriod = ledButtonHalfPeriod(led);
  long cycleCount = (long)duration * 500;
  cycleCount /= halfPeriod;

  //RUNS THE DEFINED LED TONE WHILE THE TIMER VARIABLE RUNS DOWN//
  for (; cycleCount > 0; --cycleCount) {
    digitalWrite(PIN_PIEZO, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(PIN_PIEZO, LOW);
    delayMicroseconds(halfPeriod);
  }
  
  if (led >= 0) {

    //TRANSFORMS SPECIFIED LED PIN BACK TO INPUT//
    pinMode(led, INPUT);      

    //GROUNDS SPECIFIC LED//
    digitalWrite(led, HIGH);  
  }
}

//DEFINES LED/BUTTON COMBO ACTIONS// 
void startUpLightsAndSound(void) {
  
  //RUNS THROUGH EACH OF THE LED/BUTTON VARIABLES AND HOW LONG THEY'LL EACH RUN//
  displayLightAndSound(PIN_LED_BUTTON_1, 100);
  displayLightAndSound(PIN_LED_BUTTON_2, 100);
  displayLightAndSound(PIN_LED_BUTTON_3, 100);
  displayLightAndSound(PIN_LED_BUTTON_4, 100);

  //CREATES A DELAY AFTER PUSHING A BUTTON//
  while (getButtonPush() < 0) {
    delay(100);
  }
  
  //DELAY TO AVOID POTENTIAL BUGS//
  delay(500);
}

//SETUP RUNS ONCE AT STARTUP//
void setup() {

  pinMode(PIN_LED_BUTTON_1, INPUT);
  pinMode(PIN_LED_BUTTON_2, INPUT);
  pinMode(PIN_LED_BUTTON_3, INPUT);
  pinMode(PIN_LED_BUTTON_4, INPUT);
  
  pinMode(PIN_PIEZO, OUTPUT);

  startUpLightsAndSound();

  //MICROS IS AN ARDUINO FUNCTION THAT KEEPS TRACK OF THE MICROSECONDS SINCE THE PROGRAM STARTED//
  gameSeed = micros();
}

//LOOP RUNS CONTINOUSLY//
void loop() {

  //STORES CURRENT LENGTH OF GAME SEQUENCE//
  static int sequenceLength = 1;

  //ALLOWS FOR A *NEAR* INFINITE GROWTH OF THE SIMON SAYS GAME//
  int i;
  
  //RANDOMSEED() IS AN ARDUINO FUNCTION WHICH ACTS AS A PSEUDO-RANDOM NUMBER GENERATOR//
  randomSeed(gameSeed);

  //GROWS AND STORES NEW ENTRIES INTO THE SIMON SAYS SEQUENCE//
  for (i = sequenceLength; i > 0; --i) {
    displayLightAndSound(buttonLookup[random(LED_BUTTON_COUNT)], 250);
    delay(250);
  }
  
  //RUNS A SECOND RANDOMIZATION OF THE GAMESEED VARIABLE//
  randomSeed(gameSeed);

  //STORES PLAYERS INPUT TO CHECK AGAINST THE EXISTING SIMON SAYS SEQUENCE//
  for (int matches = 0; matches < sequenceLength; ++matches) {
    int button = -1;
    int timeout;
    // Wait for the player to press a button (with 3 second timeout)
    for (button = -1, timeout = 30; timeout && button < 0; --timeout) {
      button = getButtonPush();
      delay(100);
    }
  
    //RESETS GAME AND PLAYS FAILURE TONE IF INCORRECT BUTTON IS PRESSED//
    if (button != random(LED_BUTTON_COUNT)) {
      
      displayLightAndSound(-1, 1000);
      gameSeed = micros();
      delay(1000);
      sequenceLength = 0;
      startUpLightsAndSound();
    }
  }

  ++sequenceLength;
  delay(1000);
}
