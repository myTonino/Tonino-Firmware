// Arduino code for Tonino (my-tonino.com) using
//    - DFRduino Nano
//    - Adafruit 0.56" 7-segment LCD backpack display
//      see http://learn.adafruit.com/adafruit-led-backpack/0-dot-56-seven-segment-backpack
//    - TSC3200 DFRobot sensor
//      see http://www.dfrobot.com/wiki/index.php/TCS3200_Color_Sensor_(SKU:SEN0101)
//
// - Serial communication:
//   - baud rate: 115200
//   - receive command XXX y1 y2 y3, answer with XXX:z1 z2 z3
//   - parameter separator: SEPARATOR (default is a space)
//   - answers are ended with \n
// 
//
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2017, Paul Holleis, Marko Luther
// All rights reserved.
//
// Author:  Paul Holleis, Marko Luther
//
// Redistribution and use in source and binary forms, with or without modification, are 
// permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this list of 
//   conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this list 
//   of conditions and the following disclaimer in the documentation and/or other materials 
//   provided with the distribution.
//
//   Neither the name of the copyright holder(s) nor the names of its contributors may be 
//   used to endorse or promote products derived from this software without specific prior 
//   written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ------------------------------------------------------------------------------------------


#define VERSION "1 1 7"

#include <Arduino.h>

#include <tonino.h>
#include <tonino_lcd.h>
#include <tonino_tcs3200.h>
#include <tonino_serial.h>
#include <tonino_config.h>

// lib that calls method according to serial input
// slightly adapted from
// https://github.com/kroimon/Arduino-SerialCommand, Version 20120522
#include <SerialCommand.h>

// i2c lib for LCD, built-in, see http://arduino.cc/en/Reference/Wire
#include <Wire.h>
// frequence counting lib for color sensor, http://www.pjrc.com/teensy/td_libs_FreqCount.html, Version 1.0
#include <FreqCount.h>
// lib to access EEPROM, built-in, see http://arduino.cc/en/Reference/EEPROM
#include <EEPROM.h>

// low power library, built-in, see http://playground.arduino.cc/Learning/arduinoSleepCode
#include <avr/power.h>  
// low power library, https://github.com/rocketscream/Low-Power, Version 1.30
#include <LowPower.h>

// LCD object
LCD display = LCD();

// color sensor object
#define CS_POWER 2
#define CS_S2    7
#define CS_S3    6
#define CS_LED   3
TCS3200 colorSense = TCS3200(CS_S2, CS_S3, CS_LED, CS_POWER, &display);

// object for all parameters
ToninoConfig tConfig = ToninoConfig(&colorSense, &display);
// object for serial communication
ToninoSerial tSerial = ToninoSerial(&colorSense, &display, &tConfig, VERSION);

// stores original display brightness if it has been reduced in power save mode
int8_t origBrightness = -1;


boolean checkCommands() {
  return tSerial.checkCommands();
}


// low power mode; checks every few seconds for an event
inline uint32_t checkLowPowerMode(bool isLight, uint32_t lastTimestamp) {
  if (millis() - lastTimestamp > TIME_TILL_SLEEP) {
    display.clear();
  
    int16_t loopsTillPowerDown = (TIME_TILL_POWERDOWN - TIME_TILL_SLEEP) / 4000 + 1;
  
    while (true) {
      LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);  
  
      // do we receive serial input during powerDown?
      if (Serial.available() > 0 || 
         (!isLight && colorSense.isLight()) || (isLight && colorSense.isDark())) {
        break;
      }
      // allow time for writing text (from isLight etc.) to serial line
      if (DODEBUG) {delay(200);}

      if (--loopsTillPowerDown <= 0) {
        WRITEDEBUGLN("power down");
        delay(500);
        LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
      }
    }
    display.line();
    return millis();

  } else if (millis() - lastTimestamp > TIME_TILL_DIM) {
    if (origBrightness < 0) {
      // temporarily set low brightness
      origBrightness = tConfig.getBrightness();
      display.setBrightness(1);
    }
    return lastTimestamp;
    
  } else if (origBrightness >= 0) {
    // restore original brightness
    display.setBrightness(origBrightness);
    origBrightness = -1;
  }
  return lastTimestamp;
}

// called when detected first calibration plate with quick scan
boolean calibrate() {
  sensorData sd;
  
  display.calibration1();
  WRITEDEBUGLN("Cal.");
  delay(500);
  
  // scan plate 1
  colorSense.scan(NULL, false, &sd);
  if (checkCommands()) return false;
  display.calibration1();
  delay(500);
  if (checkCommands()) return false;

  float redavg = sd.value[RED_IDX];
  float blueavg = sd.value[BLUE_IDX];
  
  if ((abs(redavg - LOW_RED) < RED_RANGE_LOW) && (abs(blueavg - LOW_BLUE) < BLUE_RANGE_LOW)) {

    float rb_low = redavg / blueavg;

    WRITEDEBUG(redavg);
    WRITEDEBUG("/");
    WRITEDEBUG(blueavg);
    WRITEDEBUG("=");
    WRITEDEBUGF(rb_low, 5);
    WRITEDEBUG("; ");

    // wait for other plate
    uint16_t delayTillUpTest = tConfig.getDelayTillUpTest() * 100;
    while (true) {
      display.calibration2();

      // wait till can is lifted and put down again
      while (colorSense.isDark()) {
        delay(delayTillUpTest);
        if (checkCommands()) return false;
      }
      display.up();
      while (colorSense.isLight()) {
        delay(delayTillUpTest);
        if (checkCommands()) return false;
      }
      display.calibration2();
      delay(500);

      // scan plate 2
      colorSense.scan(NULL, false, &sd);
      if (checkCommands()) return false;
      display.calibration2();
      delay(500);
      if (checkCommands()) return false;

      redavg = sd.value[RED_IDX];
      blueavg = sd.value[BLUE_IDX];
      float rb_high = redavg / blueavg;

      float cal[2];
      cal[0] = (HIGH_TARGET - LOW_TARGET) / (rb_high - rb_low);
      cal[1] = LOW_TARGET - cal[0]*rb_low;

      WRITEDEBUG(redavg);
      WRITEDEBUG("/");
      WRITEDEBUG(blueavg);
      WRITEDEBUG("=");
      WRITEDEBUGF(rb_high, 5);
      
      WRITEDEBUG("=>");
      WRITEDEBUGF(cal[0], 5);
      WRITEDEBUG(",");
      WRITEDEBUGLNF(cal[1], 5);

      if (checkCommands()) return false;

      tConfig.setCalibration(cal);
      delay(10);
      return true;
    }

  } else {
    // could not detect first plate even though quick check thought so - error and continue
    display.error();
    delay(3000);
    return false;
  }
}

