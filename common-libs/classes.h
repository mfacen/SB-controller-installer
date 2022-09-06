


//  ########################################
//  CLASES
//  ########################################

#include <vector>
#include <map>
#include <time.h>
#include "MQTT.h"
#ifdef ESP8266
#include <ESP8266httpUpdate.h>
#endif
#ifdef ESP32
//#include <ESP32httpUpdate.h>
#include <HTTPClient.h>
#endif
// El problema es que solo estan registrados en Page los parientes de los componentes entonces no los encuentra cuando el PostRequest busca el id, no figura porque esta solo
// el ID del pariente en page.listOf Elements[]
const char docIdStr[] = "document.getElementById('";
class Subject;
//##################################################
//             OBSERVER
//##################################################
class Observer
{
public:
  virtual ~Observer() = default;
  virtual void notify(Subject &, String) = 0;
};
//##################################################
//            SUBJECT
//##################################################
class Subject
{
public:
  virtual ~Subject() = default;
  void Attach(Observer &o) { observers.push_back(&o); }
  void Detach(Observer &o)
  {
    observers.erase(std::remove(observers.begin(), observers.end(), &o));
  }
  void Notify(String m)
  {
    for (auto *o : observers)
    {
      o->notify(*this, m);
    }
  }

private:
  std::vector<Observer *> observers;
};


//##################################################
//            JAVAQUEUE
//##################################################
#define MAX_BUFFER_VALUE 1000
class JavaQueue
{
public:
  JavaQueue(){};
  String queue = "";
  String queue1;
  StaticJsonDocument<MAX_BUFFER_VALUE> obj;
  String onLoad = "";
  void add(String q)
  {
    // queue += q;
    // queue += "\n";
    // Serial.println(queue.length());
  }
  void addJsonInner(String _id, String _text)
  {
    obj[_id] = _text;
  }
  String getJson()
  {
    //  for (int i = 0; i < obj.keys().length(); i++) {
    //     obj.keys()[i] = "";//new JSONVar ("");
    //  }
    // String tempString = JSON.stringify(obj);
    String tempString;
    serializeJson(obj, tempString);
    if (tempString.length() > MAX_BUFFER_VALUE)
      Serial.println("###############  MAX JSON BUFFER WARNING !!!!   ##################");
    // obj=null;
    obj.clear();
    // Serial.println(JSON.stringify(obj));
    // Serial.println(tempString);
    return tempString;
  }
  void addOnLoad(String s)
  {
    onLoad += s;
    onLoad += "\n";
  }
  String get()
  {
    String temp = queue;
    queue = "";

    // Serial.println (temp);
    return temp;
  }
  String getOnLoad()
  {
    String s; // = "<script>";
    s += onLoad;
    // s += "</script>";
    return s;
  }
};

//##################################################
//            AVERAGE
//##################################################
class AverageModule
{
public:
  float addValue(float v)
  {
    values[index % 10] = v;
    index++;
    return getAverage();
  }
  float getAverage()
  {
    float partial = 0;
    for (int i = 0; i < (index > 9 ? 10 : index); i++)
    {
      partial += values[i];
    }
    return partial / (index > 8 ? 10 : index);
  }

private:
  float values[10] = {0};
  int index = 0;
  int maxIndex = 9;
};

class DebugModule;

class loggableInterface{
  public:
    String name;
    String id;
    // String descriptor;
    float value = 0;
    String getName() { return name; }
    String getId() { return id; }
    float getValue() { return value; }
    void setId(String i) { id = i; }
    void setName(String n) { name = n; }
      MQTTClient *mq = NULL;
    void sendMqtt() { mq->publish(name, String(value)), "pi", "pass"; }
    void setMqtt(MQTTClient *_mq) { mq = _mq; }
    bool getMqtt() { return mq ? true : false; }
};
//##################################################
//            ELEMENTS HTML
//##################################################
class ElementsHtml : public Subject, public loggableInterface
{
public:
  ~ElementsHtml(){}; // destructor...     todas las clases abstractas deben tener un destructor definido.
                     // static Page page;

  String style = "";
  bool visible = true;
  bool firstRun = true;
  String endpoint = "";
  int typeID = 0;
  // String html;
  ElementsHtml *parent = nullptr;
  DebugModule *debug = nullptr;

  void setTypeID(int type) { typeID = type; }
  void setEndpoint(String _endpoint) { endpoint = _endpoint; }

  // Wrapper* wrapper = nullptr;
  
