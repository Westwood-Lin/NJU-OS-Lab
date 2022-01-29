#pragma once
#include <string>
#include <vector>
#include <fstream>
using namespace std;

//some Constants
const int SECTOR_SIZE = 512;
const int ENTRY_SIZE = 32;
const int FAT_SECTORS = 9;
const string ZERO_32_BYTES = string(32, 0);
extern char* DATA_ZONE;
