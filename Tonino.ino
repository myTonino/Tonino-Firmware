// Tonino_1_0_6.ino
//----------------
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
// Copyright (c) 2013, Paul Holleis, Marko Luther
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

#define VERSION "1 0 7"

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
// watchdog timer
#include <avr/wdt.h>
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

// flag whether display brightness has been reduced in power save mode
bool lowBrightness = false;


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

  // turn off brown-out enable in software
  MCUCR = _BV (BODS) | _BV (BODSE);  // turn on brown-out enable select
  MCUCR = _BV (BODS);  // this must be done within 4 clock cycles of above
  // ---- end low energy configuration
  
  // setup and start watchdog timer; will reset by calls
  // to wdt_reset and checkCommands
  watchdogSetup();
  
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
  if (tConfig.getCheckCalInit() && colorSense.isCalibrating() == LOW_PLATE) {
    display.clear();
    if (calibrate()) {
      delay(200);
      display.done();
    }
  } else {
    // no calibration plate detected, directly scan
    scanAndDisplay();
    delay(1000);
  }
}

void watchdogSetup(void) {
  cli(); // disable all interrupts
  wdt_reset(); // reset the WDT timer
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set Watchdog settings:
  WDTCSR = (1<<WDIE) | (1<<WDE) | (1<<WDP3) | (1<<WDP2) | (1<<WDP1) | (1<<WDP0);
  sei();
}

boolean checkCommands() {
  wdt_reset();
  return tSerial.checkCommands();
}


void loop() {
  // store when last action was detected (for low power idle mode)
  uint32_t lastTimestamp = 0;
  
  while(true) {
    // check this setting as it could be changed during runtime
    uint16_t delayTillUpTest = tConfig.getDelayTillUpTest() * 100;
  
    // poll if there is an incoming serial command
    if (tSerial.checkCommands()) lastTimestamp = millis();
    
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
        delay(1000);
        // poll if there is an incoming serial command
        if (tSerial.checkCommands()) lastTimestamp = millis();

        lastTimestamp = checkLowPowerMode(true, lastTimestamp);
      }
      // wait bc it might already be dark before can is fully on the surface
      // also want to give the sensor some time to recover
      delay(750);
      display.circle(1, 500);
      display.clear();
      delay(500);

      scanAndDisplay();

      lastTimestamp = millis();
      // this call is mainly to reset dipslay brightness etc. to normal
      lastTimestamp = checkLowPowerMode(false, lastTimestamp);
    }
    delay(delayTillUpTest);
    
    lastTimestamp = checkLowPowerMode(false, lastTimestamp);
  }
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
    if (!lowBrightness) {
      // temporarily set lower brightness
      display.setBrightness(display.getBrightness() / 2);
      lowBrightness = true;
    }
    return lastTimestamp;
    
  } else {
    if (lowBrightness) {
      // restore original brightness
      display.setBrightness(tConfig.getBrightness());
      lowBrightness = false;
    }
    return lastTimestamp;
  }
}


