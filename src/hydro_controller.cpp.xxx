
#include "../common-libs/header.h"

#define DEVICE_NAME "hydro_controller"   //  Aqui es importante define el nombre para los updates es el mismo para los dispositivos del mismo tipo
#define SOFT_VERSION "2.4"        //   Changed file system to LittleFS   CHECAR LINEA 311
String mdnsName = DEVICE_NAME;     // "basementController.local" no hace falta saber el IP
const char *OTAName = DEVICE_NAME; // A name and a password for the OTA service

Page page("Hydroponics Controller", "User interface");
Updater updater(DEVICE_NAME, SOFT_VERSION);

TelegramAlarm alarma;
TimeLabel lblTime("lblTime", "");
LabelFreeHeap lblFreeHeap("lblHeap", "this");
Logger logger("Logger", "/dataLog.csv", 60, &wifiClient);
Label lblVersion("lblVersion", "Device: " + String(DEVICE_NAME) +
                   " - Ver: " + String(SOFT_VERSION)+
                   " - ID = "+mdnsName);
DirCapture dirCapture("dCapt", "/capturas");
//RS485Main serialCom(&page); // Esta es la #DEFINICION srlCtrlPin en todo el programa.
//ModbusLed ledModbus ( 1 );
//ModbusRelay (1,1);
//Set modLedSet ("Led","idLed",&ledModbus);
ModbusRelay RelayBlowerSup(1,8);
ModbusRelay relayResiduosSup(1,7);
ModbusRelay bombaClarSup(1,6);
ModbusRelay evClarSup(1,5);
//ModbusRelay SkmSup(1,4);
ModbusRelay RelayFeederAirSup(1,3);
ModbusRelay RelayFeederFeedSup(1,2);
ModbusRelay relayLucesSup(1,1);

ModbusRelay RelayBlowerInf(1,9);
ModbusRelay relayResiduosInf(1,10);
ModbusRelay bombaClarInf(1,11);
ModbusRelay evClarInf(1,12);
//ModbusRelay SkmInf(1,13);
ModbusRelay RelayFeederAirInf(1,14);
ModbusRelay RelayFeederFeedInf(1,15);
ModbusRelay relayLucesInf(1,16);


Set switchBlowerSup("Lights", "bls", &RelayBlowerSup);
Set switchBlowerInf("Vent", "bli", &RelayBlowerInf);



GenericTimer residuosSup("Nursery_1", "rs", &relayResiduosSup);
GenericTimer residuosInf("Nursery_2", "ri", &relayResiduosInf);

GenericTimer lucesSup("Growout_1", "ls", &relayLucesSup);
GenericTimer lucesInf("Growout_2", "li", &relayLucesInf);



FakeOutput alrmInf;
FakeOutput alrmSup;
Set alarmSup ( "Alarm_Sup","aS",&alrmSup);
Set alarmInf ( "Alarm_Inf","aI",&alrmInf);

Digital_Alarm alarm32("a32",32, &alarma);


//FlowMeter flowMeter(GPIO_NUM_5);
//FlowMeter flowMeter1(GPIO_NUM_18);
//DigitalIn flowMeter(GPIO_NUM_4,"Test");
//GenericInputPanel flowMeterPanel("Flow Meter","L/min",&flowMeter,false,false);
//GenericInputPanel flowMeterPanel1("Flow Meter1","L/min",&flowMeter1,false,false);

#include "../common-libs/footer.h"

// Button btnWifi("switchToStation","WiFi");
// Graphic graphic("tempGraph");
///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////              SETUP                                          ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////

void setup()
{
    startUpWifi();
    //Serial2.begin(115200, SERIAL_8N1, 16, 17);
    SerialInterface.begin(9600, SERIAL_8N1, 16, 17);
    //nodeRelays.begin(SerialInterface);
    // modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    // Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    // for (size_t i = 0; i < length; ++i) {
    //   Serial.printf("%02x", data[i]);
    // }
    // std::reverse(data, data + 4);  // fix endianness
    // Serial.printf("\nval: %.2f", *reinterpret_cast<float*>(data));
    // Serial.print("\n\n");
  //});
  
    //Serial.println(spdInf.pressure->id);
    logger.addInput(&residuosInf);
    logger.addInput(&residuosSup);
    logger.addInput(&lucesInf);
    logger.addInput(&lucesSup);
    //Serial.println(spdInf.pressure->id);
    page.addElement(&lblTime);
    //page.addElement(&vfdSup);
 
    //page.addElement(&vfdInf);
    page.addString("<p>"
                   "<a href=/settings>Preferencias</a>"
                   "</p>"
                    "<h3>Control Panel</h3><div>");
    page.addElement(&alarm32);
    page.addString("<div class='card'>");
   // page.addElement(&flowMeterPanel);
   // page.addElement(&flowMeterPanel1);
    page.addElement(&alarmInf);
    page.addString("</div><div class='card'>");
    page.addElement(&residuosInf);
    //page.addElement(&skimmer_inf);
    page.addElement(&lucesInf);
    page.addString("</div><div class='card'>");
    page.addString("</div>");
    page.addString("</div><h3>Tanque Superior</h3><div>");
    page.addString("<div class='card'>");
    page.addElement(&alarmSup);
    page.addString("</div><div class='card'>");
    page.addElement(&residuosSup);
    //page.addElement(&skimmer_sup);
    page.addElement(&lucesSup);
    page.addString("</div><div class='card'>");
    page.addString("</div>");
    page.addString("</div>");
    page.addElement(&logger);
    //page.addString("</div>");
     page.addElement(&dirCapture);
    page.addString("<br>");
    page.addElement(&lblVersion);
    page.addString("<br>");
    page.addElement(&lblFreeHeap);
    // page.addElement(&btnWifi);

    //page.setDebug(true); //

    Serial.println("page.getHTML()");

    page.getHtml(); // Generate the index.html File
    Serial.println("Generated index.html");
    updater.checkUpdate();
    MapFile::saveLog("Start up unixTime= "+String(timeUNIX));
}

///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////              LOOP                                          ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////
unsigned long lastCheck;
unsigned long lastAlarmSup;
unsigned long lastAlarmInf;
int alarmInterval = 600000;
int x=0;
void loop()
{
    generalLoop();

    if (deviceID == "Indiana_3") lastAlarmSup = millis();


    if (millis() - lastCheck > 10000)
    {
                   if (alarmInf.value) {
                        
              }
              //mdLed.update();
              if (alarmSup.value) {
                        
              }
      }
}