  void setStyle(String s) { style = s; }
  void setVisible(bool v) { v ? javaQueue.add(docIdStr + this->id + "').style.display='inline';") : javaQueue.add(docIdStr + id + "').style.display='none';"); } //{visible=v;}
  void setDisabled(bool v)
  {
    v ? javaQueue.add("var a=document.getElementById('" + id + "').setAttribute('disabled','disabled');") : javaQueue.add("var a=document.getElementById('" + id + "').removeAttribute('disabled');");
  }
  void clearHtml() { javaQueue.add("var a=document.getElementById('" + id + "').innerHTML=''"); }

  virtual String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    else
      return "";
  }
  virtual String getHtml() { return ""; };
  virtual void update(){};
  virtual void update(String s){};
  virtual void update(float f){};
  virtual ElementsHtml *searchById(String id_) { return NULL; };
  static JavaQueue javaQueue;
  static String getJavaQueue() { return javaQueue.getJson(); }

  static std::vector<ElementsHtml *> allHTMLElements;
  static void pushElement(ElementsHtml *e) { allHTMLElements.push_back(e); }
  static void deleteElement(ElementsHtml *e) {} //  Todavia no he implementado esto

  
private:
};

JavaQueue ElementsHtml::javaQueue; // Los miembros STATIC necesitan esta inicializacion, si no no funcionan... ???
std::vector<ElementsHtml *> ElementsHtml::allHTMLElements;
// char ElementsHtml::html[1000];
// String ElementsHtml::s="                                                                                                                                                                                              ";

// ########################################
//  Debug Output
// ########################################
class DebugModule
{
public:
  DebugModule() {}

  void p(String text)
  { // Print Method
    Serial.println(text);
    if (webConsole)
      ElementsHtml::javaQueue.add("console.log ('" + text + "';");
  }
  void p(float f) { p(String(f)); }
  void setSerial(bool s) { serial = s; }
  void setWebConsole(bool s) { webConsole = s; }
  String text = "debug module text";

private:
  bool serial = true;
  bool webConsole = true;
};

class CompositeHtml : public ElementsHtml
{
public:
  void addChild(ElementsHtml *e) { childs.push_back(e); }
  String getHtml()
  {
    String s;
    for (unsigned int i = 0; i < childs.size(); i++)
    {
      s += childs[i]->getHtml();
    };
    return s;
  } // iterate through the childs
  void update()
  {
    String s;
    for (unsigned int i = 0; i < childs.size(); i++)
    {
      childs[i]->update();
    };
  } // iterate through the childs
private:
  std::vector<ElementsHtml *> childs;
};

void consoleLog(String id, String s) { ElementsHtml::javaQueue.addJsonInner("ConsoleLog() " + id, "log=" + s); }

#include "output.h"
#include "controls.h"
#include "input.h"
#include "commands.h"
#include "devices.h"

// ########################################
//  Page
// ########################################

class Page : public Observer
{
public:
  ElementsHtml *listOfElements[20];
  String strings[20];
  int elementCount = 0;

