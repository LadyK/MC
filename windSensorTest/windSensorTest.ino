/*
 * Test Wind Sensor from Modern Device
 * Sensor code from Moder Device: https://github.com/moderndevice/Wind_Sensor
 * 
 * 
 */

#include <Adafruit_GFX.h>

//#include <StackArray.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>
#include <SPI.h>
#include <SD.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literalr
#endif

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 4

#define analogPinForRV    1   // change to pins you the analog pins are using
#define analogPinForTMP   0
#define analogLED         5

const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa. // was.2

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;
float Wind_speed;
float wind;
boolean breathe;
//unsigned long lastShow = 0;
unsigned long previousMillis = 0;
unsigned long previousErase = 0;
long interval = 1000;
unsigned long currentMillis;


void setup() {

  Serial.begin(57600);   // faster printing to get a bit better throughput on extended info
  // remember to change your serial monitor

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(SD_CS)) {
    Serial.println(F("failed!"));
    return;
  }
  Serial.println(F("OK!"));


  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);        // GND pin
  pinMode(A3, INPUT);        // VCC pin
  pinMode(A5, OUTPUT);
  digitalWrite(A3, LOW);     // turn off pullups


  delay(1000);
  Serial.println(F("start"));
  //Serial.println(hSize4);
  Serial.println();
  //tft.fillScreen(ILI9340_BLACK);
  randomSeed(analogRead(A5));

  lastMillis = 0;

}

void loop() {
  delay(50);
  float s = checkWind();
  //Serial.println(s);

}


float checkWind() {
  if (millis() - lastMillis > 200) {     // read every 200 ms - printing slows this down further
    TMP_Therm_ADunits = analogRead(analogPinForTMP);
    RV_Wind_ADunits = analogRead(analogPinForRV);
    RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);

    // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
    // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
    TempCtimes100 = (0.005 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;

    zeroWind_ADunits = -0.0006 * ((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172; //  13.0C  553  482.39

    zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;

    // This from a regression from data in the form of
    // Vraw = V0 + b * WindSpeed ^ c
    // V0 is zero wind at a particular temperature
    // The constants b and c were determined by some Excel wrangling with the solver.

    WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) / .2300) , 2.7265);
    /*
      Serial.print("  TMP volts ");
      Serial.print(TMP_Therm_ADunits * 0.0048828125);

      Serial.print(" RV volts ");
      Serial.print((float)RV_Wind_Volts);

      Serial.print("\t  TempC*100 ");
      Serial.print(TempCtimes100 );

      Serial.print("   ZeroWind volts ");
      Serial.print(zeroWind_volts);
*/
      Serial.print("   WindSpeed MPH ");
      Serial.println((float)WindSpeed_MPH);
   lastMillis = millis(); 

   // WindSpeed_MPH = WindSpeed_MPH * 100.0;
    //Serial.print("   WindSpeed MPH_upped ");
    //Serial.println((float)WindSpeed_MPH);


    // map the light to an LED
    /*
      if ((float)WindSpeed_MPH > 0.01) {

        int brightness = float(map(WindSpeed_MPH, 0, 500.0, 0, 1023));
        analogWrite(analogLED, brightness);
      }
    */
    return WindSpeed_MPH;
  }


}
