// tonino_tcs3200.cpp
//----------------
// library for Tonino color sensor
//
// Basic parts of the TCS3200 code have been adapted from
// MD_TCS230_v1.0_20130222.zip
// at https://code.google.com/p/arduino-code-repository/
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

#include <tonino_tcs3200.h>


// for direct display access
LCD *TCS3200::_display;


TCS3200::TCS3200(uint8_t s2, uint8_t s3, uint8_t led, uint8_t power, LCD *display) :
  _S2(s2), _S3(s3), _LED(led), _POWER(power),
  _readDiv(NORMAL_SAMPLING), _colorMode(COLOR_FULL) {
  
  _display = display;
  
  #if NR_CAL_VALUES == 2
    _cal[0] = DEFAULT_CAL_0;
    _cal[1] = DEFAULT_CAL_1;
  #else
    for (int i = 0; i < NR_CAL_VALUES; ++i) {
      _cal[i] = 1.0;
    }
  #endif
  #if NR_SCALE_VALUES == 3
    _scale[0] = DEFAULT_SCALE_0;
    _scale[1] = DEFAULT_SCALE_1;
    _scale[2] = DEFAULT_SCALE_2;
  #elif NR_SCALE_VALUES == 4
    _scale[0] = DEFAULT_SCALE_0;
    _scale[1] = DEFAULT_SCALE_1;
    _scale[2] = DEFAULT_SCALE_2;
    _scale[3] = DEFAULT_SCALE_3;
  #else
    for (int i = 0; i < NR_SCALE_VALUES; ++i) {
      _scale[i] = 0.001;
    }
  #endif
}

TCS3200::~TCS3200(void) {
  // empty
}

// initialize TCS3200 color sensor
void TCS3200::init() {
  pinMode(_S2, OUTPUT);
  pinMode(_S3, OUTPUT);
  pinMode(_LED, OUTPUT);
  pinMode(_POWER, OUTPUT);
  
  sensorOff();
}

// uses fitting data to compute T-value from sensorData
int32_t TCS3200::fitValue(sensorData *sd, float* raw, uint8_t colorMode, boolean* averaged) {
#if DODEBUG
  WRITEDEBUG("scan:");
  for (int i = 0; i < 4; ++i) {
    WRITEDEBUG(sd->value[i]);
    WRITEDEBUG(SEPARATOR);
  }
#endif

  // check for devide by 0
  if (sd->value[BLUE_IDX] == 0) {
    WRITEDEBUGLN("ERR:div/0(blue)");
    return -1;
  }
  // calibrate
  float r = (float)sd->value[RED_IDX];
  float b = (float)sd->value[BLUE_IDX];
  float v = (r / b) * _cal[0] + _cal[1];
  
  // averaging
  if (raw != NULL && *raw != 0.0 && abs(v - *raw) < AVERAGE_THRESHOLD) {
    WRITEDEBUGLN("averaged:");
    WRITEDEBUG(v);
    WRITEDEBUG(SEPARATOR);
    WRITEDEBUGLN(*raw);
    v = (v + *raw) / 2.0;
    if (averaged != NULL) {
      *averaged = true;
    }
  } else {
    if (averaged != NULL) {
      *averaged = false;
    }
  }
    
  WRITEDEBUG(v);
  WRITEDEBUG(SEPARATOR);
  // scale
  int32_t tval = (int32_t)(_scale[0] * v*v*v + _scale[1] * v*v + _scale[2] * v + _scale[3] + 0.5);
  WRITEDEBUG("=");
  WRITEDEBUGLN(tval);
  if (raw != NULL) {
    *raw = v;
  }
  return tval;
}

