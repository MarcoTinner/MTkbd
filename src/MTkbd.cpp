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

#include "MTkbd.h"

MTkbd::MTkbd()
{
    _patternMode = PATTERN_NONE;
    clearPattern();
    clearData();
}
MTkbd::~MTkbd()
{
    delete _keys;
};

/// @brief setup keyboard with key pins
/// @param activeLow digital inputs are active low or high
/// @param numKeys number of keys to handle with keyboard (1..8)
/// @param keys array of the key pins lsb to msb
/// @return true if settings are correct
bool MTkbd::begin(const bool activeLow, const uint8_t numKeys, const uint8_t keys[])
{
    _patternMode = PATTERN_NONE;
    _activeLow = activeLow;
    if (numKeys < 1 || numKeys > 8)
    {
        return false;
    }

    _numKeys = numKeys;
    _keys = new uint8_t[_numKeys];
    for (uint8_t idx = 0; idx < _numKeys; idx++)
    {
        _keys[idx] = keys[idx];
        if (_activeLow)
            pinMode(_keys[idx], INPUT_PULLUP);
        else
            pinMode(_keys[idx], INPUT_PULLDOWN);
    }
    clearPattern();
    clearData();
    return true;
}

uint8_t MTkbd::KeyCode() { return _keyCode; }
uint8_t MTkbd::Repeat() { return _repeatNr > 0 ? _repeatNr + 1 : 0; }
uint32_t MTkbd::Duration() { return _durationMS; }
bool MTkbd::IsPattern() { return _patternMode != PATTERN_NONE; }
String MTkbd::Pattern() { return String(_patternString); }

/// @brief set waitHandled, the handled function must be called to continue keyboard loop
/// @param waitHandled true if handled function must be called
void MTkbd::setWaitHandled(bool waitHandled) { _waitHandled = waitHandled; }

/// @brief set waitHandled, the handled function must be called to continue keyboard loop
/// @return witHandled
bool MTkbd::getWaitHandled() { return _waitHandled; }

/// @brief set if show key pressed is printed on serial port when longer pressed then showInfo ms
/// @param showInfo true if show info on serial port
void MTkbd::setShowInfo(bool showInfo) { _showLongPressInfo = showInfo; }

/// @brief set if show key pressed is printed on serial port when longer pressed then showInfo ms
/// @return true if show info on serial port
bool MTkbd::getShowInfo() { return _showLongPressInfo; }

/// @brief set to display pattern when chars are added
/// @param showPattern true = yes
void MTkbd::setShowPattern(bool showPattern) { _showPatternInfo = showPattern; }

/// @brief get if display is on when pattern when chars are added
/// @return true = yes
bool MTkbd::getShowPattern() { return _showPatternInfo; }

/// @brief set max length for pattern before automatic end pattern
/// @param maxPatternLength number of characters pattern will be +1 char for '\0'
void MTkbd::setMaxPatternLength(uint8_t maxPatternLength) { _maxPatternLength = maxPatternLength; }

/// @brief get max length for pattern before automatic end pattern
/// @return number of characters pattern will be -1 char for '\0'
uint8_t MTkbd::getMaxPatternLength() { return _maxPatternLength; }

/// @brief time in ms before a pressed key is recognized as stable
/// @param ms timeout
void MTkbd::setBounceMS(uint32_t ms) { _bounceMS = ms; }

/// @brief time in ms before a pressed key is recognized as stable
/// @return timeout
uint32_t MTkbd::getBounceMS() { return _bounceMS; }

/// @brief max time between twice pressing the same key to recognize as multiple press
/// @param ms timeout
void MTkbd::setDoubleClickMS(uint32_t ms) { _doubleClickMS = ms; }

/// @brief max time between twice pressing the same key to recognize as multiple press
/// @return timeout
uint32_t MTkbd::getDoubleClickMS() { return _doubleClickMS; }

/// @brief show info when long press a key after this timout each
/// @param ms timeout
void MTkbd::setInfoResponse(uint32_t ms) { _infoResponse = ms; }

