// tonino_serial.h
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

#ifndef _TONINO_SERIAL_H
#define _TONINO_SERIAL_H


// external lib that calls method according to serial input
// slightly adapted from
// https://github.com/kroimon/Arduino-SerialCommand, Version 20120522
// (increased command buffer size and readSerial return true if command encountered)
#include <SerialCommand.h>

#include <tonino.h>
#include <tonino_config.h>
#include <tonino_lcd.h>


class ToninoSerial {
  public:
    // constructor taking pointers to color sensor, display, configuration, and a version string
    ToninoSerial(TCS3200 *colorSense, LCD *display, ToninoConfig *tConfig, const char *ver);
    ~ToninoSerial(void);

    // initializes serial communication and registers functions for serial commands
    void init(uint32_t speed);

    // poll if there is an incoming serial command
    static boolean checkCommands();
    
    // following methods need to be static for SerialCommand lib

    // print version to serial, e.g. TONINO:0 3 002
    static void getVersion();

    // make a full scan and print resulting T-value to serial (and LCD), e.g. SCAN:121
    static void scan();

    // make a full scan and print w/b to serial (and T-value to LCD), e.g. I_SCAN:1.34543
    static void i_scan();

    // make a full scan and print raw measurement to serial (and T-value to LCD), e.g. II_SCAN:216575 86428
    static void ii_scan();

    // make a full scan with LEDs switched off and print raw measurement to serial, e.g. D_SCAN:100 50
    static void d_scan();

    // store calibration data from serial to local vars and EEPROM; response: SETCAL
    static void setCalibration();

    // print current calibration data to serial, e.g. GETCAL:0.95 1.12 1.32 0.89
    static void getCalibration();

    // store scaling data from serial to local vars and EEPROM; response: SETSCALING
    static void setScaling();

    // print current scaling data to serial, e.g. GETSCALING:132.232 99.232 -6.3232
    static void getScaling();

    // save sampling rate setting from serial to sensor library and EEPROM; response: SETSAMPLING
    // responds with SETSAMPLING ERROR if <=0 or >=255
    static void setSampling();

    // retrieve sampling rate setting from sensor library, e.g. GETSAMPLING:7
    static void getSampling();

    // save color mode setting from serial to sensor library and EEPROM; response SETCMODE
    // responds with SETCMODE ERROR if not one of COLOR_XXX constants
    static void setColorMode();

    // retrieve color mode setting from sensor library, e.g. GETCMODE:15
    // see COLOR_XXX constants
    static void getColorMode();

    // save whether initial calibration is tried (1) or not (0) to EEPROM and config manager
    // response SETCALINIT
    // responds with SETCALINIT ERROR if not 0 or 1
    static void setCheckCalInit();

    // retrieve whether initial white calibration is tried (1) or not (0), e.g. SETCALINIT:1
    static void getCheckCalInit();

    // set the display brightness (0-15, 15=max brightness)
    static void setBrightness();

    // get the display brightness (0-15, 15=max brightness)
    static void getBrightness();

    // save delay time between successive measurements that test if can was lifted to EEPROM; response SETLTDELAY
    // ATTENTION: one byte value (0-255) in 1/10sec, e.g. 20 means 2 seconds
    static void setDelayTillUpTest();

    // retrieve delay time between successive measurements that test if can was lifted; e.g. GETLTDELAY:20
    static void getDelayTillUpTest();

    // reset settings back to defaults
    static void resetToDefaults();

    
  private:
    // object for lib that calls methods according to serial input
    static SerialCommand _sCmd;

    // returns true if the value is not a valid float
    static boolean isInvalidNumber(float f);

    // object for communication with the color sensor
    static TCS3200 *_colorSense;
    // object for communication with the display
    static LCD *_display;
    // object for communication with the configuration manager
    static ToninoConfig *_tConfig;
    // version string passed by main program
    static const char *_version;
};

#endif