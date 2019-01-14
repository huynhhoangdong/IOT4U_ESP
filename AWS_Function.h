#define AWSTopic_Callback ([](const char* topic, const char* jsMsg){  AWSTopic_CallbackFunction(topic, jsMsg);  } )

/* AWS MQTT Callback handle */
void AWSTopic_CallbackFunction(const char* topic, const char* jsMsg) {
  Serial.printf("Got msg '%s' on topic %s\n", jsMsg, topic);
  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);
  display.println(jsMsg);
  display.display();
  deserializeJson(receivedJSON, jsMsg);
  JsonObject  receivedJSONObject = receivedJSON.as<JsonObject>();

  String commandFromAPP = receivedJSONObject[String("FUNCTION")];
  if (commandFromAPP == "ON") {
    for (int i = 0; i < PIXEL_COUNT; i++) stringLED.setPixelColor(i, 64, 64, 64); //WHITE
    stringLED.show();
  } else if (commandFromAPP == "OFF") {
    for (int i = 0; i < PIXEL_COUNT; i++) stringLED.setPixelColor(i, 0, 0, 0);    //OFF
    stringLED.show(); 
  } else if (commandFromAPP == "RED") {
    for (int i = 0; i < PIXEL_COUNT; i++) stringLED.setPixelColor(i, 64, 0, 0);   //RED
    stringLED.show(); 
  } else if (commandFromAPP == "GREEN") {
    for (int i = 0; i < PIXEL_COUNT; i++) stringLED.setPixelColor(i, 0, 64, 0);   //GREEN
    stringLED.show();
  } else if (commandFromAPP == "BLUE") {
    for (int i = 0; i < PIXEL_COUNT; i++) stringLED.setPixelColor(i, 0, 0, 64);   //BLUE
    stringLED.show();
  }

  //--Send back to phone--
  JsonObject sendJSONObject = sendJSON.to<JsonObject>();

  sendJSONObject["FUNCTION"] = commandFromAPP;
  Serial.print("Send back message to phone: ");
  serializeJson(sendJSONObject, Serial);
  Serial.println("\n----------------------------");
  String shadow;
  serializeJson(sendJSONObject, shadow);

  AWSMQTTClient.publish(topicPub, shadow.c_str(), 0, false);
  AWSMQTTClient.yield();
  yield();
  //--

  
}

/* Connect to MQTT and Subscribe to topics */
uint8_t ___mqtt_AWS_CONNECT(){ 
  WiFi.mode(WIFI_STA);                                                            // Totally stop Access Point mode
  //WiFi.mode(WIFI_AP_STA); 
  delay(100);
  if(WiFi.status() == WL_CONNECTED){                                              // WiFi connection is successed
    //blinkLED_Hz(BLINKING_FREQ_MQTT_CONNECTING);
    if(mqttResult == MQTT_STATUS_FAIL)  mqttResult = AWSMQTTClient.connectByClientID(mqttClientID);
    Serial.printf(" %s", mqttResult ? "Failed and retry one more time!..." : "");
    if(mqttResult == MQTT_STATUS_FAIL)  mqttResult = AWSMQTTClient.connectByClientID(mqttClientID);
    Serial.printf(" %s\n", mqttResult ? "Failed" : "Successed!");
    //blinkLED_Hz(BLINKING_FREQ_NORMAL_WAITING);
    
    if(mqttResult == MQTT_STATUS_CONNECTED){
      AWSMQTTClient.subscribe(topicSub, 1, AWSTopic_Callback );
      return MQTT_STATUS_CONNECTED;
    }
    else{
      Serial.println(">>> Failed to connect to MQTT Server");
      return MQTT_STATUS_FAIL;
    }
  }
  else{                                                                         // WiFi connection is failed
    Serial.println("Could not connect to WiFi - Can not connect to MQTT Server");
    return MQTT_STATUS_FAIL;
  }
}

