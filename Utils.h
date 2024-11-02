#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>  // For system()

// Cross-platform clear screen function
inline void clearscreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#endif // UTILS_H
