/*
 * Asteroid X Pinball v0.1
 * 10.april 2017
 * Anders Holm
 */

#define MAX7219_DIN 6  // set up for the display
#define MAX7219_CS  7
#define MAX7219_CLK 8

#define targetCount 4   // set up the number of targets in the target set

#define targetLed1 A0
#define targetLed2 A1
#define targetLed3 A2
#define targetLed4 A3

#define targetPin1 A4
#define targetPin2 A5
#define targetPin3 5
#define targetPin4 12

#define GREEN 2
#define BLUE 3
#define RED 4
#define blueLed1 9
#define blueLed2 10
#define blueLed3 11

// define function for LED fade
#define fadeDefault 0
#define fadeTargetHit 1
// State Variable for LED fucntion
byte LEDfunction = fadeTargetHit;
int orangefadetime = 1000;                       // Run the new fade this long
unsigned long previousOrangeFadeMillis = 0;      // The clock time in millis()
unsigned long howLongItsBeen;                   // A calculated value


#define blueLedcount 3   // set up the total number of Led in the Led set

struct blueled   
{                                             // See corresponding table below
  byte blueLedPin;                            // A
  byte blueLedfadedirection;                  // B
  int blueLedVal;                             // C
  int blueLedfadeInterval;                    // D
  unsigned long previousblueLedFadeMillis;    // E
};
                                //    A      B  C  D  E    
blueled blueleds[blueLedcount] = {{blueLed1, 0, 0, 5, 0}
                                 ,{blueLed2, 0, 0, 10, 0}
                                 ,{blueLed3, 0, 0, 15, 0}
};

//The RGB LEDS
#define UP 0                                // define directions for rgb LED fade
#define DOWN 1
byte fadeDirection = UP;                    // State Variable for rgb Fade Direction
int redVal = 0;
int greenVal = 0;
int blueVal = 0;
int fadeInterval = 10;                // delaytime between led value increments
unsigned long previousFadeMillis;    // millis() timing Variable, just for fading

struct target   
{                         
  byte detectPin;
  byte ledPin;
  byte hit;
  byte state;
  byte laststate;
};

const int debugInterval = 10;           // number of millisecs between serial prints, for debugging
unsigned long previuosdebug = 0;        // for debugging using serial

const int debounceinterval = 10;        // switch debounce, number of millisecs between checks
unsigned long previuosdebounce = 0;

uint16_t score;       //this is where score is put

target targets[targetCount] = {{targetPin1,targetLed1, 0, 0, 0}
                              ,{targetPin2,targetLed2, 0, 0, 0}
                              ,{targetPin3,targetLed3, 0, 0, 0}
                              ,{targetPin4,targetLed4, 0, 0, 0}
};


