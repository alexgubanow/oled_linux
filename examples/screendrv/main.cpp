#include "SSD1306Spi.h"
#include <ctime>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>

SSD1306Spi display(10, 16, 15, 1);
void getIP(std::string &ipSTR);

int main()
{
  // Initialising the UI will init the display too.
  display.init();
  display.clear();
  //display.drawRect(0,0,25,25);
  display.flipScreenVertically();
  std::string ipSTR;
  getIP(ipSTR);
  while (1)
  {
    std::time_t t = std::time(0); // get time now
    std::tm *now = std::localtime(&t);
    char str[9] = {'\0'};
    std::sprintf(str, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 0, str);
    display.setFont(Monospaced_plain_10);
    display.drawString(0, 24, ipSTR);
    display.display();
    //delay(1000);
    display.clear();
  }

  return 0;
}


void getIP(std::string &ipSTR)
{
  struct ifaddrs *ifa = NULL;
  getifaddrs(&ifa);
  while(ifa != NULL)
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