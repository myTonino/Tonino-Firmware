// tonino_config.cpp
//----------------
// library for Tonino parameters
//
// *** BSD License ***
// ------------------------------------------------------------------------------------------
// Copyright (c) 2014, Paul Holleis, Marko Luther
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


#include <tonino_config.h>


// constructor needs color sensor object for passing parameters
ToninoConfig::ToninoConfig(TCS3200 *c, LCD *d) :
  _colorSense(c), _display(d), _doInitCal(true) {
  // empty
}

ToninoConfig::~ToninoConfig(void) {
  // empty
}

// look if EEPROM has been used and load values, or store defaults otherwise
void ToninoConfig::init() {
  // see if there are already some stored defaults
  if (isEepromChanged()) {
    // yes, load parameters from EEPROM
    readStoredParameters();
  } else {
    // no, write some defaults
    writeDefaults();
  }
}

// store calibration data to sensor library and EEPROM
void ToninoConfig::setCalibration(float *cal) {
  _colorSense->setCalibration(cal);

  floatByteData_t data;
  int addr = EEPROM_CAL_ADDRESS;
  for (int c = 0; c < NR_CAL_VALUES; ++c) {
    data.f = cal[c];
    WRITEDEBUG("Storing Calib.");
    WRITEDEBUGF(data.f, 6);
    WRITEDEBUG(" as ");
    for (int b = 0; b < 4; ++b) {
      WRITEDEBUG(data.b[b]);
      WRITEDEBUG(SEPARATOR);
      checkedEepromWrite(addr++, data.b[b]);
    }
    WRITEDEBUGLN();
  }
}

// store scaling data to sensor library and EEPROM
void ToninoConfig::setScaling(float *cal) {
  _colorSense->setScaling(cal);

  floatByteData_t data;
  int addr = EEPROM_SCALE_ADDRESS;
  for (int c = 0; c < NR_SCALE_VALUES; ++c) {
    data.f = cal[c];
    WRITEDEBUG("Storing Scaling ");
    WRITEDEBUGF(data.f, 6);
    WRITEDEBUG(" as ");
    for (int b = 0; b < 4; ++b) {
      WRITEDEBUG(data.b[b]);
      WRITEDEBUG(SEPARATOR);
      checkedEepromWrite(addr++, data.b[b]);
    }
    WRITEDEBUGLN();
  }
}
  
// store sampling rate to sensor library and EEPROM
void ToninoConfig::setSampling(uint8_t sampling) {
  _colorSense->setSampling(sampling);
  checkedEepromWrite(EEPROM_SAMPLING_ADDRESS, sampling);
}

// sets and stores the display brightness to EEPROM (0-15, 15=max brightness)
void ToninoConfig::setBrightness(uint8_t b) {
  _display->setBrightness(b);
  checkedEepromWrite(EEPROM_BRIGHTNESS_ADDRESS, b);
}

// gets the display brightness from EEPROM (0-15, 15=max brightness)
uint8_t ToninoConfig::getBrightness() {
  return EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
}

// store color mode setting to sensor library and EEPROM
void ToninoConfig::setColorMode(uint8_t cmode) {
  _colorSense->setColorMode(cmode);
  checkedEepromWrite(EEPROM_CMODE_ADDRESS, cmode);
}

// store whether initial calibration is tried setting to local variable and EEPROM
void ToninoConfig::setCheckCalInit(bool cci) {
  _doInitCal = cci;
  checkedEepromWrite(EEPROM_CCI_ADDRESS, cci ? 1 : 0);
}

// get initial calibration setting
bool ToninoConfig::getCheckCalInit() {
  return _doInitCal;
}

// set delay time (ATTENTION: in 100ms i.e. 1/10sec, <255) to wait between test measurements whether cup was lifted
void ToninoConfig::setDelayTillUpTest(uint8_t ltdelay) {
  _delayTillUpTest = ltdelay;
  checkedEepromWrite(EEPROM_DELAYTILLUPTEST_ADDRESS, ltdelay);
}

// get delay time (ATTENTION: in 100ms i.e. 1/10sec) to wait between test measurements whether cup was lifted
uint8_t ToninoConfig::getDelayTillUpTest() {
  return _delayTillUpTest;
}


// stores default config values in EEPROM and sets local vars and sensor library values accordingly
// should only be used if isEepromChanged() returned false
void ToninoConfig::writeDefaults() {
  WRITEDEBUGLN("Storing default values to EEPROM");
  setSampling(DEFAULT_SAMPLING);
  setBrightness(DEFAULT_BRIGHTNESS);
  setColorMode(DEFAULT_COLORS);
  setCheckCalInit(DEFAULT_DOCALINIT);
  setDelayTillUpTest(DEFAULT_DELAYTILLUPTEST);

  float cal[MAX_CAL_VARS];
  #if NR_CAL_VALUES == 2
    cal[0] = DEFAULT_CAL_0;
    cal[1] = DEFAULT_CAL_1;
  #else
    for (int i = 0; i < NR_CAL_VALUES; ++i) {
      cal[i] = 1.0;
    }
  #endif
  setCalibration(cal);
#if NR_SCALE_VALUES == 3
  cal[0] = DEFAULT_SCALE_0;
  cal[1] = DEFAULT_SCALE_1;
  cal[2] = DEFAULT_SCALE_2;
#elif NR_SCALE_VALUES == 4
  cal[0] = DEFAULT_SCALE_0;
  cal[1] = DEFAULT_SCALE_1;
  cal[2] = DEFAULT_SCALE_2;
  cal[3] = DEFAULT_SCALE_3;
#else
  for (int i = 0; i < NR_SCALE_VALUES; ++i) {
    cal[i] = 0.001;
  }
#endif
  setScaling(cal);
}


