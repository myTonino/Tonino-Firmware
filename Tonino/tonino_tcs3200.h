// tonino_tcs3200.h
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


#ifndef _TONINO_TCS3200_H
#define _TONINO_TCS3200_H


// frequence counting lib for color sensor, http://www.pjrc.com/teensy/td_libs_FreqCount.html, Version 1.0
#include <FreqCount.h>

#include <tonino.h>
#include <tonino_lcd.h>


// sampling rates; x means scan duration of 1/x per color
#define FULL_SAMPLING 1
#define SLOW_SAMPLING 2
#define NORMAL_SAMPLING 3
#define QUICK_SAMPLING 100

// RED color will be measured longer by this factor
#define REDSAMPLING_FACTOR 2

// flags for selecting which colors are measured
#define COLOR_WHITE  0b00000001
#define COLOR_RED    0b00000010
#define COLOR_GREEN  0b00000100
#define COLOR_BLUE   0b00001000
#define COLOR_FULL  (COLOR_WHITE|COLOR_RED|COLOR_GREEN|COLOR_BLUE)

// indices for filter selection etc.
#define   WHITE_IDX   0  // 'Clear' filter selection
#define   RED_IDX     1
#define   GREEN_IDX   2
#define   BLUE_IDX    3
#define   T_IDX       4  // index for storing T-value

// number of parameters for scaling / calibration
#define NR_SCALE_VALUES 4
#define NR_CAL_VALUES 2
#define MAX_CAL_VARS max(NR_SCALE_VALUES,NR_CAL_VALUES)

// delay (ms) after turning sensor on
#define SENSOR_ON_DELAY 1

// delay (ms) after switching sensor channel (in readSingle)
#define SENSOR_SWITCH_DELAY 0

// threshold for detecting can lifting and replacing
#define LIGHT_MIN 199

// calibration plates (see isCalibrating method)
#define LOW_PLATE  1  // first, low, dark, brown, calibration plate
#define HIGH_PLATE 2  // second, high, bright, red, calibration plate

// threshold up to wich averaging with previous value in scan/fitValue is enabled
#define AVERAGE_THRESHOLD 0.044 // 0,011 correspond to about 1 value on the Tonino scale

// thresholds for detecting calibration plates at startup
#define LOW_RED      2600 // brown disk red reading
#define LOW_BLUE     1600 // brown disk blue reading
#define HIGH_RED    15000 // red disk red reading
#define HIGH_BLUE    3600 // red disk blue reading
#define RED_RANGE_LOW    2100
#define RED_RANGE_HIGH   7000
#define BLUE_RANGE_LOW   1500
#define BLUE_RANGE_HIGH  2100
#define LOW_TARGET  1.5   // brown disk target r/b value
#define HIGH_TARGET 3.7   // red disk target r/b value


// to store one measurement (4 colors and T-value)
typedef struct {
  int32_t value[5];
} sensorData;


class TCS3200 {
  public:
    // constructor taking all control pins and display
    TCS3200(uint8_t s2, uint8_t s3, uint8_t led, uint8_t power, LCD *display);
    ~TCS3200(void);

    // configures pins and calls sensorOff()
    void init();

    // runs a measurement with current config
    // returns T-value
    // if displayAnim is true shows small "progress bar" 
    // if raw is not NULL it is calibrated single value (e.g. w/b)
    // if sd is not NULL it contains the actual raw color measurement values
    // with the T-value at T_IDX
    // if ledon is true, LEDs are switched on during measurement
    // if removeExtLight is true, additional 'dark' measurement is done
    int32_t scan(float *raw = NULL, bool displayAnim = false, sensorData *sd = NULL, boolean ledon = true, boolean removeExtLight = false, boolean *averaged = NULL);
    
    // switch sensor completely off
    void sensorOff();

    // returns 1 if the first, darker, 2 if the second, lighter calibration plate is detected
    // 0 otherwise; a quick sampling with LEDs in conducted
    uint8_t isCalibrating();
    // true if it detects that the can was lifted
    bool isLight();
    // true if it detects that can is not lifted
    bool isDark();

    // set the NR_SCALE_VALUES values to derive T-value
    void setScaling(float *scale);
    // get the NR_SCALE_VALUES values to derive T-value
    void getScaling(float *scale);
    // set the NR_CAL_VALUES values for calibration
    void setCalibration(float *cal);
    // get the NR_CAL_VALUES values for calibration
    void getCalibration(float *cal);
    // set sampling rate, see xxx_SAMPLING constants, [0..100]
    void setSampling(uint8_t sampling);
    // get sampling rate
    uint8_t getSampling();
    // set color mode, see COLOR_xxx constants
    void setColorMode(uint8_t mode);
    // get color mode, see COLOR_xxx constants
    uint8_t getColorMode();

      
  private:
    // object for communication with the display
    static LCD *_display;

    // pin to power the whole sensor board
    uint8_t _POWER;
    // pin to switch on/off the 4 LEDs
    uint8_t _LED;
    // pins for photodiode filter selection
    uint8_t _S2, _S3;
    
    // sampling rate, i.e. fraction of 1 second read
    uint8_t _readDiv;
    // specifies which color sensors are used
    uint8_t _colorMode;
    // scaling data
    float _scale[NR_SCALE_VALUES];
    // calibration data
    float _cal[NR_CAL_VALUES];
    
    // synchronously (blocking) read a value
    uint32_t readSingle();
    // set the photodiode filter, must be one of xxx_IDX constants
    void setFilter(uint8_t f);
    // convert raw sensor data (in sd) into T-value using calibration and scaling
    // return value is T-value
    // if raw is not NULL, it contains the calibrated single value
    int32_t fitValue(sensorData *sd, float *raw, uint8_t colorMode = COLOR_FULL, boolean *averaged = NULL);
};

#endif
