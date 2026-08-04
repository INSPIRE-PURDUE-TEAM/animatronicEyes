# animatronicEyes
INSPIRE Labs Purdue - Servo Project


# ESP32 Multi-Mode Animatronic Eyes

## Overview
This project controls a highly articulated, dual-eye animatronic system using an ESP32. The system calculates inverse kinematics to drive six servo motors, keeping the eyes parallel and naturally focused at a specific depth. It features three distinct operating modes: a Bluetooth micro:bit controller, a serial-based hand tracker, and an autonomous random idle mode.

## Hardware & Pin Map
The project requires six servo motors (three per eye) and three physical push buttons configured with internal pull-ups.

### Left Eye
| Mechanism | ESP32 Pin |
| :--- | :--- |
| Left/Right (Pan) | GPIO 4 |
| Up/Down (Tilt) | GPIO 22 |
| Eyelid (Open/Close)| GPIO 23 |

### Right Eye
| Mechanism | ESP32 Pin |
| :--- | :--- |
| Left/Right (Pan) | GPIO 19 |
| Up/Down (Tilt) | GPIO 18 |
| Eyelid (Open/Close)| GPIO 5 |

### Control Buttons
| Button | Mode | ESP32 Pin |
| :--- | :--- | :--- |
| Button A | Micro:bit BLE Mode | GPIO 16 |
| Button B | Hand Tracker Mode | GPIO 17 |
| Button C | Random Idle Mode | GPIO 21 |

## Operating Modes

### 1. Micro:bit Control Mode (Button A)
* Uses the ESP32's built-in Bluetooth Low Energy (BLE) to scan for a specific micro:bit broadcasting the name "pezip".
* Translates the micro:bit's X/Y tilt data into physical look angles for the servos.
* Pressing Button A on the micro:bit closes the right eyelid, while Button B closes the left eyelid.
* Pressing both buttons simultaneously triggers a full blink.

### 2. Hand Tracker Mode (Button B)
* Listens over the Serial port (115200 baud) for incoming tracking data from an external Python script.
* Converts 2D webcam coordinates (640x480) into a 160mm x 110mm physical 3D workspace using inverse kinematics.
* A closed fist command ("BLINK") detected by the camera causes both eyelids to shut.

### 3. Random Idle Mode (Button C)
* Operates completely autonomously using internal logic.
* Generates randomized XYZ coordinates within safe mechanical limits to simulate natural glancing.
* Triggers periodic, mathematically timed blinks and random double-blinks to maintain a lifelike appearance.

## Software Dependencies
* **ServoEasing**: Used extensively to apply smooth acceleration and deceleration curves to all servo movements.
* **BLEDevice & nvs_flash**: Native ESP32 libraries used for handling the micro:bit Bluetooth connection.

---

## Installation & Setup Guide

Follow these instructions to clone this project from GitHub and upload the firmware to your ESP32 using PlatformIO.

### Step 1: Install Visual Studio Code (VS Code)
1. Go to the [Visual Studio Code website](https://code.visualstudio.com/) and download the installer for your operating system (Windows, Mac, or Linux).
2. Run the installer and follow the standard installation prompts.

### Step 2: Install the PlatformIO Extension
1. Open VS Code.
2. Click on the **Extensions** icon on the left sidebar (it looks like four squares).
3. Type **PlatformIO IDE** in the search bar.
4. Click **Install** on the official extension (usually the first result with an alien logo).
5. *Note: Wait for PlatformIO to finish its background installation. You may be prompted to restart VS Code once it's done.*

### Step 3: Clone the Repository
1. In VS Code, open a new terminal by clicking **Terminal > New Terminal** in the top menu.
2. Navigate to the folder on your computer where you want to save the project (e.g., `cd Documents`).
3. Run the following git command to download the project:
   ```bash
   git clone [https://github.com/INSPIRE-PURDUE-TEAM/animatronicEyes.git](https://github.com/INSPIRE-PURDUE-TEAM/animatronicEyes.git)
4. Go to File > Open Folder... and select the newly created animatronicEyes folder.

### Step 4: Build and Upload the Code
1. Auto-Configuration: Because the repository includes a platformio.ini file, PlatformIO will automatically detect it. Wait a minute or two as it automatically downloads the ESP32 framework, the C++ compiler, and all required libraries (like ServoEasing).

2. Connect the Hardware: Plug your ESP32 into your computer using a data-capable USB cable.

3. Build: Look at the very bottom blue status bar in VS Code. Click the Checkmark icon (✓) to compile the code and ensure there are no errors.

4. Upload: Click the Right Arrow icon (→) right next to the checkmark to upload the compiled firmware directly to the ESP32.

5. Troubleshooting: If the upload fails to connect, you may need to hold down the physical BOOT button on your ESP32 board while it tries to connect.

### Important Notes:
1. Always turn on the Microbit before powering the ESP32
2. To get the 'Hand Tracking' to work, run the 'webcamBodyTracker.py' script in the VS Code Terminal.
3. If the microbit needs to be changed: Go to Line 51 in 'MicrobitBLE.h' and change the device name to the name of the microbit to be used

### ~ Created By Saket Kanagal
