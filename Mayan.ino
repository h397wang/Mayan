#include <Bounce2.h>

#define RELAY_PIN 16
#define NUM_LEDS 3
#define NUM_BUTTONS 10
#define SEQUENCE_COUNT 3
#define LOCKOUT_TIME 300 // amount of time (ms) that inputs from the other pins are locked out for
#define DEBOUNCE_TIME 100 // amount of time (ms) that inputs from that same pin are locked out for (prevents double tap)
#define PLAYER_LOCKOUT_TIME 2 // in seconds, 
#define RESET_BUTTON 5 // index of the button to be held
#define OTHER_RESET_BUTTON 6

const int ledPins[NUM_LEDS] = {2,3,4};
const int buttonPins[NUM_BUTTONS] = {5,6,7,8,9,10,11,12,13,14};
const int correctSequence[SEQUENCE_COUNT] = {2,2,9};


int currentSequence[SEQUENCE_COUNT] = {0,0,0};
int sequenceCounter = 0;

Bounce debounce[NUM_BUTTONS];

int long previousTime = 0; 

bool firstButtonPressed = false;

int ledFlags[NUM_LEDS] = {0,0,0}; // indicates which leds to be lit up

bool isGameOver = false;

void setup(){
  
  for (int i = 0; i < NUM_BUTTONS; i++){
    
    debounce[i] = Bounce();
    debounce[i].attach(buttonPins[i]);
    debounce[i].interval(DEBOUNCE_TIME);   
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < NUM_LEDS; i++){
      pinMode(ledPins[i], OUTPUT);
      digitalWrite(ledPins[i], LOW);
  }
  
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.begin(9600);
}

void loop(){

  // not sure if this is doing anything 
  for (int i = 0; i < NUM_BUTTONS; i++){
    debounce[i].update();   
  }

  if (isGameOver){  
    if (debounce[RESET_BUTTON].read() == LOW && debounce[OTHER_RESET_BUTTON].read() == LOW){
      Serial.println("5 ,6 Being held");
      delay(4000);
      if (debounce[RESET_BUTTON].read() == LOW && debounce[OTHER_RESET_BUTTON].read() == LOW){
        //delay(PLAYER_LOCKOUT_TIME *500);
        reset();
        previousTime = millis(); // this is needed 
      }
    }
    return;
  }
  
  if (millis() - previousTime < LOCKOUT_TIME){
      return; 
  }
  
  // poll the state of the buttons
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


boolean checkSequence(){
  for (int i = 0; i < SEQUENCE_COUNT; i++){
    if (currentSequence[i] != correctSequence[i]){
      return false;
    }
  }
  return true;
}


void pushButton(int i){ // index of the button, 0 to 9

  if (firstButtonPressed == false){
    firstButtonPressed = true;
  }
  
  currentSequence[sequenceCounter] = i;

  // display the index of the button that was just pressed
  Serial.print(i);  

  if (sequenceCounter == 0){
    ledFlags[0] = HIGH;
  }else if (sequenceCounter == 1){
    ledFlags[0] = LOW;
    ledFlags[1] = HIGH;
  }else if (sequenceCounter == SEQUENCE_COUNT - 1){
    ledFlags[1] = LOW;
    ledFlags[2] == HIGH;
     
    if (checkSequence()){
      Serial.println("Correct Sequence");
      win();
    }else{
      Serial.println("Incorrect Sequence");
      reset();
    }
      
    return; // must be here or else sequenceCounter will incremenet unintentionally
  }
  sequenceCounter++;
}


void reset(){

  //only one led is on at any time
  if (isGameOver){
    digitalWrite(ledPins[0], LOW); // temporarily display 
    digitalWrite(ledPins[1], LOW); // temporarily display 
    digitalWrite(ledPins[2], HIGH); // temporarily display 
  }else{
    digitalWrite(ledPins[0], LOW); // temporarily display 
    digitalWrite(ledPins[1], LOW); // temporarily display 
    digitalWrite(ledPins[2], HIGH); // temporarily display 
  }
  
  for (int i = 0; i < SEQUENCE_COUNT; i++){
    currentSequence[i] = 0;
    ledFlags[i] = LOW; // happens to be 3 as well so same loop
  }
    
  firstButtonPressed = false;
  sequenceCounter = 0;
  isGameOver = false;

  digitalWrite(RELAY_PIN, HIGH); // turn on the magnet lock. not redundant
  delay(PLAYER_LOCKOUT_TIME * 500);
}

void win(){
  
  digitalWrite(ledPins[0], LOW); // temporarily display 
  digitalWrite(ledPins[1], LOW); // temporarily display 
  digitalWrite(ledPins[2], HIGH); // temporarily display 
 
  isGameOver = true;
  
  digitalWrite(RELAY_PIN, LOW);// turn off the magnet 
  
  for (int i = 0; i < NUM_LEDS; i++){
    ledFlags[i] = LOW; // reset the leds to blank
  }
  
  delay(PLAYER_LOCKOUT_TIME * 500); // displayed for 1 second
}








