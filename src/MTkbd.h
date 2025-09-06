/*
 * KEY HANDLING LIBRARY
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Marco Tinner www.marcotinner.com All right reserved.
 *                    marco.tinner@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * NO commercial use without prior permit by copyright owner.
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MTKBD_H
#define MTKBD_H

#include <Arduino.h>

#ifndef OUTPORT
#define OUTPORT Serial
#endif

class MTkbd
{
public:
    enum pattern_e : uint8_t
    {
        PATTERN_NONE,
        PATTERN_START,
        PATTERN_RUN,
        PATTERN_END,
        PATTERN_READY
    };

    MTkbd();
    ~MTkbd();
    bool begin(const bool activeLow = true,
               const uint8_t numKeys = 4,
               const uint8_t keys[] = new uint8_t[4]{0, 2, 4, 36});

    uint8_t KeyCode();
    uint8_t Repeat();
    uint32_t Duration();
    bool IsPattern();
    String Pattern();

    void setWaitHandled(bool waitHandled);
    bool getWaitHandled();
    void setShowInfo(bool showInfo);
    bool getShowInfo();
    void setShowPattern(bool showPattern);
    bool getShowPattern();
    void setMaxPatternLength(uint8_t maxPatternLength);
    uint8_t getMaxPatternLength();
    void setBounceMS(uint32_t ms);
    uint32_t getBounceMS();
    void setDoubleClickMS(uint32_t ms);
    uint32_t getDoubleClickMS();
    void setInfoResponse(uint32_t ms);
    uint32_t getInfoResponse();
    void setPatternMS(uint32_t ms);
    uint32_t getPatternMS();

    void loop();
    bool available();
    void handled();

    uint8_t getKeyCodeOfPin(uint8_t pin);

private:
    void clearPattern();
    void clearData();
    char hex_digit(uint8_t v);
    std::array<char, 2> byte_to_hex(uint8_t b);

    uint8_t _numKeys;                      // number of key pins
    uint8_t *_keys;                        // array of key pins
                                           //
    uint8_t _patternKey = 0;               // pattern key
    char *_pattern;                        // saved key pattern
    String _patternString = "";            // Pattern as string
    uint8_t _patternPos = 0;               // pattern curscor pos
                                           //
    uint8_t _rawKeyCode = 0;               // read key code before stable
    uint8_t _lastRawKeyCode = 0;           // last read key code before stable
    bool _keyDown = false;                 // key is pressed
    uint8_t _keyCode = 0;                  // pressed key code after stable
    uint8_t _lastKeyCode = 0;              // last pressed key code
                                           //
    uint8_t _repeatNr = 0;                 // nr of same keycode pressed within double click
                                           //
    bool _activeLow = true;                // key pins are active low
    bool _keyCodeValid = false;            // keys pressed are valid >> stable after bounce time
    bool _keyCodeReady = false;            // keycode are ready for handle >> stable for > doubleclickms
    pattern_e _patternMode = PATTERN_NONE; // kbd is in pattern mode 0=NONE, 1=START, 2=RUN, 3=END
    bool _waitHandled = false;             // wait until kbd handled req to call handled()
    uint32_t _rawReadMS = 0;               // keys read ms
    uint32_t _stableMS = 0;                // ms when keys are stable (no bounce)
    uint32_t _firstPressMS = 0;            // stable keycode first pressed
    uint32_t _lastPressMS = 0;             // stable keycode last pressed if same as before
    uint32_t _releaseMS = 0;               // key released
    uint32_t _durationMS = 0;              // duration of keycode pressed
    uint32_t _lastInfoMS;                  // last time info was shown
                                           //
    uint32_t _bounceMS = 50;               // bouce time before keycode become valid
    uint32_t _doubleClickMS = 300;         // double click time before keycode become ready to handle
    uint32_t _infoResponse = 500;          // timeout for display key duration
    uint32_t _patternMS = 2500;            // timeount before start pattern mode
    uint8_t _maxPatternLength = 8;         // max length of pattern buffer
                                           //
    bool _showLongPressInfo = true;        // show info when key is long pressed every _infoResponse
    bool _showPatternInfo = true;          // show info when in pattern mode
};
#endif