/*
You need to install Adafruit SSD1306 library to work with OLED
See a guide here: https://arduinogetstarted.com/tutorials/arduino-oled
*/

/*
You may alsowant to look at the example code on how to work with pulse sensor:
https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int PulseSensorPurplePin = 0;        // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0

int Signal;                // holds the incoming raw data. Signal value can range from 0-1024

int x=0;                   // current position of the cursor
int y=0;
int lastx=0;                // last position of the cursor
int lasty=0;

void setup() {
  Serial.begin(9600);

  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 0);        // position to display
  oled.println("Hello World!"); // text to display
  oled.display();               // show on OLED
  delay(2000);
  oled.clearDisplay();        // clear display
}

void loop() {
  
  Signal = analogRead(PulseSensorPurplePin);  // Read the PulseSensor's value.
  Serial.println(Signal);                    // Send the Signal value to Serial Plotter.

  if(x>SCREEN_WIDTH - 1){                    // reset the screen when cursor reaches the border of the LED screen
      oled.clearDisplay();
      x=0;
      lastx=x;
  }

  //These values need to be determined from your signal dynamically
  int lower_bound = 0;                      //minimum signal values
  int upper_bound = 1000;                   //Maximum signal values

  float conversion = (upper_bound-lower_bound)/float(SCREEN_HEIGHT);  // a variable is needed fit signal range received by ADC into the screen height

  y = SCREEN_HEIGHT-((Signal-lower_bound)/conversion)-1;              // strength of the signal in the screen coordinates
  oled.writeLine(lastx,lasty,x,y,WHITE);                              // write a line between previous and the current cursor positions
  lasty=y;
  lastx=x;
  x++;
  oled.display();

  delay(10);
}