// writes val to EEPROM address addr but only if the stored value is different
bool ToninoConfig::checkedEepromWrite(uint8_t addr, uint8_t val) {
  // save that EEPROM has been changed
  if (!isEepromChanged()) {
    EEPROM.write(EEPROM_CHANGED_ADDRESS, EEPROM_SET);
  }
  if (EEPROM.read(addr) != val) {
    EEPROM.write(addr, val);
    return true;
  }
  return false;
}

// true if EEPROM has already been (most probably) used to store config
// (checks whether byte at EEPROM_CHANGED_ADDRESS is set to EEPROM_SET)
inline bool ToninoConfig::isEepromChanged() {
  return (EEPROM.read(EEPROM_CHANGED_ADDRESS) == EEPROM_SET);
}

// read all config data from EEPROM to colorSense and local variables;
// should only be used if isEepromChanged() returned true
// if some settings have not been set, uses and stores default values
void ToninoConfig::readStoredParameters() {
  WRITEDEBUGLN("Reading stored params from EEPROM");
  // sampling rate
  uint8_t value = EEPROM.read(EEPROM_SAMPLING_ADDRESS);
  if (value == 255) {
    setSampling(DEFAULT_SAMPLING);
  } else {
    _colorSense->setSampling(value);
  }
  WRITEDEBUG("Sampling: ");
  WRITEDEBUG(_colorSense->getSampling());
  WRITEDEBUG(SEPARATOR);

  // display brightness
  value = EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
  if (value > 15) {
    setBrightness(DEFAULT_BRIGHTNESS);
  } else {
    _display->setBrightness(value);
  }
  WRITEDEBUG("Brightness: ");
  WRITEDEBUG(_display->getBrightness());
  WRITEDEBUG(SEPARATOR);

  // color mode
  value = EEPROM.read(EEPROM_CMODE_ADDRESS);
  if (value == 255) {
    setColorMode(DEFAULT_COLORS);
  } else {
    _colorSense->setColorMode(value);
  }
  WRITEDEBUG("Color mode: ");
  WRITEDEBUG(_colorSense->getColorMode());
  WRITEDEBUG(SEPARATOR);

  // setting for initial calibration
  value = EEPROM.read(EEPROM_CCI_ADDRESS);
  if (value == 255) {
    setCheckCalInit(DEFAULT_DOCALINIT ? 1 : 0);
  } else {
    _doInitCal = (value == 0 ? false : true);
  }
  WRITEDEBUG("Do_init_cal: ");
  WRITEDEBUG(_doInitCal ? 1 : 0);
  WRITEDEBUG(SEPARATOR);

  // setting for delay till "can up" test
  value = EEPROM.read(EEPROM_DELAYTILLUPTEST_ADDRESS);
  if (value == 255) {
    setDelayTillUpTest(DEFAULT_DELAYTILLUPTEST);
  } else {
    _delayTillUpTest = value;
  }
  WRITEDEBUG("Delay_till_up_test: ");
  WRITEDEBUG(_delayTillUpTest);
  WRITEDEBUGLN("*100ms");

  // calibration
  floatByteData_t data;
  float cal[MAX_CAL_VARS];

  uint8_t addr = EEPROM_CAL_ADDRESS;
  bool unknown = true;
  WRITEDEBUGLN("Calib.: ");
  for (int c = 0; c < NR_CAL_VALUES; ++c) {
    for (int b = 0; b < 4; ++b) {
      data.b[b] = EEPROM.read(addr++);
      if (data.b[b] != 255) {
        unknown = false;
      }
      // WRITEDEBUG(data.b[b]);
      // WRITEDEBUG(SEPARATOR);
    }
    // WRITEDEBUGLNF(data.f, 6);
    cal[c] = data.f;
  }
  if (unknown) {
    // no calib has been written, use some default
    #if NR_CAL_VALUES == 2
      cal[0] = DEFAULT_CAL_0;
      cal[1] = DEFAULT_CAL_1;
    #else
      for (int i = 0; i < NR_CAL_VALUES; ++i) {
        cal[i] = 1.0;
      }
    #endif
  }
  setCalibration(cal);

  addr = EEPROM_SCALE_ADDRESS;
  unknown = true;
  WRITEDEBUGLN("Scale: ");
  for (int c = 0; c < NR_SCALE_VALUES; ++c) {
    for (int b = 0; b < sizeof(data.f); ++b) {
      data.b[b] = EEPROM.read(addr++);
      if (data.b[b] != 255) {
        unknown = false;
      }
      // WRITEDEBUG(data.b[b]);
      // WRITEDEBUG(SEPARATOR);
    }
    // WRITEDEBUGLNF(data.f, 6);
    cal[c] = data.f;
  }
  if (unknown) {
#if NR_SCALE_VALUES == 3
    cal[0] = DEFAULT_SCALE_0;
    cal[1] = DEFAULT_SCALE_1;
    cal[2] = DEFAULT_SCALE_2;
#elif NR_SCALE_VALUES == 4
    cal[0] = DEFAULT_SCALE_0;
    cal[1] = DEFAULT_SCALE_1;
    cal[2] = DEFAULT_SCALE_2;
    cal[3] = DEFAULT_SCALE_3;
#else
    for (int i = 0; i < NR_SCALE_VALUES; ++i) {
      cal[i] = 0.001;
    }
#endif
  }
  setScaling(cal);
}