// called when detected first calibration plate with quick scan
boolean calibrate() {
  sensorData sd_1;
  
  display.calibration1();
  WRITEDEBUG("Calibrating ... ");
  delay(1000);
  
  // scan for calibration plate 1
  int32_t tval1 = colorSense.scan(NULL, false, &sd_1);

  if ((abs(sd_1.value[RED_IDX]-LOW_RED) < RED_RANGE_LOW) && 
      (abs(sd_1.value[BLUE_IDX]-LOW_BLUE) < BLUE_RANGE_LOW)) {
    // really found calibration plate 1 - do a second scan for averaging and display OK
    sensorData sd_2;
    display.calibration1();

    // dark scan to remove potential external light
    if (tSerial.checkCommands()) return false;
    delay(750);
    if (tSerial.checkCommands()) return false;
    sensorData dsd_2;
    colorSense.scan(NULL, false, &dsd_2, false);
    delay(750);
    if (tSerial.checkCommands()) return false;

    // make a second scan and calc an average
    int32_t tval2 = colorSense.scan(NULL, false, &sd_2);

    float rb_low = (sd_1.value[RED_IDX] + sd_2.value[RED_IDX] - 2*dsd_2.value[RED_IDX]) / 
            (float)(sd_1.value[BLUE_IDX] + sd_2.value[BLUE_IDX] - 2*dsd_2.value[BLUE_IDX]);

    WRITEDEBUG((sd_1.value[RED_IDX] + sd_2.value[RED_IDX] - 2*dsd_2.value[RED_IDX]) / 2);
    WRITEDEBUG("/");
    WRITEDEBUG((sd_1.value[BLUE_IDX] + sd_2.value[BLUE_IDX] - 2*dsd_2.value[BLUE_IDX]) / 2);
    WRITEDEBUG("=");
    WRITEDEBUGF(rb_low, 5);
    WRITEDEBUG("; ");

    // DON'T write measured average T-value to display as it is confusing for the user
    // displayNum((int32_t)((tval1 + tval2) / 2.0 + 0.5));

    // wait for other plate
    while (true) {
      display.calibration2();

      // wait till can is lifted and put down again
      while (colorSense.isDark()) {
        delay(250);
        if (tSerial.checkCommands()) return false;
      }
      display.up();
      while (colorSense.isLight()) {
        delay(250);
        if (tSerial.checkCommands()) return false;
      }
      display.calibration2();
      delay(500);

      // scan for calibration plate 2
      // this is done with a quick (only white) measurement
      if (colorSense.isCalibrating() == HIGH_PLATE) {
        delay(200);
        // make a thorough scan
        tval1 = colorSense.scan(NULL, false, &sd_1);

        if ((abs(sd_1.value[RED_IDX]-HIGH_RED) < RED_RANGE_HIGH) && 
            (abs(sd_1.value[BLUE_IDX]-HIGH_BLUE) < BLUE_RANGE_HIGH)) {
          // really found calibration plate 2 - calc and save calibration
  
          display.calibration2();
          if (tSerial.checkCommands()) return false;
          
          // dark scan to remove potential external light
          delay(750);
          colorSense.scan(NULL, false, &dsd_2, false);
          if (tSerial.checkCommands()) return false;
          delay(750);
          if (tSerial.checkCommands()) return false;
      
          // make a second scan and calc an average
          int32_t tval2 = colorSense.scan(NULL, false, &sd_2);
          display.calibration2();
          float rb_high = (sd_1.value[RED_IDX] + sd_2.value[RED_IDX] - 2*dsd_2.value[RED_IDX]) / 
                   (float)(sd_1.value[BLUE_IDX] + sd_2.value[BLUE_IDX] - 2*dsd_2.value[BLUE_IDX]);

          float cal[2];
          cal[0] = (HIGH_TARGET - LOW_TARGET) / (rb_high - rb_low);
          cal[1] = LOW_TARGET - cal[0]*rb_low;

          WRITEDEBUG((sd_1.value[RED_IDX] + sd_2.value[RED_IDX] - 2*dsd_2.value[RED_IDX]) / 2);
          WRITEDEBUG("/");
          WRITEDEBUG((sd_1.value[BLUE_IDX] + sd_2.value[BLUE_IDX] - 2*dsd_2.value[BLUE_IDX]) / 2);
          WRITEDEBUG("=");
          WRITEDEBUGF(rb_high, 5);
          
          WRITEDEBUG(" => ");
          WRITEDEBUGF(cal[0], 5);
          WRITEDEBUG(", ");
          WRITEDEBUGLNF(cal[1], 5);
  
          // DON'T write measured average T-value to display as it is confusing for the user
          // displayNum((int32_t)((tval1 + tval2) / 2.0 + 0.5));
          if (tSerial.checkCommands()) return false;

          tConfig.setCalibration(cal);
          delay(2500);
          return true;
        }
      }
      delay(500);
    }

  } else {
    // could not detect first plate even though quick check thought so - error and continue
    display.error();
    delay(3000);
  }
}

// make a full scan and display on LCD
inline void scanAndDisplay() {
  // make a measurement with stored configuration (last 'param' toggles external light removal)
  int32_t tval = colorSense.scan(NULL, false, NULL, true, true);

  displayNum(tval);
}

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
