#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// Heltec LoRa32 Pinouts
#define SCK     5
#define MISO    19
#define MOSI    27
#define SS      18
#define RST     14
#define DI0     26
#define BAND    868E6  // Set your frequency

// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    16  // Reset pin is 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SDA and SCL Pins for OLED I2C
#define SDA_PIN 4
#define SCL_PIN 15

// GPS pins (SoftwareSerial)
#define GPS_RX 17  // GPS RX pin
#define GPS_TX 33  // GPS TX pin

// Buzzer pin
#define BUZZER_PIN 23

// Node ID
const char* NODE_ID = "NODE_1";  // Unique Node Identifier

// Create TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);  // Initialize SoftwareSerial for GPS

// Geofence coordinates (polygon vertices)
struct Point {
  double lat;
  double lng;
};

const Point geofence[] = {
  {27.6250, 84.5064},  // 27°37'30"N 84°30'23"E
  {27.6247, 84.5067},  // 27°37'29"N 84°30'24"E
  {27.6256, 84.5069},  // 27°37'32"N 84°30'25"E
  {27.6256, 84.5064}   // 27°37'32"N 84°30'23"E
};
const int numVertices = sizeof(geofence) / sizeof(geofence[0]);

// Function to check if a point is inside the polygon
bool isInsideGeofence(double lat, double lng) {
  bool inside = false;
  for (int i = 0, j = numVertices - 1; i < numVertices; j = i++) {
    if (((geofence[i].lng > lng) != (geofence[j].lng > lng)) &&
        (lat < (geofence[j].lat - geofence[i].lat) * (lng - geofence[i].lng) / (geofence[j].lng - geofence[i].lng) + geofence[i].lat)) {
      inside = !inside;
    }
  }
  return inside;
}

void setup() {
  // Initialize serial monitor
  Serial.begin(9600);
  while (!Serial);

  // Initialize I2C communication for the OLED with custom SDA and SCL pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Starting LoRa...");
  display.display();

  // Initialize LoRa
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("LoRa Init Failed");
    display.display();
    while (1);
  }

  Serial.println("LoRa Transmitter");

  // Display success on OLED
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("LoRa Initialized");
  display.display();

  // Initialize GPS module with SoftwareSerial
  gpsSerial.begin(9600);  // Set baud rate for GPS
  display.setCursor(0, 20);
  display.print("GPS Initialized");
  display.display();

  // Initialize buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer initially
}

void loop() {
  // Check if GPS data is available
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    // If new GPS data is available, send it via LoRa
    if (gps.location.isUpdated()) {
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();

      // Print and display the data
      Serial.print("Node: ");
      Serial.print(NODE_ID);
      Serial.print(" | Lat: ");
      Serial.print(latitude, 6);
      Serial.print(", Lon: ");
      Serial.println(longitude, 6);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Node: ");
      display.println(NODE_ID);  // Display Node ID
      display.print("Lat: ");
      display.println(latitude, 6);
      display.print("Lon: ");
      display.println(longitude, 6);

      // Transmit GPS data over LoRa
      String gpsData = String(NODE_ID) + "," + String(latitude, 6) + "," + String(longitude, 6);
      LoRa.beginPacket();
      LoRa.print(gpsData);
      LoRa.endPacket();

      // Check if GPS point is inside the geofence
      if (!isInsideGeofence(latitude, longitude)) {
        // Turn on buzzer if outside geofence
        digitalWrite(BUZZER_PIN, HIGH);
        Serial.println("Outside geofence! Buzzer ON.");

        display.setCursor(0, 40);
        display.print("Outside Geofence!");
        display.display();
      } else {
        // Turn off buzzer if inside geofence
        digitalWrite(BUZZER_PIN, LOW);
        Serial.println("Inside geofence.");

        display.setCursor(0, 40);
        display.print("Inside Geofence");
        display.display();
      }
    }
  }
}
