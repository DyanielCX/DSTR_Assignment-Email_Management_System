// Utils.h
#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>

inline void clearscreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#endif // UTILS_H
