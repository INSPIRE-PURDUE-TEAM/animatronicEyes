#ifndef EYE_CONVERTER_H
#define EYE_CONVERTER_H

#include <Arduino.h>

struct SpatialTarget {
    float x;
    float y; 
    float z; 
    bool holdClosed;
};

class EyeConverter {
private:
    const int CAM_WIDTH = 640;
    const int CAM_HEIGHT = 480;

    // Workspace limits in physical millimeters
    const float X_WORKSPACE_MM = 160.0;  
    const float Z_WORKSPACE_MM = 110.0;  
    const float DEFAULT_DEPTH  = 450.0; // Perfect depth to keep eyes parallel

public:
    SpatialTarget convert(int rawX, int rawY, bool fistState) {
        SpatialTarget target;

        // X-AXIS: Left/Right tracking
        // NOTE: If the eyes move left when your hand goes right, swap the last two numbers to: -X_WORKSPACE_MM, X_WORKSPACE_MM
        target.x = map(rawX, 0, CAM_WIDTH, X_WORKSPACE_MM, -X_WORKSPACE_MM);
        
        // Y-AXIS: Depth (Distance forward from face)
        target.y = DEFAULT_DEPTH; 
        
        // Z-AXIS: Up/Down tracking
        // NOTE: If the eyes look down when your hand goes up, swap the last two numbers to: -Z_WORKSPACE_MM, Z_WORKSPACE_MM
        target.z = map(rawY, 0, CAM_HEIGHT, Z_WORKSPACE_MM, -Z_WORKSPACE_MM);
        
        target.holdClosed = fistState;

        return target;
    }
};

#endif