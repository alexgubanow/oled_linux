/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
 * Copyright (c) 2019 by Helmut Tschemernjak - www.radioshuttle.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ThingPulse invests considerable time and money to develop these open source libraries.
 * Please support us by buying our products (and not the clones) from
 * https://thingpulse.com
 *
 */

#ifndef OLEDDISPLAY_h
#define OLEDDISPLAY_h

#include "OLEDDisplayFonts.h"
#include <string>

#ifndef DEBUG_OLEDDISPLAY
#define DEBUG_OLEDDISPLAY(...) printf("%s", __VA_ARGS__)
#endif
#define OLEDDISPLAY_REDUCE_MEMORY
// Use DOUBLE BUFFERING by default
#ifndef OLEDDISPLAY_REDUCE_MEMORY
#define OLEDDISPLAY_DOUBLE_BUFFER
#endif

// Header Values
#define JUMPTABLE_BYTES 4

#define JUMPTABLE_LSB 1
#define JUMPTABLE_SIZE 2
#define JUMPTABLE_WIDTH 3
#define JUMPTABLE_START 4

#define WIDTH_POS 0
#define HEIGHT_POS 1
#define FIRST_CHAR_POS 2
#define CHAR_NUM_POS 3

// Display commands
#define CHARGEPUMP 0x8D
#define COLUMNADDR 0x21
#define COMSCANDEC 0xC8
#define COMSCANINC 0xC0
#define DISPLAYALLON 0xA5
#define DISPLAYALLON_RESUME 0xA4
#define DISPLAYOFF 0xAE
#define DISPLAYON 0xAF
#define EXTERNALVCC 0x1
#define INVERTDISPLAY 0xA7
#define MEMORYMODE 0x20
#define NORMALDISPLAY 0xA6
#define PAGEADDR 0x22
#define SEGREMAP 0xA0
#define SETCOMPINS 0xDA
#define SETCONTRAST 0x81
#define SETDISPLAYCLOCKDIV 0xD5
#define SETDISPLAYOFFSET 0xD3
#define SETHIGHCOLUMN 0x10
#define SETLOWCOLUMN 0x00
#define SETMULTIPLEX 0xA8
#define SETPRECHARGE 0xD9
#define SETSEGMENTREMAP 0xA1
#define SETSTARTLINE 0x40
#define SETVCOMDETECT 0xDB
#define SWITCHCAPVCC 0x2

#ifndef _swap_short
#define _swap_short(a, b) \
  {                       \
    short t = a;          \
    a = b;                \
    b = t;                \
  }
#endif

enum OLEDDISPLAY_COLOR
{
  BLACK = 0,
  WHITE = 1,
  INVERSE = 2
};

enum OLEDDISPLAY_TEXT_ALIGNMENT
{
  TEXT_ALIGN_LEFT = 0,
  TEXT_ALIGN_RIGHT = 1,
  TEXT_ALIGN_CENTER = 2,
  TEXT_ALIGN_CENTER_BOTH = 3
};

enum OLEDDISPLAY_GEOMETRY
{
  GEOMETRY_128_64 = 0,
  GEOMETRY_128_32 = 1,
  GEOMETRY_64_48 = 2,
  GEOMETRY_64_32 = 3,
  GEOMETRY_RAWMODE = 4
};

typedef char (*FontTableLookupFunction)(const unsigned char ch);
char DefaultFontTableLookup(const unsigned char ch);

