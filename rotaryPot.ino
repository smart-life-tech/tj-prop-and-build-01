/****************************************************************************************
   Toby Brandon - Arduino Source for 2022 Ghostbusters Proton Pack
   LED pinout 2 / 3 / 4 using addressable LED's (NewPixels) and 1 Adafruit Jewel for the wand gun
   Pinout 2 - Cyclotron LEDs 40 ring (change to suit)
   Pintout 3 - powercell - 14 leds
    Pintout 4 Cyclotron Vent 0 - 11, 12 Slow-Blow, 13, 14, 15 Wand Vent, 16 White LED, 17 White LED2, 18, Yellow LED, , 19 Orange Hat LED, 20-26 NeoPixel Jewel
   Sound Module -  serial mp3 player - caltex yx5300
   Some Reference Code from Eric Banker (Wand LED write helper function)
   Some Reference Code from mikes11 (modified led write helpers)
   modifications and additions added from various sources and help from friends and the ghostbuster community

   thank you!

*/

#include "afterlife_cyclotron.h"
#include <Adafruit_NeoPixel.h>
#include <Wire.h> // Include the I2C library (required)
#include <SparkFunSX1509.h> // Include SX1509 library
#include "SerialMP3Player.h"
#define CLK 11
#define DT 12
#define SW 13

#define TX 19
#define RX 18

SerialMP3Player mp3(RX, TX);

bool playOnce, play1,  play2, play9, play11, play16, play18, playsafe1, playsafe2, playwand, playwand2 = false;
// Create a new POWERCELL object
#define PM_PIN      3        // input pin Powercell strips are attached to
#define PM_PIXELS   14     // total number of neopixels in powercell

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";
unsigned long lastButtonPress = 0;
int playing = 5;
Adafruit_NeoPixel powercell = Adafruit_NeoPixel(PM_PIXELS, PM_PIN, NEO_GRB + NEO_KHZ800);
int  PM_TIME =  800;       // time to max power in milliseconds
int meterlevel = 0;
unsigned long lastMeterResetTime;
bool safeOff = false;
bool  safeTwo = false;
int seq_1_current = 0;  // current led in sequence 1
unsigned long firing_interval = 40;     // interval at which to cycle firing lights on the bargraph. We update this in the loop to speed up the animation so must be declared here (milliseconds).

const int ventStart = 0;
const int ventEnd = 11;
const int SloBloLED = 12;
const int WandVentstart = 13;
const int WandVentsend = 15;
const int ThemeLED = 16;
const int WhiteLED1 = 17;
const int WhiteLED2 = 18;
const int OrangeHatLED = 19;
const int GunLEDStart = 20;
const int GunLEDEnd = 26;
unsigned long firePrev, firingTimer = 0;
bool play14 = false;
// Vent + Wand LED Count
const int NeoPixelLEDCount2 = 26;
int lastvol = 0;
#define NEO_WAND 4 // for powercell
Adafruit_NeoPixel wandLights = Adafruit_NeoPixel(NeoPixelLEDCount2, NEO_WAND, NEO_GRB + NEO_KHZ800);


#define PIN 2           // Which pin on the Arduino is connected to the NeoPixels?
#define FIRE_PIN  9     // Which pin on the Arduino is connected to the "overheat" signal?
#define NUMPIXELS 40    // How many NeoPixels are attached to the Arduino?
#define BRIGHTNESS 255  // How bright should the pixels be? (min = 0, max = 255)
#define GROUP 2         // How big of a group of pixels do you want to rotate?
#define INIT_SPD  255   // How slow do you want the animation to rotate at the beginning of the boot? (higher = slower, 255 max)
#define IDLE_SPD  2   // How fast do you want the animation to rotate during "normal" operation (lower = faster, 0 min)
#define HEAT_SPD 0      // How fast do you want the animation to rotate at overheat? (lower = faster, 0 min)
#define BOOT_DLY  5000  // How long do you want the boot animation to last?
#define HEAT_DLY  5000  // How long should the "overheat" ramp up last?

// Create a new cyclotron object
Cyclotron cyclotron(PIN, NUMPIXELS, GROUP, INIT_SPD);

const int STARTPACK_SWITCH = 5;  // main power
const int STARTWAND_SWITCH = 6;  //wand power
const int SAFETY_ONE = 7;
const int SAFETY_TWO = 8;
//const int FIRE_PIN = 9;
const int MUSIC_SWITCH = 10; // music



