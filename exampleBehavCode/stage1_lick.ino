/*
BotRew10s_discreteTrial_101310a.pde
 TopRew_randAlt_020511b.pde
 
 09/10: updated this sketch ( headFixTime2VacToneStep_091409a -> TwoStimLever1_110209a -> discreteTrial) for the new headfix setup using normal Arduino and new Pololu steppers
 ALSO: using Tone library for cue, punishment noise, and reward tones, and using Stepper library to run whisker stim steppers
 
 101310: following up on sketch from 100510, put in check for holding down lever, plus made variables for intertrial interval and timeouts
 (NOTE: previous one had Top reward instead of bottom, so name is actually inaccurate)
 
 112410: now changing the program to trigger the trials when the lever is depressed and reward upon lifting
 
 020511: reduced sensitivity of lever by requiring certain time of continuous lifting for choice triggering.
 Also, started usin+-portant experiment variables to beginning of sketch to allow easy manipulation of experimental parameters.
 ...and made special variables to specify whether top/bottom stim is rewarded.
 */

//////////////////////////////////////

/////////////////MAIN SESSION VARIABLES
// set these parameters for different trial permutations

//MOST IMPORTANT
int rewSolDur = 25; // time (in ms) that reward valve is open (determines reward volume): 30ms for box #1 (COM4), 9ms for box#4 (COM7);
int rewDir = 1; // specifies top (1) vs. bottom (2) reward
int contigTrial = 4; //max number of contiguous trials of same type
int rewStimFreq = 100; // percent of rewarded trials
int cueToneOn = 1; // specify cue tone on (1) or off (0)

//LESSER
int iti = 500;  // inter-trial interval (time after trial when no new trial can occur)
int levPressThresh = 100;//(how long lever has to be pressed to trigger trial)
int choiceLevThresh = 40;// how long lever has to be lifted to signal a choice

int stimDur = 3000;  // duratio n of whisker stimulus
int drinkDur = 1000; // time given for animal to drink reward water before stimulus ends

int nostimTO = 12000; // nostim lever press punishment timeout
int unrewTO = 12000;  // unrewarded stim lever press punishment timeout

int cueToneDur = 1000; 
int cueFreq = 1000;  // frequency of reward cue tone with Tone library
int rewToneDur = 1000; 
int rewFreq = 2000;  // frequency of reward tone with Tone library


////////////LIBRARIES
// Initialize Truerandom random number generator library
//#include <TrueRandom.h>

// Initialize TONE library info
#include <Tone.h>
Tone cueTones[1];

// Initialize STEPPER library info
#include <Stepper.h>

int motorPin1 = 13;  // mapping for new boxes, Jan. 2011
int motorPin2 = 12;   
int motorPin3 = 7; 
int motorPin4 = 6;  

#define motorSteps 200  // 200steps/rot with Pololu steppers

Stepper upperStepper(motorSteps, motorPin1, motorPin2);
Stepper lowerStepper(motorSteps, motorPin3, motorPin4);

int enable1 = 11;
int enable2 = 5;

//stepper variables
int delayTime = 5;  
int stepCW = -50;  // step size (50 = 1/4 rotation of arm)
int stepCCW = 50;


///////////// OTHER VARIABLES (NON-LIBRARY)

long randNumber;    // random number for whisker stimulus
long levPressTime = 0;

long del = 0;    // initialize variables for making white noise stimulus (now presented during timeouts) 3/4/10                    
long del2 = 0;

// various time counter variables
long currTime = 0;
long elapTime = 0;
long elapTime2 = 0;
long levPressDur = 0;

// defining lever pins and variables
int leverPin = 9;    // define pin for mouse lever
int leverPin2 = 8;    // define pin for exp lever
int levState  = 0;
int levState2  = 0;
int prevLevState = 0;

int lickPin = 3;
int lickState = 0;

int prevLevState2 = 0; 
long levPressTime2 = 0; 
long levPressDur2 = 0;

// defining speaker and solenoid pins
int rewPin = 4;      //  define pin for turing solenoid valve ON
//int ledPin = 13;                // LED connected to digital pin 13
int vacPin = 3;      // define pin for turning vacuum line ON
int airpuffPin = 2;      // define pin for turning vacuum line OFF
int speakerOut = 10;    // define PWM pin for speaker/tone output

// trial type alternation variables
int stimType = 1;
int prevType = 1;
int stimCount = 0;

// other variables
int totalEntries = 0;    // variable for number of total beam breaks
int trigNum = 0;    // variable for number of actual reward administrations
int trigDuration = 0;  // variable for continuous duration of a single beam break

int state = 0;    // state variables for logging new beam breaks versus continuous
int prevState = 0;

int sensorValue = 100;   // initialize sensor value to something greater than threshold

// time variables
long time = 0;    // initialize time since beginning of trial (I think Processing resets this, conveniently)
long trigTime1 = 0;  // initialize times of beam breaks
long trigTime2 = 0;  // and times of reward

long trialTime = 0;
long endTime = 0;


//SETUP////////////////////////////////
void setup()                   
{
  //pinMode(ledPin, OUTPUT);      // sets the digital pin as LED output
  pinMode(rewPin, OUTPUT);  // sets digital pin for turing solenoid valve ON
  pinMode(vacPin, OUTPUT);  // sets digital pin for turing solenoid valve ON
  pinMode(airpuffPin, OUTPUT); 
  pinMode(leverPin, INPUT);
  pinMode(leverPin2, INPUT);


  cueTones[0].begin(speakerOut);
  //pinMode(speakerOut, OUTPUT); // use this when not using Tone library

  upperStepper.setSpeed(100);  
  lowerStepper.setSpeed(100);
  //  pinMode(motorPin1, OUTPUT); // use these when not using stepper library
  //  pinMode(motorPin2, OUTPUT);
  //  pinMode(motorPin3, OUTPUT);
  //  pinMode(motorPin4, OUTPUT);

  pinMode(enable1, OUTPUT);  // ENABLE pin for motor 1 (had forgotten these before)
  pinMode(enable2, OUTPUT);  // ENABLE pin for motor 2

  Serial.begin(9600);    // initialize serial for output to Processing sketch
  randomSeed(analogRead(2));
}

//LOOP/////////////////////////////////
void loop()          {           // run over and over again

  // check for licks
  
  lickState = analogRead(lickPin);
  //Serial.println(lickState);

  ////////////////////START STIMULUS TRIAL

  if (lickState > 800)  { 
    levPressTime = millis();
    Serial.println("mouse REWARD!!!");
    Serial.println(levPressTime);

    // PLAY TONE
    cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing

    // REWARD SEQUENCE
    // go through reward/vacuum solenoid sequence
    digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
    delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
    digitalWrite(rewPin, LOW);

    delay(drinkDur);

    //// section to vacuum away excess reward water
    //          digitalWrite(vacPin, HIGH);    // turns the vacuum ON to remove remaining liquid
    //          delay(2000);
    //          digitalWrite(vacPin, LOW);    // and then turn the vacuum OFF
    //          //digitalWrite(ledPin, LOW);

    elapTime = stimDur + 1;  // break out of the reward stimulus loop after receiving reward
  }  // end IF for levPressDur > 100 (REWARD sequence)
  
  // NOW DO SAME THING FOR EXPERIMENTER TRIGGERS/REWARDS
    levState2 = digitalRead(leverPin2);

  // see if animal is pressing lever to start trial
  if (levState2 == HIGH) { //   && prevLevState == 0) {    // if no stim lever press 
    if (prevLevState2 == 0) {
      levPressTime2 = millis();

      //prevLevState = 1;  // anytime lever is pressed (checked during nostim and stims), this goes to TRUE until lever is unpressed at sub
      //equent check

      Serial.println("experimenter lever press start");
      Serial.println(levPressTime2);

      prevLevState2 = 1;

    }
    else {  // ELSE if lever was already pressed
      time = millis();
      levPressDur2 = time - levPressTime2;  // record how long ago
    }
  }
  else {  // ELSE if lever not pressed
    if (levState2 == LOW) {
      prevLevState2 = 0;    // and if no lever press, goes to FALSE
      levPressDur2 = 0;
    }
  }

  ////////////////////START STIMULUS TRIAL

  if (levPressDur2 > levPressThresh)  { 
    levPressTime2 = millis();
    Serial.println("experimenter REWARD!!!");
    Serial.println(levPressTime2);

    // PLAY TONE
    cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing

    // REWARD SEQUENCE
    // go through reward/vacuum solenoid sequence
    digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
    delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
    digitalWrite(rewPin, LOW);

    delay(drinkDur);

    elapTime = stimDur + 1;  // break out of the reward stimulus loop after receiving reward
  }  // end IF for levPressDur > 100 (REWARD sequence)
  
  
  
} // end WHILE elapTime <= stimDur (check for lever lifts)

