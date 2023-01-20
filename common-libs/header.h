#include <Arduino.h>
#include <string>;

#ifdef ESP8266
    #include  <ArduinoOTA.h>
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266SSDP.h>
    #include <LittleFS.h>
    #include <Hash.h>
    #include <ESP8266mDNS.h>
    #define FILE_SYS LittleFS
    #define REQUEST_ server.
    //#include <WebServer.h>
#endif
#ifdef ESP32
    #include  <ArduinoOTA.h>
    #include <ESPmDNS.h>
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <ESP32httpUpdate.h>
    #include <Update.h> 
    //#include <AsyncUDP_WT32_ETH01.h>
    #include <WebServer.h>  // Esto es lo que he cambiado !!!!!! La linea anterior la he comentado y he agregado esta
    #include <FS.h>
    #include <SPIFFS.h>
    //#include <AsyncTCP.h>
    //#include <ESPAsyncWebServer.h>
    //#include "~/.platformio/packages/framework-arduinoespressif32/libraries/ArduinoOTA/src/ArduinoOTA.h"
    //#include "../.pio/libdeps/node32s/WebSockets/src/WebSocketsServer.h"
    #define FILE_SYS SPIFFS
    //#define REQUEST_ request->
    #define REQUEST_ server.
    #include <esp32ModbusRTU.h>

#endif
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PID_v1.h>
//#include <ArduinoRS485.h> //  ESTA LIBRERIA LA TUVE QUE MODIFICAR !!!!
//#include <ArduinoModbus.h>
//#include <ModbusMaster.h>
#include <ModbusSlave.h>
//ModbusMaster nodeRelays;
#ifdef ESP32
#define MAX485_DE      4
#define MAX485_RE_NEG  4
#endif
#ifdef ESP8266
#define MAX485_DE      D2
#define MAX485_RE_NEG  D2
#endif
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

//#include <WiFiManager.h>
//FILE_SYS 
//#include <WiFiUdp.h>
//#include <Time.h>
//#include <TimeLib.h>
#include <vector>
#include <DNSServer.h>
//#include <WiFi.h>
//#include <FS.h>
//#include <ESP8266HTTPUpdateServer.h>
#include <map>
// #include <GDBStub.h>                  Este include sirve para debug.
    //#include <gtk/gtk.h>
    
#ifdef JELLY_NUEVO
#define DO_I2C_ADDRESS 97     
#define EC_I2C_ADDRESS 100
#define PH_I2C_ADDRESS 99
#else
#define DO_I2C_ADDRESS 33      
#define EC_I2C_ADDRESS 34
#define PH_I2C_ADDRESS 99
#endif 

#ifdef ESP32 
#define SerialInterface Serial2
#define SerialCtrlPin 4
esp32ModbusRTU modbus(&SerialInterface, 4);  // use Serial1 and pin 16 as RTS

#endif
#ifdef ESP8266
#define SerialInterface Serial
#define SerialCtrlPin D3
#endif
enum VFD_Types { SOYAN_SVD , MOLLOM_B20 };
enum GENERIC_TIMER_TYPES { TIMER , SCHEDULE };
#define LOCAL_TIME_OFFSET 6
String mqttServer = "192.168.1.115";
#include "classes.h"
#include "../common-libs/declarations.h"
//#include "ESP8266FtpServer.h"
//#include <PubSubClient.h>
#include "MQTT.h"


#define ONE_HOUR 3600000UL

//COLORES PARA EL TERMINAL
#define _Black 	"\033[30m 	// Para background \033[40m
#define _Red 	\033[31m 	// Para background \033[41m
#define _Green 	\033[32m 	// Para background \033[42m
#define _Orange \033[33m 	// Para background \033[43m
#define _Blue 	\033[34m 	// Para background \033[44m
#define _Magenta \033[35m 	// Para background \033[45m
#define _Cyan 	\033[36m 	// Para background \033[46m
#define _Gray 	\033[37m 	// Para background \033[47m
#define _Default 	\033[39m // Para background\033[49m


bool debug = true;
bool logEvents = true;
#ifdef ESP8266
ESP8266WebServer server(80); // create a web server on port 80
WiFiUDP UDP; // Create an instance of the WiFiUDP class to send and receive UDP messages
WiFiUDP UDPtime; // Create an instance of the WiFiUDP class to send and receive UDP messages
//RS485Class rs485 (Serial,TX,D2,RX);
//AsyncWebServer socketServer (81);
#endif
#ifdef ESP32
WiFiUDP UDP; // Create an instance of the WiFiUDP class to send and receive UDP messages
WiFiUDP UDPtime; // Create an instance of the WiFiUDP class to send and receive UDP messages
#include <WiFi.h>
//#include <WebServer.h>
//WebServer server(80);  Esto tambien se puede hacer !!!!
WebServer server(80);
//ESP32WebServer server(80);


//AsyncWebServer socketServer (81);
#endif
//GtkWidget *window;
DNSServer dnsServer;
//WiFiManager wifiManager;
//WebSocketsServer webSocket = WebSocketsServer(81);
//FtpServer ftpSrv;
//ESP8266HTTPUpdateServer  httpUpdater;
WiFiClient wifiClient;
WiFiClient wifiClientMQTT;
HTTPClient httpClient;
 MQTTClient  mqttClient;

//const char *OTAPassword = "esp8266";
const int mqttPort = 1883;
const char* mqttUser = "pi";
const char* mqttPassword = "pass";
String mqttTopic = "default";

//WiFiManager wifiManager;
String deviceID = "defaultID";
// IPAddress staticIP(192, 168, 0, 110); //ESP static ip
// IPAddress gateway(192, 168, 0, 1);   //IP Address of your WiFi Router (Gateway)
// IPAddress subnet(255, 255, 255, 0);  //Subnet mask
// IPAddress dns(8, 8, 8, 8);  //DNS
IPAddress timeServerIP; // The time.nist.gov NTP server's IP address
const char *ntpServerName = "pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

const String nl = "/nl";
byte packetBuffer[NTP_PACKET_SIZE]; // A buffer to hold incoming and outgoing packets

File fsUploadFile; // a File variable to temporarily store the received file
//AlarmId logAlarm;

String deviceName="default";
String versionID="0.0";
int secondsCapture = 600;
int ntpInterval = 3600;
unsigned long prevNTP = 0;
uint32_t timeUNIX = 0;
uint32_t timeNow = 0; // The most recent timestamp received from the time server
unsigned long lastNTPResponse = 0;
unsigned long lastUpdateCheck = 0;
unsigned long lastWifiCheck = 0;
int updateCheckInterval = 600;
int intervalWifiCheck = 600;   // AQUI CAMBIAR A 600
unsigned long lastUdpSend = 0;
int udpInterval = 10000;
float batteryFactor = 1;
IPAddress myIP;
bool isConnected = false;
boolean checkNtp = false;
double timeFactor = 0;
unsigned long lastUpdate;
unsigned long lastSaveTime;
bool webSocketConnected= false;
void callback(char* topic, byte* payload, unsigned int length);


 enum TypeID {
        NH4N = 1,
        Temperature = 2,
        Floc = 3,
        Oxygen = 4,
        OxygenInPercent = 5,
        pH = 6,
        Salinity = 7,
        NO2 = 8,
        NO3 = 9,
        Alcalinity = 10,
        TSS = 11,
        Phosphate = 12,
        SecchiDisk = 13
};
  enum Device_Types {
      pressure_top, pressure_bottom
  };


