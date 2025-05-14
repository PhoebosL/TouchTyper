#include "helpers.hpp"
#include <cctype>
#include <unordered_map>
#include "Context.hpp"
#include "constants.hpp"

Vector2 getCenter(int width, int height) {
    Vector2 result;
    result.x = width/2.0;
    result.y = height/2.0;
    return result;
}

void drawMonospaceText(Font font, std::string text, Vector2 position, float fontSize, Color color) {
    Vector2 sizeOfCharacter = MeasureTextEx(font, "a", fontSize, 1);

    for (auto letter : text) {
        std::string c(1, letter);

        DrawTextEx(font, c.c_str(), position, fontSize, 1, color);
        position.x += sizeOfCharacter.x;
    }
}

bool textButton(Context& context, Vector2 positon, std::string text) {
    Vector2 sizeOfCharacter = MeasureTextEx(context.fonts.tinyFont.font, "a",
            context.fonts.tinyFont.size, 1);
    Theme theme = context.themes[context.selectedTheme];

    Rectangle rect = {
        positon.x,
        positon.y,
        sizeOfCharacter.x * text.size(),
        sizeOfCharacter.y
    };

    Color color = theme.text;

    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        context.mouseOnClickable  = true;
        color = theme.highlight;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            return true;
        }
    }

    drawMonospaceText(context.fonts.tinyFont.font, text, positon, context.fonts.tinyFont.size, color);

    return false;
}

template<class BidiIter>
BidiIter random_unique(BidiIter begin, BidiIter end, size_t num_random) {
    size_t left = std::distance(begin, end);
    while (num_random--) {
        BidiIter r = begin;
        std::advance(r, rand()%left);
        std::swap(*begin, *r);
        ++begin;
        --left;
    }
    return begin;
}

bool useCaplitalNext = false;
bool previousWasDash = false;

char quotes[3][2] = {
    {'\'', '\''},
    {'\"', '\"'},
    {'(', ')'}
};

std::unordered_map<std::string, std::string> punctuations = {
    {"are", "aren't"},
    {"can", "can't"},
    {"could", "couldn't"},
    {"did", "didn't"},
    {"does", "doesn't"},
    {"do", "don't"},
    {"had", "hadn't"},
    {"has", "hasn't"},
    {"have", "haven't"},
    {"is", "isn't"},
    {"must", "mustn't"},
    {"should", "shouldn't"},
    {"was", "wasn't"},
    {"were", "weren't"},
    {"will", "won't"},
    {"would", "wouldn't"}
};

std::vector<int> string_to_unicodes(const std::string& input)  {
    std::vector<int> out;

    for (size_t i = 0; i<input.length(); ++i) {
        unsigned char byte = input[i];
        if (byte < 0x80) {
            out.push_back(byte);
        } else { 
            int unicode_value = 0;
            if ((byte & 0xE0) == 0xC0) {
                unicode_value = byte & 0x3F;
                unicode_value <<= 6;
                unicode_value |= (input[++i] & 0x3F);
            } else if ((byte & 0xF0) == 0xE0) {
                unicode_value = byte & 0x0F;
                unicode_value <<= 12;
                unicode_value |= (input[++i] & 0x3F)<<6;
                unicode_value |= (input[++i] & 0x3F);
            }
            out.push_back(unicode_value);
        }
    }
    return out;
}

std::string unicode_to_string(const int& val) {
    std::string out;
        if (val < 0x80) {
            out = static_cast<char>(val);
        } else if (val < 0x800) { // 2 byte
            out = static_cast<char>((val>>6)|0xC0);
            out += static_cast<char>((val & 0x3F) | 0x80);
        } else if (val < 0x10000) { // three byte
            out = static_cast<char>((val>>12)| 0xE0);
            out += static_cast<char>(((val>>6)& 0x3F) | 0x80);
            out += static_cast<char>((val  & 0x3F) | 0x80);
        } else  {
            // four byte character todo
        }
    return out ;
}

