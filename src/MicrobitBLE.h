#ifndef MICROBIT_BLE_H
#define MICROBIT_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <nvs_flash.h>

static BLEUUID btnServiceUUID("E95D9882-251D-470A-A062-FA1922DFA9A8");
static BLEUUID btnACharUUID("E95DDA90-251D-470A-A062-FA1922DFA9A8");
static BLEUUID btnBCharUUID("E95DDA91-251D-470A-A062-FA1922DFA9A8");

volatile int mbTiltX = 0;
volatile int mbTiltY = 0; 
volatile int mbBtnA = 0;
volatile int mbBtnB = 0;
volatile bool mbNewData = false;

// --- THE GRACE PERIOD TRACKERS ---
volatile int rawA = 0;
volatile int rawB = 0;
volatile bool actionPending = false;
volatile unsigned long pendingActionTime = 0;

static void btnACallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length >= 1) {
        rawA = pData[0];
        actionPending = true;
        pendingActionTime = millis();
    }
}

static void btnBCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (length >= 1) {
        rawB = pData[0];
        actionPending = true;
        pendingActionTime = millis();
    }
}

class MicrobitBLE {
private:
    bool doConnect = false;
    BLEAdvertisedDevice* myDevice = nullptr;

    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
        MicrobitBLE* parent;
    public:
        MyAdvertisedDeviceCallbacks(MicrobitBLE* p) : parent(p) {}
        void onResult(BLEAdvertisedDevice advertisedDevice) {
            std::string deviceName = advertisedDevice.getName();
            if (deviceName.find("pezip") != std::string::npos) { //Change name to microbit name
                BLEDevice::getScan()->stop();
                parent->setDevice(new BLEAdvertisedDevice(advertisedDevice));
            }
        }
    };

public:
    void setDevice(BLEAdvertisedDevice* device) {
        myDevice = device;
        doConnect = true;
    }

    void init() {
        Serial.println(">> Wiping ESP32 Cache...");
        nvs_flash_erase();
        nvs_flash_init();
        
        if (!BLEDevice::getInitialized()) BLEDevice::init("");
        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
        pBLEScan->setActiveScan(true);
        pBLEScan->start(10, false); 
    }

    void update() {
        if (doConnect) {
            Serial.println("\n>> Connecting to NATIVE Controller... <<");
            BLEClient* pClient = BLEDevice::createClient();
            
            if (pClient->connect(myDevice)) {
                delay(1000); 
                BLERemoteService* pBtnSvc = pClient->getService(btnServiceUUID);
                if (pBtnSvc != nullptr) {
                    
                    BLERemoteCharacteristic* pBtnAChar = pBtnSvc->getCharacteristic(btnACharUUID);
                    if (pBtnAChar != nullptr) {
                        Serial.println(">> Subscribing to Button A...");
                        pBtnAChar->registerForNotify(btnACallback);
                        delay(500); 
                    }

                    BLERemoteCharacteristic* pBtnBChar = pBtnSvc->getCharacteristic(btnBCharUUID);
                    if (pBtnBChar != nullptr) {
                        Serial.println(">> Subscribing to Button B...");
                        pBtnBChar->registerForNotify(btnBCallback);
                        delay(500); 
                    }
                }
                Serial.println(">> [ZIPIP] CONTROLLER ACTIVE! PRESS A OR B TO MOVE EYES! <<");
            }
            doConnect = false; 
        }

        // --- THE DEBOUNCER & TRANSLATOR LOGIC ---
        if (actionPending && (millis() - pendingActionTime > 150)) {
            
            if (rawA == 1 && rawB == 1) {
                // A+B Pressed together -> ALLOW BLINK
                mbBtnA = 1; mbBtnB = 1;
                mbTiltX = 0; 
                Serial.println(">> ACTION: BLINK (A+B)");
                
            } else if (rawA == 1 && rawB == 0) {
                // Only A Pressed -> LOOK LEFT
                mbBtnA = 0; mbBtnB = 0; // Swallowed the button press so no wink!
                mbTiltX = 1023; // Full turn radius
                Serial.println(">> ACTION: LOOK LEFT");
                
            } else if (rawB == 1 && rawA == 0) {
                // Only B Pressed -> LOOK RIGHT
                mbBtnA = 0; mbBtnB = 0; // Swallowed the button press so no wink!
                mbTiltX = -1023; // Full turn radius
                Serial.println(">> ACTION: LOOK RIGHT");
                
            } else {
                // Buttons released -> CENTER
                mbBtnA = 0; mbBtnB = 0;
                mbTiltX = 0; // *NOTE: Delete this line if you want the eyes to hold their pose when you let go!
                Serial.println(">> ACTION: CENTER (RELEASED)");
            }
            
            mbNewData = true;
            actionPending = false;
        }
    }
};

#endif