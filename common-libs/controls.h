#ifdef ESP8266
//#include "~/.platformio/packages/framework-arduinoespressif8266/libraries/LittleFS/src/LittleFS.h"
//#include <LittleFS>
#endif

struct tcp_pcb;
extern struct tcp_pcb *tcp_tw_pcbs;
extern "C" void tcp_abort(struct tcp_pcb *pcb);

void tcpCleanup()
{
  while (tcp_tw_pcbs != NULL)
  {
    tcp_abort(tcp_tw_pcbs);
  }
}

// HTM Decorators

class Wrapper : public ElementsHtml
{
public:
  ElementsHtml *element;
  Wrapper(String wrapBefore, String wrapAfter, ElementsHtml *e)
  {
    addAfter = wrapAfter;
    addBefore = wrapBefore;
    element = e;
  }
  void update() { element->update(); }
  String postCallBack(ElementsHtml *e, String postValue) { return element->postCallBack(e, postValue); }; // es virtual, lo tienen que implementar los hijos       ATENCION CUANDO DICE VTABLE ES QUE HE DEJADO UNA FUNCION SIN DEFINIR
  String getHtml() { return addBefore + element->getHtml() + addAfter; };

private:
  String addBefore;
  String addAfter;
};

class Prompt : public ElementsHtml
{
public:
  Prompt(String t, ElementsHtml *e)
  {
    id = e->getId();
    parent = e;
    text = t;
    //pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  void getPrompt()
  {
    String s = "handlePrompt('";
    s += id;
    s += "','";
    s += text;
    s += "');";
    javaQueue.add(s);
  };
  String getHtml() { return ""; };
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    else
      return "";
  }
  //String s="handlePrompt('";s+=e->getId();s+="','";
  //  s+=text;s+="');"; return s;}
private:
  String text;
};

class preferencesDuplet
{
public:
  String str;
  float *var;
};

class MapFile
{
public:
  // MapFile(){};
  static void saveMap(String receivedParameter, String receivedValue, String file)
  {

    // Tengo que crear un archivo temporal hasta que funcione el truncate del ESP32. !!!!!
    File mapFile; File tempFlie;
    #ifdef ESP8266
    LittleFS.exists(file.c_str()) ? mapFile = LittleFS.open(file, "r+") : mapFile = LittleFS.open(file, "r+");
    #endif
    #ifdef ESP32
    FILE_SYS.exists(file.c_str()) ? 
      mapFile = FILE_SYS.open(file, "r+") : 
      mapFile = FILE_SYS.open(file, "r+");
    #endif
    //Serial.println(file + String(mapFile));
    String outputString;
    //StaticJsonDocument<200> doc;
    //deserializeJson (doc,mapFile);
    if (mapFile)
    {
      boolean processed = false;
      //Serial.println("saveMap-- found file "+String(mapFile.name()));
      while (mapFile.available())
      {
        String line = mapFile.readStringUntil('\n');
        if (!line || line==String('\n')) break;
        String parameter = line.substring(0, line.indexOf(","));
        String value = line.substring(line.indexOf(",") + 1, line.length());
        if (value == "")
          value = " ";
         //Serial.print(parameter);Serial.println(value);
        if ((parameter != "" && parameter != ",") && (parameter == receivedParameter))
        {
          // mapFile.seek( - (line.length() ,SeekCur);

          outputString += receivedParameter + "," + receivedValue + "\n";
          //Serial.println(outputString);
          processed = true;
        }
        else
        {
          if (parameter != "" && parameter!=" " && parameter!="," && parameter.length()>1)
          {
            outputString += parameter + "," + value + "\n";
          }
        }
        
      }
      if (!processed)
      {
        //Serial.println("Output String: " + String(outputString.length()));
        if (receivedParameter != "")
          outputString += receivedParameter + "," + receivedValue + "\n";
      }
    }
    mapFile.seek(0, SeekSet);
    //mapFile.close();
    //SPIFFS.remove(file.c_str());
    mapFile = FILE_SYS.open(file, "r+");
    //if (debug) Serial.println("new Settings: \n"+settings);
    outputString+="\n";
    mapFile.write((uint8_t *)outputString.c_str(), outputString.length());
    #ifdef ESP8266
    mapFile.truncate(outputString.length()); //  ESTO ES NUEVO PARA NO DEJAR COLAS EN EL ARCHIVO
    #endif
    #ifdef ESP32
    //mapFile.truncate(outputString.length()); //  Para hacer truncate debo updatear pero hay problemas de dependencias
    // truncate viene en la version 4.00
    //mapFile.seek(outputString.length()+1);
    //mapFile.write(EOF);
    #endif
    mapFile.close();
  }

