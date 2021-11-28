/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
 * Copyright (c) 2018 by Fabrice Weinberg
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

#ifndef SSD1306Spi_h
#define SSD1306Spi_h

#include "OLEDDisplay.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>

unsigned char init_command[] =
    {
        // OLED_CMD_SET_PAGE_ADDR_MODE
        0xAE, 0xA8, 0x3F, 0xD3, 0x00, 0x40, 0xA1, 0xC8,
        0xD5, 0x80, 0xDA, 0x12, 0x81, 0xFF,
        0xA4, 0xDB, 0x40, 0x20, 0x02, 0x00, 0x10, 0x8D,
        0x14, 0x2E, 0xA6, 0xAF};
class SSD1306Spi : public OLEDDisplay
{
private:
  unsigned char _rst;
  unsigned char _dc;
  unsigned char _cs;
  unsigned char _SPIch;
  unsigned char _SPIport;

public:
  SSD1306Spi(unsigned char rst, unsigned char dc, unsigned char cs, unsigned char SPIch, unsigned char SPIport = 1, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64)
      : _rst(rst), _dc(dc), _cs(cs), _SPIch(SPIch), _SPIport(SPIport)
  {
    setGeometry(g);
  }

  bool connect()
  {
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    pinMode(_rst, OUTPUT);

    //wiringPiSPISetupMode(_SPIch, _SPIport, 32000000, 0);
    // Pulse Reset low for 10ms
    digitalWrite(_rst, HIGH);
    delay(1);
    digitalWrite(_rst, LOW);
    delay(10);
    digitalWrite(_rst, HIGH);
  /* wiringPiSetup();
  pinMode(_dc, OUTPUT);
  pinMode(_rst, OUTPUT);
  wiringPiSPISetupMode(1, 1, 32 * 1000 * 1000, 0);
  digitalWrite(_rst, HIGH);
  delay(50);
  digitalWrite(_rst, LOW);
  delay(50);
  digitalWrite(_rst, HIGH);
  digitalWrite(_dc, LOW);
  digitalWrite(_cs, LOW);
  wiringPiSPIDataRW(1, init_command, sizeof(init_command));
  digitalWrite(_cs, HIGH); */
    return true;
  }

  void display(void)
  {
#ifdef OLEDDISPLAY_DOUBLE_BUFFER
    unsigned char minBoundY = UINT8_MAX;
    unsigned char maxBoundY = 0;

    unsigned char minBoundX = UINT8_MAX;
    unsigned char maxBoundX = 0;

    unsigned char x, y;

    // Calculate the Y bounding box of changes
    // and copy buffer[pos] to buffer_back[pos];
    for (y = 0; y < (displayHeight / 8); y++)
    {
      for (x = 0; x < displayWidth; x++)
      {
        unsigned short pos = x + y * displayWidth;
        if (buffer[pos] != buffer_back[pos])
        {
          minBoundY = std::min(minBoundY, y);
          maxBoundY = std::max(maxBoundY, y);
          minBoundX = std::min(minBoundX, x);
          maxBoundX = std::max(maxBoundX, x);
        }
        buffer_back[pos] = buffer[pos];
      }
    }

    // If the minBoundY wasn't updated
    // we can savely assume that buffer_back[pos] == buffer[pos]
    // holdes true for all values of pos
    if (minBoundY == UINT8_MAX)
      return;

    
    /* sendCommand(COLUMNADDR);
    sendCommand(minBoundX);
    sendCommand(maxBoundX);

    sendCommand(PAGEADDR);
    sendCommand(minBoundY);
    sendCommand(maxBoundY); */
    
    unsigned char comArr[] = {COLUMNADDR, minBoundX, maxBoundX, PAGEADDR, minBoundY, maxBoundY};
    sendCommand((unsigned char *)&comArr, sizeof(comArr));
    digitalWrite(_cs, HIGH);
    digitalWrite(_dc, HIGH); // data mode
    digitalWrite(_cs, LOW);
    for (y = minBoundY; y <= maxBoundY; y++)
    {
      for (x = minBoundX; x <= maxBoundX; x++)
      {
        wiringPiSPIDataRW(_SPIport, &buffer[x + y * displayWidth], 1);
      }
    }
    digitalWrite(_cs, HIGH);
#else
    /* // No double buffering
    sendCommand(COLUMNADDR);
    sendCommand(0x0);
    sendCommand(0x7F);

    sendCommand(PAGEADDR);
    sendCommand(0x0);

    if (geometry == GEOMETRY_128_64 || geometry == GEOMETRY_64_48 || geometry == GEOMETRY_64_32)
    {
      sendCommand(0x7);
    }
    else if (geometry == GEOMETRY_128_32)
    {
      sendCommand(0x3);
    }
    digitalWrite(_dc, HIGH); // data mode
    digitalWrite(_cs, LOW);
    for (uint16_t i=0; i<displayBufferSize; i++) {
    wiringPiSPIDataRW(_SPIport, &buffer[i], 1);
        }
    wiringPiSPIDataRW(_SPIport, buffer, displayBufferSize);
    digitalWrite(_cs, HIGH); */
  unsigned char page_command[3];

  for (int _page = 0; _page < 8; _page++)
  {
    page_command[0] = 0x00 + 2;
    page_command[1] = 0x10;
    page_command[2] = 0xB0 + _page;
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    wiringPiSPIDataRW(1, page_command, sizeof(page_command));
    digitalWrite(_cs, HIGH);

    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    wiringPiSPIDataRW(1, &buffer[_page * 128], 128);
    digitalWrite(_cs, HIGH);
  }
#endif
  }

private:
  void sendCommand(unsigned char *com, size_t comL)
  {
    digitalWrite(_cs, HIGH);
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    wiringPiSPIDataRW(_SPIport, com, comL);
    digitalWrite(_cs, HIGH);
  }
  inline void sendCommand(unsigned char com) __attribute__((always_inline))
  {
    digitalWrite(_cs, HIGH);
    digitalWrite(_dc, LOW);
    digitalWrite(_cs, LOW);
    wiringPiSPIDataRW(_SPIport, &com, 1);
    digitalWrite(_cs, HIGH);
  }
};

#endif
