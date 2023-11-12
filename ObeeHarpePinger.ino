// Import necessary libraries
#include <ESP8266WiFi.h>   // For ESP8266 WiFi connectivity
#include <espnow.h>        // For ESP-NOW communication
#include "Wire.h"          // For I2C communication
#include "Adafruit_INA219.h" // For interfacing with INA219 sensor
#include <ESP8266mDNS.h>   // For mDNS (Over-The-Air) updates
#include <WiFiUdp.h>       // For UDP communication
#include <ArduinoOTA.h>    // For Over-The-Air (OTA) updates
#include <RemoteDebug.h>   // For remote debugging

// Replace with the appropriate MAC address depending on AP or STA mode
uint8_t broadcastAddress1[] = { ..., ..., ...., ...., 0x67, 0x51 }; // AP MODE : SETUP THERE THE MAC OF YOUR CENTRALE
// uint8_t broadcastAddress1[] = { ..., ..., ...., ...., 0x67, 0x50 }; // STA MODE

// Unique device identifier
const int BoardID = 5;
const int realId = BoardID + 1;

// Hostname for better identification
const char* HOSTNAME = "Harpe-" + realId;

unsigned long lastTime = 0;
unsigned long timerDelay = 2000; // Timer for sending readings

// Wi-Fi credentials
const char* ssid = "XXXXXXXXXX";
const char* password = "ZZZZZZZ";
const char* ATO_PASSWORD = "admin"; // Password for OTA updates

bool OTAupdate = false;
unsigned long timeoutLength = 30000; // Time to catch the AP at boot

// Threshold for power activity detection
const float POWER_THRESHOLD = 200.0;

RemoteDebug Debug; // For remote debugging

unsigned long previousMillis;

Adafruit_INA219 ina219;

// Structure for sending data
typedef struct struct_wifi_tx {
  int id;
  long counter;
  int frags;
  float shuntvoltage;
  float busvoltage;
  float current_mA;
  float loadvoltage;
  float power_mW;
  float delta_power_mW;
} struct_wifi_tx;

struct_wifi_tx WIFI_TX;

// Variables for power analysis and minimum/maximum values
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;
float power_mW = 0;

float shuntvoltage_Min = 0;
float busvoltage_Min = 0;
float current_mA_Min = 0;
float loadvoltage_Min = 0;
float power_mW_Min = 0;

float shuntvoltage_Max = 0;
float busvoltage_Max = 0;
float current_mA_Max = 0;
float loadvoltage_Max = 0;
float power_mW_Max = 0;

// Callback when data is sent
void OnDataSent(uint8_t* mac_addr, uint8_t sendStatus) {
  char macStr[18];
  Serial.print("Packet to:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

void initOTA() {
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(ATO_PASSWORD);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  if (WiFi.waitForConnectResult(timeoutLength) == WL_CONNECTED) {
    Debug.begin("ESP8266");
    initOTA();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    OTAupdate = true;
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    if (!ina219.begin()) {
      Serial.println("Failed to find INA219 chip");
      while (1) { delay(10); }
    }
  }
}

void getValue() {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
}

void showValue() {
  Serial.printf("busvoltage : %f Volts", busvoltage);
  Serial.print("\t");
  Serial.printf("shuntvoltage : %f Volts", shuntvoltage);
  Serial.print("\t");
  Serial.printf("loadvoltage : %f Volts", loadvoltage);
  Serial.print("\t");
  Serial.printf("current_mA : %f mA", current_mA);
  Serial.print("\t");
  Serial.printf("power_mW : %f mW \n", power_mW);
}

void resetValue() {
  shuntvoltage_Min = 10;
  busvoltage_Min = 10;
  current_mA_Min = 5000;
  loadvoltage_Min = 10;
  power_mW_Min = 500000;
  
  shuntvoltage_Max = 0;
  busvoltage_Max = 0;
  current_mA_Max = 0;
  loadvoltage_Max = 0;
  power_mW_Max = 0;
}

void getMinMAXValue() {
  if (shuntvoltage < shuntvoltage_Min)
    shuntvoltage_Min = shuntvoltage;
  if (shuntvoltage > shuntvoltage_Max)
    shuntvoltage_Max = shuntvoltage;
  
  if (busvoltage < busvoltage_Min)
    busvoltage_Min = busvoltage;
  if (busvoltage > busvoltage_Max)
    busvoltage_Max = busvoltage;
  
  if (current_mA < current_mA_Min)
    current_mA_Min = current_mA;
  if (current_mA > current_mA_Max)
    current_mA_Max = current_mA;
  
  if (loadvoltage < loadvoltage_Min)
    loadvoltage_Min = loadvoltage;
  if (loadvoltage > loadvoltage_Max)
    loadvoltage_Max = loadvoltage;
  
  if (power_mW < power_mW_Min)
    power_mW_Min = power_mW;
  if (power_mW > power_mW_Max)
    power_mW_Max = power_mW;
}

void showDelta() {
  Serial.printf("DELTA : busvoltage : %f Volts", busvoltage_Max - busvoltage_Min);
  Serial.print("\t");
  Serial.printf("shuntvoltage : %f Volts", shuntvoltage_Max - shuntvoltage_Min);
  Serial.print("\t");
  Serial.printf("loadvoltage : %f Volts", loadvoltage_Max - loadvoltage_Min);
  Serial.print("\t");
  Serial.printf("current_mA : %f mA", current_mA_Max - current_mA_Min);
  Serial.print("\t");
  Serial.printf("power_mW : %f mW \n", power_mW_Max - power_mW_Min);
}

void loop() {
  if (OTAupdate) {
    ArduinoOTA.handle();
    if (millis() - previousMillis >= 500) {
      previousMillis = millis();
      Serial.println(F("Code has been updated"));
      if (WiFi.waitForConnectResult(10) != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
      }
    }
  } else {
    getValue();
    getMinMAXValue();
    if ((millis() - lastTime) > timerDelay) {
      WIFI_TX.id = BoardID;
      if ((power_mW_Max - power_mW_Min) > POWER_THRESHOLD) {
        WIFI_TX.frags = 1;
      } else {
        WIFI_TX.frags = 0;
      }
      WIFI_TX.busvoltage = busvoltage;
      WIFI_TX.shuntvoltage = shuntvoltage;
      WIFI_TX.loadvoltage = loadvoltage;
      WIFI_TX.current_mA = current_mA;
      WIFI_TX.power_mW = power_mW;
      WIFI_TX.delta_power_mW = power_mW_Max - power_mW_Min;
      
      showValue();
      showDelta();
      
      esp_now_send(0, (uint8_t*)&WIFI_TX, sizeof(WIFI_TX));
      resetValue();
      lastTime = millis();
    }
  }
}