class OLEDDisplay
{
public:
  OLEDDisplay();
  virtual ~OLEDDisplay();
  unsigned short width(void) const { return displayWidth; };
  unsigned short height(void) const { return displayHeight; };
  // Use this to resume after a deep sleep without resetting the display (what init() would do).
  // Returns true if connection to the display was established and the buffer allocated, false otherwise.
  bool allocateBuffer();
  // Allocates the buffer and initializes the driver & display. Resets the display!
  // Returns false if buffer allocation failed, true otherwise.
  bool init();
  // Free the memory used by the display
  void end();
  // Cycle through the initialization
  void resetDisplay(void);
  /* Drawing functions */
  // Sets the color of all pixel operations
  void setColor(OLEDDISPLAY_COLOR color);
  // Returns the current color.
  OLEDDISPLAY_COLOR getColor();
  // Draw a pixel at given position
  void setPixel(short x, short y);
  // Draw a pixel at given position and color
  void setPixelColor(short x, short y, OLEDDISPLAY_COLOR color);
  // Clear a pixel at given position FIXME: INVERSE is untested with this function
  void clearPixel(short x, short y);
  // Draw a line from position 0 to position 1
  void drawLine(short x0, short y0, short x1, short y1);
  // Draw the border of a rectangle at the given location
  void drawRect(short x, short y, short width, short height);
  // Fill the rectangle
  void fillRect(short x, short y, short width, short height);
  // Draw the border of a circle
  void drawCircle(short x, short y, short radius);
  // Draw all Quadrants specified in the quads bit mask
  void drawCircleQuads(short x0, short y0, short radius, unsigned char quads);
  // Fill circle
  void fillCircle(short x, short y, short radius);
  // Draw an empty triangle i.e. only the outline
  void drawTriangle(short x0, short y0, short x1, short y1, short x2, short y2);
  // Draw a solid triangle i.e. filled
  void fillTriangle(short x0, short y0, short x1, short y1, short x2, short y2);
  // Draw a line horizontally
  void drawHorizontalLine(short x, short y, short length);
  // Draw a line vertically
  void drawVerticalLine(short x, short y, short length);
  // Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
  // a unsigned byte value between 0 and 100
  void drawProgressBar(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char progress);
  // Draw a bitmap in the internal image format
  void drawFastImage(short x, short y, short width, short height, const unsigned char *image);
  // Draw a XBM
  void drawXbm(short x, short y, short width, short height, const unsigned char *xbm);
  // Draw icon 16x16 xbm format
  void drawIco16x16(short x, short y, const unsigned char *ico, bool inverse = false);
  /* Text functions */
  // Draws a std::string at the given location
  void drawString(short x, short y, const std::string &text);
  // Draws a formatted std::string (like printf) at the given location
  void drawStringf(short x, short y, char *buffer, std::string format, ...);
  // Draws a std::string with a maximum width at the given location.
  // If the given std::string is wider than the specified width
  // The text will be wrapped to the next line at a space or dash
  void drawStringMaxWidth(short x, short y, unsigned short maxLineWidth, const std::string &text);
  // Returns the width of the const char* with the current
  // font settings
  unsigned short getStringWidth(const char *text, unsigned short length);
  // Convencience method for the const char version
  unsigned short getStringWidth(const std::string &text);
  // Specifies relative to which anchor point
  // the text is rendered. Available constants:
  // TEXT_ALIGN_LEFT, TEXT_ALIGN_CENTER, TEXT_ALIGN_RIGHT, TEXT_ALIGN_CENTER_BOTH
  void setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment);
  // Sets the current font. Available default fonts
  // ArialMT_Plain_10, ArialMT_Plain_16, ArialMT_Plain_24
  void setFont(const unsigned char *fontData);
  // Set the function that will convert utf-8 to font table index
  void setFontTableLookupFunction(FontTableLookupFunction function);
  /* Display functions */
  // Turn the display on
  void displayOn(void);
  // Turn the display offs
  void displayOff(void);
  // Inverted display mode
  void invertDisplay(void);
  // Normal display mode
  void normalDisplay(void);
  // Set display contrast
  // really low brightness & contrast: contrast = 10, precharge = 5, comdetect = 0
  // normal brightness & contrast:  contrast = 100
  void setContrast(unsigned char contrast, unsigned char precharge = 241, unsigned char comdetect = 64);
  // Convenience method to access
  void setBrightness(unsigned char);
  // Reset display rotation or mirroring
  void resetOrientation();
  // Turn the display upside down
  void flipScreenVertically();
  // Mirror the display (to be used in a mirror or as a projector)
  void mirrorScreen();
  // Write the buffer to the display memory
  virtual void display(void) = 0;
  // Clear the local pixel buffer
  void clear(void);
  // Log buffer implementation
  // This will define the lines and characters you can
  // print to the screen. When you exeed the buffer size (lines * chars)
  // the output may be truncated due to the size constraint.
  bool setLogBuffer(unsigned short lines, unsigned short chars);
  // Draw the log buffer at position (x, y)
  void drawLogBuffer(unsigned short x, unsigned short y);
  // Get screen geometry
  unsigned short getWidth(void);
  unsigned short getHeight(void);
// Implement needed function to be compatible with Print class
    size_t write(uint8_t c);
    size_t write(const char* s);
  unsigned char *buffer;
#ifdef OLEDDISPLAY_DOUBLE_BUFFER
  unsigned char *buffer_back;
#endif
protected:
  OLEDDISPLAY_GEOMETRY geometry;
  unsigned short displayWidth;
  unsigned short displayHeight;
  unsigned short displayBufferSize;
  // Set the correct height, width and buffer for the geometry
  void setGeometry(OLEDDISPLAY_GEOMETRY g, unsigned short width = 0, unsigned short height = 0);
  OLEDDISPLAY_TEXT_ALIGNMENT textAlignment;
  OLEDDISPLAY_COLOR color;
  const unsigned char *fontData;
  // State values for logBuffer
  unsigned short logBufferSize;
  unsigned short logBufferFilled;
  unsigned short logBufferLine;
  unsigned short logBufferMaxLines;
  char *logBuffer;
  // Send a command to the display (low level function)
  virtual void sendCommand(unsigned char com) { (void)com; };
  // Connect to the display
  virtual bool connect() { return false; };
  // Send all the init commands
  void sendInitCommands();
  // converts utf8 characters to extended ascii
  char *utf8ascii(const std::string &s);
  char *utf8ascii(char *str);
  void inline drawInternal(short xMove, short yMove, short width, short height, const unsigned char *data, unsigned short offset, unsigned short bytesInData) __attribute__((always_inline));
  void drawStringInternal(short xMove, short yMove, char *text, unsigned short textLength, unsigned short textWidth);
  FontTableLookupFunction fontTableLookupFunction;
};

#endif
