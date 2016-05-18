#include <Bounce2.h>

#define RELAY_PIN 16
#define NUM_LEDS 3
#define NUM_BUTTONS 10
#define SEQUENCE_COUNT 3
#define LOCKOUT_TIME 300 // amount of time (ms) that inputs from the other pins are locked out for
#define DEBOUNCE_TIME 100 // amount of time (ms) that inputs from that same pin are locked out for (prevents double tap)
#define RESET_TIME 5 // amount of time (s) between button presses such that if exceeded -> fail sequence and resets
#define PLAYER_LOCKOUT_TIME 2 // in seconds, 
#define RESET_BUTTON 5 // index of the button to be held

const int ledPins[NUM_LEDS] = {2,3,4};
const int buttonPins[NUM_BUTTONS] = {5,6,7,8,9,10,11,12,13,14};
const int correctSequence[SEQUENCE_COUNT] = {2,2,9};


int currentSequence[SEQUENCE_COUNT] = {0,0,0};
int sequenceCounter = 0;
int resetTimer = RESET_TIME;
int gameOverTimer = PLAYER_LOCKOUT_TIME;


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

  // INITIALIZE TIMER INTERRUPTS
  cli(); // disable global interrupt
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
  // Bit: 7,6,5,4,3,2,1,0
  // WGM12: 3, CS10: 1, CS12: 2
  // the value on the left by the number of bits on the right
  TCCR1B |= (1 << WGM12); // turn on CTC mode. clear timer on compare match
  TCCR1B |= (1 << CS10); // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12);
  // basically TCCR1B is 0000 1110
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei(); // enable global interrupts
  
  Serial.begin(9600);
}

void loop(){

  if (isGameOver){
    return;
  }
  // not sure if this is doing anything 
  for (int i = 0; i < NUM_BUTTONS; i++){
    debounce[i].update();   
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
    ledFlags[1] = HIGH;
  }else if (sequenceCounter == SEQUENCE_COUNT - 1){
    
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
  
  resetTimer = RESET_TIME;
  sequenceCounter++;
}


void reset(){

  for (int i = 0; i < NUM_LEDS; i++){
    digitalWrite(ledPins[i], HIGH); // temporarily display 
  }
  
  for (int i = 0; i < SEQUENCE_COUNT; i++){
    currentSequence[i] = 0;
    ledFlags[i] = LOW; // happens to be 3 as well so same loop
  }
    
  firstButtonPressed = false;
  sequenceCounter = 0;
  isGameOver = false;
  resetTimer = RESET_TIME;

  digitalWrite(RELAY_PIN, HIGH); // turn on the magnet lock. not redundant
  delay(PLAYER_LOCKOUT_TIME * 500);
}


// sends an interrupt every second
ISR(TIMER1_COMPA_vect){ 
 
  if (isGameOver){
    debounce[RESET_BUTTON].update();
    Serial.println("game over time");
    if (resetTimer == 0){
      Serial.println("reset by holding");
      reset();  
    }else if (debounce[RESET_BUTTON].read() == LOW){
      Serial.println("button 5 being held");
      resetTimer--;    
    }
    
  }else{
    
    if (firstButtonPressed){
      resetTimer--; // or count up, same shit
    } 
  
    if (resetTimer == 0){
      reset();
      Serial.println("Reset, waited too long in between "); // time between button presses too long
    }
  }
  

}

void win(){
  
  // temporarily display the led sequence
  for (int i = 0; i < NUM_LEDS; i++){
    digitalWrite(ledPins[i], HIGH);
  }
 
  isGameOver = true;
  
  digitalWrite(RELAY_PIN, LOW);// turn off the magnet 
  
  for (int i = 0; i < NUM_LEDS; i++){
    ledFlags[i] = LOW; // reset the leds to blank
  }
  
  delay(PLAYER_LOCKOUT_TIME * 500); // displayed for 1 second
}








