// Natural sort taken from https://stackoverflow.com/a/9745132
//
// "This function makes a natural comparison between two strings. 
// After that you can use std::sort with the comparison function as third argument."
// Christian Ammer.

#ifndef LSIM_STRNATCMP_H
#define LSIM_STRNATCMP_H

#include <string>

bool compareNat(const std::string& a, const std::string& b);

#endif
