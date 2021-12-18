#include "SSD1306Spi.h"
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <unistd.h>
#include <termios.h>
#include <wiringPi.h>
#include <OrangePi.h>

#include "utils.h"
#include "pins.h"
#include "qrcode.h"
#include "bmp280.h"
#include <signal.h>
#include <math.h>
#include <softPwm.h>
#include <softTone.h>
//SSD1306Spi(unsigned char rst, unsigned char dc, unsigned char cs
SSD1306Spi display(OLED_RST, OLED_DC, SPI1_CS, 1);

size_t curX = 0;
size_t curY = 0;
char timeSTR[9] = {'\0'};
std::string ipSTR;
bool isColor = true;
std::mutex mtx;

int hardwareInit();
void sig_handler(int);
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  1203
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 12045
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

int tempo[160] = {
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,

  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,

  90, 90, 90,
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,

  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,

  90, 90, 90,
  120, 120, 120, 120,
  120, 120, 120, 120,
  120, 120, 120, 120,
}; 

int scale [160] = { NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0 } ;
#define cL 129
#define cLS 139
#define dL 146
#define dLS 156
#define eL 163
#define fL 173
#define fLS 185
#define gL 194
#define gLS 207
#define aL 219
#define aLS 228
#define bL 232
 
#define c 261
#define cS 277
#define d 294
#define dS 311
#define e 329
#define f 349
#define fS 370
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
 
#define cH 523
#define cHS 554
#define dH 587
#define dHS 622
#define eH 659
#define fH 698
#define fHS 740
#define gH 784
#define gHS 830
#define aH 880
#define aHS 910
#define bH 933
 
//This function generates the square wave that makes the piezo speaker sound at a determinated frequency.
void beep(unsigned int note, unsigned int duration)
{
  //This is the semiperiod of each note.
  long beepDelay = (long)(1000000/note);
  //This is how much time we need to spend on the note.
  long time = (long)((duration*1000)/(beepDelay*2));
  int i;
  for(i=0;i<time;i++)
  {
    //1st semiperiod
    digitalWrite(BUZZ2, HIGH);
    delayMicroseconds(beepDelay);
    //2nd semiperiod
    digitalWrite(BUZZ2, LOW);
    delayMicroseconds(beepDelay);
  }
 
  //Add a little delay to separate the single notes
  digitalWrite(BUZZ2, LOW);
  delay(20);
}
 
//The source code of the Imperial March from Star Wars
void play()
{
  beep( a, 500);
  beep( a, 500);
  beep( f, 350);
  beep( cH, 150);
 
  beep( a, 500);
  beep( f, 350);
  beep( cH, 150);
  beep( a, 1000);
  beep( eH, 500);
 
  beep( eH, 500);
  beep( eH, 500);
  beep( fH, 350);
  beep( cH, 150);
  beep( gS, 500);
 
  beep( f, 350);
  beep( cH, 150);
  beep( a, 1000);
  beep( aH, 500);
  beep( a, 350);
 
  beep( a, 150);
  beep( aH, 500);
  beep( gHS, 250);
  beep( gH, 250);
  beep( fHS, 125);
 
  beep( fH, 125);
  beep( fHS, 250);
 
  delay(250);
 
  beep( aS, 250);
  beep( dHS, 500);
  beep( dH, 250);
  beep( cHS, 250);
  beep( cH, 125);
 
  beep( b, 125);
  beep( cH, 250);
 
  delay(250);
 
  beep( f, 125);
  beep( gS, 500);
  beep( f, 375);
  beep( a, 125);
  beep( cH, 500);
 
  beep( a, 375);
  beep( cH, 125);
  beep( eH, 1000);
  beep( aH, 500);
  beep( a, 350);
 
  beep( a, 150);
  beep( aH, 500);
  beep( gHS, 250);
  beep( gH, 250);
  beep( fHS, 125);
 
  beep( fH, 125);
  beep( fHS, 250);
 
  delay(250);
 
  beep( aS, 250);
  beep( dHS, 500);
  beep( dH, 250);
  beep( cHS, 250);
  beep( cH, 125);
 
  beep( b, 125);
  beep( cH, 250);
 
  delay(250);
 
  beep( f, 250);
  beep( gS, 500);
  beep( f, 375);
  beep( cH, 125);
  beep( a, 500);
 
  beep( f, 375);
  beep( c, 125);
  beep( a, 1000);
}
int main()
{
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGSEGV, sig_handler);
  hardwareInit();
  softPwmCreate(FAN1, 256, 1024);
  unsigned char marioTones[] = {659, 659, 0, 659, 0, 523, 659, 0, 784, 0, 0, 0, 392, 0, 0, 0, 523, 0, 0, 392, 0, 0, 330};
  //softToneCreate(BUZZ1);
  softToneCreate(BUZZ2);
/*   for (size_t i,j = 0 ; i < 160 ; ++i)
    {
      if (scale[i] >= 1) {j = 1;}
      else {j = 0;}

      //softToneWrite (BUZZ1, scale [i]);
      softToneWrite (BUZZ2, scale [i]);
      delay (tempo [i]) ;
    } */
    play();
  //softToneStop(BUZZ1);
  softToneStop(BUZZ2);
  
  //softPwmWrite(FAN1, 100);
  //digitalWrite(WS_DI, HIGH);
  //pwm();
  //std::thread fanControlTHREAD(fanControl);
  //std::thread pwmTHREAD(pwm);
  unsigned char buf = 0xFF;
  //buf |= 1<<3;
  digitalWrite(_595_CS, LOW);
  wiringPiSPIDataRW(1, &buf, 1);
  digitalWrite(_595_CS, HIGH);
  //talkToBMP();
  // Initialising the UI will init the display too.
  display.init();
  display.clear();
  //display.drawRect(0,0,25,25);
  display.flipScreenVertically();
  //display.setFont(Monospaced_plain_10);
  /*   getIP(ipSTR);
  if (ipSTR.length() == 0)
  {
    display.clear();
    display.drawString(0, 0, "No connection made");
    display.display();
  } */
  for (size_t i = 0; i < 230; i++)
  {
    display.fillRect(qr_code_OPIZ21[i][0] + 80, qr_code_OPIZ21[i][1], 2, 2);
  }
  display.setFont(Monospaced_plain_16);
  display.drawString(0, 0, "OPiZ21");
  display.setFont(Monospaced_plain_10);
  display.drawString(0, 24, "scan qr code");
  display.drawString(0, 34, "to continue");
  display.display();
  /* std::thread keyPressTHREAD(keyPress);
  std::thread currBlinkTHREAD(currBlink);
  std::thread drawFrameTHREAD(drawFrame);
  keyPressTHREAD.detach();
  currBlinkTHREAD.detach();
  drawFrameTHREAD.detach();
  while (1)
  {
    std::time_t t = std::time(0); // get time now
    std::tm *now = std::localtime(&t);
    mtx.lock();
    std::sprintf(timeSTR, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    mtx.unlock();
    delay(100);
  } */

  //fanControlTHREAD.join();
  return 0;
}
int hardwareInit()
{
  int rc = 0;
  rc = wiringPiSetup();
  if (rc)
  {
    printf("wiringPiSetup failed\n");
    abort();
  }
  rc = wiringPiSPISetupMode(1, 1, 32000000, 3);
  if (rc == 0)
  {
    printf("wiringPiSPISetupMode failed\n");
    abort();
  }
  pinMode(WS_DI, OUTPUT);
  //pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);
  //pinMode(BUZZ1, OUTPUT);
  pinMode(BUZZ2, OUTPUT);
  digitalWrite(BUZZ1, LOW);
  digitalWrite(BUZZ2, LOW);
  pinMode(_595_CS, OUTPUT);
  digitalWrite(_595_CS, HIGH);
  pinMode(SPI1_CS, OUTPUT);
  digitalWrite(SPI1_CS, HIGH);
  pinMode(_3V3EN, OUTPUT);
  digitalWrite(_3V3EN, HIGH);
  printf("_3V3EN is up\n");
  return 0;
}
void sig_handler(int sig)
{
  switch (sig)
  {
  case SIGINT:
  case SIGTERM:
    fprintf(stdout, "\nTerminating\n");
    /* digitalWrite(_595_CS, LOW);
    wiringPiSPIDataRW(1, {0x0}, 1);
    digitalWrite(_595_CS, HIGH); */
    //digitalWrite(_3V3EN, LOW);
    digitalWrite(BUZZ1, LOW);
    digitalWrite(BUZZ2, LOW);
    //digitalWrite(FAN1, HIGH);
    break;
  case SIGSEGV:
    fprintf(stderr, "\nSIGSEGV\n");
    break;
  default:
    fprintf(stderr, "\nwasn't expecting that!\n");
    break;
  }
  abort();
}
void drawFrame()
{
  while (1)
  {
    display.clear();
    mtx.lock();
    display.drawString(0, 0, timeSTR);
    display.drawString(0, 20, ipSTR);
    mtx.unlock();
    display.display();
    delay(20);
  }
}

void fanControl()
{
  while (true)
  {
    delay(1);
    digitalWrite(FAN1, HIGH);
    digitalWrite(FAN2, HIGH);
    //digitalWrite(BUZZ1, LOW);
    //digitalWrite(BUZZ2, LOW);
    delay(5);
    digitalWrite(FAN1, HIGH);
    digitalWrite(FAN2, HIGH);
    //digitalWrite(BUZZ1, HIGH);
    //digitalWrite(BUZZ2, HIGH);
  }
}
void pwm()
{
  while (true)
  {
    uint8_t buf[] = {0, 0, 0};
    size_t i = 0;

    uint32_t regval = *(OrangePi_gpio + 0x00000016);
    //printf("0x%08X\n");
    register uint32_t loop = 8 * sizeof(buf); // one loop to handle all bytes and all bits
    register uint8_t *p = buf;
    register uint32_t currByte = (uint32_t)(*p);
    register uint32_t currBit = 0x80 & currByte;
    register uint32_t bitCounter = 0;
    register uint32_t first = 1;
    while (loop--)
    {
      if (!first)
      {
        currByte <<= 1;
        bitCounter++;
      }
      // 1 is >550ns high and >450ns low; 0 is 200..500ns high and >450ns low

      regval = *(OrangePi_gpio + 0x00000016);
      regval |= (1 << 6);
      *(OrangePi_gpio + 0x00000016) = regval;
      if (currBit)
      { // ~400ns HIGH (740ns overall)
        i = 800;
        while (i--)
        {
          asm volatile("nop");
        }
      }
      i = 100;
      while (i--)
      {
        asm volatile("nop");
      }

      // 820ns LOW; per spec, max allowed low here is 5000ns //
      regval = *(OrangePi_gpio + 0x00000016);
      regval &= ~(1 << 6);
      *(OrangePi_gpio + 0x00000016) = regval;

      i = 800;
      while (i--)
      {
        asm volatile("nop");
      }
      if (bitCounter >= 8)
      {
        bitCounter = 0;
        currByte = (uint32_t)(*++p);
      }

      currBit = 0x80 & currByte;
      first = 0;
    }
  }
}