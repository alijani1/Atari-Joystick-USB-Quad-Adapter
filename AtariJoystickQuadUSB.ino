
//-------------------------------------------------------------------- 
//
// Atari USB Joystick Adapter with 4 joysticks support
// Code Developed by Ali Jani @ iCode
//
//--------------------------------------------------------------------
/*
add to boards.txt for USB name

leonardo.build.vid=0x8282
leonardo.build.pid=0x3201
leonardo.build.usb_product="Retro Joystick Adapter"
*/

#define UP   0
#define DOWN   1
#define LEFT   2
#define RIGHT   3
#define FIRE   4
#define REWIND   5
#define SELECT   6
#define START   7
#define MENU   8
#define NONE   99

#include <Joystick.h>
#include <Wire.h>

#define JOYSTICK_COUNT 4

Joystick_ Joystick[JOYSTICK_COUNT] = {
  Joystick_(0x03, JOYSTICK_TYPE_GAMEPAD,
  5, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false),  // No accelerator, brake, or steering
  Joystick_(0x04, JOYSTICK_TYPE_GAMEPAD,
  5, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false),  // No accelerator, brake, or steering
  Joystick_(0x05, JOYSTICK_TYPE_GAMEPAD,
  5, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false),  // No accelerator, brake, or steering
  Joystick_(0x06, JOYSTICK_TYPE_GAMEPAD,
  5, 0,                  // Button Count, Hat Switch Count
  true, true, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false)  // No accelerator, brake, or steering
};

// Last state of the buttons
int lastButtonState[JOYSTICK_COUNT][5] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
int cButtonsState[5] = {0, 0, 0, 0, 0};

// pins order {UP,Down,Left,Right,Fire} - for direct ports only. stick 3 and 4 use I2C so see code further below.
// ver1
// int LeoPins[JOYSTICK_COUNT][5] {{14, 16, 4, 5, 6}, {7, 8, 9, 10, 15}, {0, 0, 0, 0, 0 }, {0, 0, 0, 0, 0 }};
// ver2
int LeoPins[JOYSTICK_COUNT][5] {{19, 20, 4, 5, 6}, {7, 8, 9, 10, 15}, {0, 0, 0, 0, 0 }, {0, 0, 0, 0, 0 }};

int SpecialButtonState = NONE;

bool changed = false;

// Set I2C address
int address = 0x20;
uint16_t input;

void setup() {
//  Serial.begin(115200);
//  while (!Serial) continue;
  
  Wire.begin();
  // Set all ports as input
  pf575_write(word(B00000000,B00000000));

  pf575_write(0xFFFF);  // set default to High
      
  // Initialize input Pins for first 2 joysticks
  for (int j=0; j< 2; j++) {
    for (int p=0; p<5; p++) {
      pinMode( LeoPins[j][p], INPUT_PULLUP);
    }
  }

  // Initialize Joystick Library
  for (int index = 0; index < JOYSTICK_COUNT; index++)
  {
    Joystick[index].begin();
    Joystick[index].setXAxisRange(-1, 1);
    Joystick[index].setYAxisRange(-1, 1);
  }
}

void pf575_write(uint16_t data)
{
  Wire.beginTransmission(address);
  Wire.write(lowByte(data));
  Wire.write(highByte(data));
  Wire.endTransmission();
}

uint8_t error;
uint8_t hi,lo;

uint16_t pcf8575_read(){
  Wire.beginTransmission(address);
  error = Wire.endTransmission();

  if(error == 0){ //okay!
    Wire.requestFrom(address,2); 
    if(Wire.available()){
      lo = Wire.read();
      hi = Wire.read();
      return(word(hi,lo)); //joing bytes 
    }
    else{//error
      //do something here if you like  
    }
  }
  else{ //error
    //do something here if you like  
  }
}