std::string generateSentence(Context& context, int numberOfWords) {
    auto words = context.wordsLists[context.selectedWordList].words;
    std::string output = "";


    // increase number of words just in case there are not enough in asset file
    int i = 0;
    while (words.size() < numberOfWords) {
        words.push_back(words[i]);
        i++;
    }

    // Shuffle the amount of words we need
    random_unique(words.begin(), words.end(), numberOfWords);

    // Put the words in the senctence
    for(int i = 0; i < numberOfWords; ++i) {
        std::string word = words[i];

        if (context.testSettings.usePunctuation) {
            if (punctuations.find(word) != punctuations.end()) {
                word = punctuations[word];
            }
        }

        bool inQuotes = context.testSettings.usePunctuation && (GetRandomValue(0, 10) == 10);
        bool itsDashTime = context.testSettings.usePunctuation && (GetRandomValue(0, 10) == 10) && !previousWasDash && !useCaplitalNext;
        bool itsNumber = context.testSettings.useNumbers && (GetRandomValue(0, 10) == 10);

        if (useCaplitalNext) {
            word[0] = toupper(word[0]);
        }

        if (inQuotes) {
            int quote = GetRandomValue(0, 2);
            output.push_back(quotes[quote][0]);
            word += quotes[quote][1];
        } else if(itsDashTime) {
            word = "-";
        }

        if (itsNumber) {
            word = std::to_string(GetRandomValue(0, 1000));
        }

        output += word;

        useCaplitalNext = false;

        // Put . or , randomly
        if (GetRandomValue(0, 10) > 8 &&
                context.testSettings.usePunctuation &&
                !previousWasDash &&
                !itsDashTime &&
                !inQuotes) {
            switch (GetRandomValue(0, 3)) {
                case 0:
                    output.push_back(',');
                    break;
                case 1:
                    output.push_back('.');
                    useCaplitalNext = true;
                    break;
                case 2:
                    output.push_back('!');
                    useCaplitalNext = true;
                    break;
                case 3:
                    output.push_back('?');
                    useCaplitalNext = true;
                    break;
            }
        }

        previousWasDash = itsDashTime;

        output += ' ';
    }

    output.pop_back();

    // Append the new characters to the existing sentence_list
    std::vector<int> new_unicodes = string_to_unicodes(output);
    context.sentence_list.insert(context.sentence_list.end(), new_unicodes.begin(), new_unicodes.end());

    return output;
}

bool restartTest(Context &context, bool repeat, bool start_over) {
    if (start_over) {
        // do shared code below
    } else if (repeat) {
        // create new text to write here, and continue to shared code
        int amount = context.testSettings.testModeAmounts[context.testSettings.selectedAmount];

        // If time mode is set put two words for each sec, it will be generated more if neede
        if (context.testSettings.testMode == TestMode::TIME) {
            amount = amount * 2;
        }

        std::string sentence = generateSentence(context, amount);
        if (context.testSettings.usePunctuation) {
            sentence[0] = toupper(sentence[0]);
            if (context.testSettings.testMode == TestMode::WORDS && !useCaplitalNext) {
                sentence += '.';
            }
        }

        context.sentence_list = string_to_unicodes(sentence);
    } else if (!START_OVER_ON_ENTER) {
        context.input_list.push_back(10); // just write enter into current text
        return true;
    }

    context.input_list.clear();
    context.currentScreen = Screen::TEST;
    context.wpm = 0;
    context.cpm = 0;
    context.accuracy = 0;
    context.raw = 0;
    context.testRunning = false;
    context.correctLetters = 0;
    context.incorrecLetters = 0;
    context.furthestVisitedIndex = -1;
    return false; // restart
}

void endTest(Context &context) {
    context.testRunning = false;
    context.currentScreen = Screen::RESULT;
    context.testEndTime = GetTime();
}