// the setup function runs once when you press reset or power the board
void setup() {
  
  Serial.begin(9600);
  Serial.println("Starting the PINBALL TARGET sketch");  // to know what sketch is running
 
  pinMode(targetLed1, OUTPUT);
  pinMode(targetLed2, OUTPUT);
  pinMode(targetLed3, OUTPUT);
  pinMode(targetLed4, OUTPUT);
  pinMode(targetPin1, INPUT);
  pinMode(targetPin2, INPUT);
  pinMode(targetPin3, INPUT);
  pinMode(targetPin4, INPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(blueLed1, OUTPUT);
  pinMode(blueLed3, OUTPUT);
  pinMode(blueLed2, OUTPUT);
  
  initialiseMAX7219();
  output(0x0f, 0x00);    //display test register - test mode off
  output(0x0c, 0x01);    //shutdown register - normal operation
  output(0x0b, 0x07);    //scan limit register - display digits 0 thru 7
  output(0x0a, 0x0f);    //intensity register - max brightness
  output(0x09, 0xff);    //decode mode register - CodeB decode all digits
  output(0x08, 0x0c);    //digit 7 (leftmost digit) data
  output(0x07, 0x0b);
  output(0x06, 0x0d);
  output(0x05, 0x0e);
  output(0x04, 0x08);
  output(0x03, 0x07);
  output(0x02, 0x06);
  output(0x01, 0x05); //digit 0 (rightmost digit) data
 }


// the loop function runs over and over again forever
void loop() 
{
  CheckTargets();
  redrawTargets();
  checkAllTargets();
  displayScore(score);
  unsigned long currentMillis = millis();
  RGBled(currentMillis);
  RGBledOrangeFade(currentMillis);
  blueLEDpulse(currentMillis);
  UpdateLEDs();

  howLongItsBeen = millis() - previousOrangeFadeMillis;
         if ( howLongItsBeen >= orangefadetime ) {
              LEDfunction = fadeDefault;    //runs all the time unnless button is pushed
         }else{
              LEDfunction = fadeTargetHit;   //change fade function
         }
  
  //debug();            // serial print for debugging
}


void debug(){                  
if (millis() - previuosdebug >= debugInterval) {
   Serial.print("score: ");
   Serial.print(score);
   Serial.print("     redVal: ");
   Serial.print(redVal);
   Serial.print("     greenVal: ");
   Serial.print(greenVal);
   Serial.print("     blueVal: ");
   Serial.println(blueVal);
previuosdebug += debugInterval;   }
}


// 
void UpdateLEDs(){
                analogWrite( RED, 255 - redVal );
                analogWrite( GREEN, 255 - greenVal );
                analogWrite( BLUE, 255 - blueVal );                 
}

//This shows the score variable on the 8-digit display
void displayScore(int score)
{
  //calculate number of digits
  String myString = String(score);
  int numberOfDigits = myString.length();
    //clear unused digits
    for (int i = 8; i > numberOfDigits; i--)
    {
      output(i, 0x0F);
    }
  
    //write digits to the display
    for (int i = 1; i <= numberOfDigits; i++)
    {
      output(i, myString[numberOfDigits-i]);
    }
} //end of score display




//This checks if targets are hit
void CheckTargets() {
if (millis() - previuosdebounce >= debounceinterval) {              // Delay a little bit to avoid switch bouncing  
          for(int i = 0; i < targetCount; i++) {
          targets[i].state = digitalRead(targets[i].detectPin);     // read the switch input pin
                 if(targets[i].state != targets[i].laststate) {    // Compare the switch state to its previous state
                    if (targets[i].state == HIGH) {                // if switch is pressed
                    } else {                                        // else, when switch is released 
                      targets[i].hit = 1;                           // mark as target is hit
                      score += 5;                                   // and add points
                      Serial.print("Target hit! "); Serial.print("New score: "); Serial.print(score); Serial.print("!");Serial.println();
                      previousOrangeFadeMillis = millis();          // to start the rgb led orange fade
                    }
                  targets[i].laststate = targets[i].state;          // save the current state as the last state, for next time through the loop
                  }
          }   //end for loop
previuosdebounce += debounceinterval;
}   //end if debounce   
}   //end CheckTargets function





void redrawTargets() {
  for(int i = 0; i < targetCount; i++)                    // targets that are hit, have value 1
    digitalWrite(targets[i].ledPin, 1-targets[i].hit);    // and will not have lights
}

void dimTargets() {
  for(int i = 0; i < targetCount; i++)                    // all values set to low
    digitalWrite(targets[i].ledPin, LOW);                 // all targets off
}

void checkAllTargets() {                                  // checking if all targets are hit
  for(int i = 0; i < targetCount; i++)
    if(targets[i].hit == 0)                  
      return;
    score += 200;                                         // add a bonus
    Serial.print("..and 200 Bonus! New score: "); Serial.println(score); 
    for(int i = 0; i < targetCount; i++)           // reset all target values to zero
    targets[i].hit = 0;                            // this makes redrawtargets() show all targets again
    blinkTargets();
 
}

void blinkTargets() {       
previousOrangeFadeMillis = millis();        // to start the rgb led orange fade
    for(int i = 0; i < 5; i++)            
  {
    redrawTargets();
    delay(100);               // using delay for the blinking needs to be
    dimTargets();             // changed with blink-without-delay type code
    delay(100);               // when used in a complete pinball game. 
  }
  redrawTargets();
  
}

void initialiseMAX7219() {              //  MAX7219 7-Segment LED Display 
  digitalWrite(MAX7219_CS, HIGH);
  pinMode(MAX7219_DIN, OUTPUT);
  pinMode(MAX7219_CS, OUTPUT);
  pinMode(MAX7219_CLK, OUTPUT);
}

void output(byte address, byte data) {   //common routine to talkt o MAX7219
  digitalWrite(MAX7219_CS, LOW);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, address);
  shiftOut(MAX7219_DIN, MAX7219_CLK, MSBFIRST, data);
  digitalWrite(MAX7219_CS, HIGH);
}