// SX1509 I2C address (set by ADDR1 and ADDR0 (00 by default):
const byte ADDRESS = 0x3E;  // SX1509 I2C address
SX1509 io; // Create an SX1509 object to be used throughout

// bargraph helper variables
const int num_led = 12; // total number of leds in bar graph

// SX1509 pin definitions for the leds on the graph:
// SX1509 pin definitions for the leds on the graph:
const byte BAR_01 = 0;
const byte BAR_02 = 1;
const byte BAR_03 = 2;
const byte BAR_04 = 3;
const byte BAR_05 = 4;
const byte BAR_06 = 5;
const byte BAR_07 = 6;
const byte BAR_08 = 7;
const byte BAR_09 = 8;
const byte BAR_10 = 9;
const byte BAR_11 = 10;
const byte BAR_12 = 11;


// Possible Pack states

bool songplaying = true;
bool power_up = false;
bool startup = false;
bool state1 = true;
bool state2 = false;
bool  paused = true;

// physical switch states

bool safety_one = false;
bool safety_two = false;
bool fire = false;
bool warning = false;

// mp3 timers

const unsigned long mp3_delay = 5000;

void setup() {
  if (!io.begin(ADDRESS)) {
    while (1) ;
  }
  mp3.showDebug(1);       // print what we are sending to the mp3 board.

  Serial.begin(9600);     // start serial interface
  mp3.begin(9600);        // start mp3-communication
  delay(500);             // wait for init

  mp3.sendCommand(CMD_SEL_DEV, 0, 2);   //select sd-card
  delay(500);             // wait for init

  //  for signal testing //
  pinMode(13, OUTPUT);
  // ***** Assign Proton Pack Switches / Buttons ***** //

  pinMode(STARTPACK_SWITCH, INPUT_PULLUP);
  digitalWrite(STARTPACK_SWITCH, HIGH);
  pinMode(STARTWAND_SWITCH, INPUT_PULLUP);
  digitalWrite(STARTWAND_SWITCH, HIGH);
  pinMode(SAFETY_ONE, INPUT_PULLUP);
  digitalWrite(SAFETY_ONE, HIGH);
  pinMode(SAFETY_TWO, INPUT_PULLUP);
  digitalWrite(SAFETY_TWO, HIGH);
  pinMode(FIRE_PIN, INPUT_PULLUP);
  digitalWrite(FIRE_PIN, HIGH);
  pinMode(MUSIC_SWITCH, INPUT_PULLUP);
  digitalWrite(MUSIC_SWITCH, HIGH);




  //start the powercell
  powercell.begin();
  lastMeterResetTime = millis();
  powercell.clear();
  WAND_LEDstateOFF();
  powercell_off();
  // ***** Configure LED's in wand lights (Including Vent LEDs) ***** //
  wandLights.begin();
  wandLights.setBrightness(240);
  wandLights.show();


  // *** cyclotron start ********
  cyclotron.setBrightness(0, 0);
  cyclotron.start();
  cyclotron.setSpeed(IDLE_SPD, BOOT_DLY);

  WAND_LEDstateOFF();
  powercell_off();

  // configuration for the bargraph LED's
  io.pinMode(BAR_01, OUTPUT);
  io.pinMode(BAR_02, OUTPUT);
  io.pinMode(BAR_03, OUTPUT);
  io.pinMode(BAR_04, OUTPUT);
  io.pinMode(BAR_05, OUTPUT);
  io.pinMode(BAR_06, OUTPUT);
  io.pinMode(BAR_07, OUTPUT);
  io.pinMode(BAR_08, OUTPUT);
  io.pinMode(BAR_09, OUTPUT);
  io.pinMode(BAR_10, OUTPUT);
  io.pinMode(BAR_11, OUTPUT);
  io.pinMode(BAR_12, OUTPUT);

  shutdown_leds();
  // Set encoder pins as inputs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor
  Serial.begin(9600);

  // Read the initial state of CLK
  lastStateCLK = digitalRead(CLK);
}

char c;  // char from Serial
char cmd = ' ';
char cmd1 = ' ';


int song = 0;


/* Wand LED assighment
  const int ventStart = 0;
  const int ventEnd = 11;
  const int SloBloLED = 12;
  const int WandVentstart = 13;
  const int WandVentsend = 15;
  const int ThemeLED = 16;
  const int WhiteLED1 = 17;
  const int WhiteLED2 = 18;
  const int OrangeHatLED = 19;
  const int GunLEDStart = 20;
  const int GunLEDEnd = 27;

*/
void loop() {

  /*************************************************************************
                                 Main Loop Function
  *************************************************************************/

  /********sd card sounds in order - folder (01)********
    wand on - 1
    venting - 2
    higher and higher song - 3
    epic gb theme - 4
    on our own song - 5
    under the floor song - 6
    punkrock gb - 7
    startup - 8
    shutdown - 9
    idle - 10
    safe 1 - 11
    safe 2 - 12
    error - 13
    endfire start - 14
    endfire silent - 15
    click - 16
    blast - 17
    warning - 18


  ***************mp3 commands*********************

        mp3.play(nval);
                      ("Play Folder");
        mp3.playF(nval);
        Serial.println("Play loop");
       mp3.playSL(nval);
        Serial.println("Play file at 30 volume");
        mp3.play(nval,30);
        Serial.println("Play");
        mp3.play();
        Serial.println("Pause");
        mp3.pause();
        Serial.println("Stop");
        mp3.stop();
        Serial.println("Next");
        mp3.playNext();
        Serial.println("Previous");
        mp3.playPrevious();
        Serial.println("Volume UP");
        mp3.volUp();
        Serial.println("Volume Down");
        mp3.volDown();
        Serial.println("Set to Volume");
          mp3.setVol(nval);
          mp3.qVol();
        Serial.println("Query current file");
        mp3.qPlaying();
        Serial.println("Query status");
        mp3.qStatus();
        Serial.println("Query folder count");
        mp3.qTFolders();
        Serial.println("Query total file count");
        mp3.qTTracks();
        mp3.reset();
        mp3.sleep();
        mp3.wakeup();



  ****************/

  /* const int STARTPACK_SWITCH = 5;  // main power
    const int STARTWAND_SWITCH = 6;  //wand power
    const int SAFETY_ONE = 7;
    const int SAFETY_TWO = 8;
    const int FIRE_PIN = 9;*/

  int volume = map(analogRead(A0), 0, 1024, 0, 30);
  if (volume > lastvol + 3 || volume < lastvol - 3) {
    lastvol = volume;
    Serial.print("Set to Volume :");
    Serial.println(volume);
    mp3.setVol(volume);
  }


  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  int startpack = digitalRead (STARTPACK_SWITCH);
  int startwand = digitalRead(STARTWAND_SWITCH);
  int safe1 = digitalRead (SAFETY_ONE);
  int safe2 = digitalRead (SAFETY_TWO);
  int fire = digitalRead (FIRE_PIN);


  // song selection
  int theme_switch = digitalRead(MUSIC_SWITCH);
  if (startpack == HIGH ) {
    encoderRead();
  }
  if (theme_switch == LOW  && startpack == HIGH) {
    //  encoderRead();

    if (songplaying == true)
      song ++;
    if (song > 6)song = 1;
    switch (song) {
      case 1:
        mp3.play(3);
        break;
      case 2 :
        mp3.play(4);
        break;
      case 3:
        mp3.play(5);
        break;
      case 4 :
        mp3.play(6);
        break;
      case 5 :
        mp3.play(7);
        break;
      case 6:
        mp3.stop();
        break;
    }
    songplaying = false;
  }

  else if (theme_switch == LOW  && startpack == LOW && state1 == false) {
    if (play16) {
      //  mp3.play(10);
      play16 = false;
    }
  }
  if (theme_switch == HIGH) {
    songplaying = true;
    play16 = true;
  }

  if (startpack == LOW) { // if button is pressed
    if (state1) {
      cyclotron.update();
      Serial.println ("power on");
      barGraphCHARGING (currentMillis);
      powercell_idle();
      wand_charge();
      cyclotron.setBrightness(BRIGHTNESS, BOOT_DLY);
      cyclotron.setSpeed(IDLE_SPD, BOOT_DLY);
      power_up = true;
      if ((power_up) && (startup == false)) {
        mp3.play(8);
        startup = true;
      }
    }
    // ***********do not touch above this line ***********  SERIOUSLY!! ********** DONT! *********  PLEASE? *****************
    if (startwand == LOW) { // if button is pressed
      playwand2 = true;
      state1 = false;
      cyclotron.update();
      Serial.println ("wand on");
      barGraphSequenceOne(currentMillis);
      if (digitalRead (SAFETY_ONE)) {// so that color dont mix up
        setWandLightState(18, 1, 0);    // white led flash
      }
      setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
      setWandLightState(12, 0, currentMillis); // Set slo blo flashing
      cyclotron.setSpeed(IDLE_SPD, BOOT_DLY);
      powercell_idle();
      if (playwand) {
        mp3.play(1);
        playwand = false;
      }
      int safe1 = digitalRead (SAFETY_ONE);
      if (safe1 == LOW) {// safety swicth 1
        safeOff = true;
        if (playsafe1) {
          mp3.play(11);
          playsafe1 = false;
        }
        Serial.println("safety one on");
        setWandLightState(18, 9, 0);
        int safe2 = digitalRead (SAFETY_TWO);
        if (safe2 == LOW) {
          safeTwo = true;
          Serial.println("safety 2 on");
          // WAND_ledstate3();
          setWandLightState(17, 2, 0);
          //setWandLightState(18, 1, 0);
          if (play11) {
            mp3.play(12);
            play11 = false;
          }
          int fire = digitalRead (FIRE_PIN);
          if (fire == LOW) { // firing squad lol
            firingTimer = millis();
            play14 = true;
            if (firingTimer - firePrev < 10000) {
              Serial.println("fire button pressed less than 10 sec");
              //firePrev = millis();
              fireStrobe(currentMillis);
              barGraphSequenceTwo(currentMillis);
              if (playOnce) {
                mp3.play(17);
                playOnce = false;
              }
            } else if (firingTimer - firePrev > 10000 && firingTimer - firePrev < 20000) {
              Serial.println("fire button pressed greater than 10 sec");
              // firePrev = millis();
              if (play18) {
                mp3.play(18);
                play18 = false;
              }
              //play14 = true;

            }
            else if (firingTimer - firePrev > 20000) {
              //play18 = true;
              Serial.println("fire button pressed greater than 20 sec");
              setVentLightState(ventStart, ventEnd, 0);
              WAND_LEDstateOFF();
              if (play2) {
                mp3.play(2);
                play2 = false;
              }
              barGraphCHARGING (currentMillis);
              wand_charge();
            }


          }
          else {
            firePrev = millis();// rest timer
            playOnce = true;
            play2 = true;
            play18 = true;
            if (play14) {
              Serial.println("fire button released before 10 seconds");
              clearGunLEDs();
              mp3.play(14);
              play14 = false;
            }
          }
        } else {
          play11 = true;
          if (safeTwo) {
            mp3.play(16);
            playsafe2 = false;
            setWandLightState(13, 4, 0);    //  vent start off
            setWandLightState(14, 4, 0);    //  vent mid   off
            setWandLightState(15, 4, 0);    // vent end   off
            safeTwo = false;
          }
        }
      }
      else {
        playsafe1 = true;
        if (safeOff) {
          mp3.play(16);
          safeOff = false;
        }
      }


    } else {
      if (playwand2) {
        mp3.play(16);
        playwand2 = false;
      }
      playwand = true;
      state1 = true;
    }


    // need this track  ( 8 ) to play for 12 seconds
    // before moving onto the idle stage but not interfering with the bargraph charging or charging sequence from the wand


    /* this is the order to enable heat pin to work and to start the warning and venting procedure

      read STARTPACK_SWITCH - if condition is on -  start pack and run charging stage
                            if off - do nothing
      read STARTWAND_SWITCH - if condition is on  -  run idle stage - read >>(safety one)
                           if off return to previous (STARTPACK_SWITCH)


             SAFETY_ONE - toggle switch

                           if condition is on - read >> (safety two)
                           if off, return to startwand stage.

             SAFETY_TWO - toggle switch

                           if condition is on - read >> (FIRE_PIN)
                           if not, return to safety one.

              FIRE_PIN  -  push button

                           if condition is pressed - run fire_seq >> (warning procedure)
                           if not, return to safety two.

      (warning procedure) -  if FIRE_PIN is pressed for 15 seconds -
                           enter warning stage
                           (WAND LED STATE CHANGE)

                           warning stage for 15 seconds

                           vent stage
                           return to safety 2 stage, ready to read fire again


    */
    play9 = true;
  }
  else {
    state1 = true;
    if (play9) {
      mp3.play(9);
      play9 = false;
    }
    cyclotron.update();
    WAND_LEDstateOFF();
    cyclotron.setSpeed(IDLE_SPD, BOOT_DLY);
    cyclotron.setBrightness(0, BOOT_DLY);
    shutdown_leds();
    powercell_off();
    Serial.println ("power off");
    power_up = false;
    startup = false;

    // mp3.pause();

  }
}
/* -------------------------------------------------------------    SEQUENCES    -----------------------------------------------------------------*/


void doCycle() {
  lastMeterResetTime = millis();
  meterlevel = 0;
}
void powercell_idle()
{
  unsigned long currentMillis = millis();
  meterlevel = ((currentMillis - lastMeterResetTime) * PM_PIXELS) / PM_TIME;
  if (meterlevel > PM_PIXELS)
    doCycle();
  for (int i = 0; i < PM_PIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    if (meterlevel >= i + 1)
      powercell.setPixelColor(i, powercell.Color(0, 0, 200));
    else
      powercell.setPixelColor(i, powercell.Color(0, 0, 0));
  }
  powercell.show();
}

void powercell_off() {
  unsigned long currentMillis = millis();
  for (int i = 0; i < PM_PIXELS; i++)  {
    powercell.setPixelColor(i,  powercell.Color(0, 0, 0));
  }
  powercell.show();
}

/*************** Wand Light Helpers *********************
    Modified from Eric Bankers source
*/
unsigned long prevFlashMillis = 0; // Last time we changed wand sequence 1
unsigned long prevFlashMillis2 = 0; // Last time we changed wand sequence 2
unsigned long prevFlashMillis3 = 0; // Last time we changed wand sequence 3
unsigned long prevFlashMillis4 = 0; // Last time we changed wand sequence 4
unsigned long prevFlashMillis5 = 0; // Last time we changed wand sequence 5
unsigned long prevFlashMillis6 = 0; // Last time we changed wand sequence 6
unsigned long prevFlashMillis7 = 0; // Last time we changed wand sequence 7
unsigned long prevFlashMillis8 = 0; // Last time we changed wand sequence 8
unsigned long prevFlashMillis9 = 0; // Last time we changed wand sequence 8

bool flashState1 = false;
bool flashState2 = false;
bool flashState3 = false;
bool flashState4 = false;
bool flashState5 = false;
bool flashState6 = false;
bool flashState7 = false;
bool flashState8 = false;
bool flashState9 = false;
const unsigned long wandFastFlashInterval = 400; // interval at which we flash the top led on the wand
const unsigned long wandMediumFlashInterval = 700; // interval at which we flash the top led on the wand
const unsigned long wandSlowFlashInterval = 1000; // interval at which we flash the slow led on the wand
void setWandLightState(int lednum, int state, unsigned long currentMillis) {
  switch ( state ) {
    case 0: // set led red
      wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
      break;
    case 1: // set led white
      wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
      break;
    case 2: // set led orange
      wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
      break;
    case 3: // set led blue
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 255));
      break;
    case 4: // set led off
      wandLights.setPixelColor(lednum, 0);
      break;
    case 5: // fast white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandFastFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState1 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState1 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState1 = false;
        }
      }
      break;
    case 6: // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis2) >= wandMediumFlashInterval) {
        prevFlashMillis2 = currentMillis;
        if ( flashState2 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 051, 0));
          flashState2 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState2 = false;
        }
      }
      break;
    case 7: // medium red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis3) >= wandMediumFlashInterval) {
        prevFlashMillis3 = currentMillis;
        if ( flashState3 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState3 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState3 = false;
        }
      }
      break;
    case 8: // fast red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis4) >= wandFastFlashInterval) {
        prevFlashMillis4 = currentMillis;
        if ( flashState4 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState4 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState4 = false;
        }
      }
      break;
    case 9: // set LED green
      wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
      break;
    case 10: // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis5) >= wandMediumFlashInterval) {
        prevFlashMillis5 = currentMillis;
        if ( flashState5 == false ) {
          wandLights.setPixelColor(lednum, 0);
          flashState5 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 051, 0));
          flashState5 = false;
        }
      }
      break;
    case 11: // slower orange flashing from red
      if ((unsigned long)(currentMillis - prevFlashMillis6) >= wandMediumFlashInterval) {
        prevFlashMillis6 = currentMillis;
        if ( flashState6 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(0, 255, 0));
          flashState6 = true;
        } else {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState6 = false;
        }
      }
      break;
    case 12: // slowest red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandSlowFlashInterval) {
        prevFlashMillis7 = currentMillis;
        if ( flashState7 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState7 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState7 = false;
        }
      }
      break;
    case 13: // slower white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandSlowFlashInterval) {
        prevFlashMillis8 = currentMillis;
        if ( flashState8 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState8 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState8 = false;
        }
      }
      break;
    case 14: // medium white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis7) >= wandMediumFlashInterval) {
        prevFlashMillis9 = currentMillis;
        if ( flashState9 == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState9 = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState9 = false;
        }
      }
      break;




  }
  wandLights.show();
}
/***************** Vent Light *************************
  Modified from Eric Banker's source code
*/
void setVentLightState(int startLed, int endLed, int state ) {
  switch ( state ) {
    case 0: // set all leds to white
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, wandLights.Color(255, 255, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW

      break;
    case 1: // set all leds to blue
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, wandLights.Color(0, 0, 255));
      }
      // Set the relay to on while venting. If relay is off set the pin LOW

      break;
    case 2: // set all leds off
      for (int i = ventStart; i <= ventEnd; i++) {
        wandLights.setPixelColor(i, 0);
      }
      // Set the relay to OFF while not venting. If relay is onf set the pin HIGH

      break;
  }
  wandLights.show();
}


/*************** Firing Animations *********************/
unsigned long prevFireMillis = 0;
const unsigned long fire_interval = 50;     // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void clearGunLEDs()
{
  for (int i = GunLEDStart; i <= GunLEDEnd; i++) {
    wandLights.setPixelColor(i, 0);
  }
}

void fireStrobe(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevFireMillis) >= fire_interval) {
    prevFireMillis = currentMillis;

    switch ( fireSeqNum ) {
      case 0:
        wandLights.setPixelColor(20, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(22, 0);
        wandLights.setPixelColor(23, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(24, 0);
        wandLights.setPixelColor(25, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(26, 0);
        break;
      case 1:
        wandLights.setPixelColor(20, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(24, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 2:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(21, 0);
        wandLights.setPixelColor(22, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(23, 0);
        wandLights.setPixelColor(24, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(25, 0);
        wandLights.setPixelColor(26, wandLights.Color(255, 0, 0));
        break;
      case 3:
        wandLights.setPixelColor(20, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, wandLights.Color(255, 0, 255));
        wandLights.setPixelColor(24, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 4:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(21, 0);
        wandLights.setPixelColor(22, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(23, 0);
        wandLights.setPixelColor(24, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(25, 0);
        wandLights.setPixelColor(26, wandLights.Color(255, 255, 255));
        break;
      case 5:
        wandLights.setPixelColor(20, wandLights.Color(255, 0, 255));
        wandLights.setPixelColor(21, wandLights.Color(0, 255, 0));
        wandLights.setPixelColor(22, wandLights.Color(255, 0, 0));
        wandLights.setPixelColor(23, wandLights.Color(0, 0, 255));
        wandLights.setPixelColor(24, wandLights.Color(255, 0, 255));
        wandLights.setPixelColor(25, wandLights.Color(255, 255, 255));
        wandLights.setPixelColor(26, wandLights.Color(0, 0, 255));
        break;
    }

    wandLights.show();

    fireSeqNum++;
    if ( fireSeqNum > fireSeqTotal ) {
      fireSeqNum = 0;
    }
  }
}

void wand_charge() {
  unsigned long currentMillis = millis();
  setWandLightState(12, 12, currentMillis);    // sloblo red slow flash
  setWandLightState(13, 4, 0);    //  vent start off
  setWandLightState(14, 4, 0);    //  vent mid   off
  setWandLightState(15, 4, 0);    // vent end   off
  setWandLightState(16, 4, 0 );     // theme led off
  setWandLightState(17, 4, 0);    //  white led off
  setWandLightState(18, 4, 0);    // white led off
  setWandLightState(19, 10, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);

}

void WAND_ledstate2() {

  unsigned long currentMillis = millis();

  setWandLightState(12, 0, 0);    // sloblo red
  setWandLightState(13, 4, 0);    //  vent start white off
  setWandLightState(14, 4, 0);    //  vent mid   white off
  setWandLightState(15, 4, 0);    // vent end  white off
  setWandLightState(16, 4, 0 );     // theme led slow flash orange
  setWandLightState(17, 4, 0 );    //  white led flash
  setWandLightState(18, 1, 0);    // white led flash
  setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);
}

void WAND_ledstate3() {
  unsigned long currentMillis = millis();

  setWandLightState(12, 0, 0);    // sloblo red
  setWandLightState(13, 1, 0);    //  vent start white
  setWandLightState(14, 1, 0);    //  vent mid   white
  setWandLightState(15, 1, 0);    // vent end   white
  setWandLightState(16, 10, currentMillis );     // theme led flash orange
  setWandLightState(17, 14, currentMillis);    //  white led flash slow
  setWandLightState(18, 5, currentMillis);    // white led flash fals
  setWandLightState(19, 6, currentMillis); // Set orange hat barrel flashing
  setVentLightState(ventStart, ventEnd, 2);

}
void WAND_LEDstateOFF() {
  setWandLightState(12, 4, 0);    //  set sloblo light off
  setWandLightState(13, 4 , 0);    //  set wand vent led off
  setWandLightState(14, 4, 0);   //  set wand vent led off
  setWandLightState(15, 4, 0);   //  set wand vent led off
  setWandLightState(16, 4, 0);    //  set sloblo  led off
  setWandLightState(17, 4, 0);     // Top LED off
  setWandLightState(18, 4, 0); // set back led off
  setWandLightState(19, 4, 0);    //   orange hat light off
  setVentLightState(ventStart, ventEnd, 2);  // off
}



/*************** Bar Graph Animations *********************/

// This is the idle sequence
unsigned long prevBarMillis_on = 0;          // bargraph on tracker
const unsigned long pwrcl_interval = 25;     // interval at which to cycle lights (milliseconds).
bool reverseSequenceOne = false;

void barGraphSequenceOne(unsigned long currentMillis) {
  // normal sync animation on the bar graph
  if ((unsigned long)(currentMillis - prevBarMillis_on) > pwrcl_interval) {
    // save the last time you blinked the LED
    prevBarMillis_on = currentMillis;

    if ( reverseSequenceOne == false ) {
      switch_graph_led(seq_1_current, HIGH);
      seq_1_current++;
      if ( seq_1_current > num_led ) {
        reverseSequenceOne = true;
      }
    } else {
      switch_graph_led(seq_1_current, LOW);
      seq_1_current--;
      if ( seq_1_current < 0  ) {
        reverseSequenceOne = false;
      }
    }
  }
}
// This is the firing sequence
unsigned long prevBarMillis_fire = 0; // bargraph firing tracker
int fireSequenceNum = 1;

void barGraphSequenceTwo(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevBarMillis_fire) > firing_interval) {
    // save the last time you blinked the LED
    prevBarMillis_fire = currentMillis;
    switch (fireSequenceNum) {
      case 1:
        switch_graph_led(1, HIGH);
        switch_graph_led(12, HIGH);
        switch_graph_led(11, LOW);
        switch_graph_led(2, LOW);
        fireSequenceNum++;
        break;
      case 2:
        switch_graph_led(2, HIGH);
        switch_graph_led(11, HIGH);
        switch_graph_led(1, LOW);
        switch_graph_led(12, LOW);
        fireSequenceNum++;
        break;
      case 3:
        switch_graph_led(3, HIGH);
        switch_graph_led(10, HIGH);
        switch_graph_led(2, LOW);
        switch_graph_led(11, LOW);
        fireSequenceNum++;
        break;
      case 4:
        switch_graph_led(4, HIGH);
        switch_graph_led(9, HIGH);
        switch_graph_led(3, LOW);
        switch_graph_led(10, LOW);
        fireSequenceNum++;
        break;
      case 5:
        switch_graph_led(5, HIGH);
        switch_graph_led(8, HIGH);
        switch_graph_led(4, LOW);
        switch_graph_led(9, LOW);
        fireSequenceNum++;
        break;
      case 6:
        switch_graph_led(6, HIGH);
        switch_graph_led(7, HIGH);
        switch_graph_led(5, LOW);
        switch_graph_led(8, LOW);
        fireSequenceNum++;
        break;
      case 7:
        switch_graph_led(5, HIGH);
        switch_graph_led(8, HIGH);
        switch_graph_led(6, LOW);
        switch_graph_led(7, LOW);
        fireSequenceNum++;
        break;
      case 8:
        switch_graph_led(4, HIGH);
        switch_graph_led(9, HIGH);
        switch_graph_led(5, LOW);
        switch_graph_led(8, LOW);
        fireSequenceNum++;
        break;
      case 9:
        switch_graph_led(3, HIGH);
        switch_graph_led(10, HIGH);
        switch_graph_led(4, LOW);
        switch_graph_led(9, LOW);
        fireSequenceNum++;
        break;
      case 10:
        switch_graph_led(2, HIGH);
        switch_graph_led(11, HIGH);
        switch_graph_led(3, LOW);
        switch_graph_led(10, LOW);
        fireSequenceNum++;
        break;
      case 11:
        switch_graph_led(1, HIGH);
        switch_graph_led(12, HIGH);
        switch_graph_led(2, LOW);
        switch_graph_led(11, LOW);
        fireSequenceNum = 1;
        break;
    }
  }
}


// This is the bargraph CHARGING sequence
unsigned long prevBarCHARGEMillis = 0;          // bargraph on tracker
const unsigned long CHARGE_interval = 700;     // interval at which to charge lights (milliseconds).
bool CHARGESequenceOne = true;

void barGraphCHARGING(unsigned long currentMillis) {
  // normal sync animation on the bar graph

  if ((unsigned long)(currentMillis - prevBarCHARGEMillis) > CHARGE_interval) {
    // save the last time you blinked the LED
    prevBarCHARGEMillis = currentMillis;

    if ( CHARGESequenceOne == true )   {
      switch_graph_led(seq_1_current, HIGH);
      seq_1_current++;
      if ( seq_1_current > num_led ) {
        CHARGESequenceOne = false;
      }
    } else {
      switch_graph_led(seq_1_current, LOW);
      seq_1_current--;
      if ( seq_1_current < 0  ) {
        CHARGESequenceOne = true;
      }
    }
  }
}

/************************* Shutdown and helper functions ****************************/
void shutdown_leds() {
  // reset the sequence
  seq_1_current = 1;
  fireSequenceNum = 1;

  // shut all led's off
  for (int i = 1; i <= 12; i++) {
    switch_graph_led(i, LOW);
  }
}
void switch_graph_led(int num, int state) {
  switch (num) {
    case 1:
      io.digitalWrite(BAR_01, state);
      break;
    case 2:
      io.digitalWrite(BAR_02, state);
      break;
    case 3:
      io.digitalWrite(BAR_03, state);
      break;
    case 4:
      io.digitalWrite(BAR_04, state);
      break;
    case 5:
      io.digitalWrite(BAR_05, state);
      break;
    case 6:
      io.digitalWrite(BAR_06, state);
      break;
    case 7:
      io.digitalWrite(BAR_07, state);
      break;
    case 8:
      io.digitalWrite(BAR_08, state);
      break;
    case 9:
      io.digitalWrite(BAR_09, state);
      break;
    case 10:
      io.digitalWrite(BAR_10, state);
      break;
    case 11:
      io.digitalWrite(BAR_11, state);
      break;
    case 12:
      io.digitalWrite(BAR_12, state);
      break;
  }
}

void encoderRead() {
  //------------------------------------------------------------------------------------------------------------------------------------
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;
      if (counter <= 0) {
        playing--;
        counter = 10;
      }
      if (playing < 3)playing = 3;
      mp3.play(playing);
      currentDir = "CCW";
    } else {
      counter ++;
      // Encoder is rotating CW so increment
      if (counter >= 20) {
        playing++;
        counter = 10;
      }

      if (playing > 7)playing = 7;
      mp3.play(playing);
      currentDir = "CW";
    }

    Serial.print("Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Counter: ");
    Serial.println(counter);
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  // Read the button state
  int btnState = digitalRead(SW);

  //If we detect LOW signal, button is pressed
  if (btnState == LOW) {
    //if 50ms have passed since last LOW pulse, it means that the
    //button has been pressed, released and pressed again
    if (millis() - lastButtonPress > 300) {
      Serial.println("Button pressed!");
      paused = !paused;
      if (paused) {
        Serial.println("Play");
        mp3.play();

      } else {
        Serial.println("Pause");
        mp3.pause();
      }
    }

    // Remember last button press event
    lastButtonPress = millis();
  }
  //----------------------------------------------------------------------------------------------------------------------------
}
