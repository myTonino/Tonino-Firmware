// tonino.h
//----------------
// library for Tonino
// mostly contains default settings and debug code
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

#define DODEBUG false
// values will be stored in EEPROM value+1 timez; set to 0 for no redundancy
// pay attention that EEPROM_START_ADDRESS + (EEPROM_REDUNDANT_CYCLES+1)*EEPROM_SIZE
// is still within EEPROM limits
#define EEPROM_REDUNDANT_CYCLES 2

// separator between multiple values in one line
#define SEPARATOR " "

#ifndef _TONINO_H
#define _TONINO_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

// low power mode if no action during this time
// DIM < SLEEP < POWERDOWN
#define TIME_TILL_DIM        120000 //  2 minutes = 120000
#define TIME_TILL_SLEEP      600000 // 10 minutes = 600000
#define TIME_TILL_POWERDOWN 7200000 //  2 hours =  7200000

// default values for parameters
#define DEFAULT_SAMPLING SLOW_SAMPLING
#define DEFAULT_COLORS (COLOR_RED|COLOR_BLUE)
#define DEFAULT_DELAYTILLUPTEST 10 // in 100ms, i.e. 10 means 1 second
#define DEFAULT_BRIGHTNESS 10
#define DEFAULT_DOCALINIT true
#define DEFAULT_SCALE_0 0.0
#define DEFAULT_SCALE_1 0.0
#define DEFAULT_SCALE_2 102.2727273
#define DEFAULT_SCALE_3 -128.4090909
#define DEFAULT_CAL_0 1.011949
#define DEFAULT_CAL_1 -0.094599

#define AVERAGE_TIME_SPAN 4000 // 4000=4sec; time in milliseconds after which averaging is restarted

#if DODEBUG
#define WRITEDEBUG(s) Serial.print(s)
#define WRITEDEBUGF(s, f) Serial.print(s, f)
#define WRITEDEBUGLN(s) Serial.println(s)
#define WRITEDEBUGLNF(s, f) Serial.println(s, f)
#else
#define WRITEDEBUG(s)
#define WRITEDEBUGF(s, f)
#define WRITEDEBUGLN(s)
#define WRITEDEBUGLNF(s, f)
#endif


#endif