// tonino_serial.cpp
//----------------
// library for Tonino serial communication
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


#include <tonino_serial.h>


SerialCommand ToninoSerial::_sCmd;
TCS3200 *ToninoSerial::_colorSense;
LCD *ToninoSerial::_display;
ToninoConfig *ToninoSerial::_tConfig;
const char *ToninoSerial::_version;
const uint32_t NOTSET = 4242424242;

ToninoSerial::ToninoSerial(TCS3200 *colorSense, LCD *display, ToninoConfig *tConfig, const char *ver) {
  _colorSense = colorSense;
  _display = display;
  _tConfig = tConfig;
  _version = ver;
}

ToninoSerial::~ToninoSerial(void) {
  // empty
}

// returns true if the value is not a valid float
boolean ToninoSerial::isInvalidNumber(float f) {
  return isnan(f) || isinf(f) || f > 4294967040.0 || f <-4294967040.0 || f == 0x7fffffff || f == (-0x7fffffff -1L);
}

// poll if there is an incoming serial command
boolean ToninoSerial::checkCommands() {
   return _sCmd.readSerial();
}

// initializes serial communication and registers functions for serial commands
void ToninoSerial::init(uint32_t speed) {
  Serial.begin(speed);

//  WRITEDEBUGLN("[Tonino]");
//  WRITEDEBUGLN("Available Commands");
//  WRITEDEBUGLN("  TONINO : return version information");
//  WRITEDEBUGLN("  SCAN : scan and return T-value");
//  WRITEDEBUGLN("  I_SCAN : scan and return w/b");
//  WRITEDEBUGLN("  II_SCAN : scan and return raw values");
//  WRITEDEBUGLN("  D_SCAN : scan without LEDs and return raw values");
//  WRITEDEBUGLN("  SETCAL : store calibration values");
//  WRITEDEBUGLN("  GETCAL : retrieve current calibration values");
//  WRITEDEBUGLN("  SETSCALING : store scaling values");
//  WRITEDEBUGLN("  GETSCALING : retrieve scaling values");
//  WRITEDEBUGLN("  SETBRIGHTNESS : set brightness (0-15)");
//  WRITEDEBUGLN("  GETBRIGHTNESS : get brightness (0-15)");
//  WRITEDEBUGLN("  SETSAMPLING: set sampling");
//  WRITEDEBUGLN("  GETSAMPLING : get sampling");
//  WRITEDEBUGLN("  SETCMODE: set color measure mode");
//  WRITEDEBUGLN("  GETCMODE : get color measure mode");
//  WRITEDEBUGLN("  SETCALINIT: use (1) or not (0) check for calib at start");
//  WRITEDEBUGLN("  GETCALINIT: is (1) or is not (0) checked for calib at start");
//  WRITEDEBUGLN("  SETLTDELAY: set delay between can-up measurements, in 1/10sec");
//  WRITEDEBUGLN("  GETLTDELAY: get delay between can-up measurements, in 1/10sec");
//  WRITEDEBUGLN("  RESETDEF: reset settings back to defaults");
  
  // ATTENTION: only the first SERIALCOMMAND_MAXCOMMANDLENGTH (def=8) characters of the command are used
  
  // setup callbacks for SerialCommand commands
  _sCmd.addCommand("TONINO", getVersion);
  _sCmd.addCommand("SCAN", scan);
  _sCmd.addCommand("I_SCAN", i_scan);
  _sCmd.addCommand("II_SCAN", ii_scan);
  _sCmd.addCommand("D_SCAN", d_scan);
  _sCmd.addCommand("SETCAL", setCalibration);
  _sCmd.addCommand("GETCAL", getCalibration);
  _sCmd.addCommand("SETSCALI", setScaling);
  _sCmd.addCommand("GETSCALI", getScaling);
  _sCmd.addCommand("SETBRIGH", setBrightness);
  _sCmd.addCommand("GETBRIGH", getBrightness);
  _sCmd.addCommand("SETSAMPL", setSampling);
  _sCmd.addCommand("GETSAMPL", getSampling);
  _sCmd.addCommand("SETCMODE", setColorMode);
  _sCmd.addCommand("GETCMODE", getColorMode);
  _sCmd.addCommand("SETCALIN", setCheckCalInit);
  _sCmd.addCommand("GETCALIN", getCheckCalInit);
  _sCmd.addCommand("SETLTDEL", setDelayTillUpTest);
  _sCmd.addCommand("GETLTDEL", getDelayTillUpTest);
  _sCmd.addCommand("RESETDEF", resetToDefaults);
}

// print version to serial
void ToninoSerial::getVersion() {
  Serial.print("TONINO:");
  Serial.print(_version);
  Serial.print("\n");
  
  _display->connected();
}

