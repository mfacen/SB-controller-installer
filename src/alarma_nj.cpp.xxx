
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



Digital_Alarm alarm32("a32",32, &alarma); // I can try alarm on IO0 is on the header already.
Digital_Alarm alarm33("a33",33, &alarma);
Digital_Alarm alarm25("a25",25, &alarma);
Digital_Alarm alarm26("a26",26, &alarma);

#include "../common-libs/footer.h"

///////////////////////////////////////////////////////////////////////////
////                                                               ////////
///////              SETUP                                          ////////
////                                                               ////////
///////////////////////////////////////////////////////////////////////////

void setup()
{
    startUpWifi();
    SerialInterface.begin(9600, SERIAL_8N1, 16, 17); // ??????


    //Serial.println(spdInf.pressure->id);
    page.addElement(&lblTime);
    //page.addElement(&vfdSup);
 
    //page.addElement(&vfdInf);
    page.addString("<p>"
                   "<a href=/settings>Preferencias</a>"
                   "</p>"
                    "<h3>Alarmas:</h3><div class='divMain'>");
    page.addElement(&alarm32);
    page.addElement(&alarm33);
    page.addElement(&alarm25);
    page.addElement(&alarm26);
    page.addString("</div>");
    page.addElement(&logger);
    page.addElement(&dirCapture);
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
void loop()
{
    generalLoop();
}
