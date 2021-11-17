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

 /*
  * TODO Helmut
  * - test/finish dislplay.printf() on mbed-os
  * - Finish _putc with drawLogBuffer when running display
  */

#include "OLEDDisplay.h"
#include <string.h>

OLEDDisplay::OLEDDisplay() {

	displayWidth = 128;
	displayHeight = 64;
	displayBufferSize = displayWidth * displayHeight / 8;
	color = WHITE;
	geometry = GEOMETRY_128_64;
	textAlignment = TEXT_ALIGN_LEFT;
	fontData = ArialMT_Plain_10;
	fontTableLookupFunction = DefaultFontTableLookup;
	buffer = NULL;
#ifdef OLEDDISPLAY_DOUBLE_BUFFER
	buffer_back = NULL;
#endif
}

OLEDDisplay::~OLEDDisplay() {
  end();
}

bool OLEDDisplay::allocateBuffer() {

  logBufferSize = 0;
  logBufferFilled = 0;
  logBufferLine = 0;
  logBufferMaxLines = 0;
  logBuffer = NULL;

  if (!connect()) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Can't establish connection to display\n");
    return false;
  }

  if(this->buffer==NULL) {
    this->buffer = (unsigned char*) malloc((sizeof(unsigned char) * displayBufferSize) + BufferOffset);
    this->buffer += BufferOffset;

    if(!this->buffer) {
      DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create display\n");
      return false;
    }
  }

  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  if(this->buffer_back==NULL) {
    this->buffer_back = (unsigned char*) malloc((sizeof(unsigned char) * displayBufferSize) + BufferOffset);
    this->buffer_back += BufferOffset;

    if(!this->buffer_back) {
      DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create back buffer\n");
      free(this->buffer - BufferOffset);
      return false;
    }
  }
  #endif

  return true;
}

bool OLEDDisplay::init() {

  BufferOffset = getBufferOffset();

  if(!allocateBuffer()) {
    return false;
  }

  sendInitCommands();
  resetDisplay();

  return true;
}

void OLEDDisplay::end() {
  if (this->buffer) { free(this->buffer - BufferOffset); this->buffer = NULL; }
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  if (this->buffer_back) { free(this->buffer_back - BufferOffset); this->buffer_back = NULL; }
  #endif
  if (this->logBuffer != NULL) { free(this->logBuffer); this->logBuffer = NULL; }
}

void OLEDDisplay::resetDisplay(void) {
  clear();
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  memset(buffer_back, 1, displayBufferSize);
  #endif
  display();
}

void OLEDDisplay::setColor(OLEDDISPLAY_COLOR color) {
  this->color = color;
}

OLEDDISPLAY_COLOR OLEDDisplay::getColor() {
  return this->color;
}

void OLEDDisplay::setPixel(short x, short y) {
  if (x >= 0 && x < this->width() && y >= 0 && y < this->height()) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * this->width()] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * this->width()] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y / 8) * this->width()] ^=  (1 << (y & 7)); break;
    }
  }
}

void OLEDDisplay::setPixelColor(short x, short y, OLEDDISPLAY_COLOR color) {
  if (x >= 0 && x < this->width() && y >= 0 && y < this->height()) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * this->width()] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * this->width()] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y / 8) * this->width()] ^=  (1 << (y & 7)); break;
    }
  }
}

void OLEDDisplay::clearPixel(short x, short y) {
  if (x >= 0 && x < this->width() && y >= 0 && y < this->height()) {
    switch (color) {
      case BLACK:   buffer[x + (y >> 3) * this->width()] |=  (1 << (y & 7)); break;
      case WHITE:   buffer[x + (y >> 3) * this->width()] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y >> 3) * this->width()] ^=  (1 << (y & 7)); break;
    }
  }
}


// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void OLEDDisplay::drawLine(short x0, short y0, short x1, short y1) {
  short steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_short(x0, y0);
    _swap_short(x1, y1);
  }

  if (x0 > x1) {
    _swap_short(x0, x1);
    _swap_short(y0, y1);
  }

  short dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  short err = dx / 2;
  short ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      setPixel(y0, x0);
    } else {
      setPixel(x0, y0);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void OLEDDisplay::drawRect(short x, short y, short width, short height) {
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width - 1, y, height);
  drawHorizontalLine(x, y + height - 1, width);
}