  Page(String stitle, String sSubTitle, DebugModule *d = nullptr)
  {
    title = stitle;
    subTitle = sSubTitle;
    debug = d;
  }
  void addElement(ElementsHtml *el)
  {
    el->debug = debugModule;
    listOfElements[elementCount] = el;
    elementCount++;
    if (elementCount == 50)
    {
      elementCount = 49;
      Serial.println("Page elementCount limit reached 50#################");
    }
  }
  void addString(String s) { strings[elementCount] += s; }
  void setDebug(bool d) { debug = d; }
  void deleteElement(int index)
  {
    for (int i = index; i < 49; i++)
      listOfElements[i] = listOfElements[i + 1];
    elementCount--;
  }
  void notify(Subject &s, String m) { alarmText = m; }
  String getAlarm()
  {
    return alarmText;
    alarmText = "";
  }
  void update()
  {
    for (int i = 0; i < elementCount; i++)
    {
      listOfElements[i]->update();
      // Serial.println(listOfElements[i]->getName());
    }
  }
  float serialCallback(String _id)
  {
    for (int i = 0; i < elementCount; i++)
    {
      if (listOfElements[i]->getId() == _id)
      {
        return listOfElements[i]->value;
      }
    }
    return -1;
  }
  ElementsHtml *searchById(String _id)
  {
    for (int i = 0; i < elementCount; i++)
    {
      // Serial.println(listOfElements[i]->id);
      if (listOfElements[i]->getId() == _id)
      {
        // Serial.println("Page::searchById()");
        return listOfElements[i];
      }
      else
      {
        ElementsHtml *temp = listOfElements[i]->searchById(_id);
        if (temp != NULL)
          return temp;
      }
    }
    return NULL;
  }
  void mqttMessage(String topic, String payload)
  { // esto esta muy raro, lo tengo que revisar
    Serial.println("MQTT msg received :" + topic + ":" + payload);

    for (int i = 0; i < elementCount; i++)
    {
      if (listOfElements[i]->getId() == topic)
      {
        listOfElements[i]->update(payload.toFloat());
        Serial.println("Updating from MQTT (REGULAR)");
      }
    }
    for (unsigned int i = 0; i < ElementsHtml::allHTMLElements.size(); i++)
    {
      // Serial.println(ElementsHtml::allHTMLElements[i]->id);
      if (ElementsHtml::allHTMLElements[i]->id == (topic))
      {
        Serial.println("Updating from MQTT (ALLELEMENTS)");
        ElementsHtml::allHTMLElements[i]->update(payload.toFloat());
      }
    }
  }
  String getJavaQueue()
  {
    String reply; //  for ( int i=0; i<elementCount; i++) {   reply+=listOfElements[i]->getJavaQueue(); }
    reply = ElementsHtml::getJavaQueue();
    return reply;
  }
  String getHtml()
  {
    String htmlStr;
#ifdef ESP8266
    if (LittleFS.exists("/index.html"))
      LittleFS.remove("/index.html");
    File htmlFile = LittleFS.open("/index.html", "w");
#endif
#ifdef ESP32
    if (SPIFFS.exists("/index.html"))
      SPIFFS.remove("/index.html");
    File htmlFile = SPIFFS.open("/index.html", "w");
#endif
    htmlFile.seek(0, SeekSet);
    // htmlStr.reserve(5000);        /////    Reserva espacio en la memoria para evitar fragmentacion.
    htmlStr += F("<!DOCTYPE html><html><head> <link rel='stylesheet' type='text/css' href='style.css'>");
    // "<script src='moment.min.js'></script>");//PUTA MADRE moment ANTES de Chart, si no no funciona he perdido horas !!!
    htmlStr += F("<meta name ='viewport' content='width=device-width, initial-scale=1.0'>"); //
    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
    htmlStr = F("<title>");
    htmlStr += title;
    htmlStr += F("</title>");
    htmlStr += F("</head><body>\n<h1>");
    htmlStr += title + F("</h1><h3>");
    htmlStr += subTitle + F("</H3>");
    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
    if (debug)
    {
      htmlStr = F("<nav><a href='edit.html'>Upload </a><a href='dataLog.csv'>dataLog</a><a href='delete?file=/dataLog.csv'>delete</a>"
                  "<a href='/status.sta'>Status File</a>"
                  "<a href='/log.txt'>Log File</a>"
                  "<a href='/delete?file=/log.txt'>Delete Log</a>"
                  "<a href='settings'>Preferencias </a><a href='list?dir=/'>Directory</a><a href='reset'>reset</a></nav>\n");
      htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
    }

    for (int i = 0; i < elementCount; i++)
    {
      htmlStr = "";
      if (strings[i])
      {
        htmlStr = strings[i];
      }
      htmlStr += listOfElements[i]->getHtml();
      htmlStr += "\n";
      // Serial.println(listOfElements[i]->name+" : HtmlStringSize: "+ String (  htmlStr.length() ) );
      htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
    }
    // if (strings[elementCount]) htmlStr+=strings[elementCount];

    htmlStr = F("<br><span id='errorLabel'></span><br>"); //\n<button type = 'button' dataValue='primus,2323' onclick='btnClick(this)' id='switchToStation'>Connectar WIFI</button><br>");
    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
#ifdef ESP8266
    FSInfo fs_info;
    LittleFS.info(fs_info);
    float fileTotalKB = (float)fs_info.totalBytes / 1024.0;
    float fileUsedKB = (float)fs_info.usedBytes / 1024.0;
#endif
#ifdef ESP32
    float fileTotalKB = 0;
    float fileUsedKB = 0;
#endif

    htmlStr = "Total KB: ";
    htmlStr += String(fileTotalKB);
    htmlStr += "Kb / Used: ";
    htmlStr += String(fileUsedKB);
    htmlStr += " Page Size: ";
    htmlStr += String(sizeof(this));
    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());