void loop() {
  input = pcf8575_read(); 
//  Serial.print("Read, ");
//  Serial.println(input,BIN); //~ inverses the results, this removes the ambiguity of leading zero 

  for (int js = 0; js < JOYSTICK_COUNT; js++)
  {
    changed = false;

    if (js<2) { // normal leonardo ports
      for (int index = 0; index < 5; index++)
      {
        cButtonsState[index] = !digitalRead(LeoPins[js][index]);
        if  (cButtonsState[index] != lastButtonState[js][index]) {
          changed = true;
        }
        lastButtonState[js][index] = cButtonsState[index];
      }
    } else { // expanded PCF8575 ports
      input = ~(pcf8575_read());
      if (js == 2) {  // 3rd joystick port
        cButtonsState[4] = (input & word(B00000000,B00010000)) >> 4;
        cButtonsState[3] = (input & word(B00000000,B00001000)) >> 3;
        cButtonsState[2] = (input & word(B00000000,B00000100)) >> 2;
        cButtonsState[1] = (input & word(B00000000,B00000010)) >> 1;
        cButtonsState[0] = (input & word(B00000000,B00000001)) ;
        /*
        cButtonsState[0] = (input & word(B00000000,B01000000)) >> 6;
        cButtonsState[1] = (input & word(B00000000,B00100000)) >> 5;
        cButtonsState[2] = (input & word(B00000000,B00010000)) >> 4;
        cButtonsState[3] = (input & word(B00000000,B00001000)) >> 3;
        cButtonsState[4] = (input & word(B00000000,B00000100)) >> 2;
        */
      } else {  // 4th joystick port
        cButtonsState[4] = ((input & word(B01000000,B00000000)) >> 6) / 256;
        cButtonsState[3] = ((input & word(B00100000,B00000000)) >> 5) / 256;
        cButtonsState[2] = ((input & word(B00010000,B00000000)) >> 4) / 256;
        cButtonsState[1] = ((input & word(B00001000,B00000000)) >> 3) / 256;
        cButtonsState[0] = ((input & word(B00000100,B00000000)) >> 2)  / 256;
        /*
        cButtonsState[0] = ((input & word(B00000001,B00000000)) >> 0) / 256;
        cButtonsState[1] = ((input & word(B00000100,B00000000)) >> 2) / 256;
        cButtonsState[2] = ((input & word(B00001000,B00000000)) >> 3) / 256;
        cButtonsState[3] = ((input & word(B00010000,B00000000)) >> 4) / 256;
        cButtonsState[4] = ((input & word(B00000010,B00000000)) >> 1)  / 256;
        */
      }

      for (int index = 0; index < 5; index++)
      {
        if  (cButtonsState[index] != lastButtonState[js][index]) {
          changed = true;
        }
        lastButtonState[js][index] = cButtonsState[index];
      }
    }

    if (changed) {
      /* Handle special buttons on 5 button joysticks
         REWIND = LEFT + RIGHT + UP
         SELECT = LEFT + RIGHT
         START = UP + DOWN + LEFT
         MENU = UP + DOWN
      */

      SpecialButtonState = NONE;
      if ((cButtonsState[LEFT] == 1) && (cButtonsState[RIGHT] == 1) && (cButtonsState[UP] == 1)) {
        SpecialButtonState = REWIND;
      } else if ((cButtonsState[LEFT] == 1) && (cButtonsState[RIGHT] == 1) ) {
        SpecialButtonState = SELECT;
      } else if ((cButtonsState[UP] == 1) && (cButtonsState[DOWN] == 1) && (cButtonsState[LEFT] == 1)) {
        SpecialButtonState = START;
      } else if ((cButtonsState[UP] == 1) && (cButtonsState[DOWN] == 1)) {
        SpecialButtonState = MENU;
      }

      Joystick[js].setButton(4, (SpecialButtonState == REWIND) ? 1 : 0);
      Joystick[js].setButton(2, (SpecialButtonState == SELECT) ? 1 : 0);
      Joystick[js].setButton(3, (SpecialButtonState == START) ? 1 : 0);
      Joystick[js].setButton(1, (SpecialButtonState == MENU) ? 1 : 0);

      // handle directions and Fire
      if (SpecialButtonState == NONE) {
        if (cButtonsState[UP] == 1) {
          Joystick[js].setYAxis(-1);
        } else if (cButtonsState[DOWN] == 1) {
          Joystick[js].setYAxis(1);
        } else {
          Joystick[js].setYAxis(0);
        }

        if (cButtonsState[LEFT] == 1) {
          Joystick[js].setXAxis(-1);
        } else if (cButtonsState[RIGHT] == 1) {
          Joystick[js].setXAxis(1);
        } else {
          Joystick[js].setXAxis(0);
        }

        if (cButtonsState[FIRE] == 1) {
          Joystick[js].setButton(0, 1);
        } else {
          Joystick[js].setButton(0, 0);
        }
      }
    }
    delay(5);
  }
}