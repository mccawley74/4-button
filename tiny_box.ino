//-----------------------------------------------------------------------------
//  Tiny Gaming LLC, Hanna Button Box Thingy
//  Copyright (C) 2022 Tiny Gaming LLC  - All Rights Reserved 
//    
//  This file is part of {Hanna Button Box Thingy}
//  Unauthorized copying of this file, via any medium is strictly
//  prohibited proprietary and confidential
//
//  {Tiny Gaming Hanna Button Box Thingy} can not be copied and/or distributed
//  without the express permission of {Tiny Gaming LLC}
//
//  Authors: McCawley Mark
//  Version 1.3.0
//
//  Functions:
//  --------------------------------------
//  void read_drake_buttons(int polltime)
//  void read_drake_switch(int polltime)
//  void press_keyboard_key(int polltime)
//  void read_drake_keypad(int polltime)
//  void set_profile(int polltime)
//  void oled_write(String text)
//  void setup()
//  void loop()
//
//             Arduino Pro Micro wiring diagram
//           ____________________________________
//
//                          +-----+
//                 +--------| USB |-------+
//                 |        +-----+       |
//                 | [1] TX         [RAW] |
//                 | [0] RX         [GND] |  Ground
//         Ground  | [GND]          [RST] |
//         Ground  | [GND]          [VCC] |  Voltage for OLED
//       OLED SDA  | [2] SDA      A3 [21] |  L.E.D. key 1 (green)
//       OLED SCL  | [3] SCL      A2 [20] |  L.E.D. key 2 (green)
//    Input Key-1  | [4] A6       A1 [19] |  L.E.D. key 3 (green)
//    Input Key-2  | [5]          A0 [18] |  L.E.D. key 4 (green)
//    Input Key-3  | [6] A7     SCLK [15] |  L.E.D. key 1 (red)
//    Input Key-4  | [7]        MISO [14] |  L.E.D. key 2 (red)
//     Cmd Button  | [8] A8     MOSI [16] |  L.E.D. key 3 (red)
//                 | [9] A9      A10 [10] |  L.E.D. key 4 (red)
//                 |                      |
//                 +----------------------+
//
//  NOTE:
//  1. Switch One and Switch Five is each half of a 2-way switch, center to ground
//  2. All switches are connected from the arduino GPIO to ground with Internal pullups.
//  3. L.E.D key backlighting uses one 330ohm resistor per three L.E.Ds
//
//  4. This version of code uses two OLED screens. One side mounted 128x32
//     And onea top mounted 128x64. However they are both set to 128x64 and on
//     the same I2C address 0x3C [duplicated screens]
//     The plan was to have seperate screens, but the OLEDs I have are not I2C
//     address modifyable, plus it came out looking just fine afer some formatting.
//  
//         I2C OLED Display
//         0.91 Inch SSD1306
//         OLED 128x32
//         Address: 0x3C
//          _______________________
//         | |                   |*|
//         | | Oled Menu 128x32  |*|
//         |_|___________________|*|

//
//-----------------------------------------------------------------------------

// Include header files
#include <Keyboard.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Bounce2.h>
#include <Arduino.h>
#include "menumap.h"

// Defines
#define HIGH 0x1
#define LOW  0x0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define LOGO_HEIGHT 16
#define OLED_RESET -1
#define LOGO_WIDTH 16
#define key_1 4
#define key_2 5
#define key_3 6
#define key_4 7
#define led_1_grn 21
#define led_2_grn 20
#define led_3_grn 19
#define led_4_grn 18
#define led_1_red 15
#define led_2_red 14
#define led_3_red 16
#define led_4_red 10
#define ctrl_btn 8

// Constant declarations
const byte ROWS = 3; // Matrix keypad three rows
const byte COLS = 3; // Matrix keypad three columns
const long LOOP_INTERVAL = 10;  // Polling rate for main loop
const long KEY_INTERVAL = 0; // Polling rate for LED brightness button
const unsigned long DISP_TIMEOUT = 300; // OLED refresh DISP_TIMEOUT

// Init variables other mix
int key_pressed;
int pin_array_grn [] {led_1_grn, led_2_grn, led_3_grn, led_4_grn,};
int pin_array_red [] {led_1_red, led_2_red, led_3_red, led_4_red};
unsigned long oled_cnt; // Counter for OLED text to timeout
bool key_update = false;
int cur_menu = 1;
String menu_name = "LANDING";

// Initialize an instance of Keypad and OLED screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Instantiate an instance of a Bounce object for corresponding buttons
Bounce b_key_1 = Bounce();
Bounce b_key_2 = Bounce();
Bounce b_key_3 = Bounce();
Bounce b_key_4 = Bounce();
Bounce b_ctrl_1 = Bounce();

Blinker key_1_blink(led_1_grn);
Blinker key_2_blink(led_2_grn);
Blinker key_3_blink(led_3_grn);
Blinker key_4_blink(led_4_grn);

// The menu item each key accesses
int keymnu_1 = 0;
int keymnu_2 = 1;
int keymnu_3 = 2;
int keymnu_4 = 3;

/*!
 * @function    check_for_keypress
 * @abstract    Poll for button change.
 * @discussion  Writes text to OLED, and presses key
 * @param       polling delay in milliseconds
 * @result      void
*/
void check_for_keypress(int polltime)
{
  b_key_1.update();
  b_key_2.update();
  b_key_3.update();
  b_key_4.update();

  if (b_key_1.fell()) {
    memcpy_P(&menuSRAM, &keyMenu[keymnu_1], sizeof(OLEDMENU));
    key_update = true;
    key_pressed = 1;
  }
  if (b_key_2.fell()) {
    memcpy_P(&menuSRAM, &keyMenu[keymnu_2], sizeof(OLEDMENU));
    key_update = true;
    key_pressed = 2;
  }
  if (b_key_3.fell()) {    
    memcpy_P(&menuSRAM, &keyMenu[keymnu_3], sizeof(OLEDMENU));
    key_update = true;
    key_pressed = 3;
  }
  if (b_key_4.fell()) {
    memcpy_P(&menuSRAM, &keyMenu[keymnu_4], sizeof(OLEDMENU));
    key_update = true;
    key_pressed = 4;
  }

  // If any of the keys were pressed, update the OLED screen
  if(key_update) {
    oled_write_text(menuSRAM.name1, menuSRAM.name2);
    press_keyboard_key(menuSRAM.keyId);
    set_key_led(key_pressed, LOW);
    key_update = false;
    delay(50);
  }
  
  delay(polltime);
}

/*!
 * @function    check_command_key
 * @abstract    Poll for button change.
 * @discussion  Writes text to OLED, and presses key
 * @param       polling delay in milliseconds
 * @result      void
*/
void check_command_key(int polltime)
{
  b_ctrl_1.update();

  if (b_ctrl_1.fell()) {
    set_menu();

    switch (cur_menu) {
      case 1:
        keymnu_1 = 0;
        keymnu_2 = 1;
        keymnu_3 = 2;
        keymnu_4 = 3;        
        break;
      case 2:
        keymnu_1 = 4;
        keymnu_2 = 5;
        keymnu_3 = 6;
        keymnu_4 = 7;
        break;
      case 3:
        keymnu_1 = 8;
        keymnu_2 = 9;
        keymnu_3 = 10;
        keymnu_4 = 11;
        break;
    }
  }
  delay(polltime);
}


/*!
 * @function    set_menu(int key, uint8_t state)
 * @abstract    Set the key L.E.D color.
 * @discussion  Switch mode. Switch one and five is a two way switch
 * @param       key switch one state
 * @param       state switch five state
 * @result      void
*/
void set_menu()
{
  cur_menu++;
  if(cur_menu == 4){cur_menu = 1;}
  switch (cur_menu) {
    case 1:
      menu_name = "LANDING";
      break;
    case 2:
      menu_name = "COMMS";
      break;
    case 3:
      menu_name = "POWER";
      break;
  }
  oled_write_text(menu_name, "MENU:" + String(cur_menu));
}

/*!
 * @function    set_key_led(int key, uint8_t state)
 * @abstract    Set the key L.E.D color.
 * @discussion  Switch mode. Switch one and five is a two way switch
 * @param       key switch one state
 * @param       state switch five state
 * @result      void
*/
void set_key_led(int key, uint8_t state)
{
  if(key == 0) { // Default state all red ON, all green OFF
    for (int i=0; i<sizeof pin_array_grn/sizeof pin_array_grn[0]; i++) {
      digitalWrite(pin_array_grn[i], HIGH);
      digitalWrite(pin_array_red[i], LOW);
    }
  } else if(key == 5) { // Default state all green ON, all red OFF
    for (int i=0; i<sizeof pin_array_grn/sizeof pin_array_grn[0]; i++) {
      digitalWrite(pin_array_grn[i], LOW);
      digitalWrite(pin_array_red[i], HIGH);
    }
  } else { // Individual state
    switch (key) {
      case 1:
      digitalWrite(led_1_grn, LOW);
      digitalWrite(led_1_red, HIGH);
      key_1_blink.blinkOn = true;
      break;
    case 2:
      digitalWrite(led_2_grn, LOW);
      digitalWrite(led_2_red, HIGH);
      key_2_blink.blinkOn = true;
      break;
    case 3:
      digitalWrite(led_3_grn, LOW);
      digitalWrite(led_3_red, HIGH);
      key_3_blink.blinkOn = true;
      break;
    case 4:
      digitalWrite(led_4_grn, LOW);
      digitalWrite(led_4_red, HIGH);
      key_4_blink.blinkOn = true;
      break;
    default:
      // ON - Green
      // OFF - RED
      // Blink Green
      break;
    }
  }
}

/*!
 * @function    oled_write(String text)
 * @abstract    Write test to the OLED screen
 * @discussion  Send a string of text to the OLED screen
 * @param       String text: the text to write to screen
 * @result      void
*/
void oled_write_text(String text1, String text2)
{
  int font_width = SCREEN_WIDTH / 11;
  int start;
  
  display.clearDisplay(); // Clear OLED
  display.setTextColor(WHITE);

  display.setTextSize(4); // Font size
  display.setCursor(0, 20);
  display.println(String(cur_menu));

  // First line of the display
  display.setTextSize(2); // Font size
  start = (font_width - text1.length()+3) * 5;
  display.setCursor(start, 10);
  display.println(text1);
  
  // Second line of the display
  start = (font_width - text2.length()+3) * 5;
  display.setCursor(start, 42);
  display.println(text2);

  display.display(); // Write to screen
  oled_cnt = 0; // Incr text timeout
}

/*!
 * @function    press_keyboard_key
 * @abstract    press a key
 * @discussion  press a key on the keyboard
 * @param       char key: The key to press
 * @result      void
*/
void press_keyboard_key(char key)
{
  Keyboard.press(key);
  delay(50);
  Keyboard.release(key);
  delay(50);
}

/*
 * @function    setup
 * @abstract    Inital setup method
 * @discussion  Define pin pullups and set variables
 * @param       
 * @result      none
*/
void setup()
{
  Serial.begin(9600);
  Keyboard.begin();

  // Serial.print("void setup()");
  // Serial.println("Inital setup method");

  // Set internal pullups on buttons
  pinMode(key_1, INPUT_PULLUP);
  pinMode(key_2, INPUT_PULLUP);
  pinMode(key_3, INPUT_PULLUP);
  pinMode(key_4, INPUT_PULLUP);

  // Set initial state
  set_key_led(0, HIGH);

  // Set pin mode
  for (int i=0; i<sizeof pin_array_grn/sizeof pin_array_grn[0]; i++) {
    pinMode(pin_array_grn[i], OUTPUT);
  }
  for (int i=0; i<sizeof pin_array_red/sizeof pin_array_red[0]; i++) {
    pinMode(pin_array_red[i], OUTPUT);
  }
  pinMode(ctrl_btn, INPUT_PULLUP);

  // Attach debouncers to buttons
  b_key_1.attach(key_1, INPUT_PULLUP);
  b_key_2.attach(key_2, INPUT_PULLUP);
  b_key_3.attach(key_3, INPUT_PULLUP);
  b_key_4.attach(key_4, INPUT_PULLUP);
  b_ctrl_1.attach(ctrl_btn, INPUT_PULLUP);

  // Set button debounce intervals
  b_key_1.interval(5);
  b_key_2.interval(5);
  b_key_3.interval(5);
  b_key_4.interval(5);
  b_ctrl_1.interval(5);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Setup initial OLED display
  oled_cnt = 0;
  display.display();
  display.clearDisplay();
  display.drawBitmap(0, 4, drakeLogo, 128, 64, WHITE);
  display.display();

  // Start eachj  L.E.D blink object
  key_1_blink.begin(500, 500);
  key_2_blink.begin(500, 500);
  key_3_blink.begin(500, 500);
  key_4_blink.begin(500, 500);
}

/*!
 * @function    loop
 * @abstract    Main program loop.
 * @discussion  Polls for changes in switch, pad,  and button state.
 * @param       
 * @result      none
*/
void loop()
{
  // Poll for key press, and command button press
  check_for_keypress(KEY_INTERVAL);
  check_command_key(KEY_INTERVAL);
  key_1_blink.check();
  key_2_blink.check();
  key_3_blink.check();
  key_4_blink.check();
  
  // Text on OLED will timeout and return to this message
  if (oled_cnt >= DISP_TIMEOUT) {
    //display.clearDisplay();
    //display.drawBitmap(0, 4, drakeLogo, 128, 64, WHITE);
    //display.display();
    set_key_led(0, HIGH);
    oled_cnt = 0;
    key_1_blink.blinkOn = false;
    key_2_blink.blinkOn = false;
    key_3_blink.blinkOn = false;
    key_4_blink.blinkOn = false;
    // delay(1000);
    // oled_write_text("Drake", "DRAKE");
  }
  ++oled_cnt; // Increment text timeout
  delay(LOOP_INTERVAL);
}