    htmlStr = getJavaScript();

    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());

    // htmlStr = ElementsHtml::javaQueue.getOnLoad();
    // if (htmlStr)
    //  htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());

    htmlStr = "</body></html>";
    htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());

    // Serial.println(htmlStr);  // Atencion no usar Serial en Constructor !!!
    //   ElementsHtml::htmlAdd(htmlStr.c_str());
    //  htmlFile.write((uint8_t *)htmlStr.c_str(), htmlStr.length());
    htmlFile.close();
    return ""; // htmlStr;
  }

  String getJavaScript()
  {
    String s = "<script src='javascript.js'></script>";
    s += "<script>window.onload=function(){";
    s += ElementsHtml::javaQueue.getOnLoad();
    s += ";var now = new Date();btnClickText('Time',now.getTime()/1000);}</script>";
    return s;
  }

private:
  String title;
  String subTitle;
  DebugModule *debugModule = nullptr;
  bool debug = false;
  String alarmText;
};

//##################################################
//         MEDIATOR                               ##
//##################################################

class Binder
{
public:
  Binder(Input *inp, Output *out) : input{inp}, output{out} {}
  float update()
  {
    output->update(input->getValue());
    return input->getValue();
  }

private:
  Input *input;
  Output *output;
};

//##################################################
//         UPDATER                               ##
//##################################################

class Updater
{
public:
  Updater(String devName, String softVer)
  {
    deviceName = devName;
    softVersion = softVer;
    client = new HTTPClient();
  }
  void setIp(String addr, String mdns)
  {
    ip = addr;
    deviceID = mdns;
  }
  void setFileName(String f) { fileName = f; }
  void setID(String id) { deviceID = id; }
  void checkUpdate()
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      String payload = "";
      String address = MapFile::lookUpMap("serverName", "/settings.set");
      // String a="http://httpbin.org/post?filename=test.csv";  // this server is for testing POST requests
      // String a="http://192.168.0.109/capturas/fileUpload.php?fileName=" + fileNameUpload;
      // String a = "http://michel.atarraya.dev/capturas/fileUpload.php?fileName=" + fileNameUpload;
      if (address == "")
        address = "michel.atarraya.ai";
      String a;
      a += "http://" + address + "/Update/checkUpdate.php?device=" + deviceName + "&version=" + softVersion;
      a += "&ip=" + WiFi.localIP().toString();
      a += "&fileName=" + fileName;
      if (deviceID == "")
        deviceID = WiFi.macAddress();
      a += "&ID=" + deviceID;
      // String a = "http://michel.atarraya.dev/Update/checkUpdate.php?device="+deviceName+"&version="+softVersion;
      Serial.println("Check Update: " + a);
      if (client->begin(wifiClient, a))
      {
        Serial.println("- connected");

        client->addHeader("Content-Type", "text/plain");

        if (client->POST(a))
        {
          payload = client->getString(); // Get the response payload
        }
        Serial.println("Payload: " + payload);
        if (payload.startsWith("http://"))
        {
          const String uStr = payload;         // "http://"+address+"/Update/"+payload;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          Serial.println("Updating: " + uStr); //  EL PROGRAMA YA ES MAS GRANDE QUE LA MITAD DEL FLASH !!!!!!!!!!
        if (FILE_SYS.exists("/capturas/"+fileName));
            FILE_SYS.remove("/capturas/"+fileName);
#ifdef ESP8266
          t_httpUpdate_return ret = ESPhttpUpdate.update(wifiClient, uStr); // ret = ESPhttpUpdate.update( uStr ); //AQUI HE APAGADO HABIA PROBLEMAS de MEMORIA !!!!
#endif
#ifdef ESP32
          t_httpUpdate_return ret = ESPhttpUpdate.update(uStr); // ret = ESPhttpUpdate.update( uStr ); //AQUI HE APAGADO HABIA PROBLEMAS de MEMORIA !!!!
#endif
          switch (ret)
          { //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;

          case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

          case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
          }
        }
        if (payload == "sendStatus")
        {
          File f=FILE_SYS.open("/status.sta","r");
          client->begin(wifiClient,address+"/Update/"+deviceID+"/status.sta");
        } // HERE I CAN DO STUFF TO RESET SETTINGS
          // OR EVEN ACTIONS TO PARTICULAR IC'S
          // if payload=="analogin3432ed" etc
        // Serial.println("- done");
        // client->end();
      }
      // disconnect when done
      // TelegramAlarm alarm;
      // alarm.alarm(deviceName);
      Serial.println("Disconnecting from server...");
      // Serial.println("- bye!");
    }
  }
  void update()
  {
    if ((now() - lastUpdateCheck) > (updateInterval * 3600))
    {
      checkUpdate();
      lastUpdateCheck = now();
    }
    if (!lastUpdateCheck)
      checkUpdate(); // if first time
  }

