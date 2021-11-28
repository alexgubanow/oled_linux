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
//SSD1306Spi(unsigned char rst, unsigned char dc, unsigned char cs
SSD1306Spi display(OLED_RST, OLED_DC, SPI1_CS, 1);

size_t curX = 0;
size_t curY = 0;
char timeSTR[9] = {'\0'};
std::string ipSTR;
bool isColor = true;
std::mutex mtx;
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
    delay(3);
    digitalWrite(FAN1, LOW);
    //digitalWrite(BUZZ1, LOW);
    //digitalWrite(BUZZ2, LOW);
    delay(5);
    digitalWrite(FAN1, HIGH);
    //digitalWrite(BUZZ1, HIGH);
    //digitalWrite(BUZZ2, HIGH);
  }  
}

#include "bmp280.h"
#include <signal.h>
void sig_handler(int);
int main()
{
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGSEGV, sig_handler);
  wiringPiSetup();
  //wiringPiSetupSys();
  /* pinModeAlt(WS_DI, PWM_OUTPUT);
  //pwmSetRange(1024);
  //pwmSetClock(10);
  //pwmWrite(WS_DI, 512);
  //pinMode(WS_DI, OUTPUT);
  while(true)
  {
  
    delay(100);
  digitalWrite(WS_DI, HIGH);
    delay(100);
  digitalWrite(WS_DI, LOW);
  } */
  //FAN1
  pinMode(FAN1, OUTPUT);
  pinMode(BUZZ1, OUTPUT);
  pinMode(BUZZ2, OUTPUT);
  pinMode(_595_CS, OUTPUT);
  digitalWrite(_595_CS, LOW);
  pinMode(SPI1_CS, OUTPUT);
  digitalWrite(SPI1_CS, LOW);
  pinMode(_3V3EN, OUTPUT);
  digitalWrite(_3V3EN, HIGH);
  printf("_3V3EN is up\n");
  digitalWrite(BUZZ1, LOW);
  digitalWrite(BUZZ2, LOW);

  wiringPiSPISetupMode(1, 1, 32000000, 0);
  std::thread fanControlTHREAD(fanControl);
  unsigned char buf = 0;
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
  std::string connStatus = exec("nmcli dev wifi | grep \'^\*\'");
  if (connStatus.length() == 0)
  {
    std::string knownConnectionsOUT = exec("nmcli c | grep wifi | grep -o \'^\\S*\'");
    printf("known connections:\n%s", knownConnectionsOUT.c_str());
    std::vector<std::string> knownConnections;
    split(knownConnectionsOUT, "\n", knownConnections);
    for (auto conn : knownConnections)
    {
      std::string connCMD = "nmcli c up id \'";
      connCMD += conn.c_str();
      connCMD += "\'";
      std::string connResult = exec(connCMD.c_str());
      if (connResult.find("Connection successfully activated") != std::string::npos)
      {
        printf("%s", connResult.c_str());
        break;
      }
    }
    if (knownConnections.size() == 0)
    {
      display.clear();
      display.drawString(0, 0, "choose SSID");
      display.display();
    }
  }
  getIP(ipSTR);
  if (ipSTR.length() == 0)
  {
    display.clear();
    display.drawString(0, 0, "No connection made");
    display.display();
  }
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

  fanControlTHREAD.join();
  return 0;
}
void sig_handler(int sig)
{
  switch (sig)
  {
  case SIGINT:
  case SIGTERM:
    fprintf(stdout, "terminating\n");
    /* digitalWrite(_595_CS, LOW);
    wiringPiSPIDataRW(1, {0x0}, 1);
    digitalWrite(_595_CS, HIGH); */
    //digitalWrite(_3V3EN, LOW);
    digitalWrite(BUZZ1, LOW);
    digitalWrite(BUZZ2, LOW);
    digitalWrite(FAN1, HIGH);
    break;
  case SIGSEGV:
    fprintf(stderr, "SIGSEGV\n");
    break;
  default:
    fprintf(stderr, "wasn't expecting that!\n");
    break;
  }
    abort();
}