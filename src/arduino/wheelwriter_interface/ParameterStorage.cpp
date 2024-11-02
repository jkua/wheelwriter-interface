// Parameter storage class for an Arduino Nano RP2040 Connect
// Buffers and write parameters to flash
// Copyright (c) 2024 John Kua <john@kua.fm>
//
#include "ParameterStorage.h"

using namespace std::literals::string_literals; 

const std::string ParameterStorage::startString_ = "PARAM STORAGE START";
const std::string ParameterStorage::endString_ = "PARAM STORAGE END";

void ParameterStorage::printBlockDeviceInfo() {
  flashManager_.printInfo();
}
int ParameterStorage::readParameter(const std::string& key, std::string& value) {
  auto it = parameters_.find(key);
  if (it != parameters_.end()) {
    value = it->second;
    return 1;
  }
  return 0;
}
int ParameterStorage::writeParameter(const std::string& key, const std::string& value) {
  if (key.size() > maxStringLength_ || value.size() > maxStringLength_) {
    return 0;
  }
  std::string curValue;
  if (readParameter(key, curValue)) {
    if (curValue == value) {
      return 1;
    }
  }
  parameters_[key] = value;
  changed = 1;
  return 1;
}
int ParameterStorage::deleteParameter(const std::string& key) {
  auto it = parameters_.find(key);
  if (it != parameters_.end()) {
    parameters_.erase(it);
    changed = 1;
    return 1;
  }
  return 0;
}

int ParameterStorage::loadParametersFromFlash() {
  Serial.println("\nLoading parameters from flash");
  Serial.println("-----------------------------");
  parameters_.clear();

  // Read the first block to get the number of parameters
  Serial.println("Reading header block...");
  flashManager_.readBlock(0, storageBlockLength_);
  std::string startString(flashManager_.data(), storageBlockLength_);
  if (startString.substr(0, startString_.size()) != startString_ ){
    Serial.println("--> No start string found! Initializing flash!");
    storeParametersToFlash(true, false);
    return 0;
  }
  int numParams = std::stoi(startString.substr(startString_.size()));
  Serial.println("--> " + String(numParams) + " parameters");

  for (int i = 0; i < numParams; i++) {
    flashManager_.readBlock((2*i+1)*storageBlockLength_, storageBlockLength_);
    std::string key(flashManager_.data());
    flashManager_.readBlock((2*i+2)*storageBlockLength_, storageBlockLength_);
    std::string value(flashManager_.data());
    parameters_[key] = value;
  }

  flashManager_.readBlock((2*numParams+1)*storageBlockLength_, storageBlockLength_);
  std::string endString(flashManager_.data(), endString_.size());
  if (endString_ != endString) {
    Serial.print("--> No end string found! Got ");
    Serial.println(endString.c_str());
  }
  return 1;
}

int ParameterStorage::storeParametersToFlash(bool force, bool dryRun) {
  Serial.println("\nStoring parameters to flash");
  if (!changed && !force) {
    Serial.println("--> No changes to store, skipping");
    return 1;
  }
  
  // Write parameter map to buffer
  size_t numBlocks = parameters_.size() * 2 + 2;
  size_t bufferSize = numBlocks * storageBlockLength_;
  char* buffer = new char[bufferSize]();

  std::string header = startString_ + " "s + std::to_string(parameters_.size());
  memcpy(buffer, header.c_str(), header.size());
  int paramIdx = 0;
  for (auto const& x : parameters_) {
    memcpy(buffer+(2*paramIdx+1)*storageBlockLength_, x.first.c_str(), storageBlockLength_);
    memcpy(buffer+(2*paramIdx+2)*storageBlockLength_, x.second.c_str(), storageBlockLength_);
    paramIdx++;
  }
  memcpy(buffer+(2*parameters_.size()+1)*storageBlockLength_, endString_.c_str(), endString_.size());

  Serial.println("\nBuffer");
  Serial.println("------");
  for (int i = 0; i < numBlocks; i++) {
    Serial.print("Block " + String(i) + ": ");
    Serial.println(buffer+i*storageBlockLength_);
  }

  if (dryRun) {
    Serial.println("Dry run -> will not actually write to flash");
    delete[] buffer;
    return 1;
  }

  // Erase flash
  flashManager_.erase(0, bufferSize);

  // Write buffer to flash
  flashManager_.program(0, buffer, bufferSize);

  delete[] buffer;
  return 1;
}

int ParameterStorage::printParameters(bool hidePasswords) {
  int paramIdx = 0;
  for (auto const& x : parameters_) {
    if (hidePasswords && x.first.find("PASSWORD") != std::string::npos) {
      Serial.println("    " + String(paramIdx) + ") " + x.first.c_str() + ", ********");
    }
    else {
      Serial.println("    " + String(paramIdx) + ") " + x.first.c_str() + ", " + x.second.c_str());
    }
    paramIdx++;
  }
}
