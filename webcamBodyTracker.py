import cv2
from cvzone.HandTrackingModule import HandDetector
import serial
import time

# --- SERIAL SETUP ---
# Adjust port string if your device moves to another COM assignment
try:
    esp32 = serial.Serial(port='COM5', baudrate=115200, timeout=0.1)
    time.sleep(2)  # Give the ESP32 a brief window to re-initialize safely
    print("Successfully opened connection to ESP32 on COM5")
except Exception as e:
    print(f"CRITICAL SERIAL ERROR: {e}")
    print("Ensure your ESP32 is plugged in and PlatformIO's Serial Monitor is closed!")
    exit()

# --- HARDWARE CAMERA & TRACKER INITIALIZATION ---
cap = cv2.VideoCapture(0)
detector = HandDetector(maxHands=1, detectionCon=0.8)

# Debounce counter variable to eliminate tracking frame noise
fist_counter = 0

while True:
    success, img = cap.read()
    if not success:
        print("Webcam frame capture failure.")
        break

    # Analyze the camera matrix for hand structures
    hands, img = detector.findHands(img, draw=True)

    if hands:
        hand1 = hands[0]
        
        # Pull center bounding coordinates
        cx, cy = hand1["center"][0], hand1["center"][1]
        
        # Format coordinate data string with trailing newline 
        data_string = f"{cx},{cy}\n"

        # Read specific extended finger metrics
        fingers = detector.fingersUp(hand1)

        # Handle active fist tracking
        if fingers == [0, 0, 0, 0, 0]:
            fist_counter += 1
            # Require 3 sequential stable frames to trigger target activation
            if fist_counter >= 3:
                print("Fist held! Sending BLINK command...")
                esp32.write(b"BLINK\n")
        else:
            # Active tracking mode: reset flag count and push clean coordinates
            fist_counter = 0 
            esp32.write(bytes(data_string, 'utf-8'))
            print(f"Sending tracking metrics: X={cx}, Y={cy}")
            
        # Draw a visual target target circle on the open palm structure
        cv2.circle(img, (cx, cy), 10, (0, 255, 0), cv2.FILLED)

    else:
        # Hand has left camera profile boundary limit -> Run safe out-of-frame sleep routine
        fist_counter = 0
        print("Target lost. Sending out-of-frame sleep instruction...")
        esp32.write(b"OUT\n")

    # Open screen window interface
    cv2.imshow("Puppet Core Tracker Engine", img)
    
    # Gracefully disconnect when pressing 'q' inside video window canvas
    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("Terminating hand tracker app...")
        break

# Clean resource structures up on termination
cap.release()
cv2.destroyAllWindows()
esp32.close()