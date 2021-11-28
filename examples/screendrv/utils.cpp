#include "utils.h"

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctime>
#include <stdexcept>
#include <array>
#include <vector>
#include <memory>

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