// the blue led pulse fade function
void blueLEDpulse(unsigned long thisMillis) {
     for(int i = 0; i < blueLedcount; i++) {
              if (thisMillis - blueleds[i].previousblueLedFadeMillis >= blueleds[i].blueLedfadeInterval) {
                      if (blueleds[i].blueLedfadedirection == UP) {          // fading up
                      blueleds[i].blueLedVal += 1;
                      analogWrite(blueleds[i].blueLedPin, 255 - blueleds[i].blueLedVal);
                             if (blueleds[i].blueLedVal >= 255) {   // At max, limit and change direction
                                  blueleds[i].blueLedVal = 255;
                                  blueleds[i].blueLedfadedirection = DOWN;
                             }
                      } else {                             // if not up, then down
                      blueleds[i].blueLedVal -= 1;
                      analogWrite(blueleds[i].blueLedPin, 255 - blueleds[i].blueLedVal);  
                              if (blueleds[i].blueLedVal <= 0 ) {
                                   blueleds[i].blueLedVal = 0;        // At min, limit and change direction
                                   blueleds[i].blueLedfadedirection = UP;
                              }
                      }
                  blueleds[i].previousblueLedFadeMillis = thisMillis;   // reset millis for the next iteration (fade timer only)
                } //end if thisMillis
     } //end for loop
}

// The RGB led set for default colors
void RGBled(unsigned long thisMillis) {
  if (LEDfunction == fadeDefault) {  //Only run this function if button is pushed
          if (thisMillis - previousFadeMillis >= fadeInterval) {
                        if (fadeDirection == UP) {         
                                    redVal = 0;
                                    greenVal += 1;
                                    blueVal += 2;  
                                    if (greenVal >= 120 && blueVal >= 240) { // At max, limit and change direction
                                      greenVal = 120;
                                      blueVal = 240;
                                      fadeDirection = DOWN;
                                    }
                        } else {   //if we aren't going up, we're going down, swopping increments here
                                    redVal = 0;
                                    greenVal -= 1;
                                    blueVal -= 2;  
                                    if (greenVal <= 0 && blueVal <= 0) {
                                      greenVal = 0;         // At min, limit and change direction
                                      blueVal = 0;
                                      fadeDirection = UP;
                                    }
                         }
                      previousFadeMillis = thisMillis;   // reset millis for the next iteration (fade timer only)
                  }
    } // end if LEDfunction            
}
// The RGB led set for target hit colors
void RGBledOrangeFade(unsigned long thisMillis) {
  if (LEDfunction == fadeTargetHit) {  //Only run this function if button is pushed
     if (thisMillis - previousFadeMillis >= fadeInterval) {
                  if (fadeDirection == UP) {         
                              redVal = 255;
                              greenVal += 10;
                              blueVal = 0;  
                              if (greenVal >= 150) { // At max, limit and change direction
                                greenVal = 150;    
                                fadeDirection = DOWN;
                              }
                  } else {   //if we aren't going up, we're going down, swopping increments here
                              redVal = 255;
                              greenVal -= 10; 
                              blueVal = 0; 
                              if (greenVal <= 0) {
                                greenVal = 0;         // At min, limit and change direction
                                fadeDirection = UP;
                              }
                   }
                previousFadeMillis = thisMillis;   // reset millis for the next iteration (fade timer only)
            }

    } // end if LEDfunction
}



