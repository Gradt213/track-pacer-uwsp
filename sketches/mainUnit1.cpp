#include <FastLED.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <esp_now.h>
#include <WiFi.h>
 
#define LED_PIN 5
#define COLOR_ORDER GRB
 
int NUM_LEDS = 1200;
int PACER_LEDS = 40;
int STEP_SIZE = 20;
 
uint8_t unit2Address[] = {0xF0, 0x24, 0xF9, 0xF2, 0x10, 0x40}; // Unit 2
uint8_t unit3Address[] = {0xF0, 0x24, 0xF9, 0xE3, 0x53, 0x68}; // Unit 3
uint8_t unit4Address[] = {0x94, 0x54, 0xC5, 0xA0, 0xAB, 0x1C}; // Unit 4
uint8_t unit5Address[] = {0xF0, 0x24, 0xF9, 0xE3, 0xE3, 0xCC}; // Unit 5
unsigned long totalPacerTimeMs = 0;
unsigned long startTimeMs = 0;
 
unsigned long unitTwoDelay = 0;
unsigned long unitThreeDelay = 0;
unsigned long unitFourDelay = 0;
 
bool sentTo2 = false;
bool sentTo3 = false;
bool sentTo4 = false;
 
CRGB* leds;
int pacerPosition = 0;
unsigned long speed = 1000;
bool running = false;
CRGB pacerColor = CRGB::Red;
 
String fullStartMessage = "";
unsigned long segmentTimeMs = 0;
static unsigned long lastPrintTime = 0;
 
CRGB getColor(String color) {
  if (color == "#FFC82E") return CRGB::OrangeRed;
  if (color == "#512698") return CRGB::Purple;
  if (color == "#009CDF") return CRGB::Blue;
  if (color == "#E23838") return CRGB::Red;
  if (color == "#F78200") return CRGB::Orange;
  if (color == "#5EBD3E") return CRGB::Green;
  return CRGB::Red;
}
 
// BLE UUIDs
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
 
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
 
// BLE server callbacks
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device Connected!");
    }
 
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device Disconnected!");
        pServer->getAdvertising()->start();
    }
};

void sendStart(uint8_t *address, String startMessage);

// BLE characteristic callbacks
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String rxValue = pCharacteristic->getValue().c_str();
        if (!rxValue.isEmpty()) {
            Serial.print("Received via BLE: ");
            Serial.println(rxValue);
 
            if (rxValue.startsWith("start")) {
                String command = rxValue.substring(6);
                int firstSpace = command.indexOf(' ');
                String color = command.substring(0, firstSpace);
                String speedStr = command.substring(firstSpace + 1);
 
                pacerColor = getColor(color);
                float totalTimeSeconds = speedStr.toFloat();
                
                totalPacerTimeMs = totalTimeSeconds * 1000;
                segmentTimeMs = totalPacerTimeMs / 4;
 
                // <<< set unit delays!
                unitTwoDelay = segmentTimeMs;
                unitThreeDelay = segmentTimeMs * 2;
                unitFourDelay = segmentTimeMs * 3;
 
                int steps = (NUM_LEDS - PACER_LEDS) / STEP_SIZE;
                speed = segmentTimeMs / steps;
 
                running = true;
                pacerPosition = 0;
 
                startTimeMs = millis();
                sentTo2 = false;
                sentTo3 = false;
                sentTo4 = false;
 
                fullStartMessage = rxValue;
 
                Serial.println("Pacer started!");
                Serial.println(fullStartMessage);
            }
            else if (rxValue.startsWith("stop")) {
                running = false;
                fill_solid(leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
                Serial.println("Pacer stopped, all LEDs off!");
            }
            else if (rxValue.startsWith ("two")) {
              sendStart(unit2Address, fullStartMessage);
              Serial.println("Triggered Unit 2!");
          }
          else if (rxValue.startsWith ("three")) {
              sendStart(unit3Address, fullStartMessage);
              Serial.println("Triggered Unit 3!");
          }
          else if (rxValue.startsWith ("four")) {
              sendStart(unit4Address, fullStartMessage);
              Serial.println("Triggered Unit 4!");
          } 
          else if (rxValue.startsWith ("five")) {
            sendStart(unit5Address, fullStartMessage);
            Serial.println("Triggered Unit 5!");
        }         
          }
    }
};
 
void addPeer(const uint8_t *peerAddress) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
 
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer!");
  } else {
    Serial.println("Peer added successfully.");
  }
}
 
void setup() {
  Serial.begin(115200);
 
  leds = new CRGB[NUM_LEDS];
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
 
  BLEDevice::init("ESP32_BLE_UART");
 
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
 
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      RX_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
 
  pTxCharacteristic = pService->createCharacteristic(
      TX_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );
 
  pService->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::getAdvertising()->start();
 
  Serial.println("BLE UART Ready!");
 
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
 
  addPeer(unit2Address);
  addPeer(unit3Address);
  addPeer(unit4Address);
  addPeer(unit5Address);
}
 
void sendStart(uint8_t *address, String startMessage) {
  if (startMessage.length() == 0) {
      Serial.println("Warning: Start message is empty, not sending.");
      return;
  }
 
  esp_err_t result = esp_now_send(address, (uint8_t *)startMessage.c_str(), startMessage.length());
 
  if (result == ESP_OK) {
      Serial.println("Start message sent!");
  } else {
      Serial.println("Error sending start message");
  }
}
 
void loop() {
  if (running) {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();

    if (now - lastUpdate >= speed) {
      lastUpdate = now;

      int prevPos = pacerPosition - STEP_SIZE;
      if (prevPos >= 0) {
        for (int i = 0; i < STEP_SIZE; i++) {
          if (prevPos + i < NUM_LEDS) {
            leds[prevPos + i] = CRGB::Black;
          }
        }
      }

      for (int i = 0; i < PACER_LEDS; i++) {
        if (pacerPosition + i < NUM_LEDS) {
          leds[pacerPosition + i] = pacerColor;
        }
      }

      FastLED.show();
      pacerPosition += STEP_SIZE;

      if (pacerPosition >= NUM_LEDS - PACER_LEDS) {
        running = false;
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        Serial.println("Unit 1 pacing finished!");
      }
    }
  }
}
