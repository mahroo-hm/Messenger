#ifndef HELPER_H
#define HELPER_H

#include "iostream"
#include "vector"
#include <regex>
using namespace std;

vector<string>* splitMessage(string message, string wrapper, int startPos = 1);
void checkMessageSize(vector<string>* splittedMsg, int i = -1);

#endif