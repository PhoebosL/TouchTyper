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

Vector2 cursorPostion = { 0, 0 };
Vector2 newCursorPosition = { 0, 0 };
float cursorSpeed = 20;
float yOffset = 0;
float newYOffset = 0;
float cursorOpacity = 1;
int cursorOpacityDirection = 0;
float cursorStayVisibleTimer = 0;

std::vector<std::vector<int>> keyboard_en = {
    {49,50,51,52,53,54,55,56,57,48,45,61},    // {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '[',']'},
    {81,87,69,82,84,89,85,73,79,80,91,93,92}, // {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',']'},
    {65,83,68,70,71,72,74,75,76,59,39,96},    // {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',"'"},
    {90,88,67,86,66,78,77,44,46,47},          // {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'},
    {32}                                      // {' '}
};
std::vector<std::vector<int>> keyboard_cz = {
    {49,50,51,52,53,54,55,56,57,48,45,61},
    {81,87,69,82,84,90,85,73,79,80,218,186},  // {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[',']'},
    {65,83,68,70,71,72,74,75,76,269,187,96},  // {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',"'"},
    {89,88,67,86,66,78,77,44,46,45,0,0},      // {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'},
    {32}                                      // {' '}
};




float sinPulse(float frequency) {
    const float pi = 3.14f;
    return 0.5f * (1 + (float)std::sin(2 * pi * frequency * GetTime()));
}



void typingTest(Context& context) {
    // We are using a monospace font so every character will have same with
    Vector2 sizeOfCharacter = MeasureTextEx( context.fonts.typingTestFont.font, "a", context.fonts.typingTestFont.size, 1);
    Theme theme = context.themes[context.selectedTheme];

    // To make it responsive
    float width = std::min(context.screenWidth - (PADDING * 2), MAX_WIDTH);
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

    // Calculates how many words will be in each line according to the available screen size
    std::vector<int> lines_idx; // individual positions where to wrap text to the next line

    int key;
    int iword = 0; // counts characters until space
    int iline = 0; // counts characters until line is full
    for (int i = 0; i < context.sentence_list.size(); i++) {
        key = context.sentence_list[i];
        // Detect spacebar (the end of a word; key == 32 == ' ')
        if (key == 32 || i == (context.sentence_list.size() - 1)) {
            iword += 1;
            float widthOfWord = iword * sizeOfCharacter.x;
            float widthOfNewLine = widthOfWord + iline * sizeOfCharacter.x;

            // Go to new line if word is overflowing
            if (widthOfNewLine > width - (PADDING * 3)) {
                lines_idx.push_back(i - iword);
                iline = 0;
            }

            iline += iword;
            iword = 0;
        } else {
            iword += 1;
        }
    }
    lines_idx.push_back(context.sentence_list.size()); // ensure at least one element in array

    Rectangle textBox = {
        (float)(center.x - width / 2.0),
        (float)((center.y - height / 2.0) - 100),
        (float)width,
        (float)(height > 1 ? height : 1), // To fix a bug in Secissor Mode when it crashes when the height is < 1
    };

    if (textBox.y < 95) {
        textBox.y = 95;
    }


    // Begin Drawing sentence
    BeginScissorMode(textBox.x, textBox.y, textBox.width, textBox.height);

    Color color;
    int lineNumber = 1;
    float currentLetterX = center.x - (sizeOfCharacter.x * (lines_idx[0]) / 2);
    float currentLineY = textBox.y - yOffset;

    for (int i = 0; i < context.sentence_list.size(); i++) {
        // Check if the character is wrong
        int codepoint = context.sentence_list[i]; // do not use &, so the wrong space could be replaced with "_"
        if (context.input_list.size() > i) {
            if (codepoint == context.input_list[i]) {
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
                if (codepoint == 32) // if space expected show "_" to see error
                    codepoint = 95;
            }
        } else {
            color = theme.text;
        }


        // Draw a character (void DrawTextEx(...) Custom modification)
        int textOffsetY = 0;            // Offset between lines (on _line break '\n')
        float textOffsetX = 0.0f;       // Offset X to next character to draw
        float spacing = 1;
        float scaleFactor = context.fonts.typingTestFont.size / context.fonts.typingTestFont.font.baseSize; // Character quad scaling factor
        int index = GetGlyphIndex(context.fonts.typingTestFont.font, codepoint);

        if (codepoint == 10) { // ==> '\n'
            // textOffsetY += (int)((context.fonts.typingTestFont.font.baseSize + context.fonts.typingTestFont.font.baseSize/2)*scaleFactor);
            // textOffsetX = 0.0f;
        } else {
            if ((codepoint != 32) && (codepoint != 9)) { //  ' ' && '\t'
                DrawTextCodepoint(
                    context.fonts.typingTestFont.font,
                    codepoint, // int representation of character eg. ord-value in python
                    (Vector2) { currentLetterX + textOffsetX, currentLineY + textOffsetY },
                    context.fonts.typingTestFont.size,
                    color
                );
            }

            if (context.fonts.typingTestFont.font.glyphs[index].advanceX == 0)
                textOffsetX += ((float)context.fonts.typingTestFont.font.recs[index].width * scaleFactor + spacing);
            else
                textOffsetX += ((float)context.fonts.typingTestFont.font.glyphs[index].advanceX * scaleFactor + spacing);
        }


        // Handle cursor
        if (i == context.input_list.size()) {
            // Set the offset to make the cursor at center
            newYOffset = lineNumber > 2 ? ((lineNumber - 1) * sizeOfCharacter.y) - sizeOfCharacter.y : 1;
            newCursorPosition = { currentLetterX, currentLineY };

            Color cursorColor = theme.cursor;
            float blink = (cursorStayVisibleTimer != 0) ? 1 : cursorOpacity;

            // Draw Cursor
            if (blink > 0.5) {
                BeginBlendMode(BLEND_SUBTRACT_COLORS);
                switch (context.cursorStyle) {
                case CursorStyle::BLOCK:
                    DrawRectangle(cursorPostion.x + 1, cursorPostion.y, sizeOfCharacter.x, sizeOfCharacter.y, cursorColor);
                    // Make the color of the text inverted
                    // color = theme.background;
                    break;
                case CursorStyle::LINE:
                    DrawRectangle(cursorPostion.x, cursorPostion.y, 2, sizeOfCharacter.y, cursorColor);
                    break;
                case CursorStyle::UNDERLINE:
                    DrawRectangle(cursorPostion.x + 1, cursorPostion.y + sizeOfCharacter.y, sizeOfCharacter.x, 3, cursorColor);
                    break;
                }
                EndBlendMode();
            }
        }

        // calculate positions for placement of next character
        if (i == lines_idx[lineNumber - 1]) {
            currentLetterX = center.x - (sizeOfCharacter.x * (lines_idx[lineNumber - 1] - lines_idx[lineNumber - 2]) / 2);
            currentLineY += sizeOfCharacter.y;
            lineNumber++;
        }
        else {
            currentLetterX += sizeOfCharacter.x;
        }
    }

    EndScissorMode();
 

    // Draw Keybaord
    const int sizeOfKey = 35;
    const int margin = 5;
    sizeOfCharacter = MeasureTextEx(context.fonts.tinyFont.font, "a", context.fonts.tinyFont.size, 1);

    if (IsKeyPressed(KEY_BACKSPACE)) {
        cursorStayVisibleTimer = 1;
    }

    for (int i = 0; i < keyboard_cz.size(); i++) {
        auto row = keyboard_cz[i];
        int totalWidth = row[0] == 32 ? 350 : (sizeOfKey * row.size()) + margin * (row.size() - 1);
        Vector2 position;
        position.x = center.x - (totalWidth / 2.0);
        position.y = (textBox.y + (sizeOfCharacter.y * 10)) + (sizeOfKey * i) + margin * i;

        for (auto key : row) {
            if (key == 0)
                continue;
            Rectangle rect;
            rect.x = position.x + sizeOfKey * i / 2;
            rect.y = position.y;
            rect.width = row[0] == ' ' ? 200 : sizeOfKey;
            rect.height = sizeOfKey;
            DrawRectangleRoundedLines(rect, 0.1, 5, 1, theme.text);
            Color color = theme.text;
            Vector2 keyPosition;
            keyPosition.x = (rect.x + (sizeOfKey / 2.0)) - (sizeOfCharacter.x / 2.0);
            keyPosition.y = (rect.y + (sizeOfKey / 2.0)) - (sizeOfCharacter.y / 2.0);

            if (IsKeyDown(toupper(key))) {
                BeginBlendMode(BLEND_SUBTRACT_COLORS);
                DrawRectangleRounded(rect, 0.1, 5, theme.cursor);
                EndBlendMode();
                color = theme.background;
                cursorStayVisibleTimer = 1;
            }

            drawMonospaceText(context.fonts.tinyFont.font, unicode_to_string(key), keyPosition, context.fonts.tinyFont.size, color);
            position.x += sizeOfKey + margin;
        }
    }
}
