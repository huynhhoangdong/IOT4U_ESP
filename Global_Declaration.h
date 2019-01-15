#define SCREEN_WIDTH  128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define ESP_SSID "ESP8266_SSID"
#define ESP_PASS ""
#define ESP_MAX_CONNECTION 2

#define JSON_SEND_BUFFER_LENGTH     1200  // Tested max 1189
#define JSON_RECEIVE_BUFFER_LENGTH  600   // Tested max 566

#define ID_LENGTH_EEPROM_ADDRESS         512
#define ID_DATA_EEPROM_START_ADDRESS     514
#define ID_DATA_EEPROM_END_ADDRESS       1023

StaticJsonDocument<JSON_RECEIVE_BUFFER_LENGTH> receivedJSON;                 // Create the receiving JSON object
StaticJsonDocument<JSON_SEND_BUFFER_LENGTH> sendJSON;                        // Create the sending JSON object

char sendJSONString[JSON_SEND_BUFFER_LENGTH];

#define SOCKET_PORT 80
WiFiServer AccessPoint_WiFiServer(SOCKET_PORT);
WiFiClient clientStation[ESP_MAX_CONNECTION];
IPAddress ip(192, 168, 4, 1);             // IP address of the server
IPAddress gateway(192, 168, 4, 1);        // gateway of your network
IPAddress subnet(255, 255, 255, 0);       // subnet mask of your network

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     LED_BUILTIN // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally

//------AWS---------------
#define MQTT_STATUS_CONNECTED 0
#define MQTT_STATUS_FAIL      1
int8_t mqttResult = MQTT_STATUS_FAIL;

// See `src/aws_iot_config.h` for formatting
char *region = (char *) "us-east-1";
char *endpoint = (char *) "a2qe5byjpkn5pd-ats";
char *mqttHost = (char *) "a2qe5byjpkn5pd-ats.iot.us-east-1.amazonaws.com";
int mqttPort = 443;
char *iamKeyId = (char *) "AKIAJWZCBSFWJC65D4LA";
char *iamSecretKey = (char *) "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";
String a = "$aws/things/test/shadow/update";
const char* aws_topic  = "$aws/things/test/shadow/update";
char* topicSub  = "000000000000000000000000000000000000/A2E";
char* topicPub  = "000000000000000000000000000000000000/E2A";
char* mqttClientID = "000000000000000000000000000000000000";

ESP8266DateTimeProvider dtp;
AwsIotSigv4 sigv4(&dtp, region, endpoint, mqttHost, mqttPort, iamKeyId, iamSecretKey);
AWSConnectionParams cp(sigv4);
AWSWebSocketClientAdapter adapter(cp);
AWSMqttClient AWSMQTTClient(adapter, cp);

//------LED------
#define PIXEL_PIN   D7    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 5
Adafruit_NeoPixel stringLED = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_RGB + NEO_KHZ800);
//---------------

byte ledPin = 2;
byte setupPin = D0;

//char ssid[] = "AURIO-MEETING ROOM";      // SSID of your home WiFi
//char pass[] = "0903621088";               // password of your home WiFi
//char ssid[] = "AURIO-TECH ROOM";      // SSID of your home WiFi
//char pass[] = "0922222870";               // password of your home WiFi
char ssid[] = "MienThaoMoc";      // SSID of your home WiFi
char pass[] = "thaomoc2018";               // password of your home WiFi

String id = "";
char msg[50];
int i = 0;

uint16_t idLength; 
String ID_MSG = "";
boolean AP_CONNECTION = true;
static uint8_t disconnectCounter = 0;
