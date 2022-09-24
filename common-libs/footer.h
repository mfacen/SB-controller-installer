
#ifdef ESP8266
void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    Serial.println("\e[1;31m Connected to Wi-Fi sucessfully.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(" \u001b[0m");
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    Serial.println("\e[1;31m Disconnected from Wi-Fi, trying to connect... \u001b[0m");
    WiFi.disconnect();
    //
}
#endif

#define DEBUG_SERIAL Serial


void connectToWifi () {
     int connectionStatus = WiFi.waitForConnectResult();
    // Serial.println(connectionStatus);

    if (connectionStatus == WL_CONNECTED)
    {
        Serial.println(WiFi.localIP());
        WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
        // WiFi.setSleepMode(WIFI_LIGHT_SLEEP); // Allow light sleep during idle times                           ///// CHANGED THIS CAN BE A PROBLEM CHECK IT OUT
        myIP = WiFi.localIP();
        Serial.println("\e[1;31m Connected with saved Values From FLASH\u001b[0m");
    }
    else if (lookUpSettings("wifi_ssid"))
    {
        Serial.println("Didint connect, trying " + lookUpSettings("wifi_ssid") + ":" + lookUpSettings("wifi_pass"));
        WiFi.begin(lookUpSettings("wifi_ssid").c_str(), (lookUpSettings("wifi_pass") == "NULL") ? NULL : lookUpSettings("wifi_pass").c_str());
        if (WiFi.waitForConnectResult() == WL_CONNECTED)
        {
            Serial.println("\e[1;31m Connected with Values from Settings \u001b[0m");
            Serial.println(WiFi.localIP());
            // WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
            // WiFi.setSleepMode(WIFI_LIGHT_SLEEP); // Allow light sleep during idle times                           ///// CHANGED THIS CAN BE A PROBLEM CHECK IT OUT
            myIP = WiFi.localIP();
        }
        else
        {
            // WiFi.disconnect();
            WiFi.enableAP(true);
            WiFi.mode(WIFI_AP); // Aqui si no lo pongo en modo AP se traba despues de unos segundos. Tampoco puede estar en AP_STA
            Serial.println("Couldn't connect to wifi");
            WiFi.softAP(mdnsName.c_str(), "");
            myIP = WiFi.softAPIP();
            Serial.println("\e[1;31m ACCESS POINT Started Wrong Settings \u001b[0m");
            Serial.println("IP: " + myIP.toString());
        }
    }
    else
    {
        // WiFi.disconnect();
        // WiFi.enableAP(true);
        // WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(mdnsName.c_str());
        Serial.println("\e[1;31m ACCESS POINT Started No Setting File\u001b[0m");
        myIP = WiFi.softAPIP();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
        sendNTPpacket(timeServerIP);
        WiFi.mode(WIFI_STA);
        
        // setSyncInterval(30);
        // setSyncProvider(getTime);
    }

}
///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////             Start up Wifi                                  ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////
void iddleTime(){
    server.handleClient();
}

void startUpWifi()
{
#ifdef ESP8266
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
#endif

    // wifi_set_opmode(STATION_MODE);
    Serial.begin(115200); // Start the Serial communication to send messages to the computer
    delay(10);
    Serial.println("\r\n");

    String ss = WiFi.macAddress();
    Serial.println(ss);
    ss.remove(0, 9);
    ss.remove(2, 1);
    ss.remove(4, 1);
    mdnsName += ss;
    // gdbstub_init();S

    startSPIFFS(); // Start the SPIFFS and list all contents
    // WiFi.begin(); //No es necesario
    // WiFi.setAutoConnect(true);// Para que no use las credenciales guardadas en la memoria FLASH poner false
    // WiFi.persistent(true);


    connectToWifi();
   

    // WiFi.setAutoReconnect(true);
    // WiFi.persistent(true);
    loadPreferences();

    startMDNS(mdnsName); // Start the mDNS responder
    // httpUpdater.setup(&server);
    startServer(); // Start a HTTP server with a file read handler and an upload handler
    startUDP();    // Start listening for UDP messages to port 123
    startMQTT();
    startOTA();
    Serial.println("\e[1;31m\033[103mLocal Ip " + WiFi.localIP().toString() + "\u001b[0m");
    Serial.print("Time server IP:\t");
    Serial.println(timeServerIP);
    
    pinMode(SerialCtrlPin, OUTPUT);

    // sendNTPpacket(timeServerIP);
    //  setSyncProvider(getTime);
    // Serial.println("Persistent = " + String(WiFi.getPersistent()));
    // webSocket.begin();
    // webSocket.onEvent(webSocketEvent);
    // Serial.println("Autoreconnect = " + String(WiFi.getAutoReconnect()));
#ifdef JELLY_NUEVO
    Wire.begin(SDA, SCL); // Aqui setea los wire al default para usar el Jellyfish pero hay conflictos con otros WIRE
#endif
#ifdef JELLY_VIEJO
    Wire.begin(SDA, SCL); // Aqui setea los wire al default para usar el Jellyfish pero hay conflictos con otros WIRE
#endif
    //rs485.begin (115200);
    //if (!ModbusRTUClient.begin(9600)) {
    //Serial.println("Failed to start Modbus RTU Client!");
    //}
    pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  // Modbus communication runs at 9600 baud
  #ifdef ESP32
  SerialInterface.begin(9600);
  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      Serial.printf("%02x", data[i]);
    }
    std::reverse(data, data + 4);  // fix endianness
    Serial.printf("\nval: %.2f", *reinterpret_cast<float*>(data));
    Serial.print("\n\n");

    Modbus_Device::checkSlaveIDs (serverAddress,data[0]);
  });
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("Modbus error: 0x%02x\n\n", static_cast<uint8_t>(error));
  });
  modbus.begin();
  #endif
}
void printJavaQueue(Page page)
{
    String ss = page.getJavaQueue(); // Get the JavaScript Queue from page
    Serial.println(ss);
    // if (ss != "")
    //  webSocket.broadcastTXT(ss); //  WebSoket necesita una variable, no puedo poner page.getJavaQueue directamente
}

ICACHE_RAM_ATTR void interruptRoutine()
{
    static unsigned long last_interrupt_time = 0; // Static dentro de una subrutina crea esta variable solo una vez
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200)
    {
        // forward?distance+=pulleyDiameter:distance-+pulleyDiameter;
    }
    last_interrupt_time = interrupt_time;
}

///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////             Save and Settings                              ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////

void saveLog(String text)
{
#ifdef ESP8266
    File logFile = LittleFS.open("/log.txt", "a+");
#endif
#ifdef ESP32
    File logFile = SPIFFS.open("/log.txt", "a+");
#endif
    logFile.write((uint8_t *)text.c_str(), text.length());
    logFile.close();
    Serial.println("Logged:");
    Serial.println(text);
}

void saveSettings(String receivedParameter, String receivedValue)
{
    MapFile::saveMap(receivedParameter, receivedValue, "/settings.set");
    // if (debug)
    //    Serial.println("Saved Settings: \n" + receivedParameter);
}
void saveState(String receivedParameter, String receivedValue)
{
    MapFile::saveMap(receivedParameter, receivedValue, "/status.sta");
}

String lookUpSettings(String receivedParameter)
{
    return MapFile::lookUpMap(receivedParameter, "/settings.set");
}

void loadPreferences()
{
    Serial.println("Loading Preferences");
    ntpInterval = lookUpSettings("ntpInterval").toInt();
    int s = lookUpSettings("secondsCapture").toInt();
    if (s)
    {
        secondsCapture = s;
        logger.setInterval(s);
    }
    s = lookUpSettings("uploadInterval").toInt();
    if (s)
        logger.setUploadInterval(s);
    String ss = lookUpSettings("name");
    // Serial.println("Name: " + (ss)+"\nDevice: "+deviceID+"\nVersion: "+versionID);
    if (ss != "")
    {
        deviceID = ss;
        logger.setFileName(ss + ".csv");
        updater.setFileName(ss);
        updater.setID(mdnsName);
        lblVersion.update("Device: " + String(DEVICE_NAME) + " - Ver: " + String(SOFT_VERSION) + "  Name: " + deviceID);
    }
    ss = lookUpSettings("topic");
    if (ss == "default")
        mqttTopic = "devices/" + deviceID + "/inputs/Temperature";
    else
        mqttTopic = ss;
    ss = lookUpSettings("mqttServer");
    if (ss != "")
    {
        mqttServer = ss;
    } // startMQTT();}

    // s= lookUpSettings("feedAmount").toInt();
    //  if (s)  feeder.setDispenseWeight(s);
}

///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////             GENERAL LOOP                                    ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////

void generalLoop()
{
    unsigned long currentMillis = millis();

#ifdef ESP8266
    MDNS.update();
    tcpCleanup(); //  Supuestamente para limpiar el tcp de conecciones multiples
#endif

    ArduinoOTA.handle();
    if (mqttClient.connected())
        mqttClient.loop();
    //                Serial.println(mqttClient.connected());

    server.handleClient();

    //   while (SerialInterface.available () > 0)           // Esto he agregado para procesar entrada Serial
    //     processIncomingByte (SerialInterface.read ());   // Tambien lo puedo usar con Software Serial

    delay(50);

    if ((currentMillis - lastUpdate) > 1000) //   EVERY SECOND       ###############
    {
        String ss = page.getJavaQueue(); // Get the JavaScript Queue from page
        // Serial.print("Javaqueue: ");Serial.println(ss);
        if (ss != "")
            lastUpdate = currentMillis;
        page.update();
    }

    if ((currentMillis - lastSaveTime) > 10000) //  EVERY 10 SECONDS #########
    {
        Serial.println("\033[34m\nSTART_OF_10_SEC_LOOP");
        // Serial.println("Heap Left: " + String(ESP.getFreeHeap(), DEC)); //+" :Frag: " +String(ESP.getHeapFragmentation(),DEC)+"   Max-SIze = "+ String(ESP.getMaxFreeBlockSize()));
        saveState("unixTime", String(now()));
        if (timeStatus() == timeNotSet && (WiFi.status() == WL_CONNECTED))
        {
            sendNTPpacket(timeServerIP);
            getTime();
        }
        lastSaveTime = millis();
        IPAddress ipBroadcast(myIP[0], myIP[1], myIP[2], 255);
        String msg = "ID: " + String(mdnsName) + "\nIP: http://" + WiFi.localIP().toString() + "\nFile: " + deviceID + "\n\n";
        UDP.beginPacket(ipBroadcast, 1234);
        UDP.print(msg.c_str());
        UDP.endPacket();
        Serial.print("Time UTC: ");
        Serial.println(digitalClockDisplay());
        Serial.print("Wifi.status = ");
        Serial.println((WiFi.status() == 3) ? "Connected." : "Disconnected");

        if (WiFi.status() == WL_CONNECTED)
            lastWifiCheck = now(); // si hay coneccion resetea el counter para el reset
        if (now() - lastWifiCheck > intervalWifiCheck && lookUpSettings("wifi_ssid") != "")
            {
                Serial.println("Trying to connect to Wifi." + String(WiFi.getMode()));
                lastWifiCheck = now();
                connectToWifi();//    ESP.restart();
            }   

        if ((now() - lastUpdateCheck) > updateCheckInterval)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                updater.checkUpdate();
            }
            //else                      // Por Ahora
            lastUpdateCheck = now(); // si esta en AP y hay SSID se resetea.
        }                            // No he podido hacer funcionar el AP y STA
        if (WiFi.status() == WL_CONNECTED)
        {
            // Serial.println(mqttClient.connected());
            if (!mqttClient.connected())
            {
                startMQTT(); // CHECK THIS IS A PROBLEM MQTT NEEDS CONNECTING EVERYTIME FOR SOME REASON
            }
            else logger.sendMqtt();
        }
        Serial.println("\033[39m");
    } // Para usar en modo AP sacar el wifi_ssid de los Settings
}

///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////              methods                                         ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////
//#######################################
//   PROCESS SERIAL DATA WHEN /n RECEIVED
//#######################################
String lastIdMsg;
void process_data(const char *data)
{
    Serial.print("\033[32mReceived from SerialInterface:");
    Serial.println(data);
    const String dataStr(data); // convert data into String
    if (strcmp(data, "reset") == 0)
        ESP.restart();
    int index = dataStr.indexOf("id.req="); // es el mensage que pregunta a un Element su value
    if (index != -1)
    {
        Serial.println(index);
        String _id = dataStr.substring(index + 7, dataStr.indexOf(",", index));
        // Serial.println(_id);
        if (_id == String('?'))
            Serial.println(lastIdMsg);
        lastIdMsg = _id;
        float f = page.serialCallback(_id);
        if (f != -1)
        {
            digitalWrite(SerialCtrlPin, HIGH);
            SerialInterface.println("rp=" + String(f) + ","); // le agrega "rp=" a la respuesta.
            SerialInterface.flush();
            digitalWrite(SerialCtrlPin, LOW);
            lastIdMsg = "";
        }
    }

    index = dataStr.indexOf("rp=");
    if (index > -1)
    {
        //Serial.println(index);
        String _value = dataStr.substring(index + 3, dataStr.indexOf(",", index));
        ElementsHtml *temp = page.searchById(lastIdMsg);
        if (temp != NULL)
        {
            Serial.println("Trying with " + temp->id + " " + _value);
            temp->update(_value);
        }
    }

    Serial.print("\033[39m");
} // end of process_data

//#######################################
//   PROCESS EACH SERIAL BYTE UNTIL /n RECEIVED
//#######################################
void processIncomingByte(const byte inByte)
{
#define MAX_INPUT 50 // El tamaño del buffer del serial, es el numero max de caracteres de un msg
    static char input_line[MAX_INPUT];
    static unsigned int input_pos = 0;
    // Serial.println("Processing incomming Serial Byte: "+String(inByte));
    switch (inByte)
    {

    case '\n':                     // end of text
        input_line[input_pos] = 0; // terminating null byte

        // terminator reached! process input_line here ...
        process_data(input_line);

        // reset buffer for next time
        input_pos = 0;
        break;

    case '\r': // discard carriage return
        break;

    default:
        // keep adding if not full ... allow for terminating null byte
        if (input_pos < (MAX_INPUT - 1))
            input_line[input_pos++] = inByte;
        break;

    } // end of switch

} // end of processIncomingByte

// PUT THE ESP TO SLEEP
void espSleep()
{
    // WiFi.disconnect();
    // while (WiFi.isConnected()){}
    Serial.println("Going to sleep for " + String(secondsCapture) + " seconds.");
    saveLog("espSleep()\n\n");
    saveState("unixTime", String(now()));
    ESP.deepSleep(secondsCapture * (1000000 + timeFactor));
}

void startOTA()
{ // Start the OTA service
//#ifdef ESP8266 || ESP32
    Serial.println("Start OTA");
    ArduinoOTA.setHostname(OTAName);
    // ArduinoOTA.setPassword(OTAPassword);

    ArduinoOTA.onStart([]()
                       { Serial.println("Start"); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\r\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           Serial.printf("Error[%u]: ", error);
                           if (error == OTA_AUTH_ERROR)
                               Serial.println("Auth Failed");
                           else if (error == OTA_BEGIN_ERROR)
                               Serial.println("Begin Failed");
                           else if (error == OTA_CONNECT_ERROR)
                               Serial.println("Connect Failed");
                           else if (error == OTA_RECEIVE_ERROR)
                               Serial.println("Receive Failed");
                           else if (error == OTA_END_ERROR)
                               Serial.println("End Failed"); });
    ArduinoOTA.begin();
    Serial.println("OTA ready\r\n");
//#endif
}

String listDir(String dirname, uint8_t levels)
{
    String s;
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = FILE_SYS.open(dirname, "r");
    if (!root)
    {
        Serial.println("- failed to open directory");
        return ("failed to open Directory");
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return "not a directory.";
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            s += " DIR:  ";
            Serial.println(file.name());
            if (levels)
            {
                listDir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
            s += ("  FILE: ");
            s += (file.name());
            s += ("\tSIZE: ");
            s += (file.size());
        }
        file = root.openNextFile();
    }
    return s;
}

void startSPIFFS()
{                     // Start the LittleFS and list all contents
    FILE_SYS.begin(); // Start the SPI Flash File System (LittleFS)
    Serial.println("FILE_SYS started.");
#ifdef ESP8266
    {
        Dir dir = FILE_SYS.openDir("/");
        while (dir.next())
        { // List the file system contents
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("Ended\n");
    }
#endif
#ifdef ESP32
    Serial.println("SPIFFS started. Contents:");

    // File dir = SPIFFS.open("/");
    listDir("/", 4);
    Serial.printf("Ended\n");

#endif
}

void startMDNS(String mdnsName_)
{ // Start the mDNS responder
    Serial.println("Start MDNS");

    MDNS.begin(mdnsName.c_str()); // start the multicast domain name server
    MDNS.addService("http", "tcp", 80);
    Serial.print("mDNS responder started: http://");
    Serial.print(mdnsName_);
    Serial.println(".local");
}

void startUDP()
{
    Serial.println("Starting UDP");
    UDP.begin(1234);     // Start listening for UDP messages to port 123
    UDPtime.begin(7654); // Start listening for UDP messages to port 123
}

void startMQTT()
{
    static unsigned long lastStartMqtt;
    if (WiFi.status() == WL_CONNECTED)
    {
        if (millis() - lastStartMqtt > 60000)
        {
            Serial.println("Start MQTT on " + (mqttServer));

            mqttClient.begin(mqttServer.c_str(), mqttPort, wifiClientMQTT);
            mqttClient.onMessage(mqttMsgReceived);
            // while (!mqttClient.connected()) {
            Serial.println("Connecting to MQTT...");
            if (mqttClient.connect(deviceID.c_str(), mqttUser, mqttPassword))
            {
                Serial.print("MQTT connected: ");
                // mqttClient.subscribe((deviceID + "_com").c_str());
                Serial.println(mqttClient.connected());
            }
            else
            {
                Serial.println("MQTT Error: " + String(mqttClient.lastError()));
            }
            lastStartMqtt = millis();
        }
    }
}

////////////////////////////////////////////
//      SERVER START                       //
////////////////////////////////////////////

#if defined ESP8266 || defined ESP32
void startServer()
{ // Start a HTTP server with a file read handler and an upload handler

    server.on(
        "/edit.html", HTTP_POST, []() { // If a POST request is sent to the /edit.html address,
            server.send(200, "text/plain", "");
        },
        handleFileUpload);
    // server.on("/wifi", std::bind(&WiFiManager::handleWifi, this, true));
    server.on("/list", HTTP_GET, handleFileList);
    server.on("/delete", HTTP_GET, handleFileDelete);
    server.on("/rename", HTTP_GET, handleFileRename);
    server.on("/btnClick", HTTP_GET, handleBtnClick);
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/settingsChanged", HTTP_GET, handleSettingsChanged);
    server.on("/index1.html", HTTP_GET, handleIndex);
    server.on("/wifiScan", HTTP_GET, handleWifiManager);
    server.on("/reset", HTTP_GET, handleReset);
    server.on("/debug", HTTP_GET, handleDebug);
    server.on("/ctrl", HTTP_GET, handleControl);
    server.on("/sendEZOmsg", HTTP_GET, handleSendEzoMsg);
    server.on("/readCal", handleReadCal);
    server.on("/getJavaScript", handleGetJavaScript);
    server.onNotFound(handleNotFound); // if someone requests any other file or page, go to function 'handleNotFound'
    // and check if the file exists

    // Added
    // SSDP makes device visible on windows network
#ifdef ESP8266
    server.on("/description.xml", HTTP_GET, []()
              { SSDP.schema(server.client()); });
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName(DEVICE_NAME);
    SSDP.setURL("/");
    // Serial.println("Server begin()2");
    SSDP.setDeviceType("upnp:rootdevice");

    // SSDP.begin();                           ///   Checar porque no funciona SSDP cuando esta en AP
#endif
    server.begin(); // start the HTTP server

    Serial.println("HTTP server started.");
}

////////////////////////////////////////////////////////
////   SERVER HANDLERS                  ///////////////
///////////////////////////////////////////////////////
void handleIndex()
{
    // server.send(400,"text/html",page.getHtmlString());
}
void handleWifiManager()
{
    // wifiManager.startConfigPortal("WiFi Manager");
}
void handleForgetWifi()
{
    WiFi.disconnect();
    handleReset();
}
void handleReset()
{
    server.send(200, "text/html", "Restarting device.");
    ESP.restart();
}
void handleNotFound()
{ // if the requested file or page doesn't exist, return a 404 not found error
    if (!handleFileRead(server.uri()))
    { // check if the file exists in the flash memory (LittleFS), if so, send it
        server.send(404, "text/plain", "404: File Not Found");
    }
}
void handleDebug()
{
    debug = !debug;
    page.setDebug(debug);
    server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='3; url=index.html'></head>debug: " + String(debug) + "</html>");
}
void handleGetJavaScript()
{
    server.send(200, "text/html", ElementsHtml::javaQueue.getJson());
}
///////////////////////////////////////////////////////////////////////
//////////////   HANDLE BUTTON CLICK   ///////////////////////////////                    BUTTON CLICK
///////////////////////////////////////////////////////////////////////
#endif
//#ifdef ESP8266
void handleBtnClick()
//#endif
#ifdef ESP322
    ArRequestHandlerFunction handleBtnClick(AsyncWebServerRequest *request)
#endif
{
    String callerName = "undefinido";
    String callerValue = "undefinido";
    // String buttonDataValue = "undefinido";
    String reply = "console.log('no reply');";
    if (server.hasArg("button")  ) 
    {
        callerName = server.arg("button");
        callerValue = server.arg("value");
        // buttonDataValue = REQUEST_ arg("datavalue");
        // Serial.println("Caller:"+callerName+" value: "+callerValue);
        // Serial.println("BtnCklick(0)");

        for (int i = 0; i < page.elementCount; i++)
        {
            // Serial.println(page.listOfElements[i]->id);
            if (page.listOfElements[i]->id == (callerName))
            {

                Serial.println("LISTofELEMENTS sending post call back to: " + callerName);
                page.listOfElements[i]->postCallBack(page.listOfElements[i], callerValue);
                // Serial.println("sentpost call back to: " + callerName);
            }
        }
        for (unsigned int i = 0; i < ElementsHtml::allHTMLElements.size(); i++)
        {
            // Serial.println(ElementsHtml::allHTMLElements[i]->id);
            if (ElementsHtml::allHTMLElements[i]->id == (callerName))
            {
                Serial.println("ALL HTML ELEMENTS sending post call back to: " + callerName);
                ElementsHtml::allHTMLElements[i]->postCallBack(ElementsHtml::allHTMLElements[i], callerValue);
                // Serial.println("sent post call back to: " + callerName);
            }
        }

        if (callerName == "lblTime" || callerName == "Time")
        {
            consoleLog("main", "calledTime");
            Serial.println("Time Setting btnClick");

            ElementsHtml::javaQueue.add("console.log('setting Time');");
            if (timeStatus() == timeNotSet)
            {
                timeUNIX = callerValue.toDouble();
                lastNTPResponse = millis();
                setTime(timeUNIX); //-300*60);
                consoleLog("main", "setting Time");
            }
        }

        if (callerName == "btnSleep") // Aqui he cambiado ahora es sleep forever
        {
            //Serial.println("BtnSleepPressed");
            secondsCapture = logger.getInterval();
            Serial.println("Going to sleep for ever"); // + String(secondsCapture) + " segundos.");
            saveSettings("secondsCapture", String(secondsCapture));
            saveState("totalTime", "0");
            // saveState("timeFactor", "0");
            saveState("unixTime", "0"); // String(now()));
            server.send(200, "text/html", "alert('Apagado...');");
            saveLog(digitalClockDisplay() + "\n");
            ESP.deepSleep(0); // secondsCapture * 1000000, RF_DEFAULT);
        }
    }
    // Serial.println(1);
    server.send(200, "text/plain", "Made call to btnClick: " + callerName + " value: " + callerValue);
    // Serial.println(2);
}

#if defined ESP8266 || defined ESP32
void handleFileList()
{
#endif
#ifdef ESP322
    ArRequestHandlerFunction handleFileList(AsyncWebServerRequest * request)
    {
#endif

        if (!server.hasArg("dir"))
        {
            server.send(500, "text/plain", "BAD ARGS");
        }
        String dirname = server.arg("dir");
        Serial.println("Listing directory: %s\n" + dirname);
#ifdef ESP8266
        Dir root = FILE_SYS.openDir(dirname);
#endif
#ifdef ESP32
        File root = FILE_SYS.open(dirname);
#endif
        String output;
        output += "<form method='post' enctype='multipart/form-data' style='margin: 0px auto 8px auto' >"
                  "<input type='file' name='Choose file' accept='.gz,.html,.ico,.js,.css'>"
                  "<input class='button' type='submit' value='Upload' name='submit'> </form><br>";
        int memoryUsed;
#ifdef ESP8266
        while (root.next())
        {
            File file = root.openFile("r");
            output += "  FILE:  <a href='/List?dir=";
            output += (root.fileName());
            output += "'>";
            output += (root.fileName());
            output += ("</a>  SIZE: ");
            output += (file.size());
            memoryUsed += file.size();
            time_t cr = file.getCreationTime();
            time_t lw = file.getLastWrite();
            file.close();
#endif
#ifdef ESP32
            File file = root.openNextFile();
            while (file)
            {
                if (file.isDirectory())
                {
                    output += "  FILE:  <a href='/List?dir=/capturas";
                    output += file.name(); //.resize(strrchr(file.name(),'/'));
                }
                else
                {
                    output += " FILE: <a href='";
                    output += (file.name());
                }
                output += "'>";
                output += (file.name());
                output += ("</a>  SIZE: ");
                output += (file.size());
                memoryUsed += file.size();
                time_t cr = 0;
                time_t lw = 0;
                // file.close();
#endif
                struct tm *tmstruct = localtime(&cr);
                output += ("    CREATION: " + String((tmstruct->tm_year) + 1900) + "/" + // %d-%02d-%02d %02d:%02d:%02d\n regex
                           String((tmstruct->tm_mon) + 1) + "/" + String(tmstruct->tm_mday) + " " + String(tmstruct->tm_hour) + ":" +
                           String(tmstruct->tm_min) + ":" + String(tmstruct->tm_sec));
                tmstruct = localtime(&lw);
                output += ("  LAST WRITE: " + String((tmstruct->tm_year) + 1900) + "/" + String((tmstruct->tm_mon) + 1) + "/" + String(tmstruct->tm_mday) +
                           " " + String(tmstruct->tm_hour) + ":" + String(tmstruct->tm_min) + ":" + String(tmstruct->tm_sec));
                output += "\"<a href='/delete?file=";
#ifdef ESP8266
                output += dirname + "/" + root.fileName();
#endif
#ifdef ESP32
                output += file.name();
                file = root.openNextFile("r");
#endif
                output += "'>delete</a><br>\n";
            }
            output += "<br>Memory used: " + String(memoryUsed);
            server.send(200, "text/html", output);
        }

#if defined ESP8266 || defined ESP32
        void handleControl()
        {
#endif
#ifdef ESP322
            ArRequestHandlerFunction handleControl(AsyncWebServerRequest * request)
            {
#endif
                String device, arg1, arg2;

                if (server.hasArg("device"))
                {
                    device = server.arg("device");
                    arg1 = server.arg("arg1");
                    arg2 = server.arg("arg2");
                    Serial.println("Handling Ctrl for device: " + device);
                }

                for (int i = 0; i < page.elementCount; i++)
                {
                    // Serial.println(page.listOfElements[i]->id);
                    if (page.listOfElements[i]->id == (device))
                    {
                        Serial.println("Ctrl endpoint reached sent to: " + device + "  arg1: " + arg1);
                        // page.("console.log('Ctrl endpoint reached sent to:");// + device+"  arg1: "+arg1+"');");
                        page.listOfElements[i]->update(arg1.toFloat());
                    }
                }

                server.send(200, "text/html", "Control Handled");
            }

#if defined ESP8266 || defined ESP32
            void handleSettings()
#endif
#ifdef ESP322
                ArRequestHandlerFunction
                handleSettings(AsyncWebServerRequest * request)
#endif
            {
 if (!server.authenticate("pi","pass")) {
      server.requestAuthentication();
  }
                String reply = "<html><head>  <link rel='stylesheet' type='text/css' href='style.css'></head>"
                               "<body>"
                               "<h1>Preferencias</h1>"
                               "<a href='/'>Home</a>";
                File settingsFile = FILE_SYS.open("/settings.set", "r");
                if (settingsFile)
                {
                    while (settingsFile.available())
                    {
                        String line = settingsFile.readStringUntil('\n');
                        String parameter = line.substring(0, line.indexOf(","));
                        String value = line.substring(line.indexOf(",") + 1, line.length());
                        Serial.println(parameter + ":" + value);
                        reply = reply + "<p>" + parameter + " : <input type='text' width='10em' id='" + parameter + "' value='" + value + "' onchange='valueChanged(this)'></p>";
                    }
                }
                reply += "<script> function valueChanged(element){ name= element.id; value= element.value;var xhttp = new XMLHttpRequest();  xhttp.open('GET', 'settingsChanged?parameter='+ name + '&value=' + value, true); xhttp.send();}</script></body></html>";
                server.send(200, "text/html", reply);
            }
#if defined ESP8266 || defined ESP32
            void handleSettingsChanged()
#endif
#ifdef ESP322
                ArRequestHandlerFunction
                handleSettingsChanged(AsyncWebServerRequest * request)
#endif
            {
                saveSettings(server.arg("parameter"), server.arg("value"));
                loadPreferences();
                server.send(200, "text", "ok");
            }

#if defined ESP8266 || defined ESP32
            bool handleFileRead(String path)
            { // send the right file to the client (if it exists)

  if (!server.authenticate("pi","pass")) {
      server.requestAuthentication();
  }
 


                Serial.println("handleFileRead: " + path);
                if (path.endsWith("/"))
                    path += "index.html"; // If a folder is requested, send the index file
                if (path == "/index.html")
                    page.getHtml();
                String contentType = getContentType(path); // Get the MIME type
                String pathWithGz = path + ".gz";
                if (FILE_SYS.exists(pathWithGz) || FILE_SYS.exists(path))
                { // If the file exists, either as a compressed archive, or normal
                    String device, arg1, arg2;
                    if (server.hasArg("device"))
                    {
                        device = server.arg("device");
                        arg1 = server.arg("arg1");
                        arg2 = server.arg("arg2");
                        Serial.println("Handling Ctrl for device: " + device);
                    }
                    if (FILE_SYS.exists(pathWithGz))      // If there's a compressed version available
                        path += ".gz";                    // Use the compressed verion
                    File file = FILE_SYS.open(path, "r"); // Open the file
                    if (contentType == "csv")
                    {
                        server.send(200, "text/html", "");
                    }
                    size_t sent = server.streamFile(file, contentType); // Send it to the client
                    file.close();                                       // Close the file again
                    Serial.println(String("\tSent file: ") + path + " " + String(sent) + " bytes.");
                    return true;
                }
                Serial.println(String("\tFile Not Found: ") + path); // If the file doesn't exist, return false
                return false;
            }

            void handleFileUpload()
            { // upload a new file to the FILE_SYS
                HTTPUpload &upload = server.upload();
                String path;
                if (upload.status == UPLOAD_FILE_START)
                {
                    path = upload.filename;
                    if (!path.startsWith("/"))
                        path = "/" + path;
                    if (!path.endsWith(".gz"))
                    {                                     // The file server always prefers a compressed version of a file
                        String pathWithGz = path + ".gz"; // So if an uploaded file is not compressed, the existing compressed
                        if (FILE_SYS.exists(pathWithGz))  // version of that file must be deleted (if it exists)
                            FILE_SYS.remove(pathWithGz);
                    }
                    Serial.print("handleFileUpload Name: ");
                    Serial.println(path);
                    fsUploadFile = FILE_SYS.open(path, "w"); // Open the file for writing in LittleFS (create if it doesn't exist)
                    path = String();
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    if (fsUploadFile)
                        fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    if (fsUploadFile)
                    {                         // If the file was successfully created
                        fsUploadFile.close(); // Close the file again
                        Serial.print("handleFileUpload Size: ");
                        Serial.println(upload.totalSize);
                        server.sendHeader("Location", "/success.html"); // Redirect the client to the success page
                        server.send(303);
                    }
                    else
                    {
                        server.send(500, "text/plain", "500: couldn't create file");
                    }
                }
            }

            void handleFileDelete()
            { // Usage   192.168.1.13/delete?file=temp.csv
                if (!server.hasArg("file"))
                {
                    server.send(500, "text/plain", "BAD ARGS");
                    return;
                }
                String path = server.arg("file");
                Serial.println("handleFileDelete: " + path);
                if (path == "/")
                    return server.send(500, "text/plain", "BAD PATH");
                if (!FILE_SYS.exists(path))
                    return server.send(404, "text/plain", "FileNotFound");

                FILE_SYS.remove(path);
                // server.send(200, "text/html", "alert('Borrado archivo:\n"+path+"');");

                server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='3; url=index.html'></head>deleted" + path + "</html>");
            }

            void handleFileRename()
            { // Usage   192.168.1.13/rename?file=temp.csv&name=newNameforFile
                if (!server.hasArg("file"))
                {
                    server.send(500, "text/plain", "BAD ARGS");
                    return;
                }
                String path = server.arg("file");
                Serial.println("handleFileRename: " + path);
                if (path == "/")
                    return server.send(500, "text/plain", "BAD PATH");
                if (!FILE_SYS.exists(path))
                    return server.send(404, "text/plain", "FileNotFound");
                if (server.hasArg("name"))
                    String name = server.arg("name");
                {
                    FILE_SYS.rename(path, server.arg("name"));
                }
                String s = "<html><head><meta http-equiv='refresh' content='3; url=index.html'></head>Renamed";
                s += path;
                s += " to ";
                s += server.arg("name");
                s += "</html>";
                // server.send(200, "text/html",s.c_str());
                server.send(200, "text/plain", "alert('Nuevo nombre de captura:\n" + server.arg("name") + "');");
            }

            void handleReadCal()
            {
                server.send(200, "text/html", I2C_Msg::readProbeCalibration());
            }
            void handleSendEzoMsg()
            { // EXAMPLE http://192.168.1.121/sendEZOmsg?msg=DOL,0 apaga el led del DO

                Serial.println("handleEZOmsg");
                if (server.hasArg("msg"))
                {
                    String msg = server.arg("msg");
                    Serial.println(msg);
                    int add;
                    String address = msg.substring(0, 2);
                    Serial.println(address);
                    String argument = msg.substring(2, msg.length());
                    Serial.println(argument);
                    if (address == "DO")
                    {
                        add = DO_I2C_ADDRESS; // Aqui tabbien hay que cambiar !!!!
                        Serial.print("DO-ADDRES");
                    }
                    if (address == "EC")
                    {
                        add = EC_I2C_ADDRESS; // Aqui
                        Serial.print("EC- Address");
                    }
                    if (address == "PH")
                    {
                        add = PH_I2C_ADDRESS;
                        Serial.print("PH- Address");
                    }
                    if (address == "BA")
                    {
                        batteryFactor = argument.toFloat();
                    }
                    int stringLength = argument.length() + 1;
                    char charBuf[stringLength];
                    // argument.toCharArray(charBuf, 50)
                    argument.toCharArray(charBuf, stringLength);
                    // Serial.println(String(add));
                    msg = sendI2CCommand(add, argument);
                    server.send(200, "text/plain", "Reply: "+msg);
                }
            }
#endif

#ifdef ESP322
            void startServer()
            {
                // server.on("/edit.html", HTTP_POST, [](AsyncWebServerRequest *request) { // If a POST request is sent to the /edit.html address,
                //                                     request->send(200, "text/plain", "");
                //                                 }, handleFileUpload);

                server.onFileUpload(handleFileUpload);
                server.on("/list", HTTP_GET, handleFileList);
                server.on("/delete", HTTP_GET, handleFileDelete);
                server.on("/rename", HTTP_GET, handleFileRename);
                server.on("/btnClick", HTTP_GET, handleBtnClick);
                server.on("/settings", HTTP_GET, handleSettings);
                server.on("/settingsChanged", HTTP_GET, handleSettingsChanged);
                server.on("/index1.html", HTTP_GET, handleIndex);
                server.on("/wifiScan", HTTP_GET, handleWifiManager);
                server.on("/reset", HTTP_GET, handleReset);
                server.on("/debug", HTTP_GET, handleDebug);
                server.on("/ctrl", HTTP_GET, handleControl);
                server.on("/sendEZOmsg", HTTP_GET, handleSendEzoMsg);
                server.on("/readCal", handleReadCal);
                // server.on("/", handleIndex);
                // server.serveStatic("/",SPIFFS,"/index.html");
                server.onNotFound([](AsyncWebServerRequest *request) { // If the client requests any URI
                    Serial.println(F("On not found"));
                    if (!handleFileRead(request, request->url()))
                    {                            // send it if it exists
                        handleNotFound(request); // otherwise, respond with a 404 (Not Found) error
                    }
                });

                Serial.println(F("Set cache!"));
                // Serve a file with no cache so every tile It's downloaded
                server.serveStatic("/configuration.json", SPIFFS, "/configuration.json", "no-cache, no-store, must-revalidate");
                // Server all other page with long cache so browser chaching they
                server.serveStatic("/", SPIFFS, "/", "max-age=31536000");
                // and check if the file exists
                // server.on('/', HTTP_GET, [] (AsyncWebServerRequest *request) {
                //             //const char str[]           = "/index.html";      // this is changed
                //             String  str           = "/index.html";      // this is changed
                //              //request->send(FILE_SYS, str);
                //              request->send(SPIFFS,"text",str);
                //              });
                // server.serveStatic("/", SPIFFS, "/");
                server.begin();
            }

            bool handleFileRead(AsyncWebServerRequest * request, String path)
            {
                Serial.print(F("handleFileRead: "));
                Serial.println(path);

                if (path.endsWith("/"))
                    path += F("index.html");               // If a folder is requested, send the index file
                String contentType = getContentType(path); // Get the MIME type
                String pathWithGz = path + F(".gz");
                if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
                { // If the file exists, either as a compressed archive, or normal
                    bool gzipped = false;

                    if (SPIFFS.exists(pathWithGz))
                    {                     // If there's a compressed version available
                        path += F(".gz"); // Use the compressed version
                        gzipped = true;
                    }
                    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
                    if (gzipped)
                    {
                        response->addHeader("Content-Encoding", "gzip");
                    }
                    Serial.print("Real file path: ");
                    Serial.println(path);

                    request->send(response);

                    return true;
                }
                Serial.println(String(F("\tFile Not Found: ")) + path);
                return false; // If the file doesn't exist, return false
            }

            void handleFileUpload(AsyncWebServerRequest * request, String filename, size_t index, uint8_t * data, size_t len, bool final)
            {
                File uploadFile;
                if (!index)
                {
                    if (!filename.startsWith("/"))
                        filename = "/" + filename;
                    if (SPIFFS.exists(filename))
                    {
                        SPIFFS.remove(filename);
                    }
                    uploadFile = SPIFFS.open(filename, "w");
                }
                for (size_t i = 0; i < len; i++)
                {
                    uploadFile.write(data[i]);
                }
                if (final)
                {
                    uploadFile.close();
                    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "File Uploaded;" + filename);
                    response->addHeader("Access-Control-Allow-Origin", "*");
                    request->send(response);
                }
            }

            void handleNotFound(AsyncWebServerRequest * request)
            {
                String message = "Handle not Found File Not Found\n\n";
                message += "URI: ";
                message += request->url();
                message += "\nMethod: ";
                message += (request->method() == HTTP_GET) ? "GET" : "POST";
                message += "\nArguments: ";
                message += request->args();
                message += "\n";

                for (uint8_t i = 0; i < request->args(); i++)
                {
                    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
                }

                request->send(200, "text/plain", message);
            }

            ArRequestHandlerFunction handleIndex(AsyncWebServerRequest * request)
            {
                //                 String  str           = "/index.html";      // this is changed
                request->send(SPIFFS, "/index.html", "text/html", false);
            }

            ArRequestHandlerFunction handleReset(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleFileRead(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleFileDelete(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleFileRename(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleGetData(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleWifiManager(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleForgetWifi(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleCalibrate(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleSendEzoMsg(AsyncWebServerRequest * request) {}
            ArRequestHandlerFunction handleDebug(AsyncWebServerRequest * request) {}

            ArRequestHandlerFunction handleReadCal(AsyncWebServerRequest * request)
            {
                request->send(200, "text/html", I2C_Msg::readProbeCalibration());
            };

#endif

            String getContentType(String filename)
            {
                if (filename.endsWith(F(".htm")))
                    return F("text/html");
                else if (filename.endsWith(F(".html")))
                    return F("text/html");
                else if (filename.endsWith(F(".css")))
                    return F("text/css");
                else if (filename.endsWith(F(".js")))
                    return F("application/javascript");
                else if (filename.endsWith(F(".json")))
                    return F("application/json");
                else if (filename.endsWith(F(".png")))
                    return F("image/png");
                else if (filename.endsWith(F(".gif")))
                    return F("image/gif");
                else if (filename.endsWith(F(".jpg")))
                    return F("image/jpeg");
                else if (filename.endsWith(F(".jpeg")))
                    return F("image/jpeg");
                else if (filename.endsWith(F(".ico")))
                    return F("image/x-icon");
                else if (filename.endsWith(F(".xml")))
                    return F("text/xml");
                else if (filename.endsWith(F(".pdf")))
                    return F("application/x-pdf");
                else if (filename.endsWith(F(".zip")))
                    return F("application/x-zip");
                else if (filename.endsWith(F(".gz")))
                    return F("application/x-gzip");
                return F("text/plain");
            }

            static bool requestingEzo = 0;

            String sendI2CCommand(int address, String command)
            {

                requestingEzo = true;
                delay(600);

                Wire.requestFrom(address, 14);
                while (Wire.available())
                {
                    char result = Wire.read();
                }

                String resp;

                String msg;
                Wire.beginTransmission(address); // transmit to device in Adress
                Wire.print(command);
                Wire.endTransmission();
                Serial.println("Sent msg to EZO: address: " +String(address)+" command:"+ command);

                delay(600); // let the EZO rest a little...
                if (command != "Sleep")
                {
                    Wire.requestFrom(address, 14);
                    char result = Wire.read();
                    Serial.println("Result: " + String(result));
                    (result == 1) ? msg = "Mensaje " + String(command) + "  recibido." : msg = "Error de comunicacion.";

                    while (Wire.available())
                    {
                        char chr = char(Wire.read());
                        // if ( chr = char(1) ) chr = 44;
                        if (chr == 0)
                            break;
                        resp = resp + chr;
                    }
                }

                Serial.println("Respuesta=  " + String(resp)); // print the character
                msg = String(resp);
                requestingEzo = false;

                return msg;
            }

            // //////////////////////////////////////////////////////////////////
            // ///////////////  Web Socket Event
            // //////////////////////////////////////////////////////////////////
            // void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
            // {
            //     String ss;
            //     switch (type)
            //     {
            //     case WStype_DISCONNECTED:
            //         Serial.printf("[%u] Disconnected!\n", num);
            //         isConnected = false;
            //         page.getHtml(); //              ESTO LO HAGO PARA RENOVAR EL HTML CADA VEZ QUE DESCONECTA !!!! ES NUEVO !!!!
            //         break;
            //     case WStype_CONNECTED:
            //     {
            //         IPAddress ip = webSocket.remoteIP(num);
            //         Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            //         isConnected = true;
            //         // send message to client
            //         page.getHtml();
            //         SavedEdit::updateAll();
            //         webSocket.sendTXT(num, "console.log('Connected');");
            //     }
            //     break;
            //     case WStype_TEXT:
            //         Serial.printf("[%u] get Text: %s\n", num, payload);

            //         // send message to client
            //         ss = page.getJavaQueue();
            //         webSocket.sendTXT(num, ss);

            //         // send data to all connected clients
            //         // webSocket.broadcastTXT("message here");
            //         break;
            //     case WStype_BIN:
            //         Serial.printf("[%u] get binary length: %u\n", num, length);
            //         hexdump(payload, length, 8);
            //         // send message to client
            //         webSocket.sendBIN(num, payload, length);
            //         break;
            //     }
            // }
            // void hexdump(const void *mem, uint32_t len, uint8_t cols)
            // {
            //     const uint8_t *src = (const uint8_t *)mem;
            //     DEBUG_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
            //     for (uint32_t i = 0; i < len; i++)
            //     {
            //         if (i % cols == 0)
            //         {
            //             DEBUG_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
            //         }
            //         DEBUG_SERIAL.printf("%02X ", *src);
            //         src++;
            //     }
            //     DEBUG_SERIAL.printf("\n");
            // }

            /*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

            String formatBytes(size_t bytes)
            { // convert sizes in bytes to KB and MB
                if (bytes < 1024)
                {
                    return String(bytes) + "B";
                }
                else if (bytes < (1024 * 1024))
                {
                    return String(bytes / 1024.0) + "KB";
                }
                else if (bytes < (1024 * 1024 * 1024))
                {
                    return String(bytes / 1024.0 / 1024.0) + "MB";
                }
                return "NAN";
            }

            time_t getTime()
            { // Check if the time server has responded, if so, get the UNIX time, otherwise, return 0
                Serial.println("Checking for NTP response...");
                if (!UDPtime.parsePacket())
                { // If there's no response (yet)
                    Serial.println("No UDPtime response yet.");
                    return 0;
                }
                UDPtime.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
                // Combine the 4 timestamp bytes into one 32-bit number
                uint32_t NTPTime = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
                // Convert NTP time to a UNIX timestamp:
                // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
                const uint32_t seventyYears = 2208988800UL;
                // subtract seventy years:
                uint32_t UNIXTime = NTPTime - seventyYears;
                Serial.println("NTP response time:" + String(UNIXTime));
                setTime(UNIXTime); // Si hay respuesta automaticamente sincroniza.

                UDPtime.stop();
                return UNIXTime;
            }
#ifdef ESP322
            // UDPtime.onPacket([](AsyncUDPPacket packet) {});
            AuPacketHandlerFunction getTimeSync(AsyncUDPPacket packet)
            {
                for (int i = 0; i < packet.length(); i++)
                {
                    packetBuffer[i] = *(packet.data() + i);
                }
                uint32_t NTPTime = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
            }
#endif
            void sendNTPpacket(IPAddress & ip)
            {
                Serial.println("sending NTP packet...");
                // set all bytes in the buffer to 0
                memset(packetBuffer, 0, NTP_PACKET_SIZE);
                // Initialize values needed to form NTP request
                // (see URL above for details on the packets)
                packetBuffer[0] = 0b11100011; // LI, Version, Mode
                packetBuffer[1] = 0;          // Stratum, or type of clock
                packetBuffer[2] = 6;          // Polling Interval
                packetBuffer[3] = 0xEC;       // Peer Clock Precision
                // 8 bytes of zero for Root Delay & Root Dispersion
                packetBuffer[12] = 49;
                packetBuffer[13] = 0x4E;
                packetBuffer[14] = 49;
                packetBuffer[15] = 52;

                // all NTP fields have been given values, now
                // you can send a packet requesting a timestamp:
                UDPtime.beginPacket(ip, 123); // NTP requests are to port 123
                UDPtime.write(packetBuffer, NTP_PACKET_SIZE);
                UDPtime.endPacket();
            }
            String digitalClockDisplay()
            {
                // digital clock display of the time
                String s = String(hour());
                s += printDigits(minute());
                s += printDigits(second());
                s += String(" ");
                s += String(day());
                s += String(" ");
                s += String(month());
                s += String(" ");
                s += String(year());
                return s;
            }

            long mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
            {
                return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
            }

#define ets_wdt_disable ((void (*)(void))0x400030f0)
#define ets_delay_us ((void (*)(int))0x40002ecc)

#define _R (uint32_t *)0x60000700
            void nk_deep_sleep(uint64_t time)
            {
                ets_wdt_disable();
                *(_R + 4) = 0;
                *(_R + 17) = 4;
                *(_R + 1) = *(_R + 7) + 5;
                *(_R + 6) = 8;
                *(_R + 2) = 1 << 20;
                ets_delay_us(10);
                *(_R + 39) = 0x11;
                *(_R + 40) = 3;
                *(_R) &= 0xFCF;
                *(_R + 1) = *(_R + 7) + (45 * (time >> 8));
                *(_R + 16) = 0x7F;
                *(_R + 2) = 1 << 20;
                __asm volatile("waiti 0");
            }

            void mqttMsgReceived(String & topic, String & payload)
            {
                topic = topic.substring(topic.lastIndexOf("/") + 1); // just take the last word after the last /
                Serial.println("incoming: " + topic + " - " + payload);
                page.mqttMessage(topic, payload);
            }
