//#include <Arduino.h>
//#include <WebSocketsServer.h>


void startSPIFFS();
String lookUpSettings(String);
void startOTA();
void startMDNS(String s);
void startServer();
void startUDP();
void startUpWifi();

//void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void loadPreferences();
int lookUpTableWeight(float dist);
void evaluateState();
//void saveMap(String receivedParameter, String receivedValue, String file);
void saveState(String receivedParameter, String receivedValue);
void saveSettings(String receivedParameter, String receivedValue);
void saveMap ( String receivedParameter, String receivedValue , String file);
String lookUpSettings(String receivedParameter);
String lookUpState(String receivedParameter);
String lookUpMap(String receivedParameter, String fileName);
#if defined ESP8266 || defined ESP32
void startServer();
void handleNotFound();
void handleIndex();
void handleGetJavaScript();
void handleBtnClick();
void handleGetData();
void handleSettings();
void handleReset();
void handleSettingsChanged();
bool handleFileRead(String path);
void handleFileUpload();
void handleFileList();
void handleFileDelete();
void handleFileRename();
void handleGetData();
void handleBtnClick();
void handleFileDelete();
void handleWifiManager();
void handleForgetWifi();
void handleCalibrate();
void handleControl();
void handleSendEzoMsg();
void handleReadCal();
void handleDebug();
#endif
#ifdef ESP322
void handleNotFound(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleIndex(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleBtnClick(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleGetData(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleSettings(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleReset(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleSettingsChanged(AsyncWebServerRequest *request);
bool handleFileRead(AsyncWebServerRequest *request, String path);
void handleFileUpload(AsyncWebServerRequest * request, String filename,size_t index, uint8_t *data, size_t len, bool final);
ArRequestHandlerFunction handleFileList(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleFileDelete(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleFileRename(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleGetData(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleBtnClick(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleFileDelete(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleWifiManager(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleForgetWifi(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleCalibrate(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleControl(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleSendEzoMsg(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleReadCal(AsyncWebServerRequest *request);
ArRequestHandlerFunction handleDebug(AsyncWebServerRequest *request);
#endif

static String sendI2CCommand( int address, String command) ;
String getContentType(String filename);
void sendNTPpacket(IPAddress &address);
String formatBytes(size_t bytes);
void espSleep();
time_t getTime();
void saveLog(String text);
int32_t getRSSI(const char* target_ssid);
void interruptRoutine();
String digitalClockDisplay();
String printDigits(int digits);
void nk_deep_sleep(uint64_t);
void sendDataToServer( String address, String data);
int getRSSIasQuality(int RSSI);
void tcpCleanup ();
void handleTextEdit();
void mqttMsgReceived (String &topic, String &payload);
void mqttMsg (String &topic, String &payload);
void startMQTT();
void hexdump(const void *mem, uint32_t len, uint8_t cols);
String getContentType(String filename);
void consoleLog (String id,String s);
void process_data (const char * data);                          // Process data received.
void processIncomingByte (const byte inByte);                   // Process the Serial data received until /n detected
void iddleTime();