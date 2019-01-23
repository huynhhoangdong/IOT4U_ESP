/*  
 *   IOT4U Project
 *   TIOT Company
 *   
*/
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266AWSIoTMQTTWS.h>
#include <EEPROM.h>

#include "Global_Declaration.h"
#include "WiFi_Get_Infor.h"
#include "AWS_Function.h"


void setup() {

  Serial.begin(115200);                   // only for debug
  EEPROM.begin(1024);
  /* Setup for RGB Light String */
  stringLED.begin();
  stringLED.show();

  pinMode(ledPin, OUTPUT);
  pinMode(setupPin, INPUT);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.println("Version: 0.22");
  display.println("Releasing: October 22 2018");
  display.display();


  if (!digitalRead(setupPin)) {
    wifiOpenAccessPoint();
    Serial.print("BSSID: "); Serial.println(WiFi.macAddress());
    scanWifi();

    while (AP_CONNECTION) {
      WiFiClient client = AccessPoint_WiFiServer.available();

      if (client) {
        if (client.connected()) {
          display.clearDisplay();
          display.display();
          display.setCursor(0, 0);
          display.display();

          digitalWrite(ledPin, LOW);  // to show the communication only (inverted logic)
          Serial.println(".");
          String request = client.readStringUntil('.');    // receives the message from the client
          Serial.print("APP REQUEST: "); Serial.println(request);
          //----
          deserializeJson(receivedJSON, request);
          JsonObject  receivedJSONObject = receivedJSON.as<JsonObject>();
          /* Analyze the commands*/
          String commandFromAPP = receivedJSONObject[String("CMD")];
          if (commandFromAPP == "GET_WIFI_LIST") {
            scanWifi();
            client.println(sendJSONString);
            Serial.println(sendJSONString);

          } else if (commandFromAPP == "CONFIG_INFO") {
            String UDID = receivedJSONObject[String("UDID")];
            String SSID = receivedJSONObject[String("SSID")];
            String PASS = receivedJSONObject[String("PASS")];
            id = UDID;
            Serial.println(UDID);
            Serial.println(SSID);
            Serial.println(PASS);
            WiFi.mode(WIFI_AP_STA);
            //***************** write ID***************************
            idLength = id.length();
            for ( uint8_t i = 0; i < idLength; i++)
            {
              topicSub[i] = id[i];
              topicPub[i] = id[i];
              mqttClientID[i] = id[idLength-1-i];
            }
            Serial.print("mqttClientID: ");
            Serial.println(mqttClientID);

            Serial.print("UDID:");
            Serial.println(id);
            Serial.print("SUBSCRIBING TOPIC:");
            Serial.println(topicSub);
            Serial.print("PUBLISHING TOPIC:");
            Serial.println(topicPub);

            EEPROM.write(ID_LENGTH_EEPROM_ADDRESS, idLength);             // Save the ID data length to EEPROM (Low Byte)
            EEPROM.write(ID_LENGTH_EEPROM_ADDRESS + 1, idLength >> 8);    // Save the ID data length to EEPROM (High Byte)
            for (uint16_t i = 0; i <= idLength; i++)
            {
              if (i < idLength) EEPROM.write(ID_DATA_EEPROM_START_ADDRESS + i, id[i]);   // Save the ID data to EEPROM
              else              EEPROM.write(ID_DATA_EEPROM_START_ADDRESS + i, 0);             // To indicate the end of ID data
              yield();
            }
            EEPROM.commit();

            //**************************************************
            WiFi.begin(SSID.c_str(), PASS.c_str());
            while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              Serial.print(".");
            }

            if (client.connected()) {
              client.flush();
              client.println("{\"CMD\":\"WIFI_CONNECTION_RESULT\",\"RESULT\":\"OK\"}");
              Serial.println("{\"CMD\":\"WIFI_CONNECTION_RESULT\",\"RESULT\":\"OK\"}");
            } else {
              Serial.println("NOT CONNECTED");
            }

            //break;
            AP_CONNECTION = false;
            client.stop();
          }
          //----
          display.print(request);
          display.display();

          digitalWrite(ledPin, HIGH);
        }
        // tarminates the connection with the client
      }
    }
  }

  //******************** read ID **********************
  idLength = EEPROM.read(ID_LENGTH_EEPROM_ADDRESS + 1);  // Read the high byte of JWT length
  idLength <<= 8;
  idLength = EEPROM.read(ID_LENGTH_EEPROM_ADDRESS);               // Read the low byte of JWT length
  Serial.print("\nUDID: ");
  char idBuffer[500];                                                     // Prepare the JWT buffer
  for (uint16_t i = 0; i <= idLength; i ++) {

    if (i < idLength)
    {
      idBuffer[i] = EEPROM.read(ID_DATA_EEPROM_START_ADDRESS + i);  // Read the JWT data to EEPROM

    }
    else  idBuffer[i] = 0;                                               // Put the null terminator to indicate the end of JWT data
    yield();                                                                          // to make sure the ESP is not crashed
  }
  ID_MSG = String(idBuffer);
  Serial.println(ID_MSG);
  //**************************************************
  for ( uint8_t i = 0; i < idLength; i++)
  {
    topicSub[i] = ID_MSG[i];
    topicPub[i] = ID_MSG[i];
    mqttClientID[i] = ID_MSG[idLength-1-i];
  }
  Serial.print("mqttClientID: ");
  Serial.println(mqttClientID);
  Serial.print("SUBSCRIBING TOPIC:");
  Serial.println(topicSub);
  Serial.print("PUBLISHING TOPIC:");
  Serial.println(topicPub);
  //**************************************************

  Serial.println("Normal Working Mode");
  Serial.print("Connecting to: ");
  Serial.print(WiFi.SSID());
  Serial.print(" - Password: ");
  Serial.print(WiFi.psk());
  Serial.println(" - ");

  //--WIFI--
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n>>> WIFI IS CONNECTED");
  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);
  display.println("WIFI is Connected");
  display.display();
  
  //--MQTT--
  if(___mqtt_AWS_CONNECT() == MQTT_STATUS_FAIL){                      // Try to connect to AWS MQTT +++++ Failed to connect to AWS MQTT
    Serial.println("\n>>> WARNING: FORCED TO RESTART BECAUSE THE CONTROLLER CAN NOT CONNECT TO MQTT\n");
    display.println("\n>>> MQTT is NOT connected -> RESTART\n");
    display.display();
    ESP.restart();                                                      
  } else {
    Serial.println("\n>>> CONNECT TO AWS IOT SERVICE SUCCESS\n");
    display.println("MQTT is connected");
    display.display();           
  }

}

void loop () {
  if (AWSMQTTClient.isConnected()) {
    Serial.println(WiFi.RSSI());

    AWSMQTTClient.yield();
    yield();
    
  } else {
    Serial.println("AWSMQTTClient is not connected...");
    delay(1000);
    disconnectCounter ++;
    if(disconnectCounter >= 10){  // force to restart the Controller
      Serial.println("\n>>> WARNING: FORCED TO RESTART BECAUSE THE CONTROLLER LOST THE MQTT CONNECTION\n");
      ESP.restart();  
    delay(2000);
    }
  }
  
}
