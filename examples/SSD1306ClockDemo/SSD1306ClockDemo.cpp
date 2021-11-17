/**
   The MIT License (MIT)

   Copyright (c) 2018 by ThingPulse, Daniel Eichhorn

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   ThingPulse invests considerable time and money to develop these open source libraries.
   Please support us by buying our products (and not the clones) from
   https://thingpulse.com

*/

// For a connection via SPI include
#include "SSD1306Spi.h"

// Include the UI lib
#include "OLEDDisplayUi.h"

// Include custom images
#include "images.h"
#include <cmath>
#include <ctime>

SSD1306Spi display(10, 16, 15, 1);
OLEDDisplayUi ui(&display);

int screenW = 128;
int screenH = 64;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int clockRadius = 23;

// utility function for digital clock display: prints leading 0
std::string twoDigits(int digits)
{
  if (digits < 10)
  {
    std::string i = '0' + std::to_string(digits);
    return i;
  }
  else
  {
    return std::to_string(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState *state)
{
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, short x, short y)
{
  //  ui.disableIndicator();

  // Draw the clock face
  //  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for (int z = 0; z < 360; z = z + 30)
  {
    //Begin at 0° and stop at 360°
    float angle = z;
    angle = (angle / 57.29577951); //Convert degrees to radians
    int x2 = (clockCenterX + (sin(angle) * clockRadius));
    int y2 = (clockCenterY - (cos(angle) * clockRadius));
    int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 8))));
    int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 8))));
    display->drawLine(x2 + x, y2 + y, x3 + x, y3 + y);
  }
  std::time_t t = std::time(0); // get time now
  std::tm *now = std::localtime(&t);
  
  // display second hand
  float angle = now->tm_sec * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  int x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 5))));
  int y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 5))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display minute hand
  angle = now->tm_min * 6;
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 4))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 4))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
  //
  // display hour hand
  angle = now->tm_hour * 30 + int((now->tm_min / 12) * 6);
  angle = (angle / 57.29577951); //Convert degrees to radians
  x3 = (clockCenterX + (sin(angle) * (clockRadius - (clockRadius / 2))));
  y3 = (clockCenterY - (cos(angle) * (clockRadius - (clockRadius / 2))));
  display->drawLine(clockCenterX + x, clockCenterY + y, x3 + x, y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState *state, short x, short y)
{
  std::time_t t = std::time(0); // get time now
  std::tm *now = std::localtime(&t);
  char str[9] = { '\0' };
  std::sprintf(str, "%02d:%02d:%02d",now->tm_hour,now->tm_min,now->tm_sec );
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x, clockCenterY + y, str);
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = {analogClockFrame, digitalClockFrame};

// how many frames are there?
int frameCount = 2;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = {clockOverlay};
int overlaysCount = 1;

int main()
{
  /* ui.setTargetFPS(60);
  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);
  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);
  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  // Add frames
  ui.setFrames(frames, frameCount);
  // Add overlays
  ui.setOverlays(overlays, overlaysCount);
  // Initialising the UI will init the display too.
  ui.init();
  //display.flipScreenVertically();
  while (1)
  {
    int remainingTimeBudget = ui.update();

    if (remainingTimeBudget > 0)
    {
      // You can do some work here
      // Don't do stuff if you are below your
      // time budget.
      //delay(remainingTimeBudget);
    }
  } */
  // Initialising the UI will init the display too.
  display.init();
  display.clear();
  //display.drawRect(0,0,25,25);
  display.flipScreenVertically();
    std::string str2 = "0Oo0123456789";
    std::string str1 = "here I'm";
  while (1)
  {
    std::time_t t = std::time(0); // get time now
    std::tm *now = std::localtime(&t);
    char str[9] = {'\0'};
    std::sprintf(str, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, str);
    display.setFont(Monospaced_plain_10);
    display.drawString(0, 28, str2);
    display.drawString(0, 38, str1);
    display.display();
    //delay(1000);
    display.clear();
  }

  return 0;
}