// make a measurement and print result to serial (and LCD)
void ToninoSerial::scan() {
  sensorData col;
  int32_t val = _colorSense->scan();

  Serial.print("SCAN:");
  Serial.print(val);
  Serial.print("\n");

  if (val < 0 || val > 9999) {
    _display->line();
  } else {
    _display->snake(val);
  }
}

// make a measurement and print calibrated, w/b value to serial 
// (and T-value to LCD)
void ToninoSerial::i_scan() {
  float ratio = 0.0;
  int32_t val = _colorSense->scan(&ratio);
  
  Serial.print("I_SCAN:");
  Serial.print(ratio, 6);
  Serial.print("\n");

  if (val < -999 || val > 9999) {
    _display->line();
  } else {
    _display->printNumber(val);
  }
}

// make a measurement and print raw color values to serial 
// (and T-value to LCD)
void ToninoSerial::ii_scan() {
  sensorData sd;
  int32_t val = _colorSense->scan(NULL, false, &sd);
  
  Serial.print("II_SCAN:");
  for (int i = 0; i < 5; ++i) {
    Serial.print(sd.value[i]);
    Serial.print(SEPARATOR);
  }
  Serial.print("\n");

  if (val < -999 || val > 9999) {
    _display->line();
  } else {
    _display->printNumber(val);
  }
}

// make a measurement with LEDs switched off and print raw color values to serial 
void ToninoSerial::d_scan() {
  sensorData sd;
  _colorSense->scan(NULL, false, &sd, false);
  
  Serial.print("D_SCAN:");
  for (int i = 0; i < 4; ++i) {
    Serial.print(sd.value[i]);
    Serial.print(SEPARATOR);
  }
  Serial.print("\n");
}

// store calibration data from serial to local vars and EEPROM
void ToninoSerial::setCalibration() {
  float cal[NR_CAL_VALUES];

  // parse input
  for (int c = 0; c < NR_CAL_VALUES; ++c) {
    char *arg = _sCmd.next();
		cal[c] = atof(arg);
    if (isInvalidNumber(cal[c])) {
      WRITEDEBUG("SETCAL ERR:inv num ");
      WRITEDEBUGLN(cal[c]);
      Serial.print("SETCAL ERROR");
      Serial.print("\n");
      return;
    }
    WRITEDEBUG(cal[c]);
    WRITEDEBUG(" ");
  }
  WRITEDEBUGLN();
  _tConfig->setCalibration(cal);
  Serial.print("SETCAL");
  Serial.print("\n");
}

// print current calibration data to serial
void ToninoSerial::getCalibration() {
  float cal[NR_CAL_VALUES];
  _colorSense->getCalibration(cal);
  Serial.print("GETCAL:");
  for (int i = 0; i < NR_CAL_VALUES; ++i) {
    Serial.print(cal[i], 6);
    Serial.print(SEPARATOR);
  }
  Serial.print("\n");
}

// store scaling data from serial to local vars and EEPROM
void ToninoSerial::setScaling() {
  float scal[NR_SCALE_VALUES];

  // parse input
  WRITEDEBUG("[scale] ");
  for (int c = 0; c < NR_SCALE_VALUES; ++c) {
    char *arg = _sCmd.next();
    scal[c] = atof(arg);
    if (isInvalidNumber(scal[c])) {
      WRITEDEBUG("SETSCALING ERR:inv num ");
      WRITEDEBUGLN(scal[c]);
      Serial.print("SETSCALING ERROR");
      Serial.print("\n");
      return;
    }
    WRITEDEBUG(scal[c]);
    WRITEDEBUG(" ");
  }
  WRITEDEBUGLN();
  _tConfig->setScaling(scal);
  Serial.print("SETSCALING");
  Serial.print("\n");
}

// print current scaling data to serial
void ToninoSerial::getScaling() {
  float cal[NR_SCALE_VALUES];
  _colorSense->getScaling(cal);
  Serial.print("GETSCALING:");
  for (int i = 0; i < NR_SCALE_VALUES; ++i) {
    Serial.print(cal[i], 6);
    Serial.print(SEPARATOR);
  }
  Serial.print("\n");
}

// sets the display brightness (0-15, 15=max brightness) and store in EEPROM
void ToninoSerial::setBrightness() {
  // get from serial
  char *arg = _sCmd.next();
  int32_t b = strtol(arg, NULL, 10);
  if (isInvalidNumber(b)) {
    WRITEDEBUG("SETBRIGHTNESS ERR:inv num ");
    WRITEDEBUGLN(b);
    Serial.print("SETBRIGHTNESS ERROR");
    Serial.print("\n");
    return;
  }
  if (b < 0 || b > 15) {
    WRITEDEBUG("SETBRIGHTNESS ERR:range ");
    WRITEDEBUGLN(b);
    Serial.print("SETBRIGHTNESS ERROR");
    Serial.print("\n");
  } else {
    _tConfig->setBrightness((uint8_t)b);
    Serial.print("SETBRIGHTNESS");
    Serial.print("\n");
  }
}

