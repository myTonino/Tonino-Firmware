// tonino_lcd.cpp
//----------------
// library for Tonino display control
//
// Basic parts of the LCD code has been adapted from
// https://github.com/adafruit/Adafruit-LED-Backpack-Library
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


#include <tonino_lcd.h>


// I2C address of the display
uint8_t LCD::i2cAddr;

// LCD driver needs 8*2 bytes
uint16_t LCD::displaybuffer[8]; 

// translates 0-F to bitmask for display
static const uint8_t numbertable[] = { 
   0x3F, /* 0 */
   0x06, /* 1 */
   0x5B, /* 2 */
   0x4F, /* 3 */
   0x66, /* 4 */
   0x6D, /* 5 */
   0x7D, /* 6 */
   0x07, /* 7 */
   0x7F, /* 8 */
   0x6F, /* 9 */
   0x77, /* a */
   0x7C, /* b */
   0x39, /* C */
   0x5E, /* d */
   0x79, /* E */
   0x71, /* F */
};

// path to walk one digit
static const uint8_t snaketable[] = { 
  0b00010000,
  0b00001000,
  0b00000100,
  0b01000000,
  0b00100000,
  0b00000001,
  0b00000010
};


LCD::LCD() {
  // empty
}

LCD::~LCD() {
  // empty
}

// initializes display using I2C start sequence
// sets max brightness, no blinking
void LCD::init(uint8_t addr) {
  i2cAddr = addr;

  Wire.begin();

  // turn on oscillator
  Wire.beginTransmission(i2cAddr);
  Wire.write(0x21);
  Wire.endTransmission();
  
  // display on, no blinking
  Wire.beginTransmission(i2cAddr);
  Wire.write(0x81); 
  Wire.endTransmission();

  // max brightness
  Wire.beginTransmission(i2cAddr);
  Wire.write(0xE0 | 15);
  Wire.endTransmission();
}


// write bitmask on digit d (0, 1, 3, 4)
inline void LCD::writeDigitRaw(uint8_t d, uint8_t bitmask) {
  if (d > 4) {
    error();
    return;
  }
  displaybuffer[d] = bitmask;
}
    
// write number num (0-F) on digit d (0, 1, 3, 4)
inline void LCD::writeDigitNum(uint8_t d, uint8_t num) {
  if (d > 4 || num > 15) {
    error();
    return;
  }
  writeDigitRaw(d, numbertable[num]);
}

// display a number n from -999 to 9999
void LCD::printNumber(int16_t n) {
  if (n < -999 || n > 9999) {
    error();
  }

  uint8_t numericDigits = 4;   // available digits on display
  int8_t displayPos = 4;
  boolean isNegative = false;  // true if the number is negative
  
  // is the number negative?
  if (n < 0) {
    isNegative = true;  // need to draw sign later
    --numericDigits;    // the sign will take up one digit
    n *= -1;            // pretend the number is positive
  }
  
  if (n == 0) {
    writeDigitNum(displayPos--, 0);
  } else {
    for (uint8_t i = 0; n; ++i) {
      writeDigitNum(displayPos--, n % 10);
      if (displayPos == 2) {
        displayPos--;
      }
      n /= 10;
    }
  }

  // display negative sign if negative
  if (isNegative) {
    writeDigitRaw(displayPos--, 0x40);
  }

  // clear remaining display positions
  while (displayPos >= 0) {
    writeDigitRaw(displayPos--, 0x00);
  }

  writeDisplay();
}

// sets the display brightness (0-15, 15=max brightness)
void LCD::setBrightness(uint8_t b) {
  if (b > 15) {
    return;
  }
  _brightness = b;
  Wire.beginTransmission(i2cAddr);
  Wire.write(0xE0 | b);
  Wire.endTransmission();
}

// get the display brightness (0-15, 15=max brightness)
uint8_t LCD::getBrightness() {
  return _brightness;
}

// sets the display to blink (0=no, 1=fast ... 3=slow)
void LCD::setBlinkRate(uint8_t rate) {
  if (rate > 3) {
    return;
  }
  Wire.beginTransmission(i2cAddr);
  Wire.write(0x80 | 0x01 | (rate << 1)); 
  Wire.endTransmission();
}

// draw a horizontal line
void LCD::line() {
  writeDigitRaw(0, 0b01000000); // -
  writeDigitRaw(1, 0b01000000); // -
  writeDigitRaw(3, 0b01000000); // -
  writeDigitRaw(4, 0b01000000); // -
  writeDisplay();
}  

// display 4 eights, i.e. light all segments
void LCD::eights() {
  writeDigitRaw(0, 0b01111111); // 8
  writeDigitRaw(1, 0b01111111); // 8
  writeDigitRaw(3, 0b01111111); // 8
  writeDigitRaw(4, 0b01111111); // 8
  writeDisplay();
}

// display 4 eights one after the other, takes 1 second
void LCD::eightSequence() {
  writeDigitRaw(0, 0b01111111); // 8
  writeDigitRaw(1, 0);
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
  delay(250);
  writeDigitRaw(0, 0);
  writeDigitRaw(1, 0b01111111); // 8
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
  delay(250);
  writeDigitRaw(0, 0);
  writeDigitRaw(1, 0);
  writeDigitRaw(3, 0b01111111); // 8
  writeDigitRaw(4, 0);
  writeDisplay();
  delay(250);
  writeDigitRaw(0, 0);
  writeDigitRaw(1, 0);
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0b01111111); // 8
  writeDisplay();
  delay(250);
}

// display the letters "dose"
void LCD::dose() {
  writeDigitRaw(0, 0b1011110); // numbertable[13], d
  writeDigitRaw(1, 0b0111111); // numbertable[0],  O
  writeDigitRaw(3, 0b1101101); // numbertable[5],  S
  writeDigitRaw(4, 0b1111001); // numbertable[14], E
  writeDisplay();
}

// display the letters "hi"
void LCD::hi() {
  writeDigitRaw(0, 0b01110110); //     h
  writeDigitRaw(1, numbertable[1]); // i
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
}

// display the letters "up"
void LCD::up() {
  writeDigitRaw(0, 0b00111110); // U
  writeDigitRaw(1, 0b01110011); // P
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
}

// display the letters "PC"
void LCD::connected() {
  writeDigitRaw(0, 0b01110011); // P
  writeDigitRaw(1, 0b00111001); // numbertable[12], C
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
}

// display the letters "EEEE"
void LCD::error() {
  writeDigitRaw(0, 0b01111001); // numbertable[14], E
  writeDigitRaw(1, 0b01111001); // numbertable[14], E
  writeDigitRaw(3, 0b01111001); // numbertable[14], E
  writeDigitRaw(4, 0b01111001); // numbertable[14], E
  writeDisplay();
}

// clears the display (all segments to dark)
void LCD::clear() {
  writeDigitRaw(0, 0);
  writeDigitRaw(1, 0);
  writeDigitRaw(3, 0);
  writeDigitRaw(4, 0);
  writeDisplay();
}

// light up the dot of the first digit
void LCD::averaged(boolean dot) {
  writeDigitRaw(0, (displaybuffer[0] & 0b01111111) | (dot << 7));
  writeDisplay();
}

// shows a rotating circle; total time =repeat*timePerCircle
void LCD::circle(uint8_t repeat, uint16_t timePerCircle) {
  int16_t dtime = timePerCircle / 12;

  for (int i = 0; i < repeat; ++i) {
    writeDigitRaw(0, 0b1);
    writeDisplay();
    delay(dtime);
    
    writeDigitRaw(0, 0b0);
    writeDigitRaw(1, 0b1);
    writeDisplay();
    delay(dtime);
  
    writeDigitRaw(1, 0b0);
    writeDigitRaw(3, 0b1);
    writeDisplay();
    delay(dtime);
  
    writeDigitRaw(3, 0b0);
    writeDigitRaw(4, 0b1);
    writeDisplay();
    delay(dtime);
    writeDigitRaw(4, 0b10);
    writeDisplay();
    delay(dtime);
    writeDigitRaw(4, 0b100);
    writeDisplay();
    delay(dtime);
    writeDigitRaw(4, 0b1000);
    writeDisplay();
    delay(dtime);
  
    writeDigitRaw(4, 0b0);
    writeDigitRaw(3, 0b1000);
    writeDisplay();
    delay(dtime);
  
    writeDigitRaw(3, 0b0);
    writeDigitRaw(1, 0b1000);
    writeDisplay();
    delay(dtime);
  
    writeDigitRaw(1, 0b0);
    writeDigitRaw(0, 0b1000);
    writeDisplay();
    delay(dtime);
    writeDigitRaw(0, 0b10000);
    writeDisplay();
    delay(dtime);
    writeDigitRaw(0, 0b100000);
    writeDisplay();
    delay(dtime);
  }
}

// displays a number; effect: each digit sequentially counts from 0
void LCD::dropNumber(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }
  uint8_t dig[5];
  uint8_t startLed = 0;
  
  dig[0] = num / 1000;
  num %= 1000;
  dig[1] = num / 100;
  num %= 100;
  dig[3] = num / 10;
  dig[4] = num % 10;
  if (dig[0] == 0) {
    startLed++;
    if (dig[1] == 0) {
      startLed += 2;
      if (dig[3] == 0) {
        startLed++;
      }
    }
  }

  clear();
  
  for (int led = startLed; led < 5; ++led) {
    if (led == 2) {
      continue;
    }
    uint8_t cnt = 0;
    while (cnt <= dig[led]) {
      writeDigitNum(led, cnt++);
      writeDisplay();
      delay(50);
    }
  }
}

// displays a number; effect: quickly count from 0 to num
void LCD::countToNumber(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }
  for (int16_t i = 0; i <= num; ++i) {
    printNumber(i);
    delay(5);
  }
}

// displays a number; effect: quickly count down from 888
void LCD::countDownToNumber(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }
  if (num > 888) {
    if (num > 8888) {
      for (int16_t i = 9999; i >= num; --i) {
        printNumber(i);
      }
    } else {
      for (int16_t i = 8888; i >= num; --i) {
        printNumber(i);
      }
    }
  } else {
    for (int16_t i = 888; i >= num; --i) {
      printNumber(i);
    }
  }
}

// displays a number; effect: start with 8888, remove all segments until num appears
void LCD::numberFromEights(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }

  int16_t dtime = 100;
  uint8_t dig[5];
  uint8_t eight[5];

  for (int led = 0; led < 6; led++) {
    eight[led] = 0b01111111;
  }
  
  dig[0] = numbertable[num / 1000];
  num %= 1000;
  dig[1] = numbertable[num / 100];
  num %= 100;
  dig[3] = numbertable[num / 10];
  dig[4] = numbertable[num % 10];
  if (dig[0] == numbertable[0]) {
    dig[0] = 0;
    if (dig[1] == numbertable[0]) {
      dig[1] = 0;
      if (dig[3] == numbertable[0]) {
        dig[3] = 0;
      }
  }
  }
  
  eights();
  
  for (int i = 0; i < 7; i++) {
    delay(dtime);
    for (int led = 0; led < 5; led++) {
      if (led == 2) {
        continue;
      }
      if ((dig[led] >> i) % 2 == 0) {
        eight[led] -= (1 << i);
        writeDigitRaw(led, eight[led]);
        writeDisplay();
      }
    }
  }
}

// displays a number; effect: start empty, add all segments until num appears
void LCD::numberFromEmpty(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }
  int16_t dtime = 100;
  uint8_t dig[5];
  uint8_t eight[5];

  for (int led = 0; led < 6; led++) {
    eight[led] = 0b0;
  }
  
  dig[0] = numbertable[num / 1000];
  num %= 1000;
  dig[1] = numbertable[num / 100];
  num %= 100;
  dig[3] = numbertable[num / 10];
  dig[4] = numbertable[num % 10];
  if (dig[0] == numbertable[0]) {
    dig[0] = 0;
    if (dig[1] == numbertable[0]) {
      dig[1] = 0;
      if (dig[3] == numbertable[0]) {
        dig[3] = 0;
      }
    }
  }

  clear();
  
  for (int i = 0; i < 7; i++) {
    delay(dtime);
    for (int led = 0; led < 5; led++) {
      if (led == 2) {
        continue;
      }
      if ((dig[led] >> i) % 2 == 1) {
        eight[led] += (1 << i);
        writeDigitRaw(led, eight[led]);
        writeDisplay();
      }
    }
  }
}

// displays a number; effect: simulate oscillating value until final value num
void LCD::approx(uint16_t num) {
  if (num > 9999) {
    error();
    return;
  }
  uint8_t dtime = 20;
  for (int8_t i = 10; i > -8; --i) {
    printNumber(num + i);
    delay(dtime);
  }
  for (int8_t i = -4; i < 4; ++i) {
    printNumber(num + i);
    delay(2*dtime);
  }
  for (int8_t i = 2; i > -1; --i) {
    printNumber(num + i);
    delay(3*dtime);
  }
}

