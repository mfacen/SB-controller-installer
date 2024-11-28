
#include "../common-libs/header.h"

#define DEVICE_NAME "relayBoardModbus"   //  Aqui es importante define el nombre para los updates es el mismo para los dispositivos del mismo tipo
#define SOFT_VERSION "2.4"        //   Changed file system to LittleFS   CHECAR LINEA 311
String mdnsName = DEVICE_NAME;     // "basementController.local" no hace falta saber el IP
const char *OTAName = DEVICE_NAME; // A name and a password for the OTA service

Page page("Shrimpbox Main Controller", "User interface");
Updater updater(DEVICE_NAME, SOFT_VERSION);

//Alarm alarma;
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
DigitalOutput R1(32,"D32");
DigitalOutput R2(33,"D33");
DigitalOutput R3(25,"D25");
DigitalOutput R4(26,"D26");
DigitalOutput R5(27,"D27");
DigitalOutput R6(14,"D14");
DigitalOutput R7(12,"D12");
DigitalOutput R8(13,"D13");

//BinaryOutput spdSupCtrl( 22, 24, 25) ;
//ModbusVFD spdSupCtrl( 2 , VFD_Types::SOYAN_SVD) ;
//ModbusVFD spdInfCtrl( 3 , VFD_Types::SOYAN_SVD) ;
//ModbusVFD spdInfCtrl( 3 , VFD_Types::MOLLOM_B20) ;
//ModbusLed mdLed (4);
//Modbus_Device anIn("AnalogIn",4);
//GenericInputPanel modbusIn ("ModbusIN","",&anIn);
//Set vfdSup ("Venturi sup","vfdSup",&spdSupCtrl);
//Set vfdInf ("Venturi inf","vfdInf",&spdInfCtrl);
//Modbus_device pressureSensorSup (2,17); //17=A0 en esp8266 Aqui tengo que usar los numeros xQ estoy en ambiente esp32
//  y se va a referir a numeros del esp8266 a travez del modbus

Set switch_drenaje("C1 Drain", "bls", &R1);
Set switch_on("C1 Pump", "bli", &R2);

//Set skimmer_sup("Skimmer_Sup", "skmSup", &SkmSup);
//Set skimmer_inf("Skimmer_Inf", "skmInf", &SkmInf);

//Feeder feederSup("Alimentador_Sup", "fdrSup", &RelayFeederAirSup, &RelayFeederFeedSup, &switchBlowerSup, &logger);
//Feeder feederInf("Alimentador_Inf", "fdrInf", &RelayFeederAirInf, &RelayFeederFeedInf, &switchBlowerSup, &logger);

GenericTimer bomba_c1("Bomba C1", "rs", &R3);

//Speed_Control spdSup("Speed_Ctrl_Sup", "spd_sup",&spdSupCtrl,&pressureSensorSup, &logger);
//Speed_Control spdInf("Speed_Ctrl_Inf", "spd_inf", &spdInfCtrl,&pressureSensorInf, &logger);
//GenericInputPanel spdSup("venturi_sup","Bar",&pressureSensorSup,false,false);
//GenericInputPanel spdInf("venturi_inf","Bar",&pressureSensorInf,false,false);

FakeOutput alrmInf;
FakeOutput alrmSup;
Set alarmOn ( "Alarm_Sup","aS",&alrmSup);

Digital_Alarm alarm32("a32",0, &alarma); // I can try alarm on IO0 is on the header already.


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
  

    logger.addInput(&bomba_c1);
    //Serial.println(spdInf.pressure->id);
    page.addElement(&lblTime);
    //page.addElement(&vfdSup);
 
    //page.addElement(&vfdInf);
    page.addString("<p>"
                   "<a href=/settings>Preferencias</a>"
                   "</p>"
                    "<h3>Tanque Inferior</h3><div>");
    page.addElement(&alarm32);
    page.addString("<div class='card'>");
   // page.addElement(&flowMeterPanel);
   // page.addElement(&flowMeterPanel1);
    page.addElement(&bomba_c1);
    page.addElement(&alarmOn);
    page.addString("</div>");
  

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

    if (millis() - lastCheck > 10000)
    {
                   if (alarmOn.value) {
                        if (digitalRead(0) && (millis()-lastAlarmInf>alarmInterval || lastAlarmInf==0)) {
                            alarma.alarm(deviceID+"_ALARMA%20DE%20PRESION%20"+alarmOn.getName() + "%20Nivel:%20"+String (alarmOn.value));
                            lastAlarmInf = millis(); }
              }
      }
}