void OLEDDisplay::fillRect(short xMove, short yMove, short width, short height) {
  for (short x = xMove; x < xMove + width; x++) {
    drawVerticalLine(x, yMove, height);
  }
}

void OLEDDisplay::drawCircle(short x0, short y0, short radius) {
  short x = 0, y = radius;
	short dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + (x++) * 2 + 3;
		else
			dp = dp + (x++) * 2 - (y--) * 2 + 5;

		setPixel(x0 + x, y0 + y);     //For the 8 octants
		setPixel(x0 - x, y0 + y);
		setPixel(x0 + x, y0 - y);
		setPixel(x0 - x, y0 - y);
		setPixel(x0 + y, y0 + x);
		setPixel(x0 - y, y0 + x);
		setPixel(x0 + y, y0 - x);
		setPixel(x0 - y, y0 - x);

	} while (x < y);

  setPixel(x0 + radius, y0);
  setPixel(x0, y0 + radius);
  setPixel(x0 - radius, y0);
  setPixel(x0, y0 - radius);
}

void OLEDDisplay::drawCircleQuads(short x0, short y0, short radius, unsigned char quads) {
  short x = 0, y = radius;
  short dp = 1 - radius;
  while (x < y) {
    if (dp < 0)
      dp = dp + (x++) * 2 + 3;
    else
      dp = dp + (x++) * 2 - (y--) * 2 + 5;
    if (quads & 0x1) {
      setPixel(x0 + x, y0 - y);
      setPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2) {
      setPixel(x0 - y, y0 - x);
      setPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4) {
      setPixel(x0 - y, y0 + x);
      setPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8) {
      setPixel(x0 + x, y0 + y);
      setPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8) {
    setPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8) {
    setPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4) {
    setPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2) {
    setPixel(x0, y0 - radius);
  }
}


void OLEDDisplay::fillCircle(short x0, short y0, short radius) {
  short x = 0, y = radius;
	short dp = 1 - radius;
	do {
		if (dp < 0)
      dp = dp + (x++) * 2 + 3;
    else
      dp = dp + (x++) * 2 - (y--) * 2 + 5;

    drawHorizontalLine(x0 - x, y0 - y, 2*x);
    drawHorizontalLine(x0 - x, y0 + y, 2*x);
    drawHorizontalLine(x0 - y, y0 - x, 2*y);
    drawHorizontalLine(x0 - y, y0 + x, 2*y);


	} while (x < y);
  drawHorizontalLine(x0 - radius, y0, 2 * radius);

}

void OLEDDisplay::drawTriangle(short x0, short y0, short x1, short y1,
                               short x2, short y2) {
  drawLine(x0, y0, x1, y1);
  drawLine(x1, y1, x2, y2);
  drawLine(x2, y2, x0, y0);
}

void OLEDDisplay::fillTriangle(short x0, short y0, short x1, short y1,
                               short x2, short y2) {
  short a, b, y, last;

  if (y0 > y1) {
    _swap_short(y0, y1);
    _swap_short(x0, x1);
  }
  if (y1 > y2) {
    _swap_short(y2, y1);
    _swap_short(x2, x1);
  }
  if (y0 > y1) {
    _swap_short(y0, y1);
    _swap_short(x0, x1);
  }

  if (y0 == y2) {
    a = b = x0;
    if (x1 < a) {
      a = x1;
    } else if (x1 > b) {
      b = x1;
    }
    if (x2 < a) {
      a = x2;
    } else if (x2 > b) {
      b = x2;
    }
    drawHorizontalLine(a, y0, b - a + 1);
    return;
  }

  short
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
  int32_t
    sa   = 0,
    sb   = 0;

  if (y1 == y2) {
    last = y1; // Include y1 scanline
  } else {
    last = y1 - 1; // Skip it
  }

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b) {
      _swap_short(a, b);
    }
    drawHorizontalLine(a, y, b - a + 1);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b) {
      _swap_short(a, b);
    }
    drawHorizontalLine(a, y, b - a + 1);
  }
}

