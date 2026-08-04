#ifndef SERIAL_RECEIVER_H
#define SERIAL_RECEIVER_H

#include <Arduino.h>

class SerialReceiver {
private:
    int rawX = 320; 
    int rawY = 240;
    bool fistClosed = false;
    bool newDataAvailable = false;

public:
    void update() {
        while (Serial.available() > 0) { 
            String incomingData = Serial.readStringUntil('\n');
            incomingData.trim();

            if (incomingData.length() == 0) continue;

            if (incomingData == "BLINK") {
                fistClosed = true;
                newDataAvailable = true;
            } 
            else if (incomingData == "IDLE") {
                rawX = 320;
                rawY = 240;
                fistClosed = false; 
                newDataAvailable = true;
            }
            else {
                int cx = 0;
                int cy = 0;
                int parsed = sscanf(incomingData.c_str(), "%d,%d", &cx, &cy);
                
                if (parsed == 2) {
                    rawX = cx;
                    rawY = cy;
                    fistClosed = false; 
                    newDataAvailable = true;
                }
            }
        }
    }

    int getRawX() const { return rawX; }
    int getRawY() const { return rawY; }
    bool isFistClosed() const { return fistClosed; }
    
    bool hasNewData() {
        bool state = newDataAvailable;
        newDataAvailable = false; 
        return state;
    }
};

#endif