bool getFileContent(std::string fileName, std::vector<std::string> & vecOfStrs) {
    // Open the File
    std::ifstream in(fileName.c_str());

    // Check if object is valid
    if(!in) {
        std::cerr << "Cannot open the File : " << fileName << std::endl;
        return false;
    }

    std::string str;
    // Read the next line from File untill it reaches the end.
    while (std::getline(in, str)) {
        // Line contains string of length > 0 then save it in vector
        if(str.size() > 0)
            vecOfStrs.push_back(str);
    }

    //Close The File
    in.close();
    return true;
}

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
EM_JS(int, getStorageBrowser, (int position), {
  return localStorage.getItem(position);
});

EM_JS(void, setStorageBrowserDefault, (int position, int defaultValue), {
  if (localStorage.getItem(position) === null) {
    localStorage.setItem(position, defaultValue);
  }
});

EM_JS(void, setStorageBrowser, (unsigned int position, int value), {
  console.log("Setting value " + position + " " + value);
  localStorage.setItem(position, value);
});
#endif


// Save integer value to storage file (to defined position)
// NOTE: Storage positions is directly related to file memory layout (4 bytes each integer)
bool saveStorageValue(unsigned int position, int value) {
#if defined(PLATFORM_WEB)
    setStorageBrowser(position, value);
    return true;
#endif
    bool success = false;
    unsigned int dataSize = 0;
    unsigned int newDataSize = 0;
    const char *filePath = TextFormat("%s%s", GetApplicationDirectory(), STORAGE_DATA_FILE);
    unsigned char *fileData = LoadFileData(filePath, &dataSize);
    unsigned char *newFileData = NULL;

    if (fileData != NULL) {
        if (dataSize <= (position*sizeof(int))) {
            // Increase data size up to position and store value
            newDataSize = (position + 1)*sizeof(int);
            newFileData = (unsigned char *)RL_REALLOC(fileData, newDataSize);

            if (newFileData != NULL) {
                // RL_REALLOC succeded
                int *dataPtr = (int *)newFileData;
                dataPtr[position] = value;
            } else {
                // RL_REALLOC failed
                TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to realloc data (%u), position in bytes (%u) bigger than actual file size", filePath, dataSize, position*sizeof(int));

                // We store the old size of the file
                newFileData = fileData;
                newDataSize = dataSize;
            }
        } else {
            // Store the old size of the file
            newFileData = fileData;
            newDataSize = dataSize;

            // Replace value on selected position
            int *dataPtr = (int *)newFileData;
            dataPtr[position] = value;
        }

        success = SaveFileData(filePath, newFileData, newDataSize);
        RL_FREE(newFileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", filePath, value);
    } else {
        TraceLog(LOG_INFO, "FILEIO: [%s] File created successfully", filePath);

        dataSize = (position + 1)*sizeof(int);
        fileData = (unsigned char *)RL_MALLOC(dataSize);
        int *dataPtr = (int *)fileData;
        dataPtr[position] = value;

        success = SaveFileData(filePath, fileData, dataSize);
        UnloadFileData(fileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Saved storage value: %i", filePath, value);
    }

    return success;
}

// Load integer value from storage file (from defined position)
// NOTE: If requested position could not be found, value defaultValue is returned
int loadStorageValue(unsigned int position, int defaultValue) {
#if defined(PLATFORM_WEB)
    setStorageBrowserDefault(position, defaultValue);
    return getStorageBrowser(position);
#endif
    int value = defaultValue;
    unsigned int dataSize = 0;
    const char *filePath = TextFormat("%s%s", GetApplicationDirectory(), STORAGE_DATA_FILE);
    unsigned char *fileData = LoadFileData(filePath, &dataSize);

    if (fileData != NULL) {
        if (dataSize < (position*4)) {
            TraceLog(LOG_WARNING, "FILEIO: [%s] Failed to find storage position: %i", filePath, position);
        } else {
            int *dataPtr = (int *)fileData;
            value = dataPtr[position];
        }

        UnloadFileData(fileData);

        TraceLog(LOG_INFO, "FILEIO: [%s] Loaded storage value: %i", filePath, value);
    }

    return value;
}
