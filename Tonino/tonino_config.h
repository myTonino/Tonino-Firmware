// tonino_config.h
//----------------
// library for Tonino parameters
//
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2016, Paul Holleis, Marko Luther
// All rights reserved.
//
// Authors:  Paul Holleis, Marko Luther
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


#ifndef _TONINO_CONFIG_H
#define _TONINO_CONFIG_H


#include <tonino.h>
#include <tonino_tcs3200.h>

// lib to access EEPROM, built-in, see http://arduino.cc/en/Reference/EEPROM
#include <EEPROM.h>


#define EEPROM_SET 42

#define EEPROM_START_ADDRESS 10
#define EEPROM_CHANGED_ADDRESS         (EEPROM_START_ADDRESS)
#define EEPROM_SAMPLING_ADDRESS        (EEPROM_CHANGED_ADDRESS+1)
#define EEPROM_BRIGHTNESS_ADDRESS      (EEPROM_SAMPLING_ADDRESS+1)
#define EEPROM_CMODE_ADDRESS           (EEPROM_BRIGHTNESS_ADDRESS+1)
#define EEPROM_CCI_ADDRESS             (EEPROM_CMODE_ADDRESS+1)
#define EEPROM_DELAYTILLUPTEST_ADDRESS (EEPROM_CCI_ADDRESS+1)
#define EEPROM_CAL_ADDRESS             (EEPROM_DELAYTILLUPTEST_ADDRESS+1)
#define EEPROM_SCALE_ADDRESS           (NR_CAL_VALUES*4+EEPROM_CAL_ADDRESS)
#define EEPROM_SIZE                    (NR_SCALE_VALUES*4+EEPROM_SCALE_ADDRESS-EEPROM_START_ADDRESS)

// used to convert a number from float to bytes and vv for EEPROM
union floatByteData_t {
   float f;
   byte b[4]; // float uses 4 bytes
};


class ToninoConfig {
  public:
    // constructor needs color sensor object for passing parameters
    ToninoConfig(TCS3200 *colorSense, LCD *display);
    ~ToninoConfig(void);

    // look if EEPROM has been used and load values, or store defaults otherwise
    void init();
  

    // store calibration data to sensor library and EEPROM
    void setCalibration(float *cal);

    // store scaling data to sensor library and EEPROM
    void setScaling(float *cal);

    // store sampling rate to sensor library and EEPROM
    void setSampling(uint8_t sampling);

    // sets and stores the display brightness to EEPROM (0-15, 15=max brightness)
    void setBrightness(uint8_t b);

    // gets the display brightness from EEPROM (0-15, 15=max brightness)
    uint8_t getBrightness();

    // store color mode setting to sensor library and EEPROM
    void setColorMode(uint8_t cmode);

    // store whether initial calibration is tried setting to local variable and EEPROM
    void setCheckCalInit(bool iwc);

    // get initial calibration setting
    bool getCheckCalInit();

    // set delay time (ATTENTION: in 100ms i.e. 1/10sec, <255) to wait between test measurements whether cup was lifted
    void setDelayTillUpTest(uint8_t ltdelay);

    // get delay time (ATTENTION: in 100ms i.e. 1/10sec) to wait between test measurements whether cup was lifted
    uint8_t getDelayTillUpTest();

    // stores default config values in EEPROM and sets local vars and sensor library values accordingly
    // should only be used if isEepromChanged() returned false
    void writeDefaults();

  private:
    TCS3200 *_colorSense;
    LCD *_display;
    
    // returns true if the value is not a valid float
    static bool isInvalidNumber(float f);
    // whether or not to do initial white calibration
    bool _doInitCal;
    // delay between "can up" measurements
    uint8_t _delayTillUpTest;
    // writes val to EEPROM address addr but only if the stored value is different
    bool checkedEepromWrite(uint8_t addr, uint8_t val);
    // reads val from EEPROM address addr
    uint8_t checkedEepromRead(uint8_t addr);
    // finds most frequent value in given array
    // return true if at least one value is different
    bool getMostFrequent(uint8_t *vals, uint8_t *val);
    // true if EEPROM has already been (most probably) used to store config
    // checks whether byte at EEPROM_CHANGED_ADDRESS is 42
    bool isEepromChanged();
    // read all config data from EEPROM to colorSense and local variables;
    // should only be used if isEepromChanged() returned true
    // if some settings have not been set, uses and stores default values
    void readStoredParameters();
};

#endif