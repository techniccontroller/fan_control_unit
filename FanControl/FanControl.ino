/*
 * Fancontrol of FKT Heating
 * 
 * created by EdgarW 08.12.2020
 */

#define PIN_WIFISWITCH 6
#define PIN_PUSHBUTTON 3
#define PIN_LED 7
#define PIN_FANPOWER 13
#define PIN_FANPWM 5
#define PIN_CODE1 12
#define PIN_CODE2 11
#define PIN_CODE3 10

#define HOUR_MILLIS 3600000L

// define states of statemachine
#define OFF 0
#define MEDIUM 1
#define INTENSE 2


// define global variables
long starttime = 0;                     // time of entering INTENSE state
int lvlIntense = 255;                   // fanspeed level in state INTENSE
int lvlMedium = 100;                    // fanspeed level in state MEDIUM (adjusted via jumpers during setup)
long timeIntense = HOUR_MILLIS * 0.5;   //30min -> time in state INTENSE
long timeMedium = HOUR_MILLIS * 24L;    //24h -> time in state MEDIUM (adjusted via jumpers during setup)
int state = OFF;                        // current state
int lastSwitchState = LOW;              // last state of Wifi switch
int ledState = 0;                       // current led state (0 or 1)
long lastRunTime = 0;                   // time of last run of statemachine cycle
long presstime = 0;                     // time of detected press on pushbutton                  

void setup() {
  Serial.begin(9600);

  // setup pinmode of all pins
  pinMode(PIN_WIFISWITCH, INPUT);
  pinMode(PIN_PUSHBUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_FANPOWER, OUTPUT);
  pinMode(PIN_FANPWM, OUTPUT);
  pinMode(PIN_CODE1, INPUT_PULLUP);
  pinMode(PIN_CODE2, INPUT_PULLUP);
  pinMode(PIN_CODE3, INPUT_PULLUP);

  delay(200);

  // read coding via jumpers
  byte coding = 0;
  if(digitalRead(PIN_CODE1) == LOW){
    coding = 1;
  }
  if(digitalRead(PIN_CODE2) == LOW){
    coding += 2;
  }
  if(digitalRead(PIN_CODE3) == LOW){
    coding += 4;
  }
  Serial.print("Coding: ");
  Serial.println(coding);

  // analyse the coding and set the parameters
  switch(coding){
    case 0:
      lvlMedium = 255 * 0.2;
      timeMedium = HOUR_MILLIS * 12L;
      break;
    case 1:
      lvlMedium = 255 * 0.2;
      timeMedium = HOUR_MILLIS * 24L;
      break;
    case 2:
      lvlMedium = 255 * 0.3;
      timeMedium = HOUR_MILLIS * 48L;
      break;
    case 3:
      lvlMedium = 255 * 0.3;
      timeMedium = HOUR_MILLIS * 24L;
      break;
    case 4:
      lvlMedium = 255 * 0.4;
      timeMedium = HOUR_MILLIS * 48L;
      break;
    case 5:
      lvlMedium = 255 * 0.4;
      timeMedium = HOUR_MILLIS * 24L;
      break;
    case 6:
      lvlMedium = 255 * 0.5;
      timeMedium = HOUR_MILLIS * 12L;
      break;
    case 7:
      lvlMedium = 255 * 0.5;
      timeMedium = HOUR_MILLIS * 24L;
      break;
  }

  Serial.print("lvlMedium = ");
  Serial.println(lvlMedium);
  Serial.print("timeMedium (h) = ");
  Serial.println(timeMedium / HOUR_MILLIS);

  lastSwitchState = digitalRead(PIN_WIFISWITCH);
}

void loop() {

  // Read pushbutton state
  if(PIND & B00001000){
    // PUSHBUTTON is pressed -> do nothing
  }
  else{
    // PUSHBUTTON is not pressed 
    long pressduration = millis() - presstime;
    if(pressduration > 200){
      if(pressduration < 5000){
        // key was pressed longer than 200ms and less than 5sec
        Serial.println("switch to State INTENSE");
        state = INTENSE;
        starttime = millis();
      }
      else if(pressduration < 20000){
        // key was pressed longer then 10sec but less than 20sec
        Serial.println("switch to State OFF");
        state = OFF;
      }
      Serial.println(pressduration);
    }
   
    // reset presstime
    presstime = millis();
  }
  
  
  // run statemachine every 500ms
  if((millis() - lastRunTime) > 500){

    // check for statechange of Wifi Switch
    int currentSwitchState =  digitalRead(PIN_WIFISWITCH);
    if(currentSwitchState != lastSwitchState){
      // Wifi Switch was pressed
      Serial.println("Wifi switch pressed");
      state = INTENSE;
      starttime = millis();
    }
    lastSwitchState = currentSwitchState;
    
    // Get snapshot of time
    unsigned long currentMillis = millis();
  
    if (state == INTENSE){
      Serial.println("State INTENSE");
      digitalWrite(PIN_FANPOWER, HIGH);
      analogWrite(PIN_FANPWM, lvlIntense);
      // blinking of LED
      if(ledState > 0){
        digitalWrite(PIN_LED, HIGH);
        ledState = 0;
      }
      else{
        digitalWrite(PIN_LED, LOW);
        ledState++;
      }
      // How much time has passed, accounting for rollover with subtraction!
      if ((unsigned long)(currentMillis - starttime) >= timeIntense) {
        Serial.println("switch to State MEDIUM");
        state = MEDIUM;
      }
    }
    else if (state == MEDIUM){
      Serial.println("State MEDIUM");
      digitalWrite(PIN_FANPOWER, HIGH);
      analogWrite(PIN_FANPWM, lvlMedium);
      //turn LED constantly on 
      digitalWrite(PIN_LED, HIGH);
      // How much time has passed, accounting for rollover with subtraction!
      if ((unsigned long)(currentMillis - starttime) >= timeMedium) {
        Serial.println("switch to State OFF");
        state = OFF;
      }
    }
    else if(state == OFF){
      digitalWrite(PIN_FANPOWER, LOW);
      analogWrite(PIN_FANPWM, 0);
      digitalWrite(PIN_LED, LOW);
    }

    lastRunTime = millis();
  }
  
  delay(10);

}
