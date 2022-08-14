#include <FastLED.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

// FASTLED
#define DATA_PIN1    3
#define NUM_LEDS    64
#define DATA_PIN2    5
#define NUM_LEDS2    64
//#define CLK_PIN   4

#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS2];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

// NEO 
// Which pin on the Arduino is connected to the NeoPixels?

#define PIN            4

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      60

// How many colors do you want (more colors, more gradient)?
#define NUMOFCOLORS      6


// jump between to traces (head trace to head trace - min JUMP = 3)
#define JUMP 5

// traces power will be like wave or like teeth
#define FADE 0 //1 - traces power like a wave, 0 always first led max power

// start from the middle or from the beginning
#define FROM_MIDDLE 1 // 1 - start from the middle for both sides


// delay between iterations
int delayval = 50; // delay for half a second

// light level 1=255
float HIGHLEVEL=0.4;
float MEDIUMLEVEL=0.2;
float LOWLEVEL=0.06;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


int head_color = 0;
// END NEO
void setup() {
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN1,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN2,COLOR_ORDER>(leds2, NUM_LEDS2).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
   pixels.begin(); // This initializes the NeoPixel library.
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])(CRGB* strip, int num_pixels);
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int neo_current_pixel = 0;
int iter =0;
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber](leds,NUM_LEDS);
  gPatterns[gCurrentPatternNumber](leds2,NUM_LEDS2);

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  //FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

  
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.  
  int last_led;
  if (FROM_MIDDLE) {
    last_led = NUMPIXELS/2;  
  } else {
    last_led = NUMPIXELS -1;
  }
  iter++;
  //Serial.println(iter);
  if (iter % delayval == 0) {
    iter = 0;
    if (neo_current_pixel == 2* last_led) {
      neo_current_pixel = last_led;
    } else {
      neo_current_pixel++;
    }
    //Serial.println(head_color);
    //Serial.println(neo_current_pixel);
     
    //Serial.println(i);
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    lightAllTraces(neo_current_pixel, HIGHLEVEL); // the first led.
    if (neo_current_pixel>0) {
        lightAllTraces(neo_current_pixel-1, MEDIUMLEVEL); 
        if (neo_current_pixel>1) {
           lightAllTraces(neo_current_pixel-2, LOWLEVEL); 
           if(neo_current_pixel>2 && JUMP >3) {
                lightAllTraces(neo_current_pixel-3, 0); // turn off the distant led.            
           }
        }        
    }      
    pixels.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
  }


}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow(CRGB* strip, int num_pixels) 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( strip, num_pixels, gHue, 7);  
}

void rainbowWithGlitter(CRGB* strip, int num_pixels) 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow(strip, num_pixels);
  addGlitter(80,strip, num_pixels);
}

void addGlitter( fract8 chanceOfGlitter,CRGB strip, int num_pixels) 
{
  if( random8() < chanceOfGlitter) {
    strip[ random16(num_pixels) ] += CRGB::White;
    
  }
}

void confetti(CRGB* strip, int num_pixels) 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( strip, num_pixels, 10);  
  int pos = random16(num_pixels);
  strip[pos] += CHSV( gHue + random8(64), 200, 255);  
}

void sinelon(CRGB* strip, int num_pixels)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( strip, num_pixels, 20);
  int pos = beatsin16( 13, 0, num_pixels-1 );
  strip[pos] += CHSV( gHue, 255, 192);
}

void bpm(CRGB strip, int num_pixels)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < num_pixels; i++) { //9948
    strip[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle(CRGB* strip, int num_pixels) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( strip, num_pixels, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    strip[beatsin16( i+7, 0, num_pixels-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void lightAllTraces(int i, float power) {
  int trace_num = 0;
   //Serial.println(i);
   while (i >= 0 ) {     
     int color = (head_color+trace_num)%NUMOFCOLORS;
     if (FROM_MIDDLE) {
      pixels.setPixelColor(NUMPIXELS/2+i, Wheel(color,power));
      pixels.setPixelColor(NUMPIXELS/2-i, Wheel(color,power));
     } else {
       pixels.setPixelColor(i, Wheel(color,power));     
     }
      i-=JUMP;     
      trace_num++;
      if (FADE) {
        if (power == HIGHLEVEL) {
          power = LOWLEVEL;      
        } else if (power == LOWLEVEL) {
          power = HIGHLEVEL;      
        }
      }
   } 
}
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// the level is how brigt will be tghe light (0 to 255).
uint32_t Wheel(byte color, float level) {
  Adafruit_NeoPixel dummy_pixels;
  if (level==0) return dummy_pixels.Color(0, 0, 0);
  //level=100;   
  float power;
 // Serial.println(color);
  if(color < NUMOFCOLORS/3) {
     power=1.0*color/(NUMOFCOLORS/3)*255;
     return dummy_pixels.Color(level*(255 - power), 0, level*power); 
  } else if(color < 2*NUMOFCOLORS/3) {
      color -= NUMOFCOLORS/3;
      power=1.0*color/(NUMOFCOLORS/3)*255;
      return dummy_pixels.Color(0, level*power, level*(255 - power));
  } else {
     color -= 2*NUMOFCOLORS/3;
     power=1.0*color/(NUMOFCOLORS/3)*255;
     return dummy_pixels.Color(level*power, level*(255 - power), 0);
  }
}
