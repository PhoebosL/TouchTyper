#include "Context.hpp"
#include "helpers.hpp"
#include "typingTest.hpp"
#include <vector>
#include <cmath>
#include "../libs/raylib/src/raymath.h"
#include "constants.hpp"

#include <iostream>
#include <string>
#include <cctype>

Vector2 cursorPostion = {0, 0};
Vector2 newCursorPosition = {0, 0};
float cursorSpeed = 20;
float yOffset = 0;
float newYOffset = 0;
float cursorOpacity = 1;
int cursorOpacityDirection = 0;
float cursorStayVisibleTimer = 0;

std::vector<std::vector<char>> keyboard = {
    // {'+', 'ě', 'š', 'č', 'ř', 'ž', 'ý', 'á', 'í', 'é', '=','\''},
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',']'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '"','!'},
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'},
    {' '}
};

float sinPulse(float frequency) {
    const float pi = 3.14f;
    return 0.5f * (1 + (float)std::sin(2 * pi * frequency * GetTime()));
}

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
std::string unicode_to_string(const int& val)  {
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


void typingTest(Context &context) {
    // We are using a monospace font so every character will have same with
    Vector2 sizeOfCharacter = MeasureTextEx(
        context.fonts.typingTestFont.font,
        "a",
        context.fonts.typingTestFont.size,
        1
    );

    Theme theme = context.themes[context.selectedTheme];

    // To make it responsive
    float width = std::min(context.screenWidth-(PADDING*2), MAX_WIDTH);
    float height = sizeOfCharacter.y * 3;

    // Center of the screen
    Vector2 center = getCenter(context.screenWidth, context.screenHeight);

    // Animate scroll
    float speed = cursorSpeed * GetFrameTime();
    yOffset = Lerp(yOffset, newYOffset, (speed <= 0 || speed > 1) ? 1 : speed);

    // Animate cursor
    cursorPostion.x = Lerp(cursorPostion.x, newCursorPosition.x, (speed <= 0 || speed > 1) ? 1 : speed);
    cursorPostion.y = Lerp(cursorPostion.y, newCursorPosition.y, (speed <= 0 || speed > 1) ? 1 : speed);

    // Cursor blink timer
    if (cursorStayVisibleTimer > 0) {
        cursorStayVisibleTimer -= GetFrameTime();
    } else {
        cursorStayVisibleTimer = 0;
    }

    // Animate cursor blink
    cursorOpacity = sinPulse(1.5);

    // Calculate how many words will be in each line according to the available screen size
    std::vector<std::string> lines;
    std::string currentLine = "";
    std::string currentWord = "";

    // convert string into array of codepoints (integers)
    std::vector<int> unicode;
    unicode = string_to_unicodes(context.sentence);
    for (int i = 0; i < unicode.size(); i++) { // for (unsigned char c: context.sentence) {
        int key = unicode[i];
        std::string chr = unicode_to_string(key);
        // Detect the end of a word
        if (key == 32 || i == (unicode.size() - 1)) { // if (context.sentence[i] == ' ' || i == (context.sentence.size() - 1)) {
            currentWord += chr;

            // Calculate the width of the word
            float widthOfWord = currentWord.size() * sizeOfCharacter.x;
            float widthOfNewLine = widthOfWord + currentLine.size() * sizeOfCharacter.x;

            // Go to new line if word is overflowing
            if (widthOfNewLine > width-(PADDING*2)) {
                lines.push_back(currentLine);
                currentLine = "";
            }

            currentLine += currentWord;
            currentWord = "";
        } else {
            currentWord += chr;
            // currentWord.push_back(chr);
            // currentWord.push_back(context.sentence[i]);
        }
        context.sentenceLength = i;
    }

    lines.push_back(currentLine);

    Rectangle textBox = {
        (float)(center.x - width/2.0),
        (float)((center.y - height/2.0) - 100),
        (float)width,
        (float)(height > 1 ? height : 1), // To fix a bug in Secissor Mode when it crashes when the height is < 1
    };

    if (textBox.y < 95) {
        textBox.y = 95;
    }

    // Begin Drawing sentence
    BeginScissorMode(textBox.x, textBox.y, textBox.width, textBox.height);

    float currentLineY = textBox.y - yOffset;
    int characterIndex = 0;
    int lineNumber = 1;


    for (auto& line : lines) {
        float widthOfLine = sizeOfCharacter.x * line.size();
        float currentLetterX = center.x - (widthOfLine/2);

        std::vector<int> unicode;
        unicode = string_to_unicodes(line);
        for (int i = 0; i < unicode.size(); i++) { // for (char& letter : line) {
            int codepoint = unicode[i];

            Color color = theme.text;
            // Check if the character is wrong
            if (context.input_list.size() > characterIndex) {
                if (codepoint == context.input_list[characterIndex]) {
                    color = theme.correct;
                    if (context.canCount && context.input_list.size() == (i + 1)) {
                        context.correctLetters++;
                        context.canCount = false;
                    }
                } else {
                    color = theme.wrong;
                    if (context.canCount && context.input_list.size() == (i + 1)) {
                        context.incorrecLetters++;
                        context.canCount = false;
                    }
                    if (codepoint == 32) // if space expected
                        codepoint = 95; // ==> "_"
                }
            }


            // Custom modification from: void DrawTextEx(...)
            int textOffsetY = 0;            // Offset between lines (on line break '\n')
            float textOffsetX = 0.0f;       // Offset X to next character to draw
            float spacing = 1;
            // Character quad scaling factor
            float scaleFactor = context.fonts.typingTestFont.size/context.fonts.typingTestFont.font.baseSize;

            int index = GetGlyphIndex(context.fonts.typingTestFont.font, codepoint);
            if (codepoint == 10) { // if (codepoint == '\n') {
                // textOffsetY += (int)((context.fonts.typingTestFont.font.baseSize + context.fonts.typingTestFont.font.baseSize/2)*scaleFactor);
                // textOffsetX = 0.0f;
            } else {
                if ((codepoint != 32) && (codepoint != 9)) { // if ((codepoint != ' ') && (codepoint != '\t')) {
                    DrawTextCodepoint(
                        context.fonts.typingTestFont.font, 
                        codepoint, // int representation of character eg. ord-value in python
                        (Vector2){ currentLetterX + textOffsetX, currentLineY + textOffsetY }, 
                        context.fonts.typingTestFont.size, 
                        color
                    );
                }

                if (context.fonts.typingTestFont.font.glyphs[index].advanceX == 0)
                    textOffsetX += ((float)context.fonts.typingTestFont.font.recs[index].width*scaleFactor + spacing);
                else
                    textOffsetX += ((float)context.fonts.typingTestFont.font.glyphs[index].advanceX*scaleFactor + spacing);
            }


            // Handle cursor
            if (characterIndex == context.input_list.size()) {
                // Set the offset to make the cursor at center
                newYOffset = lineNumber > 2 ? ((lineNumber-1) * sizeOfCharacter.y) - sizeOfCharacter.y : 1;
                newCursorPosition = { currentLetterX, currentLineY };

                Color cursorColor = theme.cursor;
                float blink = (cursorStayVisibleTimer != 0 )? 1 : cursorOpacity;

                // Draw Cursor
                if (blink > 0.5) {
                    BeginBlendMode(BLEND_SUBTRACT_COLORS);
                    switch (context.cursorStyle) {
                        case CursorStyle::BLOCK:
                            DrawRectangle(cursorPostion.x+1, cursorPostion.y, sizeOfCharacter.x, sizeOfCharacter.y, cursorColor);
                            // Make the color of the text inverted
                            // color = theme.background;
                            break;
                        case CursorStyle::LINE:
                            DrawRectangle(cursorPostion.x, cursorPostion.y, 2, sizeOfCharacter.y, cursorColor);
                            break;
                        case CursorStyle::UNDERLINE:
                            DrawRectangle(cursorPostion.x+1, cursorPostion.y+sizeOfCharacter.y, sizeOfCharacter.x, 3, cursorColor);
                            break;
                    }
                    EndBlendMode();
                }
            }

            currentLetterX += sizeOfCharacter.x;
            characterIndex++;
        }

        currentLineY += sizeOfCharacter.y;
        lineNumber++;
    }

    EndScissorMode();

    // Draw Keybaord
    const int sizeOfKey = 35;
    const int margin = 5;
    sizeOfCharacter = MeasureTextEx(context.fonts.tinyFont.font, "a", context.fonts.tinyFont.size, 1);

    if (IsKeyPressed(KEY_BACKSPACE)) {
        cursorStayVisibleTimer = 1;
    }

    for (int i = 0; i < keyboard.size(); i++) {
        auto row = keyboard[i];
        int totalWidth = row[0] == ' ' ? 200 : (sizeOfKey * row.size()) + margin * (row.size()-1);
        Vector2 position;
        position.x = center.x - (totalWidth/2.0);
        position.y = (textBox.y + (sizeOfCharacter.y * 8)) + (sizeOfKey * i) + margin * i;

        for (auto key : row) {
            std::string c(1, key);
            Rectangle rect;
            rect.x = position.x;
            rect.y = position.y;
            rect.width = row[0] == ' ' ? 200 : sizeOfKey;
            rect.height = sizeOfKey;
            DrawRectangleRoundedLines(rect, 0.1, 5, 1, theme.text);
            Color color = theme.text;
            Vector2 keyPosition;
            keyPosition.x = (rect.x + (sizeOfKey/2.0)) - (sizeOfCharacter.x/2.0);
            keyPosition.y = (rect.y + (sizeOfKey/2.0)) - (sizeOfCharacter.y/2.0);

            if (IsKeyDown(toupper(key))) {
                BeginBlendMode(BLEND_SUBTRACT_COLORS);
                DrawRectangleRounded(rect, 0.1, 5, theme.cursor);
                EndBlendMode();
                color = theme.background;
                cursorStayVisibleTimer = 1;
            }

            drawMonospaceText(context.fonts.tinyFont.font, c.c_str(), keyPosition, context.fonts.tinyFont.size, color);
            position.x += sizeOfKey + margin;
        }
    }
}
