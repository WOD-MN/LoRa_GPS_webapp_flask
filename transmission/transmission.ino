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
#define BAND    915E6  // Set your frequency

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

// Create TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);  // Initialize SoftwareSerial for GPS

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
}

void loop() {
  // Check if GPS data is available
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    // If new GPS data is available, send it via LoRa
    if (gps.location.isUpdated()) {
      String latitude = String(gps.location.lat(), 6);  // 6 decimal places
      String longitude = String(gps.location.lng(), 6);

      // Print and display the data
      Serial.print("Lat: ");
      Serial.print(latitude);
      Serial.print(", Lon: ");
      Serial.println(longitude);

      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Lat: ");
      display.println(latitude);
      display.print("Lon: ");
      display.println(longitude);
      display.display();

      // Transmit GPS data over LoRa
      String gpsData = latitude + "," + longitude;
      LoRa.beginPacket();
      LoRa.print(gpsData);
      LoRa.endPacket();
    }
  }
}
