
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

const int CLK = 2; // Used for generating interrupts using CLK signal
const int SecondCLK = 3; // Used for generating interrupts using CLK signal

const int DT = 4;  // Used for reading DT signal
const int SecondDT = 5;  // Used for reading DT signal

const int SW = 6;  // Used for the push button switch
const int SecondSW = 7;  // Used for the push button switch

#define NUMPIXELS      30

// Keep track of last rotary value
int lastCount = 50;
int SecondLastCount = 50;

bool changeLEDs = true;
bool SecondChangeLEDs = true;

bool changeBrightness = false;
bool SecondChangeBrightness = false;

int brightness = 50;
int SecondBrightness = 50;

int adjuster = 1;
int SecondAdjuster = 1;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, 8, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel SecondPixels = Adafruit_NeoPixel(NUMPIXELS, 9, NEO_GRB + NEO_KHZ800);

// Updated by the ISR (Interrupt Service Routine)
volatile int virtualPosition = 50;
volatile int SecondVirtualPosition = 50;

// ------------------------------------------------------------------
// INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT     INTERRUPT
// ------------------------------------------------------------------
void isr ()  {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(DT) == LOW)
    {
      virtualPosition-- ; // Could be -5 or -10
    }
    else {
      virtualPosition++ ; // Could be +5 or +10
    }
    if (virtualPosition >= 256)
    {
      virtualPosition = 0;
    }
    else if (virtualPosition < 0) {
      virtualPosition = 255;
    }
    // Restrict value from 0 to +100
    // virtualPosition = min(255, max(0, virtualPosition));

    // Keep track of when we were here last (no more than every 5ms)
    lastInterruptTime = interruptTime;
  }

}

void isr2 ()  {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // If interrupts come faster than 5ms, assume it's a bounce and ignore
  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(SecondDT) == LOW)
    {
      SecondVirtualPosition-- ; // Could be -5 or -10
    }
    else {
      SecondVirtualPosition++ ; // Could be +5 or +10
    }
    if (SecondVirtualPosition >= 256)
    {
      SecondVirtualPosition = 0;
    }
    else if (SecondVirtualPosition < 0) {
      SecondVirtualPosition = 255;
    }
    // Restrict value from 0 to +100
    // virtualPosition = min(255, max(0, virtualPosition));

    // Keep track of when we were here last (no more than every 5ms)
    lastInterruptTime = interruptTime;
  }

}

// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------
void setup() {
  // Just whilst we debug, view output on serial monitor
  // Serial.begin(9600);

  // Rotary pulses are INPUTs
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SecondCLK, INPUT);
  pinMode(SecondDT, INPUT);
  // Switch is floating so use the in-built PULLUP so we don't need a resistor
  pinMode(SW, INPUT_PULLUP);
  pinMode(SecondSW, INPUT_PULLUP);

  // Attach the routine to service the interrupts
  attachInterrupt(digitalPinToInterrupt(CLK), isr, LOW);
  attachInterrupt(digitalPinToInterrupt(SecondCLK), isr2, LOW);

  pixels.begin(); // This initializes the NeoPixel library.
  SecondPixels.begin(); // This initializes the NeoPixel library.

  rainbowCycle(20);
  // Ready to go!
  //Serial.println("Start");
}

// ------------------------------------------------------------------
// MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP     MAIN LOOP
// ------------------------------------------------------------------
void loop() {

  // Is someone pressing the rotary switch?
  if ((!digitalRead(SW))) {
    //virtualPosition = 50;
    changeBrightness = true;
    // Serial.println("Reset");
    //while (!digitalRead(SW))
    delay(10);
    //Serial.println("ResetOff");
  }
  else {
    changeBrightness = false;
  }

  // Is someone pressing the rotary switch?
  if ((!digitalRead(SecondSW))) {
    //virtualPosition = 50;
    SecondChangeBrightness = true;
    // Serial.println("Reset");
    //while (!digitalRead(SW))
    delay(10);
    //Serial.println("ResetOff");
  }
  else {
    SecondChangeBrightness = false;
  }

  // If the current rotary switch position has changed then update everything
  if (virtualPosition != lastCount) {
    changeLEDs = true;
    // Keep track of this new value
    lastCount = virtualPosition ;
  }
  // If the current rotary switch position has changed then update everything
  if (SecondVirtualPosition != SecondLastCount) {
    SecondChangeLEDs = true;
    // Keep track of this new value
    SecondLastCount = SecondVirtualPosition ;
  }

  if (changeBrightness) {

    brightness += adjuster;

    if (brightness >= 255 || brightness <= 0) {
      adjuster *= -1;
    }
  }
  if (SecondChangeBrightness) {

    SecondBrightness += SecondAdjuster;

    if (SecondBrightness >= 255 || SecondBrightness <= 0) {
      SecondAdjuster *= -1;
    }
  }

  if (changeBrightness || changeLEDs) {
    for (int i = 0; i < NUMPIXELS; i++) {

      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, Wheel((virtualPosition) & 255));
    }
    pixels.show(); // This sends the updated pixel color to the hardware.
    changeLEDs = false;
  }

  if (SecondChangeBrightness || SecondChangeLEDs) {
    for (int i = 0; i < NUMPIXELS; i++) {

      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      SecondPixels.setPixelColor(i, SecondWheel((SecondVirtualPosition) & 255));
    }
    SecondPixels.show(); // This sends the updated pixel color to the hardware.
    SecondChangeLEDs = false;
  }

}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
      SecondPixels.setPixelColor(i, Wheel(((i * 256 / SecondPixels.numPixels()) + j) & 255));
    }
    pixels.show();
    SecondPixels.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color((255 - WheelPos * 3) * brightness / 255, 0, (WheelPos * 3) * brightness / 255);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, (WheelPos * 3) * brightness / 255, (255 - WheelPos * 3) * brightness / 255);
  }
  WheelPos -= 170;
  return pixels.Color((WheelPos * 3) * brightness / 255, (255 - WheelPos * 3) * brightness / 255, 0);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t SecondWheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return SecondPixels.Color((255 - WheelPos * 3) * SecondBrightness / 255, 0, (WheelPos * 3) * SecondBrightness / 255);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return SecondPixels.Color(0, (WheelPos * 3) * SecondBrightness / 255, (255 - WheelPos * 3) * SecondBrightness / 255);
  }
  WheelPos -= 170;
  return SecondPixels.Color((WheelPos * 3) * SecondBrightness / 255, (255 - WheelPos * 3) * SecondBrightness / 255, 0);
}

