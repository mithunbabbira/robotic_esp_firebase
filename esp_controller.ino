#include <WiFi.h>
#include <ESP32Servo.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// WiFi Credentials
const char* ssid = "No 303";
const char* password = "3.14159265";

// Firebase project configuration
#define API_KEY "AIzaSyCcf7QnocO_XnRpU2fP5wfhS_nPRuJKpHs"
#define FIREBASE_PROJECT_ID "robotic-arm-38d44"

// Define the service account credentials (required for authentication)
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQD2tuHbowGQ+iQ8\nqCq37qyHJtQaKeI4/C3oxVTrNJMmX6ZX/8hSapbanDkXDgNT9Axc45n+WuXb28tj\n0impoMuHlQVX4EApsdfTKIUoAtmRUYWtedSg0+HV85qSZGKMT7EvS2Vkyeriy1j/\nQCACzGcUZho4rDP5qwLnB3sa0lZmWPeU/g22FF8ChzUgRvCQ0AhPkpzG5MWSIAH6\n4JJL+Urxi88Pb2pQ9rO+GOgNPTlgX2h80w9rAakJdrLPHB17irASPbU60qFIEdNU\ng9AX8Hz1LHsQhu2I0toHC/gHNo6BrF2131N5uT2ubhNvOySQV3u7FMv1Gp21RMYK\n4ozIOjm1AgMBAAECggEAIXcrumEJWprPtQmjp8kAjqGp9jWVueCdMpyn2PLL/hHN\n7NRlniZWuackyFeOhIIPdj8qWZoHYXQWSf9NRvTofsbsT1/8HBvDWKF/tG50iDV8\nckxbFwi+ssilQDPNOVarBG0P/ySzj5mBRDh0zyReXCJAbs0PDNARZ/i4DmqXLTEi\njnkNvbM4XsyCHJuOBchhV5qWTC1XI9rgSapU8leDdpYJ0Sda4fltnoE0uJ6rD2v2\nfB+oZHwnGuFdBZnc/75rQGz68cWAK8FY+fRsN0X1eKpQlMzoVK5s3Lncy7VHZVfw\nbibSkiZbPswLVnnEl0Bu5AAQe9q5VJyYwXwi0fGR0QKBgQD+5bKsPrLFVi9/ndgL\nsbOgIMjUdqdyjRFkx2QDiSj4jf35udFBSupL1mhbaq2SaBBufrP/XsD+bgTiHsWb\nzoThaHCVTdBHF0+Zz7RxAZ7tp02cBWkIQv0b5D72Z9PsJhXz9F3vueCIP1kkyaN+\nLVkKfa1bB9M83WYfArCjezO4RQKBgQD3yB8mNXGnF5gYTzSRmDyTuX/MTwL+XV7W\nx6qWNudWH1kJVFiFNVpzaVUgSIekM+zd/CFWUsaSJJ0i3rC5rvCn5kajZkEU3GsX\nkLwopcnYi9sFDWu+vVQrHVguwMQssFetfdMfxj3Y+PKAUvwK1K9CpARbikLFyhXN\njZnlf1KqsQKBgQCFqnBisNJQd1ybIGvN3hbi7q5B9CLlCCLJ5WGGFSQeC1/W1+Fq\nyt1IfZvl7HAu6VFMkDbVwJuWEJ2Q8jAP/2FhTsJTGSYSUaAVeX0XEZGE8InbOl8U\nzQQr3ypBTrssB+DVOB0WfDPJXubhSrfoj+E0hiklJlpYfYVy15dHc3DwvQKBgQDE\nViSoiAyolD+Gc6x5+aF2hLRJVgpPN/kgQ1JeeN7SFUDd7aG+28nyTXKQhChG6oY0\nV8aquihyQOTqbn6mg4JZ6FPsweer+gQ5xgvyQXwxRJCl7j6EA02gZD4tvb00k/7F\nkEAsm08CkDDHkfn5MxI0wfZZ+rJyO3EPUbaybTM54QKBgEAIhWyl3/sTokdXYlc2\nBqPoygjFrsanFRiQS0Q1w8oGQvRHO2kITroAE9iW+bIrpM5X1YFfAawoGT3t0r5v\n2fGirQZJzHRKOcddtEZeR4cudQ3l6h8mh26m8rkdzyg72Zug2W/nTaGUEEeVJK1J\ns/npVCfBibP8c4GniOd+xMBw\n-----END PRIVATE KEY-----\n";

const char CLIENT_EMAIL[] PROGMEM = "firebase-adminsdk-6stg2@robotic-arm-38d44.iam.gserviceaccount.com";

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;

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

void setup() {
    Serial.begin(115200);
    setup_wifi();
    
    // Initialize Firebase
    config.api_key = API_KEY;
    config.service_account.data.client_email = CLIENT_EMAIL;
    config.service_account.data.project_id = FIREBASE_PROJECT_ID;
    config.service_account.data.private_key = PRIVATE_KEY;
    
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
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        setup_wifi();
    }
    
    // Check for updates every 100ms
    if (Firebase.ready() && (millis() - dataMillis > 100 || dataMillis == 0)) {
        dataMillis = millis();
        
        String documentPath = "roboticArms/armDegree";
        
        if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
            FirebaseJson payload;
            payload.setJsonData(fbdo.payload().c_str());
            processDocument(&payload);
        }
    }
    
    delay(10); // Small delay to prevent watchdog timer issues
}