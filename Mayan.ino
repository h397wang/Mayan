#include <Bounce2.h>

/*
 * Keypad system to replace the 229 lock on the door leading to the 
 * third room in Mayan. 
 * Three leds (enumerated as 0,1 and 2) are all initially off
 * The way the leds are lit up is to indicate the state of the input sequence
 * Three led states are as follows: all leds off, only led0 on, and only led1 on
 * Transitioning from only led1 on to all leds off, causes led2 briefly flash for 1 second
 * Pushing a button causes a state transition 
 * Note: Button presses are NOT registered by edge-triggering 
 * Therefore holding the button will register as multiple button presses
 */
 
#define RELAY_PIN 13
#define NUM_LEDS 3
#define NUM_BUTTONS 10
#define SEQUENCE_COUNT 3
#define LOCKOUT_TIME 300 // time (ms) that inputs from the OTHER pins are locked out
#define DEBOUNCE_TIME 100 // time (ms) that inputs from that SAME pin are locked out (prevents double tapping)
#define PLAYER_LOCKOUT_TIME 1000 //  time (ms) led2 is displayed for 
#define RESET_BUTTON 5 // index of the button to be held 
#define OTHER_RESET_BUTTON 6 // both buttons are to be held for the full reset
#define RESET_BUTTON_HOLD_TIME 3000 // time (ms) that the reset buttons must be held for

const int ledPins[NUM_LEDS] = {14,15,16};
const int buttonPins[NUM_BUTTONS] = {2,3,4,5,6,7,8,9,10,11};
const int correctSequence[SEQUENCE_COUNT] = {2,2,9};

int currentSequence[SEQUENCE_COUNT] = {0,0,0};
int sequenceCounter = 0; // ranges from 0 to 2

Bounce debounce[NUM_BUTTONS];

int long previousTime = 0; // keeps track of when the previous button was pushed

int ledFlags[NUM_LEDS] = {0,0,0}; // indicates which leds are to be on

bool isPuzzleSolved = false; 

// BUTTON, LED, & RELAY SETUP
void setup(){
  
  for (int i = 0; i < NUM_BUTTONS; i++){ // initiates debounce 
    debounce[i] = Bounce();
    debounce[i].attach(buttonPins[i]);
    debounce[i].interval(DEBOUNCE_TIME);   
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUM_LEDS; i++){ // turns off all leds 
      pinMode(ledPins[i], OUTPUT);
      digitalWrite(ledPins[i], LOW);
  }
  
  pinMode(RELAY_PIN, OUTPUT); // initiates relay button
  digitalWrite(RELAY_PIN, HIGH); // turns the magnet on to lock the door

  Serial.begin(9600); // initiates printing 
}

// MAIN LOOP
void loop(){

  for (int i = 0; i < NUM_BUTTONS; i++){ // updates state of buttons
    debounce[i].update();   
  }

  // Reset by holding the buttons 5 and 6 at the same time for 3 seconds
  // Polls the state of the reset buttons, waits 3 seconds then checks the state again
  if (isPuzzleSolved){  
    if (debounce[RESET_BUTTON].read() == LOW && debounce[OTHER_RESET_BUTTON].read() == LOW){ // checking if 5 and 6 are being held 
      Serial.println("5 ,6 Being held");
      delay(RESET_BUTTON_HOLD_TIME); // delays everything for 3 seconds 
      if (debounce[RESET_BUTTON].read() == LOW && debounce[OTHER_RESET_BUTTON].read() == LOW){ // checks if 5 and 6 are still being held
        reset(); 
        previousTime = millis(); // temporarily ignores input, or else registers a button press 
      }
    }
    return; // if this puzzle has been solved then the keypad is disabled
  }

  // prevents double tapping
  if (millis() - previousTime < LOCKOUT_TIME){
      return; 
  }
  
  // poll the state of the buttons for whichever was pushed
  for (int i = 0; i < NUM_BUTTONS; i++){
    if (debounce[i].read() == LOW){
      pushButton(i);
      previousTime = millis();
      break;
    }
  } 
  // light up the leds according to the flags
  for (int i = 0; i < NUM_LEDS; i++){
    digitalWrite(ledPins[i], ledFlags[i]);  
  }
}

// HELPER FUNCTIONS 
void checkSequence(){ // checks currentSequence for correctness 
  for (int i = 0; i < SEQUENCE_COUNT; i++){
    if (currentSequence[i] != correctSequence[i]){
      reset(); // resets game
      Serial.println("Incorrect Sequence");
    }
  }
  win(); // turns off relay and releases magnet 
  Serial.println("Correct Sequence"); 
}

void pushButton(int i){ // recieves button number i from poll, appends i to currentSequence, and lights leds 
  currentSequence[sequenceCounter] = i; // sets currentSequence length and appends i to the sequence
  Serial.print(i);  
  
  // change the led flags accordingly, exception of last case
  if (sequenceCounter == 0){ // led0 is on until next button press
    ledFlags[0] = HIGH;
  }
  else if (sequenceCounter == 1){ // led1 is on until next button press 
    ledFlags[0] = LOW;
    ledFlags[1] = HIGH;
  }
  else if (sequenceCounter == 2){ // led2 is on for 1 second 
    digitalWrite(ledPins[0], LOW);
    digitalWrite(ledPins[1], LOW);
    digitalWrite(ledPins[2], HIGH);
    delay(PLAYER_LOCKOUT_TIME); // allows led2 to be on for 1 second
    
    return; // must return here or else sequenceCounter will incremenet unintentionally
  }
  sequenceCounter++;
}

void win(){ // when currentSequence is correct win() opens door
  isPuzzleSolved = true;
  digitalWrite(RELAY_PIN, LOW);// turn off the magnet 
  
  for (int i = 0; i < NUM_LEDS; i++){
    ledFlags[i] = LOW; // reset the leds, all off
  }
}
