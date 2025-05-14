#ifndef HELPERS_HPP
#define HELPERS_HPP
#include "Context.hpp"
#include "../libs/raylib/src/raylib.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#define STORAGE_DATA_FILE "storage.data"

typedef enum {
    STORAGE_POSITION_SCORE      = 0,
    STORAGE_POSITION_HISCORE    = 1
} StorageData;

// Persistent storage functions
bool saveStorageValue(unsigned int position, int value);
int loadStorageValue(unsigned int position, int defaultValue);


Vector2 getCenter(int width, int height);

void drawMonospaceText(Font font, std::string text, Vector2 position, float fontSize, Color color);

bool textButton(Context& context, Vector2 positon, std::string text);

double getTimeInMin();

bool restartTest(Context &context, bool repeat, bool start_over);

void endTest(Context &context);

bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs);

std::string generateSentence(Context &context, int words);
std::string unicode_to_string(const int& val);
#endif
