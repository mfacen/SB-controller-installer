
#include "../common-libs/header.h"

#define DEVICE_NAME "controller_nj"   //  Aqui es importante define el nombre para los updates es el mismo para los dispositivos del mismo tipo
#define SOFT_VERSION "2.41"        //   Changed file system to LittleFS   CHECAR LINEA 311
String mdnsName = DEVICE_NAME;     // "basementController.local" no hace falta saber el IP
const char *OTAName = DEVICE_NAME; // A name and a password for the OTA service

Page page("Shrimpbox Main Controller", "User interface");
Updater updater(DEVICE_NAME, SOFT_VERSION);
//Alarm alarma;
//TelegramAlarm alarma;
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

GenericTimer  EV_Clarificador_izquierdo ("Clarificador_izquierdo","cli", &R2);
GenericTimer  EV_Clarificador_derecho ("Clarificador_derecho","cld", &R4);
GenericTimer  EV_Skimmers ("Skimmers","skm", &R3);
Set bomba ("Bomba","bmb",&R1);
//Dsb18b20a tempProbe(35);
//GenericInputPanel tempPanel("tempI","C",&tempProbe);
FakeOutput alrmInf;
FakeOutput alrmSup;
Set alarmOn ( "Alarm_Sup","aS",&alrmSup);

#include "../common-libs/footer.h"

// Button btnWifi("switchToStation","WiFi");
// Graphic graphic("tempGraph");
///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////              SETUP                                          ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////
void timerEvent(){
    logger.logData();
}
void setup()
{
    startUpWifi();
    EV_Clarificador_derecho.insertCallback(timerEvent);
    EV_Clarificador_izquierdo.insertCallback(timerEvent);
    EV_Skimmers.insertCallback(timerEvent);
    logger.addInput(&EV_Clarificador_izquierdo);
    logger.addInput(&EV_Clarificador_derecho);
    logger.addInput(&EV_Skimmers);
    page.addElement(&lblTime);
    page.addString("<p>"
                   "<a href=/settings>Settings</a>"
                   "</p>"
                    "<h3>Main Panel</h3><div>");
    page.addString("<div class='cardxx'>");
    page.addElement(&EV_Clarificador_izquierdo);
    page.addElement(&EV_Clarificador_derecho);
    page.addElement(&EV_Skimmers);
    page.addElement(&bomba);
 //   page.addElement(&tempPanel);
    page.addString("</div>");
    page.addElement(&logger);
    page.addElement(&dirCapture);
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
    if (millis() - lastCheck > 100)
    {

        if ( EV_Clarificador_derecho.value == 1 || EV_Clarificador_izquierdo.value == 1 || EV_Skimmers.value == 1)
        {
            bomba.update(1);
            //digitalWrite(2, HIGH);
        }
        else
        {
            bomba.update(0);
            //digitalWrite(2, LOW);
        }
        
    lastCheck=millis();
    }
}
