#include <WiFi.h>
#include <ESP32Servo.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// WiFi Credentials
const char* ssid = "babbirA";
const char* password = "123456789";

// Firebase project configuration
#define API_KEY "YOUR-FIREBASE-API-KEY"
#define FIREBASE_PROJECT_ID "robotic-arm"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool tasksRunning = false;

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

void setup_wifi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void moveServo(int servoId, int position) {
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

void processDocument(FirebaseJson* payload) {
    FirebaseJsonData axis1, axis2, axis3, axis4, axis5, axis6;
    
    payload->get(axis1, "fields/axis1/integerValue");
    payload->get(axis2, "fields/axis2/integerValue");
    payload->get(axis3, "fields/axis3/integerValue");
    payload->get(axis4, "fields/axis4/integerValue");
    payload->get(axis5, "fields/axis5/integerValue");
    payload->get(axis6, "fields/axis6/integerValue");
    
    if(axis1.success) moveServo(1, axis1.to<int>());
    if(axis2.success) moveServo(2, axis2.to<int>());
    if(axis3.success) moveServo(3, axis3.to<int>());
    if(axis4.success) moveServo(4, axis4.to<int>());
    if(axis5.success) moveServo(5, axis5.to<int>());
    if(axis6.success) moveServo(6, axis6.to<int>());
}

void firestoreStreamCallback(FirebaseStream data) {
    Serial.println("Stream Data...");
    Serial.println(data.payload().c_str());
    
    FirebaseJson payload;
    payload.setJsonData(data.payload().c_str());
    processDocument(&payload);
}

void firestoreStreamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("Stream timeout, resuming...");
    }
    if (!fbdo.httpConnected()) {
        Serial.println("Error code: " + String(fbdo.httpCode()));
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    
    // Initialize Firebase
    config.api_key = API_KEY;
    config.token_status_callback = tokenStatusCallback;
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    // Attach servos
    baseServo.attach(basePin);
    shoulderServo.attach(shoulderPin);
    elbowServo.attach(elbowPin);
    wristServo.attach(wristPin);
    wristRotServo.attach(wristRotPin);
    gripperServo.attach(gripperPin);
    
    // Initial position
    moveServo(1, 90);
    moveServo(2, 90);
    moveServo(3, 90);
    moveServo(4, 90);
    moveServo(5, 90);
    moveServo(6, 90);

    // Set up Firestore listener
    if (!tasksRunning) {
        String documentPath = "roboticArms/armDegree";
        Firebase.Firestore.beginStream(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str());
        Firebase.Firestore.setStreamCallback(&fbdo, firestoreStreamCallback, firestoreStreamTimeoutCallback);
        tasksRunning = true;
    }
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        setup_wifi();
    }
    
    if (!tasksRunning) {
        String documentPath = "roboticArms/armDegree";
        Firebase.Firestore.beginStream(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str());
        Firebase.Firestore.setStreamCallback(&fbdo, firestoreStreamCallback, firestoreStreamTimeoutCallback);
        tasksRunning = true;
    }
    
    delay(10); // Small delay to prevent watchdog timer issues
}