// makes the actual measurement with LEDs on
// according to current sampling and color mode settings
// displayAnim if true, the display shows a "progress bar"
int32_t TCS3200::scan(float *raw, bool displayAnim, sensorData *outersd, boolean ledon, boolean removeExtLight, boolean *averaged) {
  uint8_t animPos = 0;
  sensorData sd;
  for (uint8_t i = 0; i < 5; ++i) {
    sd.value[i] = 0;
  }
  
  // display off
  if (_display != NULL) {
    _display->clear();
  }
  // switch on
  digitalWrite(_POWER, HIGH);
  if (ledon) {
    digitalWrite(_LED, HIGH);
  }
  delay(SENSOR_ON_DELAY);
  
  if (_colorMode & COLOR_WHITE) {
    if (displayAnim && _display != NULL) {
      _display->lineAnim(animPos++, 0);
    }
    setFilter(WHITE_IDX); // white sensor
    sd.value[WHITE_IDX] = readSingle();
  }
  if (_colorMode & COLOR_RED) {
    if (displayAnim && _display != NULL) {
      _display->lineAnim(animPos++, 0);
      if (animPos == 2) animPos++;
    }
    setFilter(RED_IDX); // red sensor
    uint8_t samplingBackup = _readDiv;
    _readDiv = min(REDSAMPLING_FACTOR*samplingBackup, 100);
    sd.value[RED_IDX] = readSingle();
    _readDiv = samplingBackup;
  }
  if (_colorMode & COLOR_BLUE) {
    if (displayAnim && _display != NULL) {
      _display->lineAnim(animPos, 0);
      if (animPos == 2) animPos++;
    }
    setFilter(BLUE_IDX); // blue sensor
    sd.value[BLUE_IDX] = readSingle();
  }
  if (_colorMode & COLOR_GREEN) {
    if (displayAnim && _display != NULL) {
      _display->lineAnim(animPos++, 0);
    }
    setFilter(GREEN_IDX); // green sensor
    sd.value[GREEN_IDX] = readSingle();
  }

  sensorOff();

	if (removeExtLight) {
		delay(500);
		digitalWrite(_POWER, HIGH);
		delay(SENSOR_ON_DELAY);
		
		WRITEDEBUG("ext light:");
		uint32_t ds;
		if (_colorMode & COLOR_WHITE) {
			setFilter(WHITE_IDX); // white sensor
			ds = readSingle();
			WRITEDEBUG(ds);
			WRITEDEBUG(" ");
			sd.value[WHITE_IDX] -= ds;
		}
		if (_colorMode & COLOR_RED) {
			setFilter(RED_IDX); // red sensor
			ds = readSingle();
			WRITEDEBUG(ds);
			WRITEDEBUG(" ");
			sd.value[RED_IDX] -= ds;
		}
		if (_colorMode & COLOR_BLUE) {
			setFilter(BLUE_IDX); // blue sensor
			ds = readSingle();
			WRITEDEBUG(ds);
			WRITEDEBUG(" ");
			sd.value[BLUE_IDX] -= ds;
		}
		if (_colorMode & COLOR_GREEN) {
			setFilter(GREEN_IDX); // green sensor
			ds = readSingle();
			WRITEDEBUG(ds);
			WRITEDEBUG(" ");
			sd.value[GREEN_IDX] -= ds;
		}
		sensorOff();
		WRITEDEBUGLN();
	} // if (removeExtLight)

  // calculate T-value according to current formula
  int32_t tval = fitValue(&sd, raw, _colorMode, averaged);
  
  if (outersd != NULL) {
    for (int i = 0; i < 4; ++i) {
      outersd->value[i] = sd.value[i];
    }
    outersd->value[4] = tval;
  }
  
  return tval;
}

// switch sensor completely off
void TCS3200::sensorOff() {
  digitalWrite(_S2, LOW);
  digitalWrite(_S3, LOW);
  digitalWrite(_LED, LOW);
  digitalWrite(_POWER, LOW);
}

// returns 1 if the first, darker, 2 if the second, lighter calibration plate is detected
// 0 otherwise; a quick sampling with LEDs switched on is conducted
uint8_t TCS3200::isCalibrating() {
  WRITEDEBUGLN("isCalib:");
  uint8_t samplingBackup = _readDiv;
  _readDiv = QUICK_SAMPLING;

  if (_display != NULL) {
    _display->clear();
  }

  digitalWrite(_POWER, HIGH);
  digitalWrite(_LED, HIGH);
  delay(SENSOR_ON_DELAY);

  setFilter(RED_IDX); // red sensor
  int32_t wval = readSingle();
  setFilter(BLUE_IDX); // blue sensor
  int32_t bval = readSingle();
  
  WRITEDEBUGLN(wval);
  WRITEDEBUGLN(bval);
  
  sensorOff();
  _readDiv = samplingBackup;

  uint8_t cal = 0;
  if ((abs(wval-LOW_RED) < RED_RANGE_LOW) && (abs(bval-LOW_BLUE) < BLUE_RANGE_LOW)) {
    // found first calibration plate
    cal = LOW_PLATE;
  } else if ((abs(wval-HIGH_RED) < RED_RANGE_HIGH) && (abs(bval-HIGH_BLUE) < BLUE_RANGE_HIGH)) {
    // found second calibration plate
    cal = HIGH_PLATE;
  }
  // else found no calibration plate
  
  WRITEDEBUG("isCalib:");
  WRITEDEBUG(wval);
  WRITEDEBUG(",");
  WRITEDEBUG(bval);
  WRITEDEBUG("=>");
  WRITEDEBUGLN(cal);
  return cal;
}