void OLEDDisplay::drawHorizontalLine(short x, short y, short length) {
  if (y < 0 || y >= this->height()) { return; }

  if (x < 0) {
    length += x;
    x = 0;
  }

  if ( (x + length) > this->width()) {
    length = (this->width() - x);
  }

  if (length <= 0) { return; }

  unsigned char * bufferPtr = buffer;
  bufferPtr += (y >> 3) * this->width();
  bufferPtr += x;

  unsigned char drawBit = 1 << (y & 7);

  switch (color) {
    case WHITE:   while (length--) {
        *bufferPtr++ |= drawBit;
      }; break;
    case BLACK:   drawBit = ~drawBit;   while (length--) {
        *bufferPtr++ &= drawBit;
      }; break;
    case INVERSE: while (length--) {
        *bufferPtr++ ^= drawBit;
      }; break;
  }
}

void OLEDDisplay::drawVerticalLine(short x, short y, short length) {
  if (x < 0 || x >= this->width()) return;

  if (y < 0) {
    length += y;
    y = 0;
  }

  if ( (y + length) > this->height()) {
    length = (this->height() - y);
  }

  if (length <= 0) return;


  unsigned char yOffset = y & 7;
  unsigned char drawBit;
  unsigned char *bufferPtr = buffer;

  bufferPtr += (y >> 3) * this->width();
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += this->width();
  }

  if (length >= 8) {
    switch (color) {
      case WHITE:
      case BLACK:
        drawBit = (color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += this->width();
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << (length & 7)) - 1;
    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }
  }
}

