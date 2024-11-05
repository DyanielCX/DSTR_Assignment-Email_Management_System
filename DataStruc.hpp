#ifndef DATASTRUC_HPP
#define DATASTRUC_HPP
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct Email {
    string subject;
    string sender;
    string receiver;
    string date;
    string time;
    string content;
    bool isSpam = false;
    Email* next = nullptr; // Pointer to the next Email in the linked list
};

#endif