  static String lookUpMap(String receivedParameter, String fileName)
  {
    File mapFile;
     #ifdef ESP8266
    mapFile = LittleFS.open(fileName, "r+");
    #endif
    #ifdef ESP32
    mapFile = FILE_SYS.open(fileName, "r+");
    #endif
    String reply = "";
      //Serial.println("Starting LookUp " + receivedParameter);
    if (mapFile)
    {

      while (mapFile.available())
      {
        String line = mapFile.readStringUntil('\n');
        //Serial.println("Read Line: " + line+ "looking for: " + receivedParameter);
        String parameter = line.substring(0, line.indexOf(","));
        String value = line.substring(line.indexOf(",") + 1, line.length());
        //Serial.println ("Sub LookUpMap, file: "+fileName+ " found: " + String(parameter) + ":" + String(value));

        if (parameter == receivedParameter)
        {
          reply = value;
          //Serial.println ("Sub LookUpMap. found parameter: " + String(parameter) + ":" + String(value));
        }
      }
    }

    mapFile.close();
    return reply;
  }
  static void saveLog(String text)
  {
    File logFile = FILE_SYS.open("/log.txt", "a+");
    //logFile.write((uint8_t *)text.c_str(), text.length());
    logFile.println(text);
    logFile.close();
    Serial.println("Logged:");
    Serial.println(text);
  }
};

class savedVariable 
{
public:
  savedVariable(String _name) { name = _name;add(this); text=_name;}

  static std::vector<savedVariable *> list; // esta lista estatica la he creado para hacerles init() a todas las 
  static void init(){ for (auto *s :list) {s->update();}} //  variables que he creado.
  static void add(savedVariable *var){
    list.push_back(var);
  }
  void setFile(String _file) { file = _file; }

  void update(float v)
  {
    value = v;
    text = String(v);
    MapFile::saveMap(name, String(value), file);
  }

  void update()
  {
    text = MapFile::lookUpMap(name, file);
    if (text == "")
    {
      MapFile::saveMap(name, "", file);
    }
    //Serial.println("Trying to save in "+name+text+file);}
    //else
      //Serial.println("Found " + name + "<->" + text);
    value = text.toFloat();
  }
  void update(String newText)
  {
    text = newText;
    value = text.toFloat();
    //Serial.println("Updating " + name + " to " + text);
    MapFile::saveMap(name, text, file);
  }
  String toString()
  {
    return name + " : " + text;
  }
  
  //private:
  String name;
  static String file;
  float value = 0;
  String text;
};
String savedVariable::file = "/status.sta";
std::vector<savedVariable *> savedVariable::list;

class PreferencesBinder
{
  virtual String getString() { return ""; }
  virtual float getFloat() { return 0; }
  virtual bool getBool() { return false; }
};

class savedFloat
{
public:
  savedFloat() {}
  void addDuplet(String s, float *f)
  {
    pd.str = s;
    pd.var = f;
    list.push_back(pd);
  }
  void loadPreferences()
  {
    for (preferencesDuplet i : list)
    {
      String s = MapFile::lookUpMap(i.str, "/settings.set");
      if (s)
      {
        *i.var = s.toFloat();
      }
    }
  }

private:
  std::vector<preferencesDuplet> list;
  preferencesDuplet pd;
};

class OxygenComputation
{ // computacion para saber el nivel de saturacion en el agua a un cierta temp y presion y salinidad
public:
  static double compute_do(double wt = 25, double bp = 760, double sc = 20)
  { // wt: water temp (c)  bp: barometric pressure sc=salinity (gr/litro)

    String bpUnit = "mmHg";
    String scUnit = "por mil";

    // compute Kelvin temperature
    double tk = wt + 273.15;

    // compute oxygen solubility in freshwater at 1 atm and input temperature
    double sat = exp(-139.34411 + (1.575701E5 + (-6.642308E7 + (1.2438E10 - 8.621949E11 / tk) / tk) / tk) / tk);

    // convert bp to atmospheres
    if (bpUnit == "mmHg")
    {
      bp /= 760.;
    }
    else if (bpUnit == "inHg")
    {
      bp /= 29.9213;
    }
    else if (bpUnit == "mbar")
    {
      bp /= 1013.25;
    }
    else if (bpUnit == "kPa")
    {
      bp /= 101.325;
    }

    // check for input problems
    if (bp < 0.5 || bp > 1.1)
    {
      Serial.println("ERROR--\n\n"
                     "The barometric pressure input must be in the range of\n"
                     "0.5-1.1 atmospheres or 380-836 mm Hg or 14.97-32.91 in Hg or\n"
                     "507-1114 millibars or 51-112 kiloPascals.\n\n"
                     "Please try again.");
      return false;
    }

    // compute pressure correction
    if (bp != 1.)
    {
      double u = exp(11.8571 + (-3840.70 - 216961 / tk) / tk);
      double theta = 0.000975 - 1.426E-5 * wt + 6.436E-8 * wt * wt;
      sat *= (bp - u) * (1 - theta * bp) / ((1 - u) * (1 - theta));
    }

    // convert sc to salinity
    double sal = sc;
    if (scUnit == "sc")
    {
      sal = 5.572E-4 * sc + 2.02E-9 * sc * sc;
    }

    // check for input problems
    if (sal < 0 || sal > 40)
    {
      Serial.println("ERROR--\n\n"
                     "The salinity or specific conductance input must be in the range of\n"
                     "0-40 permil (o/oo) salinity or 0-59118 uS/cm specific conductance.\n\n"
                     "Please try again.");
      return false;
    }

    // compute salinity correction
    if (sc > 0)
    {
      sat *= exp(-1 * sal * (0.017674 + (-10.754 + 2140.7 / tk) / tk));
    }

    // report result
    //Serial.print(sat);
    //sat      = round(sat*100)/100;
    return sat;
  }
};


