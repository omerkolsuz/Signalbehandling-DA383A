/*
You need to install Adafruit SSD1306 library to work with OLED
See a guide here: https://arduinogetstarted.com/tutorials/arduino-oled
*/

/*
You may alsowant to look at the example code on how to work with pulse sensor:
https://pulsesensor.com/pages/installing-our-playground-for-pulsesensor-arduino
*/

#include <Wire.h> // biblioteket för I2C kommunikaiton
#include <Adafruit_GFX.h> // grafikbibliotek för OLED displayen
#include <Adafruit_SSD1306.h> // bibliotek för att styra SSD1306 OLED displayen
#include "ESP32TimerInterrupt.h"// bibliotek för att använda timer interrupt på ESP32

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define I2C_SDA 21 // I2C data pin
#define I2C_SCL 22 // I2C klock pin

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int PulseSensorPin = 32; // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 32


// Thresholds for beat detection
const int upper_bound = 3400; // övre gräns från plottan
const int lower_bound = 2700; // nedre gräns från plottan

int Signal = 0; // holds the incoming raw data. Signal value can range from 0-1024
unsigned long lastBeatTime = 0; // tidspunk för det senaste upptäckta hjärtslaget
boolean beatStarted = false; // flagga för om dett hjärtslag har startat

int BPM = 0; // hjärtfrekven i slag per minut

// Variables for signal normalization
long signalAccumulator = 0; // ackumulerar signalvärdet för att beräkna medelvärde 
int signalCount = 0; // antal insamlade signalvärden
int signalOffset = 0; // beräknat medelvärde av signalen för normalisering
const int readingsToAverage = 300; // antal läsningar som ska medelvärdes beräknas
// funktionen som körs varje gång timern når sitt intervall
bool IRAM_ATTR TimerHandler0(void * timerNo)
{
    Signal = analogRead(PulseSensorPin) - signalOffset; // läser av signalen från pulssensorn
    return true; // berättar att allt gick bra

}
// hur ofta timern ska. Utlösa ett avbrott i 1 ms.
#define TIMER0_INTERVAL_MS        1
ESP32Timer ITimer0(0); // skapar en timer med namnet ITimer0 för ESP32

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);

// initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Infinite loop
  }

  oled.clearDisplay(); // clear display
  oled.setTextSize(1); // text size
  oled.setTextColor(WHITE); // text color
  oled.setCursor(0, 0); // position to display
  oled.println("BPM: Initializing..."); // text to display
  oled.display(); // show on OLED
  delay(2000); // wait for initializing

  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0)) 
{
    // om timern lyckas att starta, skriver ut meddelande med den aktuella tiden	
    Serial.print(F("Starting ITimer0 OK, millis() = "));
    Serial.println(millis());
  } else // om timern inte kan starta, skriver ut ett felmeddelande
{
    Serial.println(F("Can't set ITimer0. Select another Timer, freq. or timer"));
  }
}

void loop() {
  // kontrollerar om antalet insamlade signaler är mindre än det önskade antalet
   if (signalCount < readingsToAverage) 

{
    signalAccumulator += Signal; // lägger till den aktuella signalen till en ackumulator
    signalCount++; // ökar antalet insamlade signaler med ett
  } else // när tillräckligt många signaler samlats, beräknar medelvärdet
{ 
    signalOffset = signalAccumulator / readingsToAverage; // beräknar medelvärdet
    signalAccumulator = 0; // återställer ackumulatorn för nästa omgång av signalinsamling

    signalCount = 0; // återställer räknaren
  }

 // om signalen är högre än övre gräns och ett slags inte redan startat 
  if (Signal > upper_bound && !beatStarted) {
    beatStarted = true; // markera att ett slag har börjat
    unsigned long currentTime = millis(); // spara nuvarande tid i millisekunder
    // om det är inte första slaget
    if (lastBeatTime > 0) {
      unsigned long beatInterval = currentTime - lastBeatTime; // beräknar tid mellan slag
      BPM = 60000 / beatInterval; // räknar ut BPM
      displayBPM(BPM); // Display BPM on the OLED
    }
 lastBeatTime = currentTime; // spara tid för senaste slaget
  } 
// om signalen är under den undre gränsen och ett slag har redan startat
else if (Signal < lower_bound && beatStarted) 
{
    beatStarted = false; // markera att slaget är slut
  }

  // Limit the refresh rate
  delay(10);
}

void displayBPM(int bpm) {
  oled.clearDisplay(); // Clear the display to update it with new BPM
  oled.setTextSize(2); // Normal 1:1 pixel scale
  oled.setTextColor(WHITE); // Draw white text
  oled.setCursor(0,0); // Start at top-left corner
  oled.print("BPM: "); // text to display
  oled.println(bpm); // show on OLED
  oled.display(); // show on OLED
}

