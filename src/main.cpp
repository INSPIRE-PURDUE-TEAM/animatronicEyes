#include "Arduino.h" 
#include "double_eye.h"
#include "SerialReceiver.h"
#include "EyeConverter.h"
#include "MicrobitBLE.h" // NEW: Added your Bluetooth Traffic Cop

Eyes myEyes(4, 22, 23,
            19, 18, 5);

SerialReceiver receiver;
EyeConverter converter;
MicrobitBLE mbListener; // NEW: Starts the Bluetooth scanner

// --- OPTIMIZED MECHANICAL LIMITS (LOCKED IN) ---
const int LEFT_RELAXED_OPEN  = 125; 
const int LEFT_SOFT_CLOSE    = 55;  

const int RIGHT_RELAXED_OPEN = 65;  
const int RIGHT_SOFT_CLOSE   = 105; 

// --- GLOBAL CENTER COORDINATES ---
const int leftHorizCenter  = 155; 
const int rightHorizCenter = 124; 
const int maxHorizSwing    = 45;  

const int leftVertCenter  = 108; //chaning..
const int rightVertCenter = 50;
const int maxVertSwing    = 35; 

// --- BUTTON MODE CONFIGURATION ---
const int BUTTON_A_PIN = 16; 
const int BUTTON_B_PIN = 17; 
const int BUTTON_C_PIN = 21; 

enum OperatingMode {
  MODE_FUTURE_PROJECT,
  MODE_HAND_TRACKER,
  MODE_RANDOM_IDLE
};

OperatingMode currentMode = MODE_HAND_TRACKER; 

void setup() {
  Serial.begin(115200);
  
  mbListener.init(); // NEW: Boot up Bluetooth and look for the Micro:bit

  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_C_PIN, INPUT_PULLUP);
  
  myEyes.leftEye.setLeftRightLowerUpperCentre(leftHorizCenter - maxHorizSwing, leftHorizCenter + maxHorizSwing, leftHorizCenter);
  myEyes.rightEye.setLeftRightLowerUpperCentre(rightHorizCenter - maxHorizSwing, rightHorizCenter + maxHorizSwing, rightHorizCenter);

  myEyes.leftEye.setUpDownLowerUpperCentre(leftVertCenter - maxVertSwing, leftVertCenter + maxVertSwing, leftVertCenter);
  myEyes.rightEye.setUpDownLowerUpperCentre(rightVertCenter - maxVertSwing, rightVertCenter + maxVertSwing, rightVertCenter);

  myEyes.leftEye.setEyeLidOpenClose(LEFT_RELAXED_OPEN, LEFT_SOFT_CLOSE);
  myEyes.rightEye.setEyeLidOpenClose(RIGHT_RELAXED_OPEN, RIGHT_SOFT_CLOSE);

  myEyes.init(); 
  myEyes.home();
}

void forceCenter() {
  myEyes.leftEye.leftRightServo.setEaseTo(leftHorizCenter, 130);
  myEyes.rightEye.leftRightServo.setEaseTo(rightHorizCenter, 130);
  myEyes.leftEye.upDownServo.setEaseTo(leftVertCenter, 130);
  myEyes.rightEye.upDownServo.setEaseTo(rightVertCenter, 130);
  
  myEyes.leftEye.eyeLidServo.setEaseTo(LEFT_RELAXED_OPEN, 130);
  myEyes.rightEye.eyeLidServo.setEaseTo(RIGHT_RELAXED_OPEN, 130);

  synchronizeAllServosAndStartInterrupt(false);
  do { delay(10); } while (!updateAllServos());
  
  delay(300); 
}

void checkButtons() {
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    if (currentMode != MODE_FUTURE_PROJECT) {
      Serial.println(">> MODE: MICROBIT CONTROL <<");
      forceCenter(); 
      currentMode = MODE_FUTURE_PROJECT;
    }
  }
  
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    if (currentMode != MODE_HAND_TRACKER) {
      Serial.println(">> MODE: HAND TRACKER <<");
      forceCenter(); 
      currentMode = MODE_HAND_TRACKER;
    }
  }

  if (digitalRead(BUTTON_C_PIN) == LOW) {
    if (currentMode != MODE_RANDOM_IDLE) {
      Serial.println(">> MODE: RANDOM IDLE <<");
      forceCenter(); 
      currentMode = MODE_RANDOM_IDLE;
    }
  }
}

void loop() {
  checkButtons();
  receiver.update(); 
  mbListener.update(); // NEW: Keep Bluetooth connected in the background

  switch (currentMode) {
    
    // --- MICROBIT MODE ---
    case MODE_FUTURE_PROJECT: {
      static bool mbRightLidClosed = false;
      static bool mbLeftLidClosed = false;

      // NEW: Reads global variables from the Bluetooth file
      if (mbNewData) {
        mbNewData = false; 
            
        // 1. Handle Tilt Tracking
        float targetX = map(mbTiltX, -700, 700, 160, -160); 
        float targetZ = map(mbTiltY, -700, 700, -110, 110); 
        myEyes.lookAt(targetX, 450.0, targetZ, 150);

        // 2. Handle Right Lid (Micro:bit Button A pressed)
        if (mbBtnA == 1) {
          if (!mbRightLidClosed) {
            myEyes.rightEye.eyeLidServo.setEaseTo(RIGHT_SOFT_CLOSE, 140);
            mbRightLidClosed = true;
          }
        } else {
          if (mbRightLidClosed) {
            myEyes.rightEye.eyeLidServo.setEaseTo(RIGHT_RELAXED_OPEN, 140);
            mbRightLidClosed = false;
          }
        }

        // 3. Handle Left Lid (Micro:bit Button B pressed)
        if (mbBtnB == 1) {
          if (!mbLeftLidClosed) {
            myEyes.leftEye.eyeLidServo.setEaseTo(LEFT_SOFT_CLOSE, 140);
            mbLeftLidClosed = true;
          }
        } else {
          if (mbLeftLidClosed) {
            myEyes.leftEye.eyeLidServo.setEaseTo(LEFT_RELAXED_OPEN, 140);
            mbLeftLidClosed = false;
          }
        }
      }
      break;
    }

    // --- HAND TRACKER MODE (UNTOUCHED) ---
    case MODE_HAND_TRACKER: {
      static bool lastFistState = false; 

      if (receiver.hasNewData()) {
        SpatialTarget action = converter.convert(
          receiver.getRawX(), 
          receiver.getRawY(), 
          receiver.isFistClosed()
        );

        if (action.holdClosed) {
          if (!lastFistState) {
            myEyes.leftEye.eyeLidServo.setEaseTo(LEFT_SOFT_CLOSE, 140);
            myEyes.rightEye.eyeLidServo.setEaseTo(RIGHT_SOFT_CLOSE, 140);
            lastFistState = true; 
          }
        } 
        else {
          if (lastFistState) {
            myEyes.leftEye.eyeLidServo.setEaseTo(LEFT_RELAXED_OPEN, 140);
            myEyes.rightEye.eyeLidServo.setEaseTo(RIGHT_RELAXED_OPEN, 140);
            lastFistState = false; 
          }
          myEyes.lookAt(action.x, action.y, action.z, 150);
        }
      }
      break;
    }

    // --- RANDOM IDLE MODE (UNTOUCHED) ---
    case MODE_RANDOM_IDLE:
      myEyes.eyeMotion(450.0);
      break;
  }

  updateAllServos();
}