/// @brief show info when long press a key after this timout each
/// @return timeout
uint32_t MTkbd::getInfoResponse() { return _infoResponse; }

/// @brief Set key press timeout in ms to enter/exit pattern mode
/// @param ms timeout
void MTkbd::setPatternMS(uint32_t ms) { _patternMS = ms; }

/// @brief Get key press timeout in ms to enter/exit pattern mode
/// @return timeout
uint32_t MTkbd::getPatternMS() { return _patternMS; }

/// @brief Loop keyboard should run in loop()
void MTkbd::loop()
{
    if (!_waitHandled || (_waitHandled & !_keyCodeReady))
    {
        _rawReadMS = (uint32_t)(esp_timer_get_time() / 1000);
        _rawKeyCode = 0;
        _keyCodeValid = false;
        for (uint8_t idx = 0; idx < _numKeys; idx++)
        {
            bool _state = digitalRead(_keys[idx]) == HIGH;
            _rawKeyCode = _rawKeyCode | (_state << idx);
        }
        if (_activeLow)
            _rawKeyCode = ~_rawKeyCode & 0b11111111 >> (8 - _numKeys);

        if (_lastRawKeyCode != _rawKeyCode) // new rawKeyCode pressed
        {
            _lastRawKeyCode = _rawKeyCode;
            _stableMS = _rawReadMS;
        }
        else                                                        // still the same keyCode pressed
            _keyCodeValid = ((_rawReadMS - _stableMS) > _bounceMS); // keyCode is valid after bounce time

        if (_keyCodeValid)
        {
            if (_rawKeyCode > 0)
                _keyCode = _rawKeyCode;
            _keyDown = _rawKeyCode != 0;

            if (_lastKeyCode != _keyCode) // keycode changed -> print out keycode
            {
                clearData();
                _keyCode = _rawKeyCode;
                _lastKeyCode = _keyCode;
            }

            if (!_keyDown) // all keys released
            {
                if (_releaseMS == 0)
                    _releaseMS = _rawReadMS;

                if (_patternMode == PATTERN_NONE)
                {
                    if (_firstPressMS > 0 && ((_stableMS + _doubleClickMS) < _rawReadMS)) // keycode valid for handle >> keyready
                    {
                        _keyCodeReady = true;
                        _durationMS = _releaseMS - _firstPressMS;
                    }
                }
                else if (_patternMode == PATTERN_START)
                {
                    _patternMode = PATTERN_RUN;
                    clearData();
                    clearPattern();
                }
                else if (_patternMode == PATTERN_RUN)
                {
                    if (_keyCode > 0)
                    {
                        if (_numKeys <= 4)
                        {
                            if (_patternPos < (_maxPatternLength))
                            {
                                char _ch = hex_digit(_keyCode);
                                _pattern[_patternPos] = _ch;
                                _patternPos++;
                                _pattern[_patternPos] = '\0';
                                if (_showPatternInfo)
                                    OUTPORT.println("KBD PatternMode add key '" + String(_ch) + "' -> act pattern is '" + String(_pattern) + "'");
                            }
                            else
                            {
                                OUTPORT.println(F("KBD PatternMode pattern full"));
                                _patternMode = PATTERN_END;
                            }
                        }
                        else
                        {
                            if (_patternPos < (_maxPatternLength - 1))
                            {
                                char _ch0 = byte_to_hex(_keyCode)[0];
                                char _ch1 = byte_to_hex(_keyCode)[1];
                                _pattern[_patternPos] = _ch0;
                                _pattern[_patternPos + 1] = _ch1;
                                _patternPos += 2;
                                _pattern[_patternPos] = '\0';
                                if (_showPatternInfo)
                                    OUTPORT.println("KBD PatternMode add key '" + String(_ch0) + String(_ch1) + "' -> act pattern is '" + String(_pattern) + "'");
                            }
                            else
                            {
                                OUTPORT.println(F("KBD PatternMode pattern full"));
                                _patternMode = PATTERN_END;
                            }
                        }
                        _patternString = String(_pattern).c_str();
                        _keyCode = 0;
                        _lastKeyCode = 0;
                    }
                }
                else if (_patternMode == PATTERN_END)
                {
                    _patternMode = PATTERN_READY;
                    _patternString = String(_pattern);
                    _keyCodeReady = true;
                }
            }
            else // some keys are pressed
            {
                if (_firstPressMS == 0)
                    _firstPressMS = _rawReadMS;
                else
                    _lastPressMS = _rawReadMS;

                if (_keyCode == _lastKeyCode) // keycode is lastkeycode
                {
                    if (_releaseMS > 0)
                    {
                        _repeatNr++;
                    }
                }
                else
                {
                    _releaseMS = 0;
                    _lastKeyCode = 0;
                }
                _durationMS = _rawReadMS - _firstPressMS;

                if (_keyCode == getKeyCodeOfPin(_patternKey) && _durationMS > _patternMS)
                {
                    if (_patternMode == PATTERN_NONE)
                    {
                        _patternMode = PATTERN_START;
                        if (_showPatternInfo)
                            OUTPORT.println(F("KBD PatternMode started"));
                        clearPattern();
                    }
                    else if (_patternMode == PATTERN_RUN)
                    {
                        _patternMode = PATTERN_END;
                        if (_showPatternInfo)
                            OUTPORT.println(F("KBD PatternMode ended"));
                    }
                }
                else if (_repeatNr == 0 &&
                         _durationMS > _infoResponse &&
                         _lastInfoMS + _infoResponse < _rawReadMS &&
                         (_patternMode == PATTERN_NONE || _patternMode == PATTERN_RUN))
                {
                    _lastInfoMS = _rawReadMS;
                    if (_showLongPressInfo)
                        OUTPORT.printf("KBD long pressed KeyCode %i duration %i ms\r\n", _keyCode, _rawReadMS - _firstPressMS);
                }
                _releaseMS = 0;
            }
        }
    }
}