// show the given number on the display if possible
inline void displayNum(int32_t tval) {
  if (tval < 0) {
    if (tval < -999) {
      display.line();
    } else {
      display.printNumber(tval);
    }
  } else if (tval > 9999) {
    display.line();
  } else {
    display.snake(tval);
  }
}

// make a full scan and display on LCD
inline void scanAndDisplay(float* lastRaw) {
  boolean averaged = false; // indicates if readings got averaged with the lastRaw (the previous one)
  
  // make a measurement with stored configuration, parameters:
  // 1: lastRaw: passes lastRaw readings for averaging
  // 2: false: no display animation during scan
  // 3: NULL: not interested in raw values
  // 4: true: switch on LEDs
  // 5: false: no explicit external light removal
  // 6: averaged: return flag that indicates that result got averaged
  int32_t tval = colorSense.scan(lastRaw, false, NULL, true, false, &averaged);
 

  displayNum(tval);
  display.averaged(averaged); // display the averaged indicator
}

void setup() {
  // ---- begin general low energy configurations
  // set all (unused) pins to input and pull-down
  for (byte i = 0; i <= A7; i++) {
    pinMode (i, INPUT);
    digitalWrite (i, LOW);
  }
  // Disable digital input buffers on all analog input pins
  // (note A6 and A7 do not have those buffers)
  DIDR0 = DIDR0 | B00111111;

  // disable unsused components
  power_adc_disable(); // disable unused analog digital converter needed only for analogRead()
  power_spi_disable(); // disable unused Serial Peripheral Interface
  SPCR = 0;
  // ---- end low energy configuration
    
  // to visualize that the Arduino is running
  pinMode (13, OUTPUT);    // changed as per below
  digitalWrite(13, HIGH);

  // initialize serial communication
  tSerial.init(115200);

  // directly check if there is an incoming serial command (connected to computer)
  checkCommands();

  // LCD init
  display.init(0x70);
  display.clear();

  // color sensor init
  colorSense.init();

  // read parameters from EEPROM or write defaults if not available
  // be sure to call this _after_ display and colorSense have been initialized (using .init())
  tConfig.init();
  
  // show that the display is working
  display.eightSequence();
  digitalWrite(13, LOW);
  pinMode (13, INPUT);

  // check whether we are calibrating (detect first calibration plate)
  if (colorSense.isDark()) {
    if (tConfig.getCheckCalInit() && colorSense.isCalibrating() == LOW_PLATE) {
      display.clear();
      if (calibrate()) {
        display.done();
      }
    } else {
      // no calibration plate detected, directly make first scan
      scanAndDisplay(NULL);
    }
  } else {
    display.clear();
  }
}


void loop() {
  // store when last action was detected (for low power idle mode)
  uint32_t lastTimestamp = 0;
  float lastRaw = 0.0; // last calibrated r/b result
  
  while(true) {
    // check this setting within the while loop as it could be changed during runtime
    uint16_t delayTillUpTest = tConfig.getDelayTillUpTest() * 100;
    
    // poll if there is an incoming serial command
    if (checkCommands()) lastTimestamp = millis();
    
    // check whether user lifted the can
    if (colorSense.isLight()) {
      lastTimestamp = millis();
      // display hint that can lifting was detected
      display.up();
      // this call is mainly to reset dipslay brightness etc. to normal
      lastTimestamp = checkLowPowerMode(true, lastTimestamp);

      // wait until user puts the can down again
      while (!colorSense.isDark()) {
        // interval for checking - need not be small
        delay(delayTillUpTest);
        // poll if there is an incoming serial command
        if (checkCommands()) lastTimestamp = millis();

        lastTimestamp = checkLowPowerMode(true, lastTimestamp);
      }
      // short wait because it might already be dark before 
      // the can is fully placed on the surface
      delay(1000);
      display.circle(2, 500);
      display.clear();
      delay(100);

      if ((millis() - lastTimestamp) > AVERAGE_TIME_SPAN) {
        // AVERAGE_TIME_SPAN milliseconds after the last scan we deactivate the averaging
        lastRaw = 0.0;
      }
      scanAndDisplay(&lastRaw);

      lastTimestamp = millis();
      // this call is mainly to potentially reset display brightness back to normal
      lastTimestamp = checkLowPowerMode(false, lastTimestamp);
    }
    delay(delayTillUpTest);
    
    if ((millis() - lastTimestamp) > AVERAGE_TIME_SPAN) {
        // AVERAGE_TIME_SPAN milliseconds after the last scan we deactivate the averaging
        lastRaw = 0.0;
        display.averaged(false); // clear the averaging indicator
    }
      
    lastTimestamp = checkLowPowerMode(false, lastTimestamp);
  }
}

