#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "No 303"
#define WIFI_PASSWORD "3.14159265"

// Firebase credentials
#define API_KEY "AIzaSyCcf7QnocO_XnRpU2fP5wfhS_nPRuJKpHs"
#define DATABASE_URL "https://robotic-arm-38d44-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Create Servo Objects
Servo baseServo;      // Base rotation
Servo shoulderServo;  // Shoulder joint
Servo elbowServo;     // Elbow joint
Servo wristServo;     // Wrist up/down
Servo wristRotServo;  // Wrist rotation
Servo gripperServo;   // Gripper open/close

// GPIO Pin Assignments
const int basePin = 13;      
const int shoulderPin = 12;  
const int elbowPin = 14;     
const int wristPin = 27;     
const int wristRotPin = 26;  
const int gripperPin = 32;   

// Servo positions
int axis1Pos = 90;
int axis2Pos = 90;
int axis3Pos = 90;
int axis4Pos = 90;
int axis5Pos = 90;
int axis6Pos = 90;

void moveServo(int servoId, int position) {
    if (position < 0 || position > 180) return;
    
    Serial.print("Moving servo ");
    Serial.print(servoId);
    Serial.print(" to position ");
    Serial.println(position);

    switch (servoId) {
        case 1: baseServo.write(position); break;
        case 2: shoulderServo.write(position); break;
        case 3: elbowServo.write(position); break;
        case 4: wristServo.write(position); break;
        case 5: wristRotServo.write(position); break;
        case 6: gripperServo.write(position); break;
    }
}

void streamCallback(FirebaseStream data)
{
    Serial.println("Stream Data Changed!");
    Serial.printf("Path: %s\n", data.dataPath().c_str());
    Serial.printf("Type: %s\n", data.dataType().c_str());
    Serial.printf("Data: %s\n", data.stringData().c_str());

    String path = data.dataPath();
    
    if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json) {
        FirebaseJson json;
        FirebaseJsonData result;
        
        json.setJsonData(data.stringData());
        
        // Check each axis
        if (json.get(result, "axis1")) {
            axis1Pos = result.intValue;
            moveServo(1, axis1Pos);
        }
        if (json.get(result, "axis2")) {
            axis2Pos = result.intValue;
            moveServo(2, axis2Pos);
        }
        if (json.get(result, "axis3")) {
            axis3Pos = result.intValue;
            moveServo(3, axis3Pos);
        }
        if (json.get(result, "axis4")) {
            axis4Pos = result.intValue;
            moveServo(4, axis4Pos);
        }
        if (json.get(result, "axis5")) {
            axis5Pos = result.intValue;
            moveServo(5, axis5Pos);
        }
        if (json.get(result, "axis6")) {
            axis6Pos = result.intValue;
            moveServo(6, axis6Pos);
        }
        
        // Print all current positions
        Serial.println("\nCurrent Positions:");
        Serial.printf("Axis 1: %d\n", axis1Pos);
        Serial.printf("Axis 2: %d\n", axis2Pos);
        Serial.printf("Axis 3: %d\n", axis3Pos);
        Serial.printf("Axis 4: %d\n", axis4Pos);
        Serial.printf("Axis 5: %d\n", axis5Pos);
        Serial.printf("Axis 6: %d\n", axis6Pos);
    }
}

void streamTimeoutCallback(bool timeout)
{
    if (timeout) {
        Serial.println("Stream timeout, resuming...");
    }
    if (!fbdo.httpConnected()) {
        Serial.printf("Error code: %d, reason: %s\n", fbdo.httpCode(), fbdo.errorReason().c_str());
    }
}

void setup()
{
    Serial.begin(115200);
    delay(2000); // Give serial time to initialize
    
    Serial.println("\nStarting up...");
    
    // Initialize Servos
    baseServo.attach(basePin);
    shoulderServo.attach(shoulderPin);
    elbowServo.attach(elbowPin);
    wristServo.attach(wristPin);
    wristRotServo.attach(wristRotPin);
    gripperServo.attach(gripperPin);
    
    // Set initial positions
    moveServo(1, 90);
    moveServo(2, 90);
    moveServo(3, 90);
    moveServo(4, 90);
    moveServo(5, 90);
    moveServo(6, 90);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        Serial.print(".");
        delay(300);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to WiFi. Restarting...");
        ESP.restart();
    }
    
    Serial.println("\nConnected with IP: ");
    Serial.println(WiFi.localIP());
    
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.timeout.serverResponse = 5000;
    
    Serial.println("Signing up to Firebase...");
    
    if (Firebase.signUp(&config, &auth, "", "")) {
        Serial.println("Authentication successful");
        config.token_status_callback = tokenStatusCallback;
        
        Firebase.begin(&config, &auth);
        Firebase.reconnectWiFi(true);
        
        // Set larger buffer size
        fbdo.setBSSLBufferSize(4096, 1024);
        
        Serial.println("Setting up stream...");
        
        if (!Firebase.RTDB.beginStream(&fbdo, "roboticArms/armDegree")) {
            Serial.printf("Stream begin error, %s\n", fbdo.errorReason().c_str());
        }
        
        Firebase.RTDB.setStreamCallback(&fbdo, streamCallback, streamTimeoutCallback);
        Serial.println("Stream setup completed");
    }
    else {
        Serial.printf("Authentication failed: %s\n", config.signer.signupError.message.c_str());
        Serial.println("Restarting...");
        delay(2000);
        ESP.restart();
    }
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected. Restarting...");
        ESP.restart();
    }
    
    if (Firebase.ready()) {
        if (!Firebase.RTDB.readStream(&fbdo)) {
            Serial.printf("Stream read error, %s\n", fbdo.errorReason().c_str());
        }
    }
    
    delay(10);
}