#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";        // Replace with your WiFi SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password

// OpenWeatherMap API key and city ID
const char* apiKey = "YOUR_API_KEY"; // Replace with your OpenWeatherMap API key
const int cityID = 5911592;          // Replace with your city ID

// TFT display pins
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

// Initialize TFT display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB); // Initialize display
  tft.setRotation(0); // Set to portrait mode

  // Display "Connecting to WiFi..." message
  tft.fillScreen(ST7735_BLACK);
  displayCenteredText("Connecting to WiFi...", 20, ST7735_WHITE);

  // Draw a progress bar
  int progressWidth = tft.width() - 20; // Width of the progress bar
  int xStart = 10; // X start position
  int yStart = 60; // Y start position for portrait mode
  int height = 10; // Height of the progress bar

  tft.drawRect(xStart, yStart, progressWidth, height, ST7735_WHITE);

  WiFi.begin(ssid, password);

  // Fill the progress bar over 4 seconds
  for (int i = 0; i <= progressWidth; i += progressWidth / 40) {
    tft.fillRect(xStart + 1, yStart + 1, i, height - 2, ST7735_WHITE);
    delay(100); // 100 ms delay * 40 iterations = 4000 ms or 4 seconds
  }

  // Check WiFi connection after progress bar is full
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
  } else {
    displayErrorMessage("WiFi Connect Failed");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { 
    String serverName = "http://api.openweathermap.org/data/2.5/weather?id=" + String(cityID) + "&appid=" + apiKey + "&units=metric";
    HTTPClient http;
    http.begin(serverName);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      displayWeatherInfo(payload);
    } else {
      displayErrorMessage("API Request Failed");
    }

    http.end();
  } else {
    displayErrorMessage("WiFi Disconnected");
  }

  delay(60000); // Update every 60 seconds
}

void displayWeatherInfo(String payload) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  String city = doc["name"];
  String weather = doc["weather"][0]["main"];
  String description = doc["weather"][0]["description"];
  float temp = doc["main"]["temp"];
  float temp_min = doc["main"]["temp_min"];
  float temp_max = doc["main"]["temp_max"];
  float precipitation = doc["rain"]["1h"] | 0;

  tft.fillScreen(ST7735_BLACK);

  int yStart = 20;
  displayCenteredText("City:", yStart, ST7735_BLUE);
  displayCenteredText(city, yStart + 20, ST7735_WHITE);
  displayCenteredText("Weather:", yStart + 40, ST7735_BLUE);
  displayCenteredText(weather, yStart + 60, ST7735_WHITE);
  displayCenteredText("Description:", yStart + 80, ST7735_BLUE);
  displayCenteredText(description, yStart + 100, ST7735_WHITE);
  displayCenteredText("Temp:", yStart + 120, ST7735_BLUE);
  displayCenteredText(String(temp) + " C", yStart + 140, ST7735_WHITE);
  displayCenteredText("High:", yStart + 160, ST7735_BLUE);
  displayCenteredText(String(temp_max) + " C", yStart + 180, ST7735_WHITE);
  displayCenteredText("Low:", yStart + 200, ST7735_BLUE);
  displayCenteredText(String(temp_min) + " C", yStart + 220, ST7735_WHITE);
  displayCenteredText("Precipitation:", yStart + 240, ST7735_BLUE);
  displayCenteredText(String(precipitation) + " mm", yStart + 260, ST7735_WHITE);
}

void displayCenteredText(String text, int y, uint16_t color) {
  int16_t x1, y1;
  uint16_t w, h;

  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((tft.width() - w) / 2, y);
  tft.println(text);
}

void displayErrorMessage(String message) {
  tft.fillScreen(ST7735_BLACK);
  displayCenteredText(message, 100, ST7735_RED);
}