// displays a number; effect: like a snake leaving behind the num
void LCD::snake(uint16_t num) {
  uint8_t dig[5];
  int16_t dtime = 10;
  uint8_t curDig = 0;

  calcDigits(dig, num);
  clear();
  
  for (int led = 0; led < 5; ++led) {
    if (led == 2) {
      continue;
    }
  
    if (led == 0) {
      writeDigitRaw(led, snaketable[0]);
      writeDisplay();
      delay(dtime);
    } else {
      // correct last digit element from previous led
      int prevLed = (led == 3) ? 1 : led -1;
      curDig |= (dig[prevLed] & snaketable[6]);
      writeDigitRaw(prevLed, curDig);
      writeDigitRaw(led, snaketable[0]);
      writeDisplay();
      delay(dtime);
      curDig = 0;
    }
    for (int digit = 1; digit < 7; ++digit) {
      // add last element if dig has it
      curDig |= (dig[led] & snaketable[digit-1]);
      writeDigitRaw(led, snaketable[digit] | curDig);
      writeDisplay();
      delay(dtime);
    }
  }
  // correct last digit element from last led
  curDig |= (dig[4] & snaketable[6]);
  writeDigitRaw(4, curDig | 1<<7);
  writeDisplay();
}

// shows 'CAL' and, if circle=true a small one time rotating circle in the rightmost digit
// the circle takes roughly 600ms
void LCD::calibration(bool circle) {
  uint8_t dtime = 100;
  writeDigitRaw(0, numbertable[12]); // C
  writeDigitRaw(1, numbertable[10]); // A
  writeDigitRaw(3, 0b00111000);      // L
  writeDisplay();
  if (circle) {
    for (int i = 0; i < 6; ++i) {
      writeDigitRaw(4, 1 << i);
      writeDisplay();
      delay(dtime);
    }
  }
}

// shows 'CAL1'
void LCD::calibration1() {
  writeDigitRaw(0, numbertable[12]); // C
  writeDigitRaw(1, numbertable[10]); // A
  writeDigitRaw(3, 0b00111000);      // L
  writeDigitRaw(4, numbertable[1]);  // 1
  writeDisplay();
}

// shows 'CAL2'
void LCD::calibration2() {
  writeDigitRaw(0, numbertable[12]); // C
  writeDigitRaw(1, numbertable[10]); // A
  writeDigitRaw(3, 0b00111000);      // L
  writeDigitRaw(4, numbertable[2]);  // 2
  writeDisplay();
}

// shows 'done'
void LCD::done() {
  writeDigitRaw(0, numbertable[13]); // d
  writeDigitRaw(1, numbertable[0]);  // O
  writeDigitRaw(3, 0b01010100);      // n
  writeDigitRaw(4, numbertable[14]); // E
  writeDisplay();
}

// if digit<0 shows a '-' moving from left to right with given delays
// if digit in {0,1,3,4}, displays a '-' at that digit
void LCD::lineAnim(int8_t digit, uint16_t dtime) {
  if (digit > 4 || digit == 2) {
    return;
  } else {
    writeDigitRaw(0, 0);
    writeDigitRaw(1, 0);
    writeDigitRaw(3, 0);
    writeDigitRaw(4, 0);
    if (digit < 0) {
      // loop once
      writeDigitRaw(0, 0b01000000);
      writeDisplay();
      delay(dtime);
      for (int led = 1; led < 5; ++led) {
        if (led == 2) {
          continue;
        }
        writeDigitRaw(led == 3 ? 1 : (led-1), 0);
        writeDigitRaw(led, 0b01000000);
        writeDisplay();
        delay(dtime);
      }
    } else {
      writeDigitRaw(digit, 0b01000000);
      writeDisplay();
    }
  }
}

// writes software display buffer to physical display
void LCD::writeDisplay(void) {
  Wire.beginTransmission(i2cAddr);
  Wire.write((uint8_t)0x00);

  for (uint8_t i = 0; i < 8; ++i) {
    Wire.write(displaybuffer[i] & 0xFF);    
    Wire.write(displaybuffer[i] >> 8);    
  }
  Wire.endTransmission();
}

// helper to remove leading zeros
void LCD::calcDigits(uint8_t* dig, int16_t num) {
  dig[0] = numbertable[num / 1000];
  num %= 1000;
  dig[1] = numbertable[num / 100];
  num %= 100;
  dig[3] = numbertable[num / 10];
  dig[4] = numbertable[num % 10];
  if (dig[0] == numbertable[0]) {
    dig[0] = 0;
    if (dig[1] == numbertable[0]) {
      dig[1] = 0;
      if (dig[3] == numbertable[0]) {
        dig[3] = 0;
      }
    }
  }
}
