unsigned char wifiOpenAccessPoint() {
  /* Setup Access Point */
  Serial.print("Wireless Access Point ID: ");
  Serial.println(ESP_SSID);
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(ip, gateway, subnet) ? "AP_Ready" : "AP_Failed!");

  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(ESP_SSID, ESP_PASS, 9, 0, ESP_MAX_CONNECTION)) {
    AccessPoint_WiFiServer.begin();
    Serial.print("DONE - OPENED ACCESS POINT - DNS: ");
    Serial.print(WiFi.softAPIP());
    Serial.print(" - Port: ");
    Serial.println(SOCKET_PORT);
    return 1;
    // Completed
  }
  else {
    Serial.println("Failure");
    return 0;
    // Failure
  }
}

unsigned int  scanWifi() {
  Serial.println("scan start");
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  JsonObject sendJSONObject = sendJSON.to<JsonObject>();
  
  sendJSONObject["CMD"] = "WIFI_LIST";
  sendJSONObject["MAC"] = WiFi.macAddress();
  JsonArray WiFiList = sendJSONObject.createNestedArray("WIFI");
    for(int i = 0; i<n; i++) {
    WiFiList.add(WiFi.SSID(i));
  }
  serializeJson(sendJSONObject, Serial);
  serializeJson(sendJSONObject, sendJSONString);
  return n;

}



