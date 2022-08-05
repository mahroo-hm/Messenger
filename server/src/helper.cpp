#include "helper.hpp"

vector<string>* splitMessage(string message, string wrapper, int startPos)
{
    vector<string>* result = new vector<string>();
    int endPos, wrapperLen = wrapper.length(); 
    while ((endPos = message.find (wrapper, startPos)) != string::npos)
    {
        result->push_back(message.substr(startPos, endPos - startPos));
        startPos = endPos + wrapperLen;
    }
    result->push_back (message.substr (startPos));
    return result;
}

void checkMessageSize(vector<string>* splittedMsg, int i)
{
    if (splittedMsg->size() == 0 || (i != -1 && splittedMsg->size() != i))
        throw "The message format is incorrect";
}