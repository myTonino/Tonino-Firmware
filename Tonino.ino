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