// POR AHORA LE HE SACADO EL TELEGRAM ALARM PORQUE EL CLIENTE SEGURO OCUPABA MUCHO ESPACIO EN EL ESP32
// ESTABA FUNCIONANDO PERO YA NO PODIA HACER UPDATES PORQUE OCUPABA MAS DEL 50% DE MEMORIA
// PERO SI FUNCIONA BIEN LA CONECCION HTTPS PERO AHORA LA ALARMA LA ENVIA EL RASPBERRY PI
// HE CREADO UN MODULO DE ALARMA MAS SIMPLE QUE SOLO SE CONECTA AL PI, LA DIRECCION ES LA MISMA QUE mqttServer.


// #ifdef ESP8266
// class TelegramAlarm
// {
// public:
//   TelegramAlarm() {  }
//    void alarm(String a)
//   {
//     //httpClient = new HTTPClient();
//     httpClient.addHeader("Content-Type", "text/plain");

//     String address = ("http://"+mqttServer+"/Update/alarm.php?msg='" + a + "'");
//     if (a != "")
//     {
//       httpClient.begin(wifiClient,address);
//       int err = httpClient.GET(); //POST((wifiClient,"192.168.1.115/Update/alarm.php?msg='"+a+"'").c_str());//("192.168.1.115/Update/alarm.php?msg="+a).c_str());
//       Serial.println("Trying to reach messenger Address: "+address+" error: "+httpClient.errorToString(err));
//     }
//   }

// private:
//   HTTPClient httpClient;
//   WiFiClient wifiClient;
// };
// //HTTPClient *TelegramAlarm::httpClient;
// #endif
// #ifdef ESP32
// const char* root_ca= \

// "-----BEGIN CERTIFICATE-----\n"
// "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"
// "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"
// "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"
// "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n"
// "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"
// "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n"
// "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n"
// "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n"
// "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n"
// "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n"
// "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n"
// "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n"
// "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n"
// "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n"
// "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n"
// "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n"
// "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n"
// "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n"
// "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n"
// "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n"
// "4uJEvlz36hz1\n"
// "-----END CERTIFICATE-----\n";
 //#include <UniversalTelegramBot.h>
// #include <WiFiClientSecure.h>
// class TelegramAlarm
// {
// public:
//   static void alarm(String a)
//   {
//     httpClient = new HTTPClient();
//     WiFiClientSecure wifiSecure;
//       wifiSecure.setCACert(root_ca);
//     //httpClient->addHeader("Content-Type", "application/x-www-form-urlencoded");
//     httpClient->addHeader("Content-Type", "app/json");
//     String address = ("https://api.telegram.org/bot1950585553:AAFCxpKbaHP8yk0A-HJR0eYwJkHAh60t8dM/sendMessage");
//     if (a != "")
//     {
//       int err=httpClient->begin(address,root_ca);
//         UniversalTelegramBot bot("1950585553:AAFCxpKbaHP8yk0A-HJR0eYwJkHAh60t8dM", wifiSecure);

//       Serial.println("HttpClient begun, result: "+String(err));
//       bot.sendMessage("1461925075",a);
//       //err = httpClient->POST("{'chat_id':1461925075,'text':'"+a+"'}"); //POST((wifiClient,"192.168.1.115/Update/alarm.php?msg='"+a+"'").c_str());//("192.168.1.115/Update/alarm.php?msg="+a).c_str());
//       //Serial.println("Trying to reach telegram, result: "+String(err));
//     }
//   }

// private:
//   static HTTPClient *httpClient;
//   //static WiFiClient *wifiClient;
// };
// HTTPClient *TelegramAlarm::httpClient;
// //WiFiClient *TelegramAlarm::wifiClient;
// #endif



// class RelayTest : public ElementsHtml {
//   public:
//     RelayTest () {
//       relayNumber = new EditBox (id+"rlNum","","number",this);
//       state = new EditBox (id+"state","","checkbox",this);
//     }
//     String getHtml(){
//       return relayNumber->getHtml()+state->getHtml();
//     }
//     String postCallBack(ElementsHtml *e, String postValue){
//       if (e==relayNumber) { pinMode ( e->value , OUTPUT ); }
      
//       return "";
//   }
//     void update(){
//       digitalWrite ( relayNumber->value , state->value );
//     }
//   private:
//     EditBox *relayNumber;
//     EditBox *state;
// };
