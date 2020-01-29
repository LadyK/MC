/*
   Test Wind Sensor from Modern Device
   Sensor code from Moder Device: https://github.com/moderndevice/Wind_Sensor

   TFT Shield code built from Adafruit's example: https://learn.adafruit.com/adafruit-2-8-tft-touch-shield-v2/drawing-bitmaps

*/
//#include <StackArray.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions


// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

//#if defined(__SAM3X8E__)
//#undef __FlashStringHelper::F(string_literal)
//#define F(string_literal) string_literalr
//#endif

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  using
// 'select' pins for each to identify the active device on the bus. For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

//#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 4

#define analogPinForRV    1   // change to pins you the analog pins are using
#define analogPinForTMP   0
#define analogLED         5

#if defined(USE_SD_CARD)
SdFat                SD;         // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
// SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
#if defined(__SAMD51__) || defined(NRF52840_XXAA)
Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
    PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
#else
#if (SPI_INTERFACES_COUNT == 1)
Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
#else
Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
#endif
#endif
Adafruit_SPIFlash    flash(&flashTransport);
FatFileSystem        filesys;
Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif


Adafruit_ILI9341     tft    = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;

/* When adding images:
  - images must be 240 x 320 bmp for windows with titles less than 7 characters
  - add string title to history array
  - increase these variable start values by number of images: storageH + numImages_hist
*/


/* When adding images:
  - images must be 240 x 320 bmp for windows with titles less than 7 characters
  - add string title to history array
  - increase these variable start values by number of images: storageH + numImages_hist
*/

// need to save space. renaming files; then using for loop indice concatenated with ".bmp"
// black.bmp = 0.bmp

byte black = 0;

/*
  char* images[] = {"black.bmp", "asburyB.bmp", "vancov.bmp", "bench.bmp", "AlexBDay.bmp", "caton.bmp", "lvrmBlu.bmp", "sunset2.bmp", "street2.bmp",
                 9 "street.bmp", "swiss.bmp", "tobys.bmp", "trainR.bmp",  "westS2.bmp", "wintree.bmp", "friebG.bmp",
                  "black2.bmp", "karok.bmp",  "nightSt.bmp",  "subway.bmp", "kayak.bmp", "blue.bmp"
                };
*/

// if we repeat 0 slot twice, will actually show image
/*
  byte natural[] = {9, 10, 16, 14, 15, 18};
  byte bday[] = {4, 5, 19, 11, 3, 8, 17, 6};
  byte dark[] = {6, 2, 20, 5, 11, 3, 15, 18};
  byte dark2[] = {6, 7, 2, 1, 20, 21, 3, 9, 13, 9};
  byte water[] = {16, 10, 2, 1, 20, 21, 7, 3};
  byte winter[] = {12, 14, 9, 11};
  byte blank[] = { 7, 0, 0, 13, 0, 14, 0, 14, 14};
  { natural, bday, dark, dark2, water, winter, blank};
*/
byte narrLimit = 10;
byte narratives[7][10] = {
  {9, 10, 16, 14, 15, 18, 0, 0, 0, 0},
  {4, 5, 19, 11, 3, 8, 17, 6, 0, 0},
  {6, 2, 20, 5, 11, 3, 15, 18, 0, 0},
  {6, 7, 2, 1, 20, 21, 3, 9, 13, 9},
  {16, 10, 2, 1, 20, 21, 7, 3, 0, 0},
  {12, 14, 9, 11, 0, 0, 0, 0, 0, 0},
  {7, 0, 0, 13, 0, 14, 0, 14, 14, 0}
};

int numImages_hist = 50;   //actual number of images to show

const float zeroWindAdjustment =  .2; // negative numbers yield smaller wind speeds and vice versa. // was.2

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;
float wind_speed;
float wind;
boolean breathe;
//unsigned long lastShow = 0;
unsigned long previousMillis = 0;
unsigned long previousErase = 0;
long interval = 1000;
unsigned long currentMillis;
unsigned long lastMillis2;

unsigned long previousMillis_m = 0;
int delays[3];

//store the random selections so not as to repeat any selections
int storageH[51];   //  // need to store zero slot, but will never use that image slot
int prob_narr = 200; //= 200;
int hist = 800;
int picD = 150;
int picH = 4000;
int image;
long r;
unsigned long previousMillis_m2;
int capacity = 0;
int chooser = 0;
char filename[6];

