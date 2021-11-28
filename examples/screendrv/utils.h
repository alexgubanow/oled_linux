#pragma once
#include <string>
#include <vector>

void getIP(std::string &ipSTR);
void split(std::string str, std::string splitBy, std::vector<std::string> &tokens);
std::string exec(const char *cmd);