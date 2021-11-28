#include "SSD1306Spi.h"
#include <ctime>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <termios.h>
#include <wiringPi.h>
#include <OrangePi.h>

#include "pins.h"
#include "qrcode.h"
//SSD1306Spi(unsigned char rst, unsigned char dc, unsigned char cs
SSD1306Spi display(OLED_RST, OLED_DC, SPI1_CS, 1);
void getIP(std::string &ipSTR);
void split(std::string str, std::string splitBy, std::vector<std::string> &tokens);
std::string exec(const char *cmd);

char getch()
{
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return (buf);
}
size_t curX = 0;
size_t curY = 0;
char timeSTR[9] = {'\0'};
std::string ipSTR;
bool isColor = true;
std::mutex mtx;
#define curXsize 6
#define curYsize 10
void currBlink()
{
  while (1)
  {
    mtx.lock();
    isColor = !isColor;
    mtx.unlock();
    delay(600);
  }
}
void keyPress()
{
  while (true)
  {
    char c = getch();
    mtx.lock();
    switch (c)
    {
    case 'a':
      if (curX > 5)
      {
        curX -= 5;
      }
      break;
    case 'w':
      if (curY > 15)
      {
        curY -= 15;
      }
      break;
    case 's':
      if (curY + 15 < display.getHeight() - curYsize)
      {
        curY += 15;
      }
      break;
    case 'd':
      if (curX + 5 < display.getWidth() - curXsize)
      {
        curX += 5;
      }
      break;

    default:
      break;
    }
    mtx.unlock();
  }
}
void drawFrame()
{
  while (1)
  {
    display.clear();
    mtx.lock();
    display.drawRect(curX, curY, curXsize, curYsize);
    if (isColor)
    {
      display.fillRect(curX, curY, curXsize, curYsize);
    }
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
std::string exec(const char *cmd)
{
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe)
  {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }
  return result;
}
void getIP(std::string &ipSTR)
{
  struct ifaddrs *ifa = NULL;
  getifaddrs(&ifa);
  while (ifa != NULL)
  {
    if (strcmp("lo", ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6))
    {
      void *tmpAddrPtr = NULL;
      char *addressBuffer = NULL;
      if (ifa->ifa_addr->sa_family == AF_INET)
      {
        tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        addressBuffer = new char[INET_ADDRSTRLEN]();
        inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      }
      else if (ifa->ifa_addr->sa_family == AF_INET6)
      {
        tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
        addressBuffer = new char[INET6_ADDRSTRLEN]();
        inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
      }
      printf("%s ip:\n%s\n", ifa->ifa_name, addressBuffer);
      char *CstrIP = new char[strlen(ifa->ifa_name) + 5 + strlen(addressBuffer) + 1]();
      std::sprintf(CstrIP, "%s ip:\n%s", ifa->ifa_name, addressBuffer);
      ipSTR = CstrIP;
      if (CstrIP)
      {
        delete[] CstrIP;
        CstrIP = NULL;
      }
      if (addressBuffer)
      {
        delete[] addressBuffer;
        addressBuffer = NULL;
      }
    }
    ifa = ifa->ifa_next;
  }
  if (ifa != NULL)
  {
    freeifaddrs(ifa); //remember to free ifAddrStruct
    ifa = NULL;
  }
}
void split(std::string str, std::string splitBy, std::vector<std::string> &tokens)
{
  /* Store the original string in the array, so we can loop the rest
     * of the algorithm. */
  tokens.push_back(str);

  // Store the split index in a 'size_t' (unsigned integer) type.
  size_t splitAt;
  // Store the size of what we're splicing out.
  size_t splitLen = splitBy.size();
  // Create a string for temporarily storing the fragment we're processing.
  std::string frag;
  // Loop infinitely - break is internal.
  while (true)
  {
    /* Store the last string in the vector, which is the only logical
         * candidate for processing. */
    frag = tokens.back();
    /* The index where the split is. */
    splitAt = frag.find(splitBy);
    // If we didn't find a new split point...
    if (splitAt == std::string::npos)
    {
      // Break the loop and (implicitly) return.
      break;
    }
    /* Put everything from the left side of the split where the string
         * being processed used to be. */
    tokens.back() = frag.substr(0, splitAt);
    /* Push everything from the right side of the split to the next empty
         * index in the vector. */
    tokens.push_back(frag.substr(splitAt + splitLen, frag.size() - (splitAt + splitLen)));
  }
}