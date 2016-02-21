// tonino_config.cpp
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

// returns true if the value is not a valid float
bool ToninoConfig::isInvalidNumber(float f) {
  return isnan(f) || isinf(f) || f > 4294967040.0 || 
         f <-4294967040.0 || f == 0x7fffffff || f == (-0x7fffffff -1L);
}

// store calibration data to sensor library and EEPROM
void ToninoConfig::setCalibration(float *cal) {
  _colorSense->setCalibration(cal);

  floatByteData_t data;
  int addr = EEPROM_CAL_ADDRESS;
  for (int c = 0; c < NR_CAL_VALUES; ++c) {
    data.f = cal[c];
    WRITEDEBUG("Calib. ");
    WRITEDEBUGF(data.f, 6);
    WRITEDEBUGLN(" as");
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
    WRITEDEBUG("Scaling ");
    WRITEDEBUGF(data.f, 6);
    WRITEDEBUGLN(" as");
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
  if (_display != NULL) {
		_display->setBrightness(b);
		checkedEepromWrite(EEPROM_BRIGHTNESS_ADDRESS, b);
	}
}

// gets the display brightness from EEPROM (0-15, 15=max brightness)
uint8_t ToninoConfig::getBrightness() {
  if (_display != NULL) {
		return _display->getBrightness();
	}
	return 0;
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
  WRITEDEBUGLN("Store def");
  WRITEDEBUG("Sampling: ");
  setSampling(DEFAULT_SAMPLING);
  WRITEDEBUG("Bright: ");
  setBrightness(DEFAULT_BRIGHTNESS);
  WRITEDEBUG("Col mode: ");
  setColorMode(DEFAULT_COLORS);
  WRITEDEBUG("Do_init_cal: ");
  setCheckCalInit(DEFAULT_DOCALINIT);
  WRITEDEBUG("Delay_till_up: ");
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
  for (uint8_t cyc = 0; cyc < EEPROM_REDUNDANT_CYCLES+1; cyc++) {
    uint8_t raddr = EEPROM_CHANGED_ADDRESS + cyc*EEPROM_SIZE;
    if (EEPROM.read(raddr) != EEPROM_SET) {
      WRITEDEBUG("upd addr ");
      WRITEDEBUG(raddr);
      WRITEDEBUG(" from ");
      WRITEDEBUG(EEPROM.read(raddr));
      WRITEDEBUG(" to ");
      WRITEDEBUG(EEPROM_SET);
      EEPROM.write(raddr, EEPROM_SET);
      WRITEDEBUG("=>");
      WRITEDEBUGLN(EEPROM.read(raddr));
    }
  }
  WRITEDEBUGLN();

  bool changed = false;
  for (uint8_t cyc = 0; cyc < EEPROM_REDUNDANT_CYCLES+1; cyc++) {
    uint8_t raddr = addr + (cyc*EEPROM_SIZE);
    WRITEDEBUG("read addr ");
    WRITEDEBUG(raddr);
    WRITEDEBUG(":");
    WRITEDEBUGLN(EEPROM.read(raddr));
    if (EEPROM.read(raddr) != val) {
      WRITEDEBUG("upd addr ");
      WRITEDEBUG(raddr);
      WRITEDEBUG(" from ");
      WRITEDEBUG(EEPROM.read(raddr));
      WRITEDEBUG(" to ");
      WRITEDEBUG(val);
      EEPROM.write(raddr, val);
      WRITEDEBUG("=>");
      WRITEDEBUGLN(EEPROM.read(raddr));
      changed = true;
    } else {
      WRITEDEBUG("no update ");
      WRITEDEBUGLN(EEPROM.read(raddr));
    }
  }
  return changed;
}

// finds most frequent value in given array
// return true if at least one value is different
// TODO more efficient for special values of EEPROM_REDUNDANT_CYCLES
bool ToninoConfig::getMostFrequent(uint8_t *vals, uint8_t *val) {
  uint8_t count = 0;
  uint8_t curCount;
  uint8_t idx = 0;
  for (uint8_t i = 0; i < EEPROM_REDUNDANT_CYCLES+1; i++) {
    if (i > 0 && vals[i] == vals[idx]) {
      // ignore if already counted (only catches most freq up to that point)
      continue;
    }
    curCount = 0;
    for (int j = 0; j < EEPROM_REDUNDANT_CYCLES+1; j++) {
      if (vals[i] == vals[j]) {
        curCount++;
      }
    }
    WRITEDEBUG("found ");
    WRITEDEBUG(vals[i]);
    WRITEDEBUG(SEPARATOR);
    WRITEDEBUG(curCount);
    WRITEDEBUGLN("times");
    if (curCount > count) {
      idx = i;
      count = curCount;
    }
  }
  *val = vals[idx];
  // return true if not all numbers were the same
  return count != EEPROM_REDUNDANT_CYCLES+1;
}

// reads value at given address
uint8_t ToninoConfig::checkedEepromRead(uint8_t addr) {
  uint8_t vals[EEPROM_REDUNDANT_CYCLES+1];
  WRITEDEBUG("read");
  for (uint8_t cyc = 0; cyc < EEPROM_REDUNDANT_CYCLES+1; cyc++) {
    uint8_t raddr = addr + cyc*EEPROM_SIZE;
    WRITEDEBUG(" addr ");
    WRITEDEBUG(raddr);
    WRITEDEBUG(":");
    vals[cyc] = EEPROM.read(raddr);
    WRITEDEBUG(vals[cyc]);
  }
  WRITEDEBUGLN();
  
  // need to find most probably correct (=most frequent) value
  uint8_t val = 0;
  if (getMostFrequent(vals, &val)) {
    // need to update bc at least one value was different
    WRITEDEBUG("using ");
    WRITEDEBUG(val);
    WRITEDEBUGLN("->correcting");
    checkedEepromWrite(addr, val);
  } else {
    WRITEDEBUG("using ");
    WRITEDEBUG(val);
    WRITEDEBUGLN("->ok");
  }
  return val;
}

// true if EEPROM has already been (most probably) used to store config
// (checks whether byte at EEPROM_CHANGED_ADDRESS is set to EEPROM_SET)
inline bool ToninoConfig::isEepromChanged() {
  return (checkedEepromRead(EEPROM_CHANGED_ADDRESS) == EEPROM_SET);
}

// read all config data from EEPROM to colorSense and local variables;
// should only be used if isEepromChanged() returned true
// if some settings have not been set, uses and stores default values
void ToninoConfig::readStoredParameters() {
  WRITEDEBUGLN("Read EEPROM");
  // sampling rate
  WRITEDEBUG("Sampling:");
  uint8_t value = checkedEepromRead(EEPROM_SAMPLING_ADDRESS);
  if (value > 100) {
    setSampling(DEFAULT_SAMPLING);
  } else {
    setSampling(value);
  }
  WRITEDEBUG(_colorSense->getSampling());
  WRITEDEBUGLN();

  // display brightness
  WRITEDEBUG("Bright:");
  if (_display != NULL) {
		value = checkedEepromRead(EEPROM_BRIGHTNESS_ADDRESS);
		if (value > 15) {
			setBrightness(DEFAULT_BRIGHTNESS);
		} else {
			setBrightness(value);
		}
		WRITEDEBUG(getBrightness());
	} else {
		WRITEDEBUG(" ignored");
	}
  WRITEDEBUGLN();

  // color mode
  WRITEDEBUG("Col mode:");
  value = checkedEepromRead(EEPROM_CMODE_ADDRESS);
  if (value == 0 || value > COLOR_FULL) {
    setColorMode(DEFAULT_COLORS);
  } else {
    setColorMode(value);
  }
  WRITEDEBUG(_colorSense->getColorMode());
  WRITEDEBUGLN();

  // setting for initial calibration
  WRITEDEBUG("Do_init_cal:");
  value = checkedEepromRead(EEPROM_CCI_ADDRESS);
  if (value == 255) {
    setCheckCalInit(DEFAULT_DOCALINIT ? true : false);
  } else {
    setCheckCalInit(value == 0 ? false : true);
  }
  WRITEDEBUG(_doInitCal ? 1 : 0);
  WRITEDEBUGLN();

  // setting for delay till "can up" test
  WRITEDEBUG("Delay_till_up:");
  value = checkedEepromRead(EEPROM_DELAYTILLUPTEST_ADDRESS);
  if (value == 255) {
    setDelayTillUpTest(DEFAULT_DELAYTILLUPTEST);
  } else {
    setDelayTillUpTest(value);
  }
  WRITEDEBUG(_delayTillUpTest);
  WRITEDEBUGLN("*100ms");

  // calibration
  floatByteData_t data;
  float cal[MAX_CAL_VARS];

  uint8_t addr = EEPROM_CAL_ADDRESS;
  bool unknown = true;
  WRITEDEBUGLN("Calib.:");
  for (int c = 0; c < NR_CAL_VALUES; ++c) {
    for (int b = 0; b < sizeof(data.f); ++b) {
      data.b[b] = checkedEepromRead(addr++);
      if (data.b[b] != 255) {
        unknown = false;
      }
      WRITEDEBUG(data.b[b]);
      WRITEDEBUG(SEPARATOR);
    }
    WRITEDEBUGLNF(data.f, 6);
    if (isInvalidNumber(data.f)) {
      unknown = true;
      WRITEDEBUGLN("err->default");
      break;
    }
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
  WRITEDEBUGLN("Scale:");
  for (int c = 0; c < NR_SCALE_VALUES; ++c) {
    for (int b = 0; b < sizeof(data.f); ++b) {
      data.b[b] = checkedEepromRead(addr++);
      if (data.b[b] != 255) {
        unknown = false;
      }
      WRITEDEBUG(data.b[b]);
      WRITEDEBUG(SEPARATOR);
    }
    WRITEDEBUGLNF(data.f, 6);
    if (isInvalidNumber(data.f)) {
      unknown = true;
      WRITEDEBUGLN("err->default");
      break;
    }
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