//String whichNarr[] = { "natural", "bday", "dark", "dark2", "water", "winter", "blank"};
int a = 0;
int wSize2 = 240 / 2;
int hSize2 = 320 / 2;
int wSize3 = 240 / 3;
int hSize3 = 320 / 3;
int wSize4 = 240 / 4;
int hSize4 = 240 / 4;

void setup() {

  ImageReturnCode stat; // Status from image-reading functions

  Serial.begin(57600);   // faster printing to get a bit better throughput on extended info
  // remember to change your serial monitor
#if !defined(ESP32)
  while (!Serial);      // Wait for Serial Monitor before continuing
#endif

  tft.begin();          // Initialize screen

  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatFileSystem object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if (!SD.begin(SD_CS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
    Serial.println(F("SD begin() failed"));
    for (;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if (!flash.begin()) {
    Serial.println(F("flash begin() failed"));
    for (;;);
  }
  if (!filesys.begin(&flash)) {
    Serial.println(F("filesys begin() failed"));
    for (;;);
  }
#endif
  Serial.println(F("OK!"));


  tft.fillScreen(ILI9341_BLUE);

  //   Uncomment the three lines below to reset the analog pins A2 & A3
  //   This is code from the Modern Device temp sensor (not required)
  pinMode(A2, INPUT);        // GND pin
  pinMode(A3, INPUT);        // VCC pin
  pinMode(A5, OUTPUT);
  digitalWrite(A3, LOW);     // turn off pullups


  delay(1000);
  Serial.println(F("start"));
  Serial.println(hSize4);
  Serial.println();
  tft.fillScreen(ILI9341_BLACK);
  randomSeed(analogRead(A5));

  lastMillis = 0;

  //stat = reader.drawBMP("asburyB.bmp", tft, 0, 0);
  //reader.printStatus(stat); // How'd we do?
  //reader.drawBMP("asburyB.bmp", tft, 0, 0);

  Serial.println(randImgfile() );

}

void loop() {
  delay(00); // slow everything down a tad for testing
  analogWrite(analogLED, 0);

  /*  this is nice with the layering and leaving the bckgnd
    tft.setRotation(random(0, 3));
    int randie = int(random(0, numImages_hist));
    Serial.println();
    Serial.println(randie);
    bmpDraw(images[randie], random(0, 241), random(0, 321));
    a++;
    if (a > 4) a = 0;
    delay(1000);
  */


  // check the sensor every so often
  // get a reading
  if (millis() - lastMillis > 200) {     // read every 200 ms - printing slows this down further
    // set the light low
    analogWrite(analogLED, 0);
    // check sensor
    wind_speed = checkWind();

    // if that reading is over the threshold
    if ((float)wind_speed > .05) {  //0.8   // this low?
      // take a time stamp
      //lastMillis = millis();

      // flutter a LED
      //analogWrite(analogLED, wind_speed);

      int smallBreeze = float(map(wind_speed, .05, 10.0, 0, 50));
      Serial.print(F("Breeze :  ")); Serial.println(smallBreeze);
      // for (int i = 0; i < smallBreeze; i++) {
      analogWrite(analogLED, smallBreeze);
      delay(100);
      //}
      // if
      //analogWrite(analogLED, 0);


    }

    /*  move this:
          //then take that wind and map to light (max light level):
          int brightness = float(map(wind, 5, 500.0, 0, 1023)); //<- no funniness?
          analogWrite(analogLED, brightness);
    */
    // can this run fast enough to make this perceptible?
    // do we need this second check?  "...if the wind is really blowing...."
    // wind = checkWind();
    // Serial.print(F("wind speed #2 is  "));
    // Serial.println(wind);

    //if (currentMillis - previousMillis > interval) { }

    wind_speed = checkWind();
    // if recent wind is greater than this level:
    if (wind_speed >= 10) {
      // maybe put wind to light light here?
      lastMillis = millis();
      Serial.print(F("wind_speed (before scale) is: "));
      Serial.println(wind_speed);
      // then take that wind and map to light (max light level):
      int brightness = float(map(wind_speed, 10, 50, 20, 1023)); //need some light

      for (int i = 0; i <= brightness; i++) {
        analogWrite(analogLED, i);
        delay(10);
      }

      analogWrite(analogLED, brightness);
      //Serial.println("about to show img;; BRIGHT");
      showImg(brightness);
      analogWrite(analogLED, 0); // bring lights low after
      //swapie();
      //Serial.println("heyo");

      if (previousErase == 0) { // if this is our first time through
        previousErase = millis();  // take a time stamp
        //Serial.print(F("previousErase at stamp is: ")); Serial.println(previousErase);
      }

      //analogWrite(analogLED, 0);

    } // wind_speed > 10
  }  // taking a reading

  // Serial.print(F("previousErase - timestamp is: ")); Serial.println(millis() - previousErase);
  if (millis() - previousErase  > 600000) { // if we've erased again after 10 minutes
    swapie();
  }
}

//*/


void swapie() {
  //int thresh = 5000;
  // after so much time has passed
  // swap around 5 images:
  Serial.println(F("Some serious rearranging narratives about to happen"));
 // Serial.println();
 // Serial.println();
  /*
    for (int i = 0; i < 6; i++) {
    int randImgA = int(sizeof(images) / sizeof(char)); // pick a random spot
    char* tempImgA = images[randImgA];  // pull out what's in that spot to temp storage
    int randImgB = int(sizeof(images) / sizeof(char)); // pick a new random image
    //store in that random location that random image
    images[randImgA] = images[randImgB]; // copy new random slot to the previous
    if (i % 3 == 0) {
    // black out
    Serial.println(F("add in a black out"));
    images[randImgB] = "black.bmp"; // black out that spot where the image was drawn from
    } else {  // just swap them around totally
    images[randImgB] = tempImgA;
    }
  */
  //  swapping around narrative points
  //images[narratives[randX][randY] = images[int(random(images.length())]
  for (int i = 0; i < 6; i++) {
    // pick a random value within the limits of the lengths of rows + columns
    byte randX_a = byte(random(7));
    byte randY_a = byte(random(10));
    byte tempNarrA = narratives[randX_a][randY_a];

    byte randX_b = byte(random(7));
    byte randY_b = byte(random(10));
    byte tempNarrB = narratives[randX_b][randY_b];

    byte temp = tempNarrB;
    tempNarrB = tempNarrA;
    tempNarrA = temp;
    if (i % 3 == 0) {
      narratives[randX_a][randY_a] = black;  // make that one a black out/0 .
    } else {
      narratives[randX_a][randY_a] = tempNarrA;
      narratives[randX_b][randY_b] = tempNarrB;
    }

  }// for loop
  previousErase = millis();  // take a time stamp

}  // swapie


void showImg(int w) {
  currentMillis = millis();  //unsigned long
  //  ensures that lights are on for image (bug work arnd):
  analogWrite(analogLED, w);
  Serial.println(F("Turned on...now img"));

  if (currentMillis - previousMillis > interval) {

    r = random(hist);   // <--- why + 50?
    Serial.println(F("random probablity is ="));
    Serial.print(r);
    Serial.println();
    // once in a while, show a narrative:
    if ( r <= prob_narr) {
      //figure out a narrative to show, along with delay lengths:
      Serial.println(F("showing narrative"));


      showNarrative();
      //currentMillis = millis();  //unsigned long


    } //if probabilty says to show a narr

    //Otherwise, show a random image from storage:
    else if ( r <= hist) {

      //select random image to show
      byte image = int(random(1, (sizeof(storageH) / sizeof(int)))); //number of images - 1, for empty slot
      //   Serial.print(F("image picked =  "));
      //   Serial.println(image);

      //let's check if we have already been shown:
      while (alreadyShown(image)) { //maybe use an if statement instead?
        Serial.println(F("let's try a different number"));
        image = int(random(1, (sizeof(storageH) / sizeof(int)))); // there are 14 slots, but 13 images. random(min, (max+1)); doesn't do top limit. up unto that value
        Serial.print(F("new image picked =  "));
        Serial.println(image);
      }
      Serial.println(F("showing reg img"));
      //delay(1000);
      // show the image since not a dup:
      //      bmpDraw(images[image], 0, 0);
      // byte randie = byte(random(1, numImages_hist));
      char filename[6]; // to ho
      char temp[3];
      strcpy(filename, itoa(byte(image), temp, 10));
      strcat(filename, ".bmp");
      // String file = String((char*)(f));
      Serial.println(F("file name to show is: "));
      Serial.println(filename);

      reader.drawBMP(filename, tft, 0, 0);


      // pick a random delay time:
      int randDelayLong = ceil(random(20, 55)) * 100;
      int randDelayShort = ceil(random(10, 30)) * 100;
      int delays[] = {randDelayLong, randDelayShort, randDelayLong};
      chooser = floor(random(sizeof(delays) / sizeof(int)));
      delay(delays[chooser]);

      // if we've been up and running awhile mix up memories  <----**
      mixingMemories(chooser);
      chooser = floor(random(sizeof(delays) / sizeof(int)));
      delay(delays[chooser]);

      //currentMillis = millis();  //unsigned long
      tft.fillScreen(ILI9341_BLACK);

      //store the image in the next available slot
      storageCheck();

    } //else if hist
    else {
      tft.fillScreen(ILI9341_BLACK);  // <------ try this off
      Serial.println(F("last ditch; black"));
      analogWrite(analogLED, 0);

    } // else
    // takes so long to load images, best to take another time stamp
    //currentMillis = millis();
    previousMillis = currentMillis;
    //Serial.println("showImg time lapse. dropping out");
  } // if enough time has passed

}

char* randImgfile() {
  byte randie = byte(random(1, numImages_hist));
  // Serial.print(F("randie is:  ")); Serial.println(randie);
  //char filename[6]; // to ho
  char temp[3];
  strcpy(filename, itoa(byte(randie), temp, 10));
  //Serial.print(F("filename is:  ")); Serial.println(filename);
  strcat(filename, ".bmp");
  //Serial.print(F("filename is:  ")); Serial.println(filename);
  return filename;

}

void showNarrative() {
  // use probability, as to which narrative to show:
  int randNarr = ceil(random(10));

  // pick a couple differnt delay times:
  int randDelayLong = ceil(random(55)) * 100;
  int randDelayShort = ceil(random(15, 20)) * 100;
  int delays[] = {randDelayLong, randDelayShort, randDelayLong};

  // this holds the final selection:
  int narrSelection;

  if ( randNarr < 2) {
    narrSelection = 2;
  }
  else if (randNarr < 3) {
    narrSelection = 3;
  }
  else if (randNarr < 4) {
    narrSelection = 5;
  }
  else if (randNarr < 5) {
    narrSelection = 0;
  }
  else if (randNarr < 7) {
    narrSelection = 4;
  }
  else if (randNarr < 9) {
    narrSelection = 1;
  }
  else {
    narrSelection = 6;
  }


  // show all those images in that narrative:
  for (int i = 0; i < narrLimit; i++) {
    int img = narratives[narrSelection][i];
    if (narratives[narrSelection][i] == 0) {
      tft.fillScreen(ILI9341_BLACK);
    }
    else {   //<---- this is narrative sequence:
      //      bmpDraw(images[narratives[narrSelection][i] ], 0, 0);
      //char filename[10]; // to hold the file name
      byte img = byte(narratives[narrSelection][i]);
      char filename[6]; // to ho
      char temp[3];
      strcpy(filename, itoa(img, temp, 10));
      strcat(filename, ".bmp");
      reader.drawBMP(filename, tft, 0, 0);
      //if we are not the last img in narrative:
      // if (i == !(narrLimit - 1)) {
      // delay a varying amount between the images:
      int choser = int(random(0, 3));
      //Serial.println("delaying in between images");
      delay(delays[choser]);    // let's see how this goes
      // }// last image in narrative?

    } // showing narrative
  }// end of narrative even if first

  tft.fillScreen(ILI9341_BLACK);
} // show narrative

bool alreadyShown(int image) {
  //Serial.println(F("am I a dup?"));
  // if statement or variable for first time as 0 slot
  for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
    // if we are at last spot && the image has already been stored:
    if ( (i == (sizeof(storageH) / sizeof(int) - 1)) && (storageH[i] == image) ) {
      Serial.println(F("storage full! zeroing out"));
      for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
        storageH[i] = 0;
      }
      capacity = 0;
      return true;
    }
    //if the selected image is already in storage:
    else if (storageH[i] == image) {
      // Serial.println(F("I'm a duplicate"));
      return true;
    } // if
  } // for
  //Serial.println(F("no, I'm not a dup"));
  return false;

} //alreadyshown

void mixingMemories(int d) {
  Serial.println(F("mixing memories"));
  //previousMillis_m = millis(); // take a time stamp for memory mixing (black and overlay);
  Serial.print("previousMillis_m2:  ");
  Serial.println(previousMillis_m2);
  Serial.print("millis  ");
  Serial.println(millis());


                 // if it's been a longer while, black out memories
  if ((millis() - previousMillis_m2) > 300000) { // 5 minutes; 15 minutes = 900000
  // black over them
  Serial.println();
    Serial.println(F("Blacking out memories"));
    Serial.println();
    Serial.println();
    //int randie = int(random(0, numImages_hist));
    //    bmpDraw(images[randie], 0, 0);
    reader.drawBMP("0.bmp", tft, random(0, 241), random(0, 321));
    delay(randDelay());
    tft.setRotation(random(0, 3));
    //    bmpDraw(images[0], random(0, 241), random(0, 321));
    reader.drawBMP("0.bmp", tft, random(0, 241), random(0, 321));
    delay(randDelay());
    tft.setRotation(random(0, 3));
    //    bmpDraw(images[0], random(0, 241), random(0, 321));
    reader.drawBMP("0.bmp", tft, random(0, 241), random(0, 321));
    delay(randDelay());
    tft.setRotation(random(0, 3));
    previousMillis_m2 = millis();;  // restart variable
  }

  // combine one other slice of an image with a different image
  else if ((millis() - previousMillis_m ) > 210000) { // 3.5 minutes 
  // mix up memories. this does an overlay of a few images before moving on
  // Serial.println();
  Serial.println(F("now we are going to mix them up on display!"));
    // Serial.println();
    // Serial.println();
    for (int i = 0; i < 3; i++) {
      //      bmpDraw(images[random(0, numImages_hist)], 0, 0);
      //char f = randImgfile();
      Serial.print(F("moving file around: "));
      //Serial.println(f);
      reader.drawBMP(randImgfile(), tft, 0, 0);
      tft.setRotation(random(0, 3));

      // Serial.println();
      // Serial.println(randie);
      //      bmpDraw(images[randie], random(0, 241), random(0, 321));
      reader.drawBMP(randImgfile(), tft, random(0, 241), random(0, 321));
      delay(delays[d]);
    }
    //delay(2000);
    previousMillis_m = millis();
    //previousMillis_m2 = previousMillis_m2 + previousMillis_m;

    //tft.fillScreen(ILI9341_BLACK);
  } else {
    Serial.println(F("not mixing today"));
    //previousMillis_m = millis();
    //previousMillis_m2 = previousMillis_m2 + previousMillis_m;
  }

  tft.setRotation(0);
}

int randDelay() {
  int randDelayLong = ceil(random(20, 55)) * 100;
  int randDelayShort = ceil(random(10, 30)) * 100;
  int delays[] = {randDelayLong, randDelayShort, randDelayLong};
  int chooser = floor(random(sizeof(delays) / sizeof(int)));
  return chooser;
}

void storageCheck() {

  Serial.println(F("let's get me stored"));
  analogWrite(analogLED, 0);
  for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
    if (storageH[i] == 0) { // need to check this about 1st slot & 0 showing
      storageH[i] = image;
      //  Serial.print(F("storing the image for the future  "));
      //  Serial.println(image);
      capacity++;
      break;
    } // if
  } //for

  //Serial.println(F("are we at capacity? let's check"));
  // Serial.print(F("Capacity is  "));
  // Serial.println(capacity);
  if (capacity == numImages_hist) { //  no longer true ---> // 14 slots, but can't add 0, the first slot. 13 images
    Serial.println(F("storage full! zeroing out"));
    for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
      storageH[i] = 0;
    }
    capacity = 0;
  }

}

float checkWind() {
  //if (millis() - lastMillis2 > 200) {     // read every 200 ms - printing slows this down further
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
  //Serial.print("   WindSpeed MPH ");
  //Serial.println((float)WindSpeed_MPH);
  // lastMillis2 = millis();

  // sensor is sensitive. scale numbers for ease:
  WindSpeed_MPH = WindSpeed_MPH * 10.0;
  //Serial.print("   windspeed:  ");
  //Serial.println((float)WindSpeed_MPH);
  return WindSpeed_MPH;
  // }

}