// returns true if quick sample without LEDs returns a rather high value
bool TCS3200::isLight() {
  uint8_t samplingBackup = _readDiv;
  _readDiv = QUICK_SAMPLING;

  digitalWrite(_LED, LOW);
  digitalWrite(_POWER, HIGH);
  setFilter(WHITE_IDX); // white sensor
  delay(SENSOR_ON_DELAY);
  
  uint32_t val = readSingle();
  sensorOff();
  _readDiv = samplingBackup;

  WRITEDEBUG("isLight:");
  WRITEDEBUG(val);
  WRITEDEBUG(" ");
  WRITEDEBUGLN((val > LIGHT_MIN) ? "T" : "F");
  // brighter than threshold?
  return val > LIGHT_MIN;
}

// returns true if quick sample without LEDs returns a low value
bool TCS3200::isDark() {
   return !isLight();
}  

// blocking read of a single sensor value
uint32_t TCS3200::readSingle(void) {
  delay(SENSOR_SWITCH_DELAY);
  FreqCount.begin(1000/_readDiv);    // start
  while (!FreqCount.available());   // wait
  FreqCount.end();                  // stop

  return(FreqCount.read() * _readDiv);
}

// set the sensor color filter
void TCS3200::setFilter(uint8_t f) {
  //WRITEDEBUG("setFilter ");
  switch (f) {
    case RED_IDX:   /*WRITEDEBUGLN("R");*/ 
			digitalWrite(_S2, LOW);
			digitalWrite(_S3, LOW);  
			break;
    case GREEN_IDX:  /*WRITEDEBUGLN("G");*/ 
			digitalWrite(_S2, HIGH); 
			digitalWrite(_S3, HIGH); 
			break;
    case BLUE_IDX:  /*WRITEDEBUGLN("B");*/ 
			digitalWrite(_S2, LOW);  
			digitalWrite(_S3, HIGH); 
			break;
    case WHITE_IDX:  /*WRITEDEBUGLN("X");*/ 
			digitalWrite(_S2, HIGH); 
			digitalWrite(_S3, LOW);  
			break;
    default:  
			WRITEDEBUG("ERR:unk ");
			WRITEDEBUGLN(f);
  }
}

// store new calibration data
void TCS3200::setCalibration(float *cal) {
  for (int i = 0; i < NR_CAL_VALUES; ++i) {
    _cal[i] = cal[i];
  }
}

// retrieve current calibration data
void TCS3200::getCalibration(float *cal) {
  for (int i = 0; i < NR_CAL_VALUES; ++i) {
    cal[i] = _cal[i];
  }
}

// store new scaling data
void TCS3200::setScaling(float *scale) {
  for (int i = 0; i < NR_SCALE_VALUES; ++i) {
    _scale[i] = scale[i];
  }
}

// retrieve current scaling data
void TCS3200::getScaling(float *scale) {
  for (int i = 0; i < NR_SCALE_VALUES; ++i) {
    scale[i] = _scale[i];
  }
}

// store new sampling value, [0..100]
void TCS3200::setSampling(uint8_t sampling) {
  _readDiv = ((sampling > 0 && sampling <= 100) ? sampling : _readDiv);
}

// retrieve current sampling value
uint8_t TCS3200::getSampling() {
  return _readDiv;
}

// store new color mode
void TCS3200::setColorMode(uint8_t colorMode) {
  if (colorMode == 0 || colorMode > COLOR_FULL) {
    WRITEDEBUG("ERR:unk ");
    WRITEDEBUGLN(colorMode);
    return;
  }
  _colorMode = colorMode;
}

// retrieve current colorMode
uint8_t TCS3200::getColorMode() {
  return _colorMode;
}