/// @brief key is ready for handling
/// @return true = ready
bool MTkbd::available()
{
    return _keyCodeReady;
}

/// @brief after handled call this to reset keyboard for next keys
void MTkbd::handled()
{
    _keyCodeReady = false;
    _patternMode = PATTERN_NONE;
    _patternPos = 0;
    clearPattern();
    clearData();
}

/// @brief get the keycode for a key pin number
/// @param pin io pin of the key
/// @return keycode of this key when pressed
uint8_t MTkbd::getKeyCodeOfPin(uint8_t pin)
{
    uint8_t _code = 1;
    for (uint8_t idx = 0; idx < _numKeys; idx++)
    {
        if (_keys[idx] == pin)
            return _code << idx;
    }
    return 0;
}

/////////////////////////////////////
///  private functions start here ///
/////////////////////////////////////

/// @brief private for clear pattern
void MTkbd::clearPattern()
{
    _pattern = new char[_maxPatternLength + 1];
    for (uint8_t i = 0; i <= _maxPatternLength; i++)
        _pattern[i] = '\0';
}

/// @brief private for clear data
void MTkbd::clearData()
{
    _keyCode = 0;
    _lastKeyCode = 0;
    _firstPressMS = 0;
    _lastPressMS = 0;
    _durationMS = 0;
    _releaseMS = 0;
    _repeatNr = 0;
}

/// @brief convert signle digit in a hex char
/// @param v digit
/// @return hex char
char MTkbd::hex_digit(uint8_t v)
{
    return "0123456789abcdef"[v & 0xF];
}

/// @brief convert a full 8bit uint8_t value in hex chars
/// @param b value
/// @return hex chars [2]
std::array<char, 2> MTkbd::byte_to_hex(uint8_t b)
{
    return {hex_digit(b >> 4), hex_digit(b)}; // e.g., 0xAB -> {'a','b'}
}