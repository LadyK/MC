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

#define analogPinForRV    3   // change to pins you the analog pins are using
#define analogPinForTMP   0
#define analogLED         6


Adafruit_ILI9340 tft = Adafruit_ILI9340(TFT_CS, TFT_DC, TFT_RST);

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

char* images[] = {"black.bmp", "asburyB.bmp", "vancov.bmp", "bench.bmp", "AlexBDay.bmp", "caton.bmp", "lvrmBlu.bmp", "sunset2.bmp", "street2.bmp",
                  "street.bmp", "swiss.bmp", "tobys.bmp", "trainR.bmp",  "westS2.bmp", "wintree.bmp", "friebG.bmp",
                  "black2.bmp", "karok.bmp",  "nightSt.bmp",  "subway.bmp", "kayak.bmp", "blue.bmp"
                 };


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

int numImages_hist = 22;   //actual number of images to show

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

unsigned long previousMillis_m = 0;

//store the random selections so not as to repeat any selections
int storageH[22];   //  // need to store zero slot, but will never use that image slot
int prob_narr = 100; //= 200;
int hist = 800;
int picD = 150;
int picH = 4000;
int image;
long r;
unsigned long previousMillis_m2;

//String whichNarr[] = { "natural", "bday", "dark", "dark2", "water", "winter", "blank"};
int a = 0;
int wSize2 = 240 / 2;
int hSize2 = 320 / 2;
int wSize3 = 240 / 3;
int hSize3 = 320 / 3;
int wSize4 = 240 / 4;
int hSize4 = 240 / 4;

void setup() {


  Serial.begin(57600);   // faster printing to get a bit better throughput on extended info
  // remember to change your serial monitor

  Serial.print(F("Initializing SD card..."));
  if (!SD.begin(SD_CS)) {
    Serial.println(F("failed!"));
    return;
  }
  Serial.println(F("OK!"));

  tft.begin();
  tft.fillScreen(ILI9340_BLUE);

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
  tft.fillScreen(ILI9340_BLACK);
  randomSeed(analogRead(A5));

  lastMillis = 0;

}

void loop() {
  delay(200); // slow everything down a tad for testing
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
  if (millis() - lastMillis > 250) {     // read every 200 ms - printing slows this down further
    // set the light low
    analogWrite(analogLED, 0);
    // check sensor
    Wind_speed = checkWind();
    Serial.print(F("wind speed is  "));
    Serial.println(Wind_speed);

    // if that reading is over the threshold
    if ((float)Wind_speed > 5.0) {  //0.8   // this low?
      // take a time stamp
      lastMillis = millis();
      // flutter a LED
      wind = checkWind();
      Serial.print(F("         wind speed is inside: "));
      Serial.println(wind);

      //is this necessary any more??
      ///*
      // take the LED's to the level of the wind:
      //bring them up frrom darkness to the level of wind
      for (int i = 0; i < wind; i++) {
        analogWrite(analogLED, i);
        //delay(10);
      }
      //*/

      /*  move this:
            //then take that wind and map to light (max light level):
            int brightness = float(map(wind, 5, 500.0, 0, 1023)); //<- no funniness?
            analogWrite(analogLED, brightness);
      */
      wind = checkWind();
      Serial.print(F("wind speed is  "));
      Serial.println(wind);

      //if (currentMillis - previousMillis > interval) { }

      // if recent wind is greater than this level:
      if (wind > 8.0) {
        // maybe put wind to light light here?

        // then take that wind and map to light (max light level):
        int brightness = float(map(wind, 5, 500.0, 0, 1023)); //<- no funniness?
        analogWrite(analogLED, brightness);


        showImg(wind);
        analogWrite(analogLED, 0); // bring lights low after
        //swapie();
        //Serial.println("heyo");

        if (previousErase == 0) { // if this is our first time through
          previousErase = millis();  // take a time stamp
          //Serial.print(F("previousErase at stamp is: ")); Serial.println(previousErase);
        }

        //analogWrite(analogLED, 0);
      }

      Serial.print(F("previousErase - timestamp is: ")); Serial.println(millis() - previousErase);
      if (millis() - previousErase  > 600000) { // if we've erased again after 10 minutes
        swapie();
      }
    }
  }
  //*/
}

void swapie() {
  //int thresh = 5000;
  // after so much time has passed
  // swap around 5 images:
  Serial.println("Some serious swappin' about to happen");
  Serial.println();
  Serial.println();
  for (int i = 0; i < 6; i++) {
    int randImgA = int(sizeof(images) / sizeof(char)); // pick a random spot
    char* tempImgA = images[randImgA];  // pull out what's in that spot to temp storage
    int randImgB = int(sizeof(images) / sizeof(char)); // pick a new random image
    //store in that random location that random image
    images[randImgA] = images[randImgB]; // copy new random slot to the previous
    if (i % 3 == 0) {
      // black out
      Serial.println("add in a black out");
      images[randImgB] = "black.bmp"; // black out that spot where the image was drawn from
    } else {  // just swap them around totally
      images[randImgB] = tempImgA;
    }

    /* swapping around narrative points
      //images[narratives[randX][randY] = images[int(random(images.length())]
      // pick a random value within the limits of the lengths of rows + columns
      int randX = int(random(7));
      int randY = int(random(10));
      int tempNarrA = narratives[randX][randY];

      randX = int(random(7));
      randY = int(random(10));
      int tempNarrB = narratives[randX][randY];
    */

  }// for loop
  previousErase = millis();  // take a time stamp

}


void showImg(int w) {
  currentMillis = millis();  //unsigned long

  // took this out. don't think it's necessary. Already at the light level:
  //int brightness = float(map(w, 0, 500.0, 0, 1023));
  //analogWrite(analogLED, brightness);

  if (currentMillis - previousMillis > interval) {
    //Serial.println("heyo");
    //upon input, chose random value
    // Serial.println(F("choosing a new random number:  "));
    r = random(1000);
    //Serial.print(F("random selected is  "));
    //Serial.println(r);
    delay(100);

    // once in a while, show a narrative:
    if ( r <= prob_narr) {
      //figure out a narrative to show, along with delay lengths:

      // use a bit more probability, as to which narrative to show:
      int randNarr = ceil(random(10));
      // Serial.print(F("randNarr select is: ")); Serial.println(randNarr);

      // pick a couple differnt delay times
      int randDelayLong = ceil(random(55)) * 100;
      int randDelayShort = ceil(random(7, 10)) * 100;
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
      //Serial.print(F("random selection is: ")); Serial.println(narrSelection);
      //Serial.print(F("Array line is ")); Serial.println(whichNarr[narrSelection]);

      // show all those images in that narrative:
      for (int i = 0; i < narrLimit; i++) {
        int img = narratives[narrSelection][i];
        if (narratives[narrSelection][i] == 0) {
          tft.fillScreen(ILI9340_BLACK);
        }
        else {   //<---- Give this a go
          // if we are either of the 2 dark narratives
          //if (narrSelection == 2 || narrSelection == 3) {
          // add in some rotation  (would like to layer here and change size
          //  tft.setRotation(a);
          //  bmpDraw(images[narratives[narrSelection][i] ], 0, 0);
          //   a++;
          //  if (a > 4) a = 0;
          //delay(1000);
          // } else {
          //otherwise...
          bmpDraw(images[narratives[narrSelection][i] ], 0, 0);
          // }
          //}
          if (i == !(narrLimit - 1)) { //if we are not the last img in narrative:
            // delay a varying amount between the images:
            int choser = int(random(0, 3));
            Serial.print(F("delay narr"));
            Serial.println(delays[choser]);
            delay(delays[choser]);    // let's see how this goes
          }
        }
      }
      tft.fillScreen(ILI9340_BLACK);

    } //if probabilty says to show a narr

    //Otherwise, perhaps show a random image from storage:
    else if ( r <= hist) {
      //select random image to show
      image = int(random(1, (sizeof(storageH) / sizeof(int)))); //number of images - 1, for empty slot
      //   Serial.print(F("image picked =  "));
      //   Serial.println(image);

      //let's check if we have already been shown: (if so, select another image:)
      while (alreadyShown(image)) { //maybe use an if statement instead?
        Serial.println(F("let's try a different number"));
        image = int(random(1, (sizeof(storageH) / sizeof(int)))); // there are 14 slots, but 13 images. random(min, (max+1)); doesn't do top limit. up unto that value
        Serial.print(F("new image picked =  "));
        Serial.println(image);
      }

      //  /*

      int randDelayLong = ceil(random(55)) * 100;
      int randDelayShort = ceil(random(10)) * 100;
      int delays[] = {randDelayLong, randDelayShort, randDelayLong};
      int choser = int(random(0, 3));

      // draw the image here, right? If it's been selected and not shown already, then show it.
      // bmpDraw(images[image], 0, 0); //shown in alreadyShown()
      //  */

    } //else if hist
    else {
      tft.fillScreen(ILI9340_BLACK);  // <------ try this off
      //Serial.println(F("got here, turning led off"));
      analogWrite(analogLED, 0);

    } // else
    // takes so long to load images, best to take another time stamp
    currentMillis = millis();
    previousMillis = currentMillis;
  } // if enough time has passed
  //randomSeed(analogRead(A5));
}

bool alreadyShown(int image) {
  //Serial.println(F("am I a dup?"));
  // if statement or variable for first time as 0 slot
  for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {

    // Serial.print(F("value in storage  "));
    // Serial.println(storageH[i]);
    //if first time through

    if (storageH[i] == image) {


      // Serial.println(F("I'm a duplicate"));
      return true;
    } // if
  } // for
  //Serial.println(F("no, I'm not a dup"));


  // Serial.print(F("taaadaaayyyyyy image shown is  "));
  // Serial.println(history[image]);
  delay(1000);
  bmpDraw(images[image], 0, 0);
  //previousMillis = millis(); // take a time stamp for memory mixing (black and overlay);
  //Serial.println();
  //Serial.print("memory mixer stamp: ");
  //Serial.println(previousMillis_m);
  //previousMillis_m2 = previousMillis_m;

  mixingMemories();  // if we've been up and running awhile mix up memories

  int randDelayLong = ceil(random(55)) * 100;
  int randDelayShort = ceil(random(10)) * 100;
  int delays[] = {randDelayLong, randDelayShort, randDelayLong};
  int chooser = floor(random(sizeof(delays) / sizeof(int)));
  delay(delays[chooser]);
  //delay(picH);  //<---- HOW LONG to DELAY
  tft.fillScreen(ILI9340_BLACK);
  //Serial.println(F("got here"));
  //store the image in the next available slot
  storageCheck();
  //Serial.println(F("and passed through here"));
  //Serial.println("just before false");
  return false;

} //alreadyshown

void mixingMemories() {
  Serial.println(F("mixing memories"));
  //previousMillis_m = millis(); // take a time stamp for memory mixing (black and overlay);
  //Serial.print("memory mixer stamp: ");
  //Serial.println(previousMillis_m);
  //Serial.println();
  //Serial.println();
  //Serial.println(millis());
  Serial.println(millis() - previousMillis );

  if ((millis() - previousMillis_m ) > 150000 ) { // 15 mintes = 900,000
    // mix up memories. this does an overlay of a few images before moving on
    // Serial.println();
    Serial.println(F("now we are going to mix them up!"));
    // Serial.println();
    // Serial.println();
    for (int i = 0; i < 3; i++) {
      bmpDraw(images[random(0, numImages_hist)], 0, 0);
      tft.setRotation(random(0, 3));
      int randie = int(random(0, numImages_hist));
      // Serial.println();
      // Serial.println(randie);
      bmpDraw(images[randie], random(0, 241), random(0, 321));

      delay(1000);

    }
    delay(2000);
    previousMillis_m = millis();
    previousMillis_m2 = previousMillis_m2 + previousMillis_m;

    //tft.fillScreen(ILI9340_BLACK);
  }


  // if it's been a longer while, black out memories
  if ((millis() - previousMillis_m2) > 300000) { // 20 minutes = 1,200,000
    // black over them
    Serial.println();
    Serial.println("Blacking out memories");
    Serial.println();
    Serial.println();
    int randie = int(random(0, numImages_hist));
    bmpDraw(images[randie], 0, 0);
    tft.setRotation(random(0, 3));
    bmpDraw(images[0], random(0, 241), random(0, 321));
    tft.setRotation(random(0, 3));
    bmpDraw(images[0], random(0, 241), random(0, 321));
    tft.setRotation(random(0, 3));

    previousMillis_m2 = 0;

  }

  tft.setRotation(0);
}

void storageCheck() {

  Serial.println(F("let's get me stored"));
  analogWrite(analogLED, 0);
  for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
    if (storageH[i] == 0) { // need to check this about 1st slot & 0 showing
      storageH[i] = image;
      //  Serial.print(F("storing the image for the future  "));
      //  Serial.println(image);
      break;
    } // if
  } //for

  //Serial.println(F("are we at capacity? let's check"));
  int capacity = 0;
  for (int i = 1; i < sizeof(storageH) / sizeof(int); i++) {
    //if this slot isn't equal to zero, increment capacity
    if (storageH[i] != 0) {
      capacity++;
    }

  }
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

    Serial.print("   WindSpeed MPH ");
    Serial.println((float)WindSpeed_MPH);
  */

  WindSpeed_MPH = WindSpeed_MPH * 100.0;
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


#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint8_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= tft.width()) || (y >= tft.height())) return;
  /**

     Keep print statements. When comment these out, pics don't load
     timing?
  */
  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if (read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: "));
    Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: "));
    Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: "));
    Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if (read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: "));
      Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= tft.width())  w = tft.width()  - x;
        if ((y + h - 1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);

        for (row = 0; row < h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r, g, b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(F(" ms"));
      } // end goodBmp
    }
  }

  bmpFile.close();
  if (!goodBmp) Serial.println(F("BMP format not recognized."));
}


// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File & f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File & f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
