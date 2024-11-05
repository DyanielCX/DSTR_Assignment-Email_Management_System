// Utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdlib>

inline void clearscreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#endif // UTILS_HPP
