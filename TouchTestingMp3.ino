/*******************************************************************************
  SD card
  │
    TRACK000.mp3
    TRACK001.mp3
    TRACK002.mp3
    TRACK003.mp3
    TRACK004.mp3
    TRACK005.mp3
    TRACK006.mp3
    TRACK007.mp3
    TRACK008.mp3
    TRACK009.mp3
    TRACK010.mp3
    TRACK011.mp3
    TRACK012.mp3

*******************************************************************************/

// compiler error handling
#include "Compiler_Errors.h"

// touch includes
#include <MPR121.h>
#include <MPR121_Datastream.h>
#include <Wire.h>

// MP3 includes
#include <SPI.h>
#include <SdFat.h>
#include <FreeStack.h>
#include <SFEMP3Shield.h>

// touch constants
const uint32_t BAUD_RATE = 115200;
const uint8_t MPR121_ADDR = 0x5C;
const uint8_t MPR121_INT = 4;

// serial monitor behaviour constants
const bool WAIT_FOR_SERIAL = false;

// MPR121 datastream behaviour constants
const bool MPR121_DATASTREAM_ENABLE = false;

//Variable that keeps the cycle tracked
int cycle = 1;
bool pinTouched = false;
unsigned long timestamp = 0;
bool backgroundIsPaused = false;

// MP3 variables
uint8_t result;
uint8_t lastPlayed = 0;

// MP3 constants
SFEMP3Shield MP3player;

// SD card instantiation
SdFat sd;

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);

  if (WAIT_FOR_SERIAL) {
    while (!Serial);
  }

  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  if (!MPR121.begin(MPR121_ADDR)) {
    Serial.println("error setting up MPR121");
    switch (MPR121.getError()) {
      case NO_ERROR:
        Serial.println("no error");
        break;
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;
    }
    while (1);
  }

  MPR121.setInterruptPin(MPR121_INT);

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121.restoreSavedThresholds();
    MPR121_Datastream.begin(&Serial);
  } else {
    MPR121.setTouchThreshold(40);
    MPR121.setReleaseThreshold(20);
  }

  MPR121.setFFI(FFI_10);
  MPR121.setSFI(SFI_10);
  MPR121.setGlobalCDT(CDT_4US);  // reasonable for larger capacitances

  digitalWrite(LED_BUILTIN, HIGH);  // switch on user LED while auto calibrating electrodes
  delay(1000);
  MPR121.autoSetElectrodes();  // autoset all electrode settings
  digitalWrite(LED_BUILTIN, LOW);

  result = MP3player.begin();
  MP3player.setVolume(10, 10);

  if (result != 0 ) {
    Serial.print("Error code: ");
    Serial.print(result);
    Serial.println(" when trying to start MP3 player");
  }

  //MP3player.playTrack(12);
}

void loop() {
  MPR121.updateAll();

  for (int i = 0; i < 12; i++) {        //to iterate through all the pins

    pinTouched = MPR121.getTouchData(i);      //true in case pin i is being touched and false in case its not

    if (!MP3player.isPlaying()) {                                                        //Checks if audio stopped playing

      digitalWrite(LED_BUILTIN, LOW);

      switch (i) {                          //switch with all the eletrodes where every case checks if the pin is being touched and to check the current cycle, locking every pin thats currently not on route.
        case 0:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 1) {
              playMP3();
            }
          }
          break;

        case 1:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 11) {
              playMP3();
              cycle = 1;                    //resets the cycle
            }
          }
          break;

        case 2:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 6) {
              playMP3();
            }
          }
          break;

        case 3:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 3) {
              playMP3();
            }
          }
          break;

        case 4:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 2) {
              playMP3();
            }

            if (cycle == 10 && MPR121.getTouchData(6)) {      //checks double touch
              playMP3();
            }
          }
          break;

        case 5:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 12) {
                                                            //extra
            }
          }
          break;

        case 6:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 4) {
              playMP3();
            }

            if (cycle == 10 && MPR121.getTouchData(4)) {     //checks double touch
              playMP3();
            }
          }
          break;

        case 7:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 5) {
              playMP3();
            }
          }
          break;

        case 8:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 12) {
                                                            //extra
            }
          }
          break;

        case 9:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 7) {
              playMP3();
            }
          }
          break;

        case 10:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 8) {
              playMP3();
            }
          }
          break;

        case 11:
          if (pinTouched) {
            checkPinTouch(i);

            if (cycle == 9) {
              playMP3();
            }
          }
          break;

        default:
          break;
      }

      if (MPR121.isNewRelease(i) && !MPR121_DATASTREAM_ENABLE) {      //when a pin is realeased
        Serial.print("pin ");
        Serial.print(i);
        Serial.println(" is no longer being touched");
      }
    }
  }

  if (MPR121_DATASTREAM_ENABLE) {
    MPR121_Datastream.update();
  }

}

void playMP3() {                  //Plays the audio track and increments the cycle so the story can flow
  MP3player.stopTrack();
  MP3player.playTrack(cycle - 1);

  Serial.print("Currently playing track number -> ");
  Serial.println(cycle - 1);

  cycle++;

  delay(10);
}

void checkPinTouch(int i) {         //Debugs the pin that was touched to the debug console               
  Serial.print("pin ");
  Serial.print(i);
  Serial.println(" was just touched");

  digitalWrite(LED_BUILTIN, HIGH);
}

void pauseMP3() {                                     //Functions for the background music still WIP
  if (MP3player.isPlaying()) {
    MP3player.pauseMusic();
    timestamp = MP3player.currentPosition();
    backgroundIsPaused = true;
  }
}

void resumeMP3() {                                    //Functions for the background music still WIP
  if (backgroundIsPaused) {
    MP3player.playMP3("TRACK012.mp3", timestamp);
  }
}