void OLEDDisplay::drawProgressBar(unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char progress) {
  unsigned short radius = height / 2;
  unsigned short xRadius = x + radius;
  unsigned short yRadius = y + radius;
  unsigned short doubleRadius = 2 * radius;
  unsigned short innerRadius = radius - 2;

  setColor(WHITE);
  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
  drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  unsigned short maxProgressWidth = (width - doubleRadius + 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

void OLEDDisplay::drawFastImage(short xMove, short yMove, short width, short height, const unsigned char *image) {
  drawInternal(xMove, yMove, width, height, image, 0, 0);
}

void OLEDDisplay::drawXbm(short xMove, short yMove, short width, short height, const unsigned char *xbm) {
  short widthInXbm = (width + 7) / 8;
  unsigned char data = 0;

  for(short y = 0; y < height; y++) {
    for(short x = 0; x < width; x++ ) {
      if (x & 7) {
        data >>= 1; // Move a bit
      } else {  // Read new data every 8 bit
        data = pgm_read_byte(xbm + (x / 8) + y * widthInXbm);
      }
      // if there is a bit draw it
      if (data & 0x01) {
        setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void OLEDDisplay::drawIco16x16(short xMove, short yMove, const unsigned char *ico, bool inverse) {
  unsigned short data;

  for(short y = 0; y < 16; y++) {
    data = pgm_read_byte(ico + (y << 1)) + (pgm_read_byte(ico + (y << 1) + 1) << 8);
    for(short x = 0; x < 16; x++ ) {
      if ((data & 0x01) ^ inverse) {
        setPixelColor(xMove + x, yMove + y, WHITE);
      } else {
        setPixelColor(xMove + x, yMove + y, BLACK);
      }
      data >>= 1; // Move a bit
    }
  }
}

void OLEDDisplay::drawstd::stringInternal(short xMove, short yMove, char* text, unsigned short textLength, unsigned short textWidth) {
  unsigned char textHeight       = pgm_read_byte(fontData + HEIGHT_POS);
  unsigned char firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);
  unsigned short sizeOfJumpTable = pgm_read_byte(fontData + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  unsigned short cursorX         = 0;
  unsigned short cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      yMove -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      xMove -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      xMove -= textWidth;
      break;
    case TEXT_ALIGN_LEFT:
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth  < 0 || xMove > this->width() ) {return;}
  if (yMove + textHeight < 0 || yMove > this->width() ) {return;}

  for (unsigned short j = 0; j < textLength; j++) {
    short xPos = xMove + cursorX;
    short yPos = yMove + cursorY;

    unsigned char code = text[j];
    if (code >= firstChar) {
      unsigned char charCode = code - firstChar;

      // 4 Bytes per char code
      unsigned char msbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      unsigned char lsbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      unsigned char charByteSize     = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      unsigned char currentCharWidth = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(msbJumpToChar == 255 && lsbJumpToChar == 255)) {
        // Get the position of the char data
        unsigned short charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
      }

      cursorX += currentCharWidth;
    }
  }
}


void OLEDDisplay::drawstd::string(short xMove, short yMove, const std::string &strUser) {
  unsigned short lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  // char* text must be freed!
  char* text = utf8ascii(strUser);

  unsigned short yOffset = 0;
  // If the std::string should be centered vertically too
  // we need to now how heigh the std::string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) {
    unsigned short lb = 0;
    // Find number of linebreaks in text
    for (unsigned short i=0;text[i] != 0; i++) {
      lb += (text[i] == 10);
    }
    // Calculate center
    yOffset = (lb * lineHeight) / 2;
  }

  unsigned short line = 0;
  char* textPart = strtok(text,"\n");
  while (textPart != NULL) {
    unsigned short length = strlen(textPart);
    drawstd::stringInternal(xMove, yMove - yOffset + (line++) * lineHeight, textPart, length, getstd::stringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }
  free(text);
}

void OLEDDisplay::drawstd::stringf( short x, short y, char* buffer, std::string format, ... )
{
  va_list myargs;
  va_start(myargs, format);
  vsprintf(buffer, format.c_str(), myargs);
  va_end(myargs);
  drawstd::string( x, y, buffer );
}

void OLEDDisplay::drawstd::stringMaxWidth(short xMove, short yMove, unsigned short maxLineWidth, const std::string &strUser) {
  unsigned short firstChar  = pgm_read_byte(fontData + FIRST_CHAR_POS);
  unsigned short lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  char* text = utf8ascii(strUser);

  unsigned short length = strlen(text);
  unsigned short lastDrawnPos = 0;
  unsigned short lineNumber = 0;
  unsigned short strWidth = 0;

  unsigned short preferredBreakpoint = 0;
  unsigned short widthAtBreakpoint = 0;

  for (unsigned short i = 0; i < length; i++) {
    strWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[i] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (text[i] == ' ' || text[i]== '-') {
      preferredBreakpoint = i;
      widthAtBreakpoint = strWidth;
    }

    if (strWidth >= maxLineWidth) {
      if (preferredBreakpoint == 0) {
        preferredBreakpoint = i;
        widthAtBreakpoint = strWidth;
      }
      drawstd::stringInternal(xMove, yMove + (lineNumber++) * lineHeight , &text[lastDrawnPos], preferredBreakpoint - lastDrawnPos, widthAtBreakpoint);
      lastDrawnPos = preferredBreakpoint + 1;
      // It is possible that we did not draw all letters to i so we need
      // to account for the width of the chars from `i - preferredBreakpoint`
      // by calculating the width we did not draw yet.
      strWidth = strWidth - widthAtBreakpoint;
      preferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (lastDrawnPos < length) {
    drawstd::stringInternal(xMove, yMove + lineNumber * lineHeight , &text[lastDrawnPos], length - lastDrawnPos, getstd::stringWidth(&text[lastDrawnPos], length - lastDrawnPos));
  }

  free(text);
}

unsigned short OLEDDisplay::getstd::stringWidth(const char* text, unsigned short length) {
  unsigned short firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);

  unsigned short std::stringWidth = 0;
  unsigned short maxWidth = 0;

  while (length--) {
    std::stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10) {
      maxWidth = max(maxWidth, std::stringWidth);
      std::stringWidth = 0;
    }
  }

  return max(maxWidth, std::stringWidth);
}

unsigned short OLEDDisplay::getstd::stringWidth(const std::string &strUser) {
  char* text = utf8ascii(strUser);
  unsigned short length = strlen(text);
  unsigned short width = getstd::stringWidth(text, length);
  free(text);
  return width;
}

void OLEDDisplay::setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) {
  this->textAlignment = textAlignment;
}

void OLEDDisplay::setFont(const unsigned char *fontData) {
  this->fontData = fontData;
}

void OLEDDisplay::displayOn(void) {
  sendCommand(DISPLAYON);
}

void OLEDDisplay::displayOff(void) {
  sendCommand(DISPLAYOFF);
}

void OLEDDisplay::invertDisplay(void) {
  sendCommand(INVERTDISPLAY);
}

void OLEDDisplay::normalDisplay(void) {
  sendCommand(NORMALDISPLAY);
}

void OLEDDisplay::setContrast(unsigned char contrast, unsigned char precharge, unsigned char comdetect) {
  sendCommand(SETPRECHARGE); //0xD9
  sendCommand(precharge); //0xF1 default, to lower the contrast, put 1-1F
  sendCommand(SETCONTRAST);
  sendCommand(contrast); // 0-255
  sendCommand(SETVCOMDETECT); //0xDB, (additionally needed to lower the contrast)
  sendCommand(comdetect);	//0x40 default, to lower the contrast, put 0
  sendCommand(DISPLAYALLON_RESUME);
  sendCommand(NORMALDISPLAY);
  sendCommand(DISPLAYON);
}

void OLEDDisplay::setBrightness(unsigned char brightness) {
  unsigned char contrast = brightness;
  if (brightness < 128) {
    // Magic values to get a smooth/ step-free transition
    contrast = brightness * 1.171;
  } else {
    contrast = brightness * 1.171 - 43;
  }

  unsigned char precharge = 241;
  if (brightness == 0) {
    precharge = 0;
  }
  unsigned char comdetect = brightness / 8;

  setContrast(contrast, precharge, comdetect);
}

void OLEDDisplay::resetOrientation() {
  sendCommand(SEGREMAP);
  sendCommand(COMSCANINC);           //Reset screen rotation or mirroring
}

void OLEDDisplay::flipScreenVertically() {
  sendCommand(SEGREMAP | 0x01);
  sendCommand(COMSCANDEC);           //Rotate screen 180 Deg
}

void OLEDDisplay::mirrorScreen() {
  sendCommand(SEGREMAP);
  sendCommand(COMSCANDEC);           //Mirror screen
}

void OLEDDisplay::clear(void) {
  memset(buffer, 0, displayBufferSize);
}

void OLEDDisplay::drawLogBuffer(unsigned short xMove, unsigned short yMove) {
  unsigned short lineHeight = pgm_read_byte(fontData + HEIGHT_POS);
  // Always align left
  setTextAlignment(TEXT_ALIGN_LEFT);

  // State values
  unsigned short length   = 0;
  unsigned short line     = 0;
  unsigned short lastPos  = 0;

  for (unsigned short i=0;i<this->logBufferFilled;i++){
    // Everytime we have a \n print
    if (this->logBuffer[i] == 10) {
      length++;
      // Draw std::string on line `line` from lastPos to length
      // Passing 0 as the lenght because we are in TEXT_ALIGN_LEFT
      drawstd::stringInternal(xMove, yMove + (line++) * lineHeight, &this->logBuffer[lastPos], length, 0);
      // Remember last pos
      lastPos = i;
      // Reset length
      length = 0;
    } else {
      // Count chars until next linebreak
      length++;
    }
  }
  // Draw the remaining std::string
  if (length > 0) {
    drawstd::stringInternal(xMove, yMove + line * lineHeight, &this->logBuffer[lastPos], length, 0);
  }
}

unsigned short OLEDDisplay::getWidth(void) {
  return displayWidth;
}

unsigned short OLEDDisplay::getHeight(void) {
  return displayHeight;
}

bool OLEDDisplay::setLogBuffer(unsigned short lines, unsigned short chars){
  if (logBuffer != NULL) free(logBuffer);
  unsigned short size = lines * chars;
  if (size > 0) {
    this->logBufferLine     = 0;      // Lines printed
    this->logBufferFilled   = 0;      // Nothing stored yet
    this->logBufferMaxLines = lines;  // Lines max printable
    this->logBufferSize     = size;   // Total number of characters the buffer can hold
    this->logBuffer         = (char *) malloc(size * sizeof(unsigned char));
    if(!this->logBuffer) {
      DEBUG_OLEDDISPLAY("[OLEDDISPLAY][setLogBuffer] Not enough memory to create log buffer\n");
      return false;
    }
  }
  return true;
}

size_t OLEDDisplay::write(unsigned char c) {
  if (this->logBufferSize > 0) {
    // Don't waste space on \r\n line endings, dropping \r
    if (c == 13) return 1;

    // convert UTF-8 character to font table index
    c = (this->fontTableLookupFunction)(c);
    // drop unknown character
    if (c == 0) return 1;

    bool maxLineNotReached = this->logBufferLine < this->logBufferMaxLines;
    bool bufferNotFull = this->logBufferFilled < this->logBufferSize;

    // Can we write to the buffer?
    if (bufferNotFull && maxLineNotReached) {
      this->logBuffer[logBufferFilled] = c;
      this->logBufferFilled++;
      // Keep track of lines written
      if (c == 10) this->logBufferLine++;
    } else {
      // Max line number is reached
      if (!maxLineNotReached) this->logBufferLine--;

      // Find the end of the first line
      unsigned short firstLineEnd = 0;
      for (unsigned short i=0;i<this->logBufferFilled;i++) {
        if (this->logBuffer[i] == 10){
          // Include last char too
          firstLineEnd = i + 1;
          break;
        }
      }
      // If there was a line ending
      if (firstLineEnd > 0) {
        // Calculate the new logBufferFilled value
        this->logBufferFilled = logBufferFilled - firstLineEnd;
        // Now we move the lines infront of the buffer
        memcpy(this->logBuffer, &this->logBuffer[firstLineEnd], logBufferFilled);
      } else {
        // Let's reuse the buffer if it was full
        if (!bufferNotFull) {
          this->logBufferFilled = 0;
        }// else {
        //  Nothing to do here
        //}
      }
      write(c);
    }
  }
  // We are always writing all unsigned char to the buffer
  return 1;
}

size_t OLEDDisplay::write(const char* str) {
  if (str == NULL) return 0;
  size_t length = strlen(str);
  for (size_t i = 0; i < length; i++) {
    write(str[i]);
  }
  return length;
}

#ifdef __MBED__
int OLEDDisplay::_putc(int c) {

	if (!fontData)
		return 1;
	if (!logBufferSize) {
		unsigned char textHeight = pgm_read_byte(fontData + HEIGHT_POS);
		unsigned short lines =  this->displayHeight / textHeight;
		unsigned short chars =   2 * (this->displayWidth / textHeight);

		if (this->displayHeight % textHeight)
			lines++;
		if (this->displayWidth % textHeight)
			chars++;
		setLogBuffer(lines, chars);
	}

	return this->write((unsigned char)c);
}
#endif

// Private functions
void OLEDDisplay::setGeometry(OLEDDISPLAY_GEOMETRY g, unsigned short width, unsigned short height) {
  this->geometry = g;

  switch (g) {
    case GEOMETRY_128_64:
      this->displayWidth = 128;
      this->displayHeight = 64;
      break;
    case GEOMETRY_128_32:
      this->displayWidth = 128;
      this->displayHeight = 32;
      break;
    case GEOMETRY_64_48:
      this->displayWidth = 64;
      this->displayHeight = 48;
      break;
    case GEOMETRY_64_32:
      this->displayWidth = 64;
      this->displayHeight = 32;
      break;
    case GEOMETRY_RAWMODE:
      this->displayWidth = width > 0 ? width : 128;
      this->displayHeight = height > 0 ? height : 64;
      break;
  }
  this->displayBufferSize = displayWidth * displayHeight / 8;
}

void OLEDDisplay::sendInitCommands(void) {
  if (geometry == GEOMETRY_RAWMODE)
  	return;
  sendCommand(DISPLAYOFF);
  sendCommand(SETDISPLAYCLOCKDIV);
  sendCommand(0xF0); // Increase speed of the display max ~96Hz
  sendCommand(SETMULTIPLEX);
  sendCommand(this->height() - 1);
  sendCommand(SETDISPLAYOFFSET);
  sendCommand(0x00);
  if(geometry == GEOMETRY_64_32)
    sendCommand(0x00);
  else
    sendCommand(SETSTARTLINE);
  sendCommand(CHARGEPUMP);
  sendCommand(0x14);
  sendCommand(MEMORYMODE);
  sendCommand(0x00);
  sendCommand(SEGREMAP);
  sendCommand(COMSCANINC);
  sendCommand(SETCOMPINS);

  if (geometry == GEOMETRY_128_64 || geometry == GEOMETRY_64_48 || geometry == GEOMETRY_64_32) {
    sendCommand(0x12);
  } else if (geometry == GEOMETRY_128_32) {
    sendCommand(0x02);
  }

  sendCommand(SETCONTRAST);

  if (geometry == GEOMETRY_128_64 || geometry == GEOMETRY_64_48 || geometry == GEOMETRY_64_32) {
    sendCommand(0xCF);
  } else if (geometry == GEOMETRY_128_32) {
    sendCommand(0x8F);
  }

  sendCommand(SETPRECHARGE);
  sendCommand(0xF1);
  sendCommand(SETVCOMDETECT); //0xDB, (additionally needed to lower the contrast)
  sendCommand(0x40);	        //0x40 default, to lower the contrast, put 0
  sendCommand(DISPLAYALLON_RESUME);
  sendCommand(NORMALDISPLAY);
  sendCommand(0x2e);            // stop scroll
  sendCommand(DISPLAYON);
}

void inline OLEDDisplay::drawInternal(short xMove, short yMove, short width, short height, const unsigned char *data, unsigned short offset, unsigned short bytesInData) {
  if (width < 0 || height < 0) return;
  if (yMove + height < 0 || yMove > this->height())  return;
  if (xMove + width  < 0 || xMove > this->width())   return;

  unsigned char  rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  short initYMove   = yMove;
  int8_t  initYOffset = yOffset;


  for (unsigned short i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      yMove   = initYMove;
      yOffset = initYOffset;
    }

    unsigned char currentByte = pgm_read_byte(data + offset + i);

    short xPos = xMove + (i / rasterHeight);
    short yPos = ((yMove >> 3) + (i % rasterHeight)) * this->width();

//    short yScreenPos = yMove + yOffset;
    short dataPos    = xPos  + yPos;

    if (dataPos >=  0  && dataPos < displayBufferSize &&
        xPos    >=  0  && xPos    < this->width() ) {

      if (yOffset >= 0) {
        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte << yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte << yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte << yOffset; break;
        }

        if (dataPos < (displayBufferSize - this->width())) {
          switch (this->color) {
            case WHITE:   buffer[dataPos + this->width()] |= currentByte >> (8 - yOffset); break;
            case BLACK:   buffer[dataPos + this->width()] &= ~(currentByte >> (8 - yOffset)); break;
            case INVERSE: buffer[dataPos + this->width()] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte >> yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }
#ifndef __MBED__
      yield();
#endif
    }
  }
}

// You need to free the char!
char* OLEDDisplay::utf8ascii(const std::string &str) {
  unsigned short k = 0;
  unsigned short length = str.length() + 1;

  // Copy the std::string into a char array
  char* s = (char*) malloc(length * sizeof(char));
  if(!s) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n");
    return (char*) str.c_str();
  }
  str.toCharArray(s, length);

  length--;

  for (unsigned short i=0; i < length; i++) {
    char c = (this->fontTableLookupFunction)(s[i]);
    if (c!=0) {
      s[k++]=c;
    }
  }

  s[k]=0;

  // This will leak 's' be sure to free it in the calling function.
  return s;
}

void OLEDDisplay::setFontTableLookupFunction(FontTableLookupFunction function) {
  this->fontTableLookupFunction = function;
}


char DefaultFontTableLookup(const unsigned char ch) {
    // UTF-8 to font table index converter
    // Code form http://playground.arduino.cc/Main/Utf8ascii
	static unsigned char LASTCHAR;

	if (ch < 128) { // Standard ASCII-set 0..0x7F handling
		LASTCHAR = 0;
		return ch;
	}

	unsigned char last = LASTCHAR;   // get last char
	LASTCHAR = ch;

	switch (last) {    // conversion depnding on first UTF8-character
		case 0xC2: return (unsigned char) ch;
		case 0xC3: return (unsigned char) (ch | 0xC0);
		case 0x82: if (ch == 0xAC) return (unsigned char) 0x80;    // special case Euro-symbol
	}

	return (unsigned char) 0; // otherwise: return zero, if character has to be ignored
}