// print current scaling data to serial
void ToninoSerial::getBrightness() {
  Serial.print("GETBRIGHTNESS:");
  Serial.print(_display->getBrightness());
  Serial.print("\n");
}

// save sampling rate setting from serial to sensor library and EEPROM
void ToninoSerial::setSampling() {
  // get from serial
  char *arg = _sCmd.next();
  int32_t sampling = strtol(arg, NULL, 10);
  if (isInvalidNumber(sampling)) {
    WRITEDEBUG("SETSAMPLING ERR:inv num ");
    WRITEDEBUGLN(sampling);
    Serial.print("SETSAMPLING ERROR");
    Serial.print("\n");
    return;
  }
  if (sampling <= 0 || sampling > 100) {
    WRITEDEBUG("SETSAMPLING ERR:range ");
    WRITEDEBUGLN(sampling);
    Serial.print("SETSAMPLING ERROR");
    Serial.print("\n");
  } else {
    _tConfig->setSampling((uint8_t)sampling);
    Serial.print("SETSAMPLING");
    Serial.print("\n");
  }
}

// retrieve sampling rate setting from sensor library
void ToninoSerial::getSampling() {
  Serial.print("GETSAMPLING:");
  Serial.print(_colorSense->getSampling());
  Serial.print("\n");
}

// save color mode setting from serial to sensor library and EEPROM
void ToninoSerial::setColorMode() {
  // get from serial
  char *arg = _sCmd.next();
  int32_t cmode = strtol(arg, NULL, 10);
  if (isInvalidNumber(cmode)) {
    WRITEDEBUG("SETCMODE ERR:inv ");
    WRITEDEBUGLN(cmode);
    Serial.print("SETCMODE ERROR");
    Serial.print("\n");
    return;
  }
  if (cmode <= 0 || cmode > COLOR_FULL) {
    WRITEDEBUG("SETCMODE ERR:range ");
    WRITEDEBUGLN(cmode);
    Serial.print("SETCMODE ERROR");
    Serial.print("\n");
  } else {
    _tConfig->setColorMode((uint8_t)cmode);
    Serial.print("SETCMODE");
    Serial.print("\n");
  }
}

// retrieve color mode setting from sensor library
void ToninoSerial::getColorMode() {
  Serial.print("GETCMODE:");
  Serial.print(_colorSense->getColorMode());
  Serial.print("\n");
}

// save initial calibration setting to EEPROM
void ToninoSerial::setCheckCalInit() {
  // get from serial
  char *arg = _sCmd.next();
  int16_t iwc = atoi(arg);
  if (_sCmd.next() != NULL || (iwc != 0 && iwc != 1)) {
    Serial.print("SETCALINIT ERROR");
    Serial.print("\n");
  } else {
    _tConfig->setCheckCalInit(iwc == 0 ? false : true);
    Serial.print("SETCALINIT");
    Serial.print("\n");
  }
}

// retrieve initial wihte calibration setting
void ToninoSerial::getCheckCalInit() {
  Serial.print("GETCALINIT:");
  Serial.print(_tConfig->getCheckCalInit() ? 1 : 0);
  Serial.print("\n");
}

// save delay between can-up measurements, in 1/10sec to EEPROM
void ToninoSerial::setDelayTillUpTest() {
  // get from serial
  char *arg = _sCmd.next();
  int32_t ltdelay = strtol(arg, NULL, 10);
  if (isInvalidNumber(ltdelay)) {
    WRITEDEBUG("SETLTDELAY ERR:inv");
    WRITEDEBUGLN(ltdelay);
    Serial.print("SETLTDELAY ERROR");
    Serial.print("\n");
    return;
  }
  if (ltdelay < 0 || ltdelay >= 256) {
    WRITEDEBUG("SETLTDELAY ERR:range ");
    WRITEDEBUGLN(ltdelay);
    Serial.print("SETLTDELAY ERROR");
    Serial.print("\n");
  } else {
    _tConfig->setDelayTillUpTest((uint8_t)ltdelay);
    Serial.print("SETLTDELAY");
    Serial.print("\n");
  }
}

// retrieve initial wihte calibration setting
void ToninoSerial::getDelayTillUpTest() {
  Serial.print("GETLTDELAY:");
  Serial.print(_tConfig->getDelayTillUpTest());
  Serial.print("\n");
}

// reset settings back to defaults
void ToninoSerial::resetToDefaults() {
  _tConfig->writeDefaults();
  Serial.print("RESETDEF");
  Serial.print("\n");
}
