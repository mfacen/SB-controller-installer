
#include "../common-libs/header.h"

#define DEVICE_NAME "relayBoardModbus"   //  Aqui es importante define el nombre para los updates es el mismo para los dispositivos del mismo tipo
#define SOFT_VERSION "2.184"        //   Changed file system to LittleFS   CHECAR LINEA 311
String mdnsName = DEVICE_NAME;     // "basementController.local" no hace falta saber el IP
const char *OTAName = DEVICE_NAME; // A name and a password for the OTA service

Page page("Shrimpbox Main Controller", "User interface");
Updater updater(DEVICE_NAME, SOFT_VERSION);
//Alarm alarma;
TelegramAlarm alarma;
TimeLabel lblTime("lblTime", "");
LabelFreeHeap lblFreeHeap("lblHeap", "this");
Logger logger("Logger", "/dataLog.csv", 60, &wifiClient);
Label lblVersion("lblVersion", "Device: " + String(DEVICE_NAME) + " - Ver: " + String(SOFT_VERSION));
//RS485Main serialCom(&page); // Esta es la #DEFINICION srlCtrlPin en todo el programa.
//ModbusLed ledModbus ( 1 );
//ModbusRelay (1,1);
//Set modLedSet ("Led","idLed",&ledModbus);
ModbusRelay RelayBlowerSup(1,1);
ModbusRelay relayResiduosSup(1,2);
ModbusRelay bombaClarSup(1,3);
ModbusRelay evClarSup(1,4);
ModbusRelay SkmSup(1,5);
ModbusRelay RelayFeederAirSup(1,6);
ModbusRelay RelayFeederFeedSup(1,7);
ModbusRelay relayLucesSup(1,8);

ModbusRelay RelayBlowerInf(1,9);
ModbusRelay relayResiduosInf(1,10);
ModbusRelay bombaClarInf(1,11);
ModbusRelay evClarInf(1,12);
ModbusRelay SkmInf(1,13);
ModbusRelay RelayFeederAirInf(1,14);
ModbusRelay RelayFeederFeedInf(1,15);
ModbusRelay relayLucesInf(1,16);
//BinaryOutput spdSupCtrl( 22, 24, 25) ;
ModbusVFD spdSupCtrl( 2 , VFD_Types::SOYAN_SVD) ;
ModbusVFD spdInfCtrl( 3 , VFD_Types::SOYAN_SVD) ;
//ModbusVFD spdInfCtrl( 3 , VFD_Types::MOLLOM_B20) ;
//ModbusLed mdLed (4);
//Modbus_Device anIn("AnalogIn",4);
//GenericInputPanel modbusIn ("ModbusIN","",&anIn);
Set vfdSup ("Venturi sup","vfdSup",&spdSupCtrl);
Set vfdInf ("Venturi inf","vfdInf",&spdInfCtrl);
//Modbus_device pressureSensorSup (2,17); //17=A0 en esp8266 Aqui tengo que usar los numeros xQ estoy en ambiente esp32
//  y se va a referir a numeros del esp8266 a travez del modbus
AnalogIn pressureSensorInf (36);
AnalogIn pressureSensorSup (39);

Set switchBlowerSup("Blower_Sup", "bls", &RelayBlowerSup);
Set switchBlowerInf("Blower_Inf", "bli", &RelayBlowerInf);

Set skimmer_sup("Skimmer_Sup", "skmSup", &SkmSup);
Set skimmer_inf("Skimmer_Inf", "skmInf", &SkmInf);

Feeder feederSup("Alimentador_Sup", "fdrSup", &RelayFeederAirSup, &RelayFeederFeedSup, &switchBlowerSup, &logger);
Feeder feederInf("Alimentador_Inf", "fdrInf", &RelayFeederAirInf, &RelayFeederFeedInf, &switchBlowerInf, &logger);

GenericTimer residuosSup("Residuos_Sup", "rs", &relayResiduosSup);
GenericTimer residuosInf("Residuos_Inf", "ri", &relayResiduosInf);

GenericTimer lucesSup("Luces_Sup", "ls", &relayLucesSup);
GenericTimer lucesInf("Luces_Inf", "li", &relayLucesInf);

Clarificador clarificadorSup("Clarificador_Sup", "cs", &bombaClarSup, &evClarSup, &logger);
Clarificador clarificadorInf("Clarificador_Inf", "ci", &bombaClarInf, &evClarInf, &logger);

Speed_Control spdSup("Speed_Ctrl_Sup", "spd_sup",&spdSupCtrl,&pressureSensorSup, &logger);
Speed_Control spdInf("Speed_Ctrl_Inf", "spd_inf", &spdInfCtrl,&pressureSensorInf, &logger);

FakeOutput alrmInf;
FakeOutput alrmSup;
Set alarmSup ( "Alarm Sup","aS",&alrmSup);
Set alarmInf ( "Alarm Inf","aI",&alrmInf);
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
    modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      Serial.printf("%02x", data[i]);
    }
    std::reverse(data, data + 4);  // fix endianness
    Serial.printf("\nval: %.2f", *reinterpret_cast<float*>(data));
    Serial.print("\n\n");
  });
    SavedVariable::init();
    spdInf.setVfd(&spdInfCtrl);
    spdSup.setVfd(&spdSupCtrl);
    
    spdInf.init();
    spdSup.init();
    //Serial.println(spdInf.pressure->id);
    logger.addInput(&residuosInf);
    logger.addInput(&residuosSup);
    //logger.addInput(&clarificadorInf);
    //logger.addInput(&clarificadorSup);
    //Serial.println(spdInf.pressure->id);
    page.addElement(&lblTime);
    //page.addElement(&vfdSup);
    //page.addElement(&vfdInf);
    page.addString("<p>"
                   "<a href=/settings>Preferencias</a>"
                   "</p>"
                    "<h3>Tanque Inferior</h3><div>");
    //page.addElement(&modbusIn);
   // page.addString("<div class=''>");
    page.addElement(&spdInf);
    page.addString("<div class='card'>");
    page.addElement(&residuosInf);
    page.addElement(&skimmer_inf);
    page.addElement(&lucesInf);
    page.addString("</div><div class='card'>");
    page.addElement(&clarificadorInf);
    page.addElement(&alarmInf);
    page.addString("</div>");
    //page.addString("</div><div class=''>");
    page.addElement(&feederInf);
    page.addString("</div><h3>Tanque Superior</h3><div>");
    page.addElement(&spdSup);
    page.addString("<div class='card'>");
    page.addElement(&residuosSup);
    page.addElement(&skimmer_sup);
    page.addElement(&lucesSup);
    page.addString("</div><div class='card'>");
    page.addElement(&clarificadorSup);
    page.addElement(&alarmSup);
    page.addString("</div>");
    page.addElement(&feederSup);
    page.addString("</div>");
    page.addElement(&logger);
    //page.addString("</div>");
    // page.addElement(&graphic);
    page.addString("<br>");
    page.addElement(&lblFreeHeap);
    page.addElement(&lblVersion);
    // page.addElement(&btnWifi);

    //page.setDebug(true); //

    Serial.println("page.getHTML()");

    page.getHtml(); // Generate the index.html File
    Serial.println("Generated index.html");
    updater.checkUpdate();
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


    if (millis() - lastCheck > 5000)
    {
              if (alarmInf.value) {
                        if (spdInf.getPressure()->value<1200 && (millis()-lastAlarmInf>alarmInterval || lastAlarmInf==0)) {
                            alarma.alarm(deviceID+"_ALARMA%20DE%20PRESION%20"+spdInf.getId() + "%20Nivel:%20"+String (spdInf.value));
                            lastAlarmInf = millis(); }
              }
              //mdLed.update();
              if (alarmSup.value) {
                        if (spdSup.getPressure()->value<1200 && (millis()-lastAlarmSup>alarmInterval || lastAlarmSup==0)) {
                            alarma.alarm(deviceID+"_ALARMA%20DE%20PRESION%20"+spdSup.getId() + "%20Nivel:%20"+String (spdSup.value));
                            lastAlarmSup = millis();}
                lastCheck = millis();
              }
    }
}
