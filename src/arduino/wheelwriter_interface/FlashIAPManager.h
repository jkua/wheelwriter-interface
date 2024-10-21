// Flash helper class for mbed
// Based on: https://docs.arduino.cc/tutorials/portenta-h7/reading-writing-flash-memory/

// Ensures that this file is only included once
#pragma once 

#include <Arduino.h>
#include <FlashIAP.h>
#include <FlashIAPBlockDevice.h>

using namespace mbed;

class FlashIAPBlockDeviceManager {
public:
  FlashIAPBlockDeviceManager() : readBuffer_(NULL), initStatus_(0) {
    getFlashIAPLimits();
    initBlockDevice();
  }
  ~FlashIAPBlockDeviceManager() {
    blockDevice_->deinit();
    if (readBuffer_) {
      delete[] readBuffer_;
      readBuffer_ = NULL;
    }
  }
  int readBlock(size_t blockIndex, size_t dataSize) {
    if (dataSize % readBlockSize_) {
      return 0;
    }
    if (readBuffer_ && (dataSize != readBufferSize_)) {
      delete readBuffer_;
      readBuffer_ = NULL;
    }
    if (!readBuffer_) {
      readBuffer_ = new char[dataSize];
    }
    int result = blockDevice_->read(readBuffer_, blockIndex*readBlockSize_, dataSize);
    return result;
  }
  int programBlock(size_t blockIndex, char* buffer, size_t dataSize) {
    if (dataSize % programBlockSize_) {
      return 0;
    }
    Serial.println("Programming block at " + String(blockIndex));
    return blockDevice_->program(buffer, blockIndex*programBlockSize_, dataSize);
  }
  int program(size_t blockIndex, char* buffer, size_t dataSize) {
    size_t blocksToProgram = dataSize / programBlockSize_;
    if (dataSize % programBlockSize_) {
      blocksToProgram++;
    }
    Serial.println("Programming " + String(blocksToProgram) + " blocks at address " + String(blockIndex));
    char* bufferEnd = buffer + dataSize;
    for (int i = 0; i < blocksToProgram; i++) {
      char* programBuffer = new char[programBlockSize_]();
      char* start = buffer + programBlockSize_ * i;
      size_t sizeToCopy = programBlockSize_;
      if ((start + sizeToCopy) > bufferEnd) {
        sizeToCopy = bufferEnd - start;
      }
      memcpy(programBuffer, start, sizeToCopy);
      programBlock(blockIndex + i, programBuffer, programBlockSize_);
      delete[] programBuffer;
    }
  }
  int eraseBlock(size_t blockIndex) {
    Serial.println("Erasing block at " + String(blockIndex));
    return blockDevice_->erase(blockIndex*eraseBlockSize_, eraseBlockSize_);
  }
  int erase(size_t blockIndex, size_t dataSize) {
    size_t blocksToErase = dataSize / eraseBlockSize_;
    if (dataSize % eraseBlockSize_) {
      blocksToErase++;
    }
    Serial.println("Erasing " + String(blocksToErase) + " blocks at address " + String(blockIndex));
    return blockDevice_->erase(blockIndex*eraseBlockSize_, blocksToErase*eraseBlockSize_);
  }
  const char* data() {
    return readBuffer_;
  }
  void printInfo() {
    if (initStatus_ != 2) {
      Serial.println("FlashIAPManager::printInfo() - FlashIAPManager not initialized!");
      return;
    }
    Serial.println("\nFlashIAP block device info");
    Serial.println("--------------------------");
    Serial.print("Flash Size: ");
    Serial.print(flashSize_ / 1024.0 / 1024.0);
    Serial.println(" MB");
    Serial.print("FlashIAP Start Address: 0x");
    Serial.println(startAddress_, HEX);
    Serial.print("FlashIAP Size: ");
    Serial.print(availableSize_ / 1024.0 / 1024.0);
    Serial.println(" MB");
    Serial.println("Block device size: " + String((unsigned int) blockDevice_->size() / 1024.0 / 1024.0) + " MB");
    Serial.println("Readable block size: " + String((unsigned int) blockDevice_->get_read_size())  + " bytes");
    Serial.println("Programmable block size: " + String((unsigned int) programBlockSize_) + " bytes");
    Serial.println("Erasable block size: " + String((unsigned int) eraseBlockSize_ / 1024) + " KB");
  }
  uint32_t startAddress() {
    return startAddress_;
  }
  uint32_t availableSize() {
    return availableSize_;
  }
  uint32_t eraseBlockSize() {
    return eraseBlockSize_;
  }
  uint32_t writeBlockSize() {
    return programBlockSize_;
  }
  uint32_t readBlockSize() {
    return readBlockSize_;
  }
  
private:
  void getFlashIAPLimits() {
    // Alignment lambdas
    auto align_down = [](uint64_t val, uint64_t size) {
      return (((val) / size)) * size;
    };
    auto align_up = [](uint32_t val, uint32_t size) {
      return (((val - 1) / size) + 1) * size;
    };

    FlashIAP flash;
    auto result = flash.init();
    if (result != 0) {
      initStatus_ = 0;
      return;
    }

    // Find the start of first sector after text area
    int sectorSize = flash.get_sector_size(FLASHIAP_APP_ROM_END_ADDR);
    startAddress_ = align_up(FLASHIAP_APP_ROM_END_ADDR, sectorSize);
    flashStartAddress_ = flash.get_flash_start();
    flashSize_ = flash.get_flash_size();

    result = flash.deinit();

    availableSize_ = flashStartAddress_ + flashSize_ - startAddress_;
    if (availableSize_ % (sectorSize * 2)) {
      availableSize_ = align_down(availableSize_, sectorSize * 2);
    }

    initStatus_ = 1;
    return;
  }
  void initBlockDevice() {
    if (initStatus_ != 1) {
      return;
    }
    blockDevice_ = std::shared_ptr<FlashIAPBlockDevice>(new FlashIAPBlockDevice(startAddress_, availableSize_));
    blockDevice_->init();
    eraseBlockSize_ = blockDevice_->get_erase_size();
    programBlockSize_ = blockDevice_->get_program_size();
    readBlockSize_ = blockDevice_->get_read_size();
    initStatus_ = 2;
    return;
  }

  std::shared_ptr<FlashIAPBlockDevice> blockDevice_;
  char* readBuffer_;
  size_t readBufferSize_;
  int initStatus_;
  size_t eraseBlockSize_;
  size_t programBlockSize_;
  size_t readBlockSize_;
  uint32_t startAddress_;
  uint32_t flashStartAddress_;
  size_t flashSize_;
  uint32_t availableSize_;
};
