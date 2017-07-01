/*
BotRew10s_discreteTrial_101310a.pde
 TopRew_randAlt_020511b.pde
 
 09/10: updated this sketch ( headFixTime2VacToneStep_091409a -> TwoStimLever1_110209a -> discreteTrial) for the new headfix setup using normal Arduino and new Pololu steppers
 ALSO: using Tone library for cue, punishment noise, and reward tones, and using Stepper library to run whisker stim steppers
 
 101310: following up on sketch from 100510, put in check for holding down lever, plus made variables for intertrial interval and timeouts
 (NOTE: previous one had Top reward instead of bottom, so name is actually inaccurate)
 
 112410: now changing the program to trigger the trials when the lever is depressed and reward upon lifting
 
 020511: reduced sensitivity of lever by requiring certain time of continuous lifting for choice triggering.
 Also, started using random number generator library and am forcing alternation of trial type after certain number of consecutive trials.
 Also, broke out important experiment variables to beginning of sketch to allow easy manipulation of experimental parameters.
 ...and made special variables to specify whether top/bottom stim is rewarded.
 
 Aug 2011: modified the script for use in the 2p room
 
 Sep 2011: modified the script to include catch trials for Aniruddha's experiment
 
 Feb 2012: running steppers a little slower to try to address
 problem with stepper arm stopping/not moving sometimes
 Also, waiting a little longer after enable pins activated 
 before moving stim.
 
 010614: to alleviate problems with rewTime5 (based upon arduino time)
 I just made all times printed as millis() instead of based upon some
 previous time/trigger.
 */

//////////////////////////////////////

/////////////////MAIN SESSION VARIABLES
// set these parameters for different trial permutations

String programName = "headfix_RackCatchLickRandDelay_150929a.pde";
String notes = "changed randRew timing method (now at rand time)";

//MOST IMPORTANT
int rewSolDur = 40; // time (in ms) that reward valve is open (80ms for 2p)
int rewSolDur2 = 10;
int rewDir = 2;  // specifies top (1) vs. bottom (2) reward
int contigTrial = 3; // max number of contiguous trials of same type (fixed 091311 so that number is actual, not n-1)
int rewStimFreq = 60; // percent of rewarded trials
int catchFreq = 0;  // percent of catch trials (for Aniruddha experiment)
int cueToneOn = 0; // specify cue tone on (1) or off (0)

int rewDelay = 1;
int maxDelay = 0; // maxDelay*250 = maximum reward delay

//TIMING VARIABLES
int iti = 3000;  // 500 inter-trial interval (time after trial when no new trial can occur) (400 for lever hold iti, 2000 for timed trials)
int levPressThresh = 400;// usu 400ms or 50ms for untrained animals (how long lever has to be pressed to trigger trial)
int choiceLevThresh = 50; // how long lever has to be lifted to signal a choice (usu. 100ms)
int choiceLevThresh2 = 50;   // for unrewarded stim lever press

int stimDur = 3000;  // duration of whisker stimulus (fast = 1500)
int drinkDur = 3000; // time given for animal to drink reward water before stimulus ends (fast=1500)

int nostimTO = 6000; // nostim lever press punishment timeout
int unrewTO = 10000;  // unrewarded stim lever press punishment timeout (8000)

int cueToneDur = 1000; 
int cueFreq = 1000;  // frequency of reward cue tone with Tone library
int rewToneDur = 1000; 
int rewFreq = 2000;  // frequency of reward tone with Tone library

// RANDOM NUMBERS

int rewPunSwitch = 0; //2; // 0; // 2; //
int rewToneSkip = 0; //2; // 0; // 5; //
int rewSolSkip = 0; //5; // 0; // 5; //
int punToneSkip = 0; //2; // 0;  // 2; //
int randRew = 0; //1; // 0;  // 2; //

int rand1 = 0;
int rand2 = 0;
int rand3 = 0;
int rand4 = 0;
int rand5 = 0;

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
int stepCW = -20;  // step size (50 = 1/4 rotation of arm)
int stepCCW = 20;
int stepCCWtop = 20;

int hallSensPin1 = 0;  // analog pin for TOP Hall effect sensor (for stim arm placement)
int hallSensVal1 = 500;   
int hallSensPin2 = 1;  // analog pin for BOT Hall effect sensor (for stim arm placement)
int hallSensVal2 = 500; 
int hallThresh = 200;

int startLevLiftCheck = 0;


///////////// OTHER VARIABLES (NON-LIBRARY)

long randNumber;    // random number for whisker stimulus
long levPressTime = 0;

long del = 0;    // initialize variables for making white noise stimulus (now presented during timeouts) 3/4/10                    
long del2 = 0;

// various time counter variables
long currTime = 0;
long elapTime = 0;  // variable for how long lever has been pressed (for triggering stimulus) or released (for triggering reward)
long elapTime2 = 0;
long elapTime3 = 0;  // variable for how long to play punishment white noise
long levPressDur = 0;

// defining lever pins and variables
int leverPin = 9;    // define pin for mouse lever
int leverPin2 = 8;    // define pin for exp lever
int levState  = 0;
int levState2  = 0;
int prevLevState = 0;
int prevLevState2 = 0;

int lickPin = 3; // analog pin for lick input (from other Arduino in 2p setup)
int lickVal = 0; // analog value of lick input
int lickThresh = 300; // analog value for lick threshold

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

  upperStepper.setSpeed(80);  
  lowerStepper.setSpeed(80);

  pinMode(enable1, OUTPUT);  // ENABLE pin for motor 1 (had forgotten these before)
  pinMode(enable2, OUTPUT);  // ENABLE pin for motor 2

  Serial.begin(9600);    // initialize serial for output to Processing sketch
  randomSeed(analogRead(3));
  Serial.println("Program name:");
  Serial.println(programName);
  Serial.println("Notes:");
  Serial.println(notes);
  Serial.println("rewarded direction = (1=top, 2=bot)");
  Serial.println(rewDir);
  Serial.println("rewarded stimulus frequency = ");
  Serial.println(rewStimFreq);
  Serial.println("contigTrial = ");
  Serial.println(contigTrial);
  Serial.println("reward amount/duration =");
  Serial.println(rewSolDur);
  Serial.println("choiceLevThresh =");
  Serial.println(choiceLevThresh);
  Serial.println("choiceLevThresh2 (catch or unrew) =");
  Serial.println(choiceLevThresh2);
  Serial.println("stimDur =");
  Serial.println(stimDur);
  Serial.println("ITI =");
  Serial.println(iti);
  Serial.println("cueToneOn =");
  Serial.println(cueToneOn);
  
  Serial.println("maxDelay =");
  Serial.println(maxDelay);

  Serial.println("rewToneSkip =");
  Serial.println(rewToneSkip);
  Serial.println("rewSolSkip =");
  Serial.println(rewSolSkip);
  Serial.println("punToneSkip =");
  Serial.println(punToneSkip);
  Serial.println("randRew =");
  Serial.println(randRew);
  Serial.println("rewPunSwitch =");
  Serial.println(rewPunSwitch);

  Serial.println("Session START");
  Serial.println("BEGIN DATA");
}

