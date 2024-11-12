// Wheelwriter command line interface
// Copyright (c) 2024 John Kua <john@kua.fm>
//
#pragma once

#include <stdarg.h>

#include <Arduino.h>
#include <WifiNINA.h>
#include "Wheelwriter.h"
#include "utility.h"

namespace wheelwriter {

enum wwcli_interface_type {
  IF_SERIAL,
  IF_TYPEWRITER
};
enum wwcli_log_level {
  IF_LOG_ERROR,
  IF_LOG_WARN,
  IF_LOG_INFO,
  IF_LOG_DEBUG
};

class WheelwriterCommandLineInterface {
public:
  WheelwriterCommandLineInterface(Wheelwriter& typewriter, wwcli_interface_type interfaceType, wwcli_log_level logLevel=IF_LOG_INFO, size_t bufferSize=256)
      : typewriter_(typewriter), 
        interfaceType_(interfaceType),
        bufferSize_(bufferSize),
        logLevel_(logLevel) {
      inputBuffer_ = new char[bufferSize];
      outputBuffer_ = new char[bufferSize];
      }
  ~WheelwriterCommandLineInterface() {
    delete[] inputBuffer_;
    delete[] outputBuffer_;
  }
  void print(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer_, bufferSize_, format, args);
    va_end(args);

    size_t length = strlen(outputBuffer_);
    if (!length) {
      return;
    }
    size_t lastPos = strlen(outputBuffer_) - 1;

    if (interfaceType_ == IF_SERIAL) {
      Serial.print(outputBuffer_);
    }
    else {
      bool newLine;
      if (outputBuffer_[lastPos] == '\n') {
        newLine = true;
        outputBuffer_[lastPos] = '\0';
      }
      else {
        newLine = false;
      }
      typewriter_.typeAsciiString(outputBuffer_, TYPESTYLE_NORMAL, newLine);
    }
  }
  void logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer_, bufferSize_, format, args);
    va_end(args);

    _log("[ERROR] ");
  }
  void logWarn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer_, bufferSize_, format, args);
    va_end(args);

    _log("[ WARN] ");
  }
  void logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer_, bufferSize_, format, args);
    va_end(args);

    _log("[ INFO] ");
  }
  void logDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer_, bufferSize_, format, args);
    va_end(args);

    _log("[DEBUG] ");
  }
  char* readLine() {
    if (interfaceType_ == IF_SERIAL) {
      Serial.readBytesUntil('\n', inputBuffer_, bufferSize_);
    }
    else {
      typewriter_.waitReady(MOVE_CARRIAGE);
      std::string line;
      typewriter_.readLine(line, 0, false, true);
      strncpy(inputBuffer_, line.c_str(), bufferSize_);
    }
    logInfo("Read line: %s\n", inputBuffer_);

    return inputBuffer_;
  }

private:
  void _log(const char* prefix) {
    Serial.print(prefix);
    Serial.print(outputBuffer_);
  }
  Wheelwriter& typewriter_;
  wwcli_interface_type interfaceType_;
  char* inputBuffer_;
  char* outputBuffer_;
  size_t bufferSize_;
  wwcli_log_level logLevel_;
};

} // namespace wheelwriter