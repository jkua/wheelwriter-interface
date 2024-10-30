// Parameter storage class
// Buffers and write parameters to flash

// Parameter keys and values are stored as strings up to 63 characters long
// This is tuned for the RP2040 which programs in 256 byte blocks and erases in 4KB blocks (31 params + start/end)

// Flash storage format - 64 byte blocks
// -------------------------------------
// Block 0: START PARAM STORAGE <num parameters>
// Block 1: <parameter 0 key>
// Block 2: <parameter 0 value>
// Block 3: <parameter 1 key>
// Block 4: <parameter 1 value>
// Block (2n+1): <parameter n key>
// Block (2n+2): <parameter n value>
// Block (2n+3): END PARAM STORAGE

// Ensures that this file is only included once
#pragma once 

#include <string>
#include <map>
#include <Arduino.h>
#include "FlashIAPManager.h"

class ParameterStorage {
public:
  ParameterStorage() : changed(0) {}
  int readParameter(const std::string& key, std::string& value);
  int writeParameter(const std::string& key, const std::string& value);
  int deleteParameter(const std::string& key);
  int loadParametersFromFlash();
  int storeParametersToFlash(bool force=false, bool dryRun=false);
  int printParameters(bool hidePasswords=true);
  void printBlockDeviceInfo();

private:
  int changed;
  FlashIAPBlockDeviceManager flashManager_;
  std::map<std::string, std::string> parameters_;
  static const size_t storageBlockLength_ = 64;
  static const size_t maxStringLength_ = storageBlockLength_ - 1;
  static const size_t parameterStorageSize_ = storageBlockLength_ * 2;
  static const std::string startString_;
  static const std::string endString_;
};