//LOOP/////////////////////////////////
void loop()          {           // run over and over again


  if (prevLevState2 == 0) {
    levState2 = digitalRead(leverPin2);

    if (levState2 == HIGH) {
      prevLevState2 = 1;
      Serial.println("Session START");
      Serial.println(millis());

      // PLAY TONE
      cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing

      // REWARD SEQUENCE
      // go through reward/vacuum solenoid sequence
//      digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
//      delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
//      digitalWrite(rewPin, LOW);
    }
  }

  //prevLevState2 = 1;

  if (prevLevState2 == 1) {
//    // check for lever presses during this round when no stimulus is present
//    levState = digitalRead(leverPin);
//    levState2 = digitalRead(leverPin2);
//
//    /////////////// see how long lever has been pressed
//    // see if animal is pressing lever to start trial
//    if (levState == HIGH || levState2 == HIGH) { //   && prevLevState == 0) {    // if no stim lever press 
//      if (prevLevState == 0) {
//        levPressTime = millis();
//
//        //prevLevState = 1;  // anytime lever is pressed (checked during nostim and stims), this goes to TRUE
//        Serial.println("lever press start");
//        Serial.println(levPressTime);
//
//        prevLevState = 1;
//
//      }
//      else {  // ELSE if lever was already pressed
//        time = millis();
//        levPressDur = time - levPressTime;  // record how long ago
//      }
//    }
//    else {  // ELSE if lever not pressed
//      if (levState == LOW) {
//        prevLevState = 0;    // and if no lever press, goes to FALSE
//        levPressDur = 0;
//      }
//    }
//
//
//
//    ////////////////////START STIMULUS TRIAL
//
//    //if (prevLevState2 == 1) {
//
//    //    time = millis();
//    //    if (time - endTime > iti)  {    // present next stimulus after some interval
//
//    if (levPressDur > levPressThresh)  {  // start trial if animal has held lever down long enough

        digitalWrite(airpuffPin, HIGH);  // included in 2p room scripts to trigger linescan frames in ScanImage
      delay(100);
      digitalWrite(airpuffPin, LOW); 

      // record time of stimulus trigger
      trigTime2 = millis();

      // generate a random number for determining trial type
      //randNumber = TrueRandom.random(0,100);    // changed 042110, started using TrueRandom library 020311
      randNumber = random(0,100);
      ////// for pseudorandom stimulus presentation...

      //////// CALCULATE STIM TYPE
      // First, see what type of trial the randomizer pulls out
      if (randNumber <= rewStimFreq) { // (rewStimFreq+catchFreq)) && (randNumber > catchFreq)){
        stimType = 1;  // for Rewarded trials
      }
      //      else if (randNumber < catchFreq) {
      //        stimType = 3;
      //      } 
      //      else {
      //        stimType = 2; // for Unrewarded trials
      //      }

      else { 
        stimType = 3; 
      }

      // Then, compare this trial type with the previous
      // NOTE: this doesn't take into account catch trials but these are a small minority of trials and probably would not be important
      if (stimType == 1) {
        if (prevType == 1) { // if previous type is the same as current
          stimCount = stimCount + 1;  // add one to the trial type counter
        }
        else {
          stimCount = 1;  // or if it's different, restart the counter
        }
      }
      else if (stimType == 2) {  // same with the other trial type
        if (prevType == 2) {
          stimCount = stimCount + 1;
        }
        else {
          stimCount = 1;
        }
      }
      else if (stimType == 3) {  // same with the other trial type
        if (prevType == 3) {
          stimCount = stimCount + 1;
        }
        else {
          stimCount = 1;
        }
      }

      // And finally, change trial type if there have been too many in a row
      if (stimCount > contigTrial) {
        if (stimType == 1) {
          stimType = 3;
          stimCount = 1;
        }
        else {
          stimType = 1;
          stimCount = 1;
        }
      } 

      // GIVE REWARDED STIMULUS
      if (stimType == 1) {
        Serial.println("stim trigger (bottom/rewarded)");
        Serial.println(millis());
        //stimCount = stimCount + 1;

        // PRESENT CORRECT/REWARDED STIMULUS
        // move Bottom whisker stimulus into place (~20ms)

        // MOVE TOP/BOTTOM STIMULUS, WHICHEVER ONE IS REWARDED THIS SESSION
        if (rewDir == 1) {
          topStimForward();
        } 
        else {
          botStimForward();
        }

        delay(100);

        // PLAY CUE TONE (if turned on)
        if (cueToneOn == 1) {
          cueTones[0].play(cueFreq, cueToneDur);
        }

        // LEVER PRESS FOR REWARD
        // check for a little while to see if the mouse presses the lever
        elapTime = 0;
        prevLevState = 1;
        levPressDur = 0;

        while (elapTime <= stimDur) {   // for duration of the stimulus, check for lever presses
          time = millis();
          elapTime = time -  trigTime2;

          // read lever state
//          checkLevLift();

          lickVal = analogRead(lickPin);
          
          levState2 = digitalRead(leverPin2);

          // trigger REWARD if animal lifts paw during rewarded stim. (for long enough)
          if (lickVal >= lickThresh || levState2 ==1) {
            
            levPressTime = millis();
            
            if(levState2 == 1) {
              Serial.println("experimenter lever");
              Serial.println(levPressTime);
            }

            rand1 = random(0,100);
            if (rand1 >= rewPunSwitch) {  // and randomly give punishment instead of reward
              reward();
            }
            else {
              Serial.println("rew>pun switch");
              Serial.println(millis());
              punish();
            }
          }  // end IF for levPressDur > 100 (REWARD sequence)

        } // end WHILE elapTime <= stimDur (check for lever lifts)

        if (rewDir == 1) {
          topStimBack();

        } 
        else {
          botStimBack();
        }

        prevLevState = 0;   // reset state variable to allow for consecutive triggers
        prevType = 1; // for stimCount to promote alternation
      }    // end IF conditional for rand numb/reward (or Bot/REW stim vs. Top 092809)

      // GIVE INCORRECT/UNREWARDED STIMULUS
      else if (stimType == 2) {    // ELSE if odd rand numb give Top stim
        Serial.println("stim trigger (top)");
        Serial.println(millis());

        if (rewDir == 1) {
          botStimForward();

        } 
        else {
          topStimForward();
        }

        // record lever presses during non-rewarded stimuli
        elapTime = 0;
        prevLevState = 1;
        levPressDur = 0;

        while (elapTime <= stimDur) {
          time = millis();
          elapTime = time - trigTime2; 

          lickVal = analogRead(lickPin);

          // trigger REWARD if animal lifts paw during rewarded stim. (for long enough)
          if (lickVal >= lickThresh) {
            reward();
          }  // end IF for levPressDur > 100 (REWARD sequence)

        }    // end WHILE for TOP stimulus duration

        //// move the top/unrewarded stimulus back

        if (rewDir == 1) {
          botStimBack();
        } 
        else {
          topStimBack();
        }

        prevLevState = 0;
        prevType = 2; // for stimCount to promote alternation


      }    // end ELSE cond for unrewarded/Top stim.


      // Catch Trial
      else if (stimType == 3) {
        Serial.println("stim trigger (catch trial)");
        Serial.println(millis());
        //stimCount = stimCount + 1;

        // RUN BOTH STIMULI BACKWARD FOR CATCH TRIALS
        //        digitalWrite(enable1, HIGH);
        //        //digitalWrite(airpuffPin, HIGH);
        //        delay(100);
        //        upperStepper.step(20); // 042110: now using Stepper library for stim control


        digitalWrite(enable2, HIGH);
        //digitalWrite(airpuffPin, HIGH);
        delay(100);
        lowerStepper.step(stepCW);
        delay(100);
        digitalWrite(enable2, LOW);
        delay(100);


        // PLAY TONE
        if (cueToneOn == 1) {
          cueTones[0].play(cueFreq, cueToneDur);
        }

        // LEVER PRESS FOR REWARD
        // check for a little while to see if the mouse presses the lever
        elapTime = 0;
        prevLevState = 1;
        levPressDur = 0;

        while (elapTime <= stimDur) {   // for duration of the stimulus, check for lever presses
          time = millis();
          elapTime = time -  trigTime2;

          // read lever state
          lickVal = analogRead(lickPin);
          
          levState2 = digitalRead(leverPin2);

          // trigger REWARD if animal lifts paw during rewarded stim. (for long enough)
          if (lickVal >= lickThresh || levState2 ==1) {
            levPressTime = millis();
            
            if(levState2 == 1) {
              Serial.println("experimenter lever");
              Serial.println(levPressTime);
            }
            
            rand1 = random(0,100);
            if (rand1 >= rewPunSwitch) {  // and randomly give punishment instead of reward
              punish();
            }
            else {
              Serial.println("pun>rew switch");
              Serial.println(millis());
              reward();
            }
          }

        } // end WHILE elapTime <= stimDur (check for lever lifts)

        // MOVE STIM BACK
        //        upperStepper.step(-20); // 042110: now using Stepper library for stim control
        digitalWrite(enable2, HIGH);
        delay(100);  
        lowerStepper.step(stepCCW); // 042110: now using Stepper library for stim control
        delay(100);  
        digitalWrite(enable1, LOW);
        digitalWrite(enable2, LOW);

        prevLevState = 0;   // reset state variable to allow for consecutive triggers
        prevType = 3; // for stimCount to promote alternation
      }    // end IF conditional for rand numb/reward (or Bot/REW stim vs. Top 092809)


      //digitalWrite(ledPin, LOW);
      //endTime = millis();  // added this in for discrete trials (no IR for Headfixed)

      //digitalWrite(airpuffPin, LOW); // included in 2p room scripts to trigger linescan frames in ScanImage

      // record end of this stimulus epoch
      endTime = millis();

      //delay(iti);  // (not using when using timed stimulus trials)
      
      
      // 032214: added this in place of ITI for Timed sessions to allow stopping stim remotely
      while((millis()-endTime)<=iti) {
        if (prevLevState2 == 1) {
          levState2 = digitalRead(leverPin2);
          if(levState2 == 1) {
            prevLevState2 = 0;
            Serial.println("STOP session");
            Serial.println(millis());
          }
        }  
        
        delay(100);  // delay in ITI loop to not give too many rand rew

        // SEE IF YOU'RE GIVING A RANDOM REWARD
        rand5 = random(0,300);
        //    Serial.println("rand5 =");
        //    Serial.println(rand5);
  
        if (rand5 < randRew)  {
  
          Serial.println("random reward!!!");
          Serial.println(millis());
  
          rand2 = random(0,100);
  
          if (rand2 > rewToneSkip)  {
  
            // PLAY TONE
            cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing
  
          } 
          else { 
            Serial.println("random reward tone skip"); 
            Serial.println(millis());
          }
  
          rand3 = random(0,100);
  
          if (rand3 > rewSolSkip) {
  
            // REWARD SEQUENCE
            // go through reward/vacuum solenoid sequence
            digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
            delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
            digitalWrite(rewPin, LOW);
  
            delay(10);
  
            //        digitalWrite(vacPin, HIGH);    // open solenoid valve for a short time
            //        delay(50);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
            //        digitalWrite(vacPin, LOW);
  
          } 
          else { 
            Serial.println("random reward solenoid skip"); 
            Serial.println(millis());
          }
  
          delay(drinkDur);
  
        } // end IF for random rewards
      
      }  // end WHILE loop for ITI checks (STOP session or randRew)

//    }    // end IF for trial time (i.e. end of stimulus epoch)
//
//
//
//    //prevState = state;
//    //  endTime = millis();  // added this in for discrete trials (no IR for Headfixed)
//    //        }  // end IF conditional for lever press triggering start of trial
//
  } // end IF for start of behavioral session (when one presses the button on the Arduino box)
}  // end loop() section


void botStimForward() {
  digitalWrite(enable2, HIGH);
  //digitalWrite(airpuffPin, HIGH);
  delay(100);
  //        lowerStepper.step(stepCCW);

  hallSensVal2 = analogRead(hallSensPin2);
  while (hallSensVal2 >hallThresh) {
    lowerStepper.step(1);
    delay(1);  // slight delay after movement to ensure proper step before next
    hallSensVal2 = analogRead(hallSensPin2);
    //Serial.println(hallSensVal2);
    
        // break out if stim stuck in loop
    if ((millis()-trigTime2)>5000) {
        hallSensVal2 = hallThresh-1;
        Serial.println("stim aborted");
        Serial.println(millis()); 
    }
  }
  delay(100);
  digitalWrite(enable2, LOW);
}

void topStimForward() {
  digitalWrite(enable1, HIGH);
  //digitalWrite(airpuffPin, HIGH);
  delay(100);
  //        upperStepper.step(stepCW); // 042110: now using Stepper library for stim control
  hallSensVal1 = analogRead(hallSensPin1);
  while (hallSensVal1 >hallThresh) {
    upperStepper.step(-1);
    delay(1);  // slight delay after movement to ensure proper step before next
    hallSensVal1 = analogRead(hallSensPin1);
    //Serial.println(hallSensVal1);
    
        // break out if stim stuck in loop
    if ((millis()-trigTime2)>5000) {
        hallSensVal1 = hallThresh-1;
        Serial.println("stim aborted");
        Serial.println(millis()); 
    }
  }
  delay(100);
  digitalWrite(enable1, LOW);
}


void botStimBack() {
  digitalWrite(enable2, HIGH);
  delay(100);  
  lowerStepper.step(stepCW); // 042110: now using Stepper library for stim control
  delay(100);  
  digitalWrite(enable2, LOW);
}


void topStimBack() {
  digitalWrite(enable1, HIGH);
  delay(100);
  upperStepper.step(stepCCW); // 042110: now using Stepper library for stim control
  delay(100);  
  digitalWrite(enable1, LOW);
}

void checkLevLift() {
  levState = digitalRead(leverPin);
  levState2 = digitalRead(leverPin2);

  // see how long animal lifts his hand off the lever during stim presentation
  if (levState == LOW || levState2 == HIGH) {  //HIGH && prevLevState == 0) {  //|| levState2 == HIGH) {    // and if the mouse is still in the beam path, activate reward
    if (prevLevState == 1) {
      levPressTime = millis();
      prevLevState = 0;
    }
    else {
      levPressDur = millis() - levPressTime;
    }
  }
  else {
    prevLevState = 1;
    levPressDur = 0;
  }
}

void reward() {
  Serial.println("REWARD!!!");
  Serial.println(millis());

  rand2 = random(0,100);

  if (rand2 > rewToneSkip)  {

    // PLAY TONE
    cueTones[0].play(rewFreq, rewToneDur);   // changing to a frequency range that's more in the region of mouse hearing

  } 
  else { 
    Serial.println("reward tone skip"); 
    Serial.println(millis());
  }

  rand3 = random(0,100);

  if (rand3 > rewSolSkip) {
    
    rewDelay = 250*random(0,maxDelay);
    
    delay(rewDelay);

    // REWARD SEQUENCE
    // go through reward/vacuum solenoid sequence
    digitalWrite(rewPin, HIGH);    // open solenoid valve for a short time
    delay(rewSolDur);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
    digitalWrite(rewPin, LOW);

    delay(10);

    //              digitalWrite(vacPin, HIGH);    // open solenoid valve for a short time
    //              delay(50);                  // 8ms ~= 8uL of reward liquid (on box #4 011811)
    //              digitalWrite(vacPin, LOW);

  } 
  else { 
    Serial.println("reward solenoid skip"); 
    Serial.println(millis());
  }

  delay(drinkDur);

  elapTime = stimDur + 1;  // break out of the reward stimulus loop after receiving reward

}


void punish() {
  digitalWrite(vacPin, HIGH);    // send output signal for punishment to record analog timing 
  delay(50);                  // 
  digitalWrite(vacPin, LOW);

  //digitalWrite(airpuffPin, HIGH);    // give aversive light for wrong press
  Serial.println("unrewarded lever press");
  Serial.println(millis());
  delay(10);  // changed this from 2000 because air puff goes on falling phase
  //digitalWrite(airpuffPin, LOW);

  rand4 = random(0,100);

  if (rand4 > punToneSkip) {

    elapTime3 = 0;
    while (elapTime3 < unrewTO) {
      del = random(1,7);
      cueTones[0].play(del*1000); //notes[del]);
      currTime = millis();
      elapTime3 = currTime - levPressTime;
    }
    cueTones[0].stop();
    //elapTime = 0;
  } 
  else { 
    Serial.println("punishment tone skip"); 
    Serial.println(millis());
  }

  elapTime = stimDur + 1;  // 091311: need to check and make sure "elapTime1&2" are not in conflict

}

