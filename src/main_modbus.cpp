
#include "../common-libs/header.h"

#define DEVICE_NAME "relayBoardModbus"   //  Aqui es importante define el nombre para los updates es el mismo para los dispositivos del mismo tipo
#define SOFT_VERSION "1.25"        //   Changed file system to LittleFS   CHECAR LINEA 311
String mdnsName = DEVICE_NAME;     // "basementController.local" no hace falta saber el IP
const char *OTAName = DEVICE_NAME; // A name and a password for the OTA service

Page page("Shrimpbox Main Controller", "User interface");
Updater updater(DEVICE_NAME, SOFT_VERSION);
TimeLabel lblTime("lblTime", "");
LabelFreeHeap lblFreeHeap("lblHeap", "this");
Logger logger("Logger", "/dataLog.csv", 60, &wifiClient);
Label lblVersion("lblVersion", "Device: " + String(DEVICE_NAME) + " - Ver: " + String(SOFT_VERSION));
DirCapture dirCapture("dCapt", "/capturas");
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
ModbusVFD spdInfCtrl( 3 , VFD_Types::MOLLOM_B20) ;
Set vfdSup ("Venturi sup","vfdSup",&spdSupCtrl);
Set vfdInf ("Venturi inf","vfdInf",&spdInfCtrl);
//Modbus_device pressureSensorSup (2,17); //17=A0 en esp8266 Aqui tengo que usar los numeros xQ estoy en ambiente esp32
//  y se va a referir a numeros del esp8266 a travez del modbus
AnalogIn pressureSensorInf (36);
AnalogIn pressureSensorSup (39);

Set switchBlowerSup("Blower", "bls", &RelayBlowerSup);
Set switchBlowerInf("Blower", "bls", &RelayBlowerInf);

Set skimmer_sup("Skimmer Sup", "skmSup", &SkmSup);
Set skimmer_inf("Skimmer Inf", "skmInf", &SkmInf);

Feeder feederSup("Alimentador Sup", "fdrSup", &RelayFeederAirSup, &RelayFeederFeedSup, &switchBlowerSup, &logger);
Feeder feederInf("Alimentador Inf", "fdrInf", &RelayFeederAirInf, &RelayFeederFeedInf, &switchBlowerInf, &logger);

GenericTimer residuosSup("Residuos Sup", "rs", &relayResiduosSup);
GenericTimer residuosInf("Residuos Inf", "ri", &relayResiduosInf);

GenericTimer lucesSup("Luces Sup", "ls", &relayLucesSup);
GenericTimer lucesInf("Luces Inf", "li", &relayLucesInf);

Clarificador clarificadorSup("Clarificador Sup", "cs", &bombaClarSup, &evClarSup, &logger);
Clarificador clarificadorInf("Clarificador Inf", "ci", &bombaClarInf, &evClarInf, &logger);

Speed_Control spdSup("Speed Ctrl Sup", "spd_sup",&spdSupCtrl,&pressureSensorSup, &logger);
Speed_Control spdInf("Speed Ctrl Inf", "spd_inf", &spdInfCtrl,&pressureSensorInf, &logger);

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
    nodeRelays.begin(SerialInterface);
    savedVariable::init();
    
    spdInf.init();
    spdSup.init();
    logger.addInput(&clarificadorInf);
    page.addElement(&lblTime);
    //page.addElement(&vfdSup);
    //page.addElement(&vfdInf);
    page.addString("<p>"
                   "<a href=/settings>Preferencias</a>"
                   "</p>"
                    "<div><div class='divMain'><h3>Tanque Superior</h3><br>");
    //page.addElement(&modLedSet);
    page.addString("<div class=''>");
    page.addElement(&spdSup);
    page.addElement(&residuosSup);
    page.addElement(&clarificadorSup);
    page.addString("</div><div class=''>");
    page.addElement(&feederSup);
    page.addElement(&skimmer_sup);
    page.addString("</div></div><div class='divMain'><h3>Tanque Inferior</h3><br>");
    page.addElement(&spdInf);
    page.addString("<div class=' card'>");
    page.addElement(&residuosInf);
    page.addElement(&clarificadorInf);
    page.addString("</div><div class='card'>");
    page.addElement(&feederInf);
    page.addElement(&skimmer_inf);
    page.addString("</div></div></div><div>");
    page.addElement(&logger);
    page.addElement(&dirCapture);
    page.addString("</div>");

    // page.addElement(&graphic);
    page.addString("<br>");
    page.addElement(&lblFreeHeap);
    page.addElement(&lblVersion);
    // page.addElement(&btnWifi);

    page.setDebug(true); //

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
int x=0;
void loop()
{
    generalLoop();

    if (millis() - lastCheck > 1000)
    {
        
    }
}
