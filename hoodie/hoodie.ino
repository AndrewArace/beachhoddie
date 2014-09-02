#include <Adafruit_NeoPixel.h>

#define PIN 1
#define BUTTON 2
#define HOLD_THRESHOLD 20
#define BRIGHT_STEP 5
#define COLOR_CYCLE_STEP 4

#define MAX_MODE 5

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(6, PIN, NEO_GRB + NEO_KHZ800);

uint8_t mode = 0; //current mode
uint8_t color = 85; //current color (red)
uint8_t offset = 0; //for chase animation
uint8_t buttonState = 0; //current button's state
uint8_t lastButtonState = HIGH; //temp last state of button
uint8_t i; //loop counter
bool lightsOn = true; //for flash animation
//tracking button push-and-hold
bool longPress = false; 
uint8_t buttonHeld = 0;
//pulsing variables
uint8_t brightness = 255;
uint8_t brightSign = BRIGHT_STEP;

void setup() {
  pinMode(BUTTON, INPUT_PULLUP); // switch wired to ground & pin, pushing makes it go LOW
  pixels.begin();
  pixels.show(); // Initialize all pixels to 'off'
}

void loop() {
    i = 0;
    
    switch(mode) {
      
      case 0: //solid
        for(i=0; i < pixels.numPixels(); ++i) {
          pixels.setPixelColor(i, Wheel(color&255));
        }
        pixels.show();
        delay(50);
        break;
        
      case 1: //blink
        for(i=0; i < pixels.numPixels(); ++i) {
          if(lightsOn) {
            pixels.setPixelColor(i, Wheel(color&255));
          }
          else {
            pixels.setPixelColor(i, 0);
          }
        }
        pixels.show();
        delay(200);
        lightsOn = !lightsOn;
        break;
        
      case 2:
        //chase
        for(i=0; i < pixels.numPixels(); ++i) {
          pixels.setPixelColor(i, (((offset + i) % pixels.numPixels()) == 0) ? Wheel(color&255) : 0);
        }
        pixels.show();
        offset++;
        delay(50);
        break;
        
      case 3:
        //pulse
        for(i=0; i < pixels.numPixels(); ++i) {
          pixels.setPixelColor(i, Wheel(color&255));
        }
        if(brightness <= 0) {
          brightness = 0;
          brightSign = BRIGHT_STEP;
        }
        if(brightness >= 255) {
          brightness = 255;
          brightSign = BRIGHT_STEP * -1;
        }
        brightness = brightness + brightSign;
        pixels.setBrightness(brightness);
        pixels.show();
        delay(50);
        break;
        
      case 4: //solid white
        brightness = 255;
        pixels.setBrightness(brightness);
        for(i=0; i < pixels.numPixels(); ++i) {
          pixels.setPixelColor(i, pixels.Color(255,255,255));
        }
        pixels.show();
        delay(50);
        break;
        
      case 5: //rainbow
        rainbowCycle(20);
        break;
        
    }
    
    checkButton();
}


bool checkButton() {
  buttonState = digitalRead(BUTTON);
  
  if(buttonState != lastButtonState) {
    lastButtonState = buttonState;
  
    if(buttonState == LOW) {
      //just pushed
      //check for long button press/hold
      while (digitalRead(BUTTON) == LOW && buttonHeld < (HOLD_THRESHOLD+1)){
        delay(100);
        buttonHeld++;
      }
  
      if (buttonHeld > HOLD_THRESHOLD) {
        longPress = true;
      }
      else {
        //was not a long enough hold
        brightness = 255;
        pixels.setBrightness(brightness);
        longPress = false;
      }
    }
    else {
      //just released
      if(!longPress) {
        //button was released before long press, mode switch
        brightness = 255;
        pixels.setBrightness(brightness);
        mode = (mode == MAX_MODE ? 0 : ++mode);       
      }
      longPress = false;
    }
  }
  
  if(longPress) {
    //holding the button
    color = (color >= 255 ? 0 : color += COLOR_CYCLE_STEP);
  }
  
  buttonHeld = 0;
  return false;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, c);
      pixels.show();
      delay(wait);
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    checkButton();
    if(mode != 5) return;
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