private:
  HTTPClient *client;
  WiFiClient wifiClient;
  String deviceName;
  String softVersion;
  unsigned long lastUpdateCheck = 0;
  int updateInterval = 4; // en horas 0 mejor en minutos ?
  String ip;
  String deviceID;
  String fileName;
};

//##################################################
//         ALARM                              ##
//##################################################

class Alarm
{
public:
  Alarm()
  {
    client = new HTTPClient();
  }
  void setIp(String addr)
  {
    ip = addr;
  }
  void alarm(String msg)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("Trying to send Alarm !!!");
      String payload = "";
      String address = MapFile::lookUpMap("mqttServer", "/settings.set");
      String a;
      ip = mqttServer;
      a += "http://" + address + "/Update/alarm.php?msg=\"" + msg +"\"";
      Serial.println("Alarm: " + a);
      if (client->begin(wifiClient, a))
      {
        Serial.println("- connected");

        //client->addHeader("Content-Type", "text/plain");
        client->addHeader("Content-Type", "application/x-www-form-urlencoded");

        if (client->POST(a))
        {
          payload = client->getString(); // Get the response payload
        }
        Serial.println("Payload: " + payload);
          
        
      }
      Serial.println("Disconnecting from server...");
    }
  }
  

private:
  HTTPClient *client;
  WiFiClient wifiClient;
  unsigned long lastUpdateCheck = 0;
  String ip;
};


class RS485Main
{
public:
  RS485Main(Page *_page)
  {
    page = _page;
  }
  //#######################################
  //   PROCESS SERIAL DATA WHEN /n RECEIVED
  //#######################################
  void loop()
  {
    while (SerialInterface.available() > 0)        // Esto he agregado para procesar entrada Serial
      processIncomingByte(SerialInterface.read()); // Tambien lo puedo usar con Software Serial
  }
  void process_data(const char *data)
  {
    //Serial.print("\033[32mReceived from SerialInterface: ");
    //Serial.println(data);
    const String dataStr(data); // convert data into String
    if (strcmp(data, "reset") == 0)
      ESP.restart();
    int index = dataStr.indexOf("id.req="); // es el mensage que pregunta a un Element su value
    if (index != -1)
    {
      // Serial.println(index);
      String _id = dataStr.substring(index + 7, dataStr.indexOf(",", index));
      // Serial.println(_id);
      // if (_id==String('?') ) Serial.println(lastIdMsg);

      lastIdMsg = _id;
      float f = page->serialCallback(_id);
      if (f != -1)
      {
        digitalWrite(SerialCtrlPin, HIGH);
        SerialInterface.println("rp=" + String(f) + ","); // le agrega "rp=" a la respuesta.
        SerialInterface.flush();
        digitalWrite(SerialCtrlPin, LOW);
        lastIdMsg = "";
      }
    }

    index = dataStr.indexOf("id.set="); // es el mensage que pregunta a un Element su value
    if (index != -1)
    {
      String _id = dataStr.substring(index + 7, dataStr.indexOf(",", index));
      ElementsHtml *temp = page->searchById(_id);
      if (temp != NULL)
      {
        float _value = dataStr.substring(index + 7 + _id.length()).toFloat();
        temp->update(_value);
      }
    }

    index = dataStr.indexOf("rp=");
    if (index > -1)
    {
      // Serial.println(index);
      String _value = dataStr.substring(index + 3, dataStr.indexOf(",", index));
      ElementsHtml *temp = page->searchById(lastIdMsg);
      //Serial.println(lastIdMsg);
      if (temp != NULL)
      {
        //Serial.println("Trying with " + temp->id + " " + _value);
        temp->update(_value);
      }
    }

    //Serial.print("\033[39m");
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

    case '\n':                   // end of text
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

  void requestData(String _id)
  {
    digitalWrite(SerialCtrlPin, HIGH);
    SerialInterface.println("id.req=" + _id + ",");
    SerialInterface.flush();
    digitalWrite(SerialCtrlPin, LOW);
    lastIdMsg = _id;
    #ifdef ESP32
    //Serial.println("Sending Rs485 Request "+_id);
    #endif
  };

private:
  Page *page;
  String lastIdMsg;
};
