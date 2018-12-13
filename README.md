# retro-basic-ll-1--parser
This project was implemented using C++.
Many C++11 specific features were used so it is recommended to compile this program using C++11 standard or newer.
*This program has only been tested on Windows 7 64 bit.
# compiling
using GCC for compilation on the Win32 environment.
g++ parser.cpp -std=c++11 -o parser.exe\\
# usage
parser.exe source_file bcode_file\\
If there was any error during the parsing process, the program informed the user and aboard operation.
# example
parser.exe basic.txt basic_bcode.txt