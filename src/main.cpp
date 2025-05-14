#include "../libs/raylib/src/raylib.h"
#include "Context.hpp"
#include "constants.hpp"
#include "typingTest.hpp"
#include "header.hpp"
#include "helpers.hpp"
#include "result.hpp"
#include "footer.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <unordered_set>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
EM_JS(int, canvasGetWidth, (), { return document.getElementById("canvas").clientWidth; });
EM_JS(int, canvasGetHeight, (), { return document.getElementById("canvas").clientHeight; });
EM_JS(int, browserWindowWidth, (), { return window.innerWidth; });
EM_JS(int, browserWindowHeight, (), { return window.innerHeight; });
#endif


int getWindowWidth() {
#if defined(PLATFORM_WEB)
    return canvasGetWidth();
#else
    if (IsWindowFullscreen()) {
        return GetMonitorWidth(GetCurrentMonitor());
    }
    else {
        return GetScreenWidth();
    }
#endif
}

int getWindowHeight() {
#if defined(PLATFORM_WEB)
    return canvasGetHeight();
#else
    if (IsWindowFullscreen()) {
        return GetMonitorHeight(GetCurrentMonitor());
    }
    else {
        return GetScreenHeight();
    }
#endif
}

Context context;

void loop();

int main(void) {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
#if defined(PLATFORM_WEB)
    InitWindow(browserWindowWidth(), browserWindowHeight(), PROJECT_NAME);
#else
    InitWindow(1280 * 1.5, 720 * 1.5, PROJECT_NAME);
#endif
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitAudioDevice();

    context.load();

    restartTest(context, true, false);

#if defined(PLATFORM_WEB)
    std::cout << "Setting up emscripten loop" << std::endl;
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!WindowShouldClose()) {
        loop();
    }
#endif
    context.unload();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

void loop() {
    context.screenHeight = getWindowHeight();
    context.screenWidth = getWindowWidth();

    // int wh = getWindowHeight();
    // int ww = getWindowWidth();
    // context.isResized = false;
    // if (wh != context.screenHeight) {
    //     context.screenHeight = wh;
    //     context.isResized = true;
    // }
    // if (ww != context.screenWidth) {
    //     context.screenWidth = ww;
    //     context.isResized = true;
    // }

    Theme theme = context.themes[context.selectedTheme];

#if defined(PLATFORM_WEB)
    static int old_w = 0, old_h = 0;

    int w = canvasGetWidth();
    int h = canvasGetHeight();
    if (w != old_w || h != old_h) { SetWindowSize(w, h); }
#else
    if (IsKeyPressed(KEY_F11)) {
        // see what display we are on right now

        if (IsWindowFullscreen()) {
            SetWindowSize(context.screenWidth, context.screenHeight);
            ToggleFullscreen();
        }
        else {
            ToggleFullscreen();
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        }
    }
#endif

    int key = GetCharPressed();

    if (context.mouseOnClickable) {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }
    context.mouseOnClickable = false;

    if (IsKeyPressed(KEY_ENTER)) {
        // create new test using shift + enter but dont start over the test by just pressing enter
        bool start_new = (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL));
        bool start_over = (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT));
        restartTest(context, start_new, start_over);
    }

    if (context.testRunning) {
        int time = (int)((GetTime() - context.testStartTime));
        if (time >= context.testSettings.testModeAmounts[context.testSettings.selectedAmount] && context.testSettings.testMode == TestMode::TIME) {
            endTest(context);
        }
    }

    BeginDrawing();
    ClearBackground(theme.background);

    header(context);

    if (context.currentScreen == Screen::TEST) {
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (context.soundOn) PlaySoundMulti(context.sounds.clickSound1);

            if (context.input_list.size()) {
                if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
                    // CTRL + Backspace
                    while (context.input_list.size() && context.input_list[context.input_list.size() - 1] == ' ')
                        context.input_list.pop_back();
                    while (context.input_list.size() && context.input_list[context.input_list.size() - 1] != ' ')
                        context.input_list.pop_back();
                }
                else {
                    // Normal Backspace
                    context.input_list.pop_back();
                }
            }
            context.furthestVisitedIndex = (int)context.input_list.size();
            // context.furthestVisitedIndex = context.furthestVisitedIndex < 1 ? 1 : context.furthestVisitedIndex;
        }

        if (key && (context.accent)) {
            switch (key) {
            case 97:
                key = 225;    // Key: a
                break;
            case 65:
                key = 193;    // Key: A
                break;
            case 101:
                key = 233;    // Key: é
                break;
            case 69:
                key = 201;    // Key: É
                break;
            case 105:
                key = 237;    // Key: i
                break;
            case 73:
                key = 205;    // Key: I
                break;
            case 111:
                key = 243;    // Key: o
                break;
            case 79:
                key = 211;    // Key: O
                break;
            case 117:
                key = 250;    // Key: u
                break;
            case 85:
                key = 218;    // Key: U
                break;
            case 121:
                key = 253;    // Key: y
                break;
            case 89:
                key = 221;    // Key: Y
                break;
            default:
                // Handle the case where key does not match any of the above values
                break;
            }
            context.accent = false;
        }
        else if (key && (context.bccent)) {
            switch (key) {
            case 99:
                key = 269;     // Key: c
                break;
            case 67:
                key = 268;     // Key: C
                break;
            case 100:
                key = 271;     // Key: d
                break;
            case 68:
                key = 270;     // Key: D
                break;
            case 101:
                key = 283;     // Key: ě
                break;
            case 69:
                key = 282;     // Key: Ě
                break;
            case 110:
                key = 328;     // Key: n
                break;
            case 78:
                key = 327;     // Key: N
                break;
            case 114:
                key = 345;     // Key: r
                break;
            case 82:
                key = 344;     // Key: R
                break;
            case 115:
                key = 353;     // Key: s
                break;
            case 83:
                key = 352;     // Key: S
                break;
            case 116:
                key = 357;     // Key: t
                break;
            case 84:
                key = 356;     // Key: T
                break;
            case 122:
                key = 382;     // Key: z
                break;
            case 90:
                key = 381;     // Key: Z
                break;
            default:
                // Handle the case where key does not match any of the above values
                break;
            }
            context.bccent = false;
        }

        if (key == 180) {// ´ á
            context.accent = true;
        }
        else if (key == 711) {// ˇ ď
            context.bccent = true;
        }
        else if (key && (context.input_list.size() <= context.sentence_list.size())) {
            context.input_list.push_back(key);

            if (key != 0 && context.soundOn) {
                PlaySoundMulti(context.sounds.clickSound1);
            }

            if (context.input_list.size() == 1 && !context.testRunning) {
                context.testRunning = true;
                context.testStartTime = GetTime();
            }

            // Once the sentence is complete end the test
            if (context.testRunning && context.input_list.size() == context.sentence_list.size()) {
                endTest(context);
            }

            // Calculate correct and incorrect typed letters and add more words as we type
            if (context.input_list.size() > context.furthestVisitedIndex) {
                context.canCount = true;

                if ((context.testSettings.testMode == TestMode::TIME) &&
                    (context.furthestVisitedIndex > (context.sentence_list.size() * 0.8)))
                {
                    int amount = context.testSettings.testModeAmounts[context.testSettings.selectedAmount];
                    std::string throw_away = generateSentence(context, amount);
                }
            }

            context.furthestVisitedIndex = std::max(context.furthestVisitedIndex, (int)context.input_list.size());
        }

        typingTest(context);

    }
    else if (context.currentScreen == Screen::RESULT) {
        result(context);
    }

    footer(context);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        context.saveSettings();
    }

    EndDrawing();
}
