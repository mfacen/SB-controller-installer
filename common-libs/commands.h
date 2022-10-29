#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#endif
//#include <LittleFS.h>
//#include <WiFiClientSecureBearSSL.h>

// ########################################
//  Commands
// ########################################

class Commands : public ElementsHtml
{
public:
  bool executing = false;
  virtual bool run() { return false; };
  // String getHtml(){};
};

// ########################################
//  Program es un Command Composite
// ########################################

class Program : public Commands
{
public:
  // String name;
  Program(String n)
  {
    name = n;
    id = n;
    label = new Label(name + "lbl", "0");
  } // MALDITA SEA NO HACER ESTO !!!!!!!!!!!     Serial.println("Created Program "+name);}
  Commands *listOfCommands[30];
  int commandCount = 0;
  int runIndex = 0;
  Label *label;
  void addCommand(Commands *com)
  {
    listOfCommands[commandCount] = com;
    commandCount++;
  }
  void deleteCommand(int index)
  {
    for (int i = index; i < 29; i++)
      listOfCommands[i] = listOfCommands[i + 1];
    commandCount--;
  }
  bool run()
  {
    if (listOfCommands[runIndex]->run())
      runIndex++; //////  Atencion checar por null pointer si no hay ningun commando !!!!
    if (runIndex >= commandCount)
    {
      runIndex = 0;
      return true;
    }
    String s = docIdStr;
    s = s + label->id;
    s = s + "').innerHTML='";
    s = s + listOfCommands[runIndex]->name + "';";
    javaQueue.addJsonInner(label->id, listOfCommands[runIndex]->name);
    return false;
  }

  String getHtml()
  {
    String html;
    html += "<div class='card' id='";
    html += id;
    html += "'><h4>";
    html += name;
    html += "</h4>Executing = ";
    html += label->getHtml();
    html += "<br>";
    for (int i = 0; i < commandCount; i++)
    {
      html += listOfCommands[i]->getHtml() + "\n";
    }
    html += "</div>";
    return html;
  }
  void update() { run(); } //{    for ( int i=0; i<commandCount; i++) {  listOfCommands[i]->update(); } }
};

// class CompositeProgram: public Program {
//   public:
//     void addCommand(Commands* e) { childs.push_back(e);}
//     String getHtml() {for (int i = 0; i<childs.size(); i++) { }  }  // iterate through the childs
//   private:
//     std::vector <Commands*> childs;
// };



// #######################################
//  Command   SET
// ########################################

class Set : public Commands
{
public:
  Set(String n, String _id, HardwareOutput *out)
  {
    output = out;
    out->value = 0;
    name = n;
    id = _id;
    // label = new Label ( id+"lbl" ,"",this );
    // btnON = new Button("btnON" + id, "ON", this);
    // btnOFF = new Button("btnOFF" + id, "OFF", this);
    checkBox = new SavedEdit("on/off",id + "edt", "/status.sta", "checkbox", this);
  }
  bool run()
  {
    // output->update(comboBox1->value);
    return true;
  }
  String getHtml()
  {
    String s = "<div class='card'>";
    s += "<h4>"+name+"</h4>";
    // s += label->getHtml();
    // s += "</em>";
    // s += btnON->getHtml();
    s += checkBox->getHtml();
    // s += "<br>";
    // s += btnOFF->getHtml();
    // s += "</div>";
     s += "</div>";
    return s;
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    // if (e == btnON)
    //   {output->update(0);checkBox->update(0);}
    // if (e == btnOFF) {;checkBox->update(1);
    //   output->update(1);}
    // Serial.println(e->id);
    if (e == checkBox)
      output->update(checkBox->value);
      value = checkBox->value;
    return "";
  }
  void update(float f)
  {
    output->update(f);
    checkBox->update(f);
    value = f;
  }
  void update() { if (firstRun){
    checkBox->update(); firstRun=false;value = checkBox->value;}
  output->update();
  //Serial.println(id+"SET:"+String(value));
   }
  void update (String s){value=s.toFloat();update(value);}
private:
  HardwareOutput *output;
  // ComboBox* comboBox1;
  // EditBox* editOutput;
  SavedEdit *checkBox;
  // Button *btnON;
  // Button *btnOFF;
  // Label *label;
  int value;
};

// #######################################
//  Command   InputPanel
// ########################################
class InputPanel : public Commands
{
public:
  Input *input;
  int value;
  InputPanel(String n, Input *inp)
  {
    input = inp;
    name = n;
    id = n;
  }
  bool run()
  {
    input->update();
    return true;
  }
  String getHtml() { return "<div class='card' " + style + " id='" + name + "'><h6>" + name + "</h6>" + input->getHtml() + "</div>"; }
  String postCallBack(ElementsHtml *e, String postValue) { return ""; };
  void update() { input->update(); }
  // void update ( String s ) { input->update(s);}
};

// #######################################
//  Command   InputPanelText
// ########################################
class InputPanelText : public Commands
{
public:
  InputPanelText(String n, String s, String editText)
  {
    name = n;
    id = n;
    label = new Label("lbl" + name, s);
    edit = new EditBox("edt" + name, editText, "number", this);
  }
  bool run()
  {
    edit->update();
    return true;
  }
  String getHtml() { return "<div class='" + name + "' " + style + " id='" + name + "'><h6>" + name + "</h6>" + label->getHtml() + edit->getHtml() + "</div>"; }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    Serial.print("Post Call Back de " + this->name + " value: " + postValue);
    return "";
  };
  void update() { edit->update(); }
  void update(String s) { label->update(s); }
  float getValue()
  {
    return edit->value;
    // Serial.println("InputPanelText.getValue()");
  }
  void readOnly() { edit->setDisabled(true); }

private:
  EditBox *edit;
  Label *label;
};

// #######################################
//  Command   PAUSE
// ########################################             // Pause No tenia PostCallBack !!!!!!!!!  Era ese el problema ????
class Pause : public Commands
{
public:
  int value;
  Pause(String s, int i)
  {
    name = s;
    interval = i;
    value = i;
    editTime = new EditBox(name + "edtBox", String(interval), "number", this);
    //      addChild(editTime);
  }

  void setInterval(int i)
  {
    if (i != 0)
      interval = i;
    value = i;
  }
  int getInterval()
  {
    return interval;
  }

  String getHtml()
  {
    String s = "<div class='";
    s += name;
    s += "' ";
    s += style;
    s += " id='";
    s += name;
    s += "'><h4>";
    s += name;
    s += "</h4><br>Time: ";
    s += editTime->getHtml();
    s += "</div>";
    return s;
  }
  bool run()
  {
    if (firstRun)
    {
      firstRun = false;
      start();
    }
    if ((lastTimeCheck - millis()) > 1000)
    {
      value = (millis() - lastUpdate) / 1000;
      lastTimeCheck = millis();
      // Serial.println(value);
      if (value <= 0)
        value = interval;
    }
    if ((millis() - lastUpdate) > (interval - 1) * 1000)
    {
      firstRun = true;
      lastUpdate = millis();
      return true;
    }
    else
      return false;
  };
  String postCallBack(ElementsHtml *e, String postValue) { return ""; }
  void resetCounter(){lastTimeCheck = millis();}
  void start()
  {
    lastUpdate = millis();
    value = interval;
  }
  void update() {} // javaQueue.add(docIdStr + editTime->id + "').setAttribute('innerHTML', '"+String(value)+"');");    }

private:
  bool firstRun = true;
  unsigned long lastUpdate;
  unsigned int interval;
  unsigned long currentMillis;
  unsigned long lastTimeCheck;
  EditBox *editTime;
};

// ########################################
//  Command Logger
// ########################################
class Logger : public Commands
{
public:
  Logger(String _id, String _fileName, int i, WiFiClient *wfCl)
  {
    id = _id;
    name = "Logger";
    fileName = _fileName;
    wifiClient = wfCl;
    meter = new Meter("mtr" + id, "Tmr", "", 0, 100, this);
    pause = new Pause("tmr" + id, 20);
    pauseUpload = new Pause("upl" + id, 600);
    edtInterval = new Label("lb" + id, "", this);
    chkUpload = new SavedEdit( "Upload","eU" + id, "/status.sta", "checkbox", this);
    chkTimer = new SavedEdit("Timer","eT" + id, "/status.sta",  "checkbox", this);
    chkDeviation = new SavedEdit("Deviation","eD" + id,  "/status.sta", "checkbox", this);
    edtPercentage = new SavedEdit("Percent","eP" + id, "/status.sta","number", this);
    interval = i;
    btnSnap = new Button("sn" + id, "Snap", this);
    btnSave = new Button("sa" + id, "Save", this);
    //btnView = new Button("vi" + id, "View", this);
    //btnDelete = new Button("de" + id, "Delete", this);
    btnUpload = new Button("up" + id, "Upload", this);
    // chart = new Chart ("ch"+id,this);
  }
 String getHtml()
  {
    String str = "<div class='card' ";
    str += style;
    str += " id='";
    str += name;
    str += "'><h4>";
    str += name;
    str += "</h4><br><div>";
    str += chkTimer->getHtml();
    str += " Interval: ";
    str += edtInterval->getHtml();
    str += meter->getHtml();
    str += "</div><br><div>";
    str += chkDeviation->getHtml();
    str += " Threshold: ";
    str += edtPercentage->getHtml();
    str += "</div><br><div>";
    str += chkUpload->getHtml();
    str += "</div><br><div>";
    str += btnSnap->getHtml();
    str += btnSave->getHtml();
   // str += btnView->getHtml();
   // str += btnDelete->getHtml();
    str += btnUpload->getHtml();
    //         str += "<div><br>Inputs:";      Uncomment this to show inputs.
    //         if (index!=0) {
    //           str+="<ul>";
    //          for ( int i=0; i<index; i++ ) { str+="<li>";str += inputArray[i]->name ; str+= "</li>"; }
    //          }
    //          //         if (indexO!=0) {
    //          //     return "";
    //  //for ( int i=0; i<indexO; i++ ) { str += outputArray[i]->getName(); ; str+=String ( sizeof( outputArray[i])) ; str+= "<br>"; }
    //          // }
    //                  if (indexF!=0) {
    //          for ( int i=0; i<indexF; i++ ) { str +="<li>" + names[i] ; str+=  "</li>"; }
    //          }
    //                str+="</ul></div>";

    str += "</div> ";
    // str+=chart->getHtml();
    str += "</div>";

    return str;
  }
  void addInput(ElementsHtml *i)
  {
    if (index <= 29)
    {
      inputArray[index] = i;
      //Serial.println("Added: "+inputArray[index]->name);
      index++;
    }
    else {Serial.println("ERROR LIMIT OF LOGER ELEMENTS REACHED !!!!!!!!!!");}
  }
  // void addOutput ( Output *o ) { if (indexO<9) {outputArray[indexO]=o;indexO++;}}
  // void setCombo(int index) { comboBox->update(index); }
  void setFileName(String n)
  {
    fileName = n;
    fileName = n;
  }
  void setFileNameData(String n) { fileName = n; }
  void addSavedVariable(savedVariable *f)
  {
    if (indexF < 9)
    {
      names[indexF] = name;
      variableArray[indexF] = f;
      indexF++;
    }
  }
  bool run()
  {
    update();
    return true;
  }

  void update()
  {
    if (firstRun){
      chkTimer->update(); chkUpload->update();chkDeviation->update();edtPercentage->update();
    if (index == 0)
      Serial.println("No input, Logger is Disabled");
      firstRun = false;
    }
    if (chkTimer->getValue() == 1)
    { //  MODE TIMER ON
      if ((pause->run() && (index > 0)))
      {
        for (int i = 0; i < index; i++)
        {
          //inputArray[i]->update();
        }
        // pause->setInterval(interval);
        logData();
      }
      meter->update(pause->value);
    }
    // Serial.println(chkDeviation->getValue());
    if (chkDeviation->getValue() == 1)
    { // MODE DIFFERENCIAL ON
      for (int i = 0; i < index; i++)
      {
        bool yaProcesado = false;
        //consoleLog(inputArray[i]->id, "inputArrayLastSave = " + String(inputArrayLastSave[i]) + " InputArray = " +
        //                                  String(inputArray[i]->getValue()));
        if (((edtPercentage->getValue()) &&
            (abs(inputArray[i]->getValue() - inputArrayLastSave[i]) >= edtPercentage->getValue()))
              && (yaProcesado == false ) )
          {logData(); pause->resetCounter();  // Si guarda por Diferencial entonces resetea el counter del TIMER
            inputArrayLastSave[i] = inputArray[i]->value; 
            yaProcesado = true;
          }
      }
    }
    if ((chkUpload->value) && (pauseUpload->run()) || !itWorked) //      AQUI TENGO QUE HACER QUE SI NO FUNCIONO LO REPITA TODAVIA NO ESTA PROBADO
    {
      if ((millis() - lastUploadTry) > 10000)
      {
        Serial.println("Upload to server result: " + String(itWorked = uploadToServer()));
        lastUploadTry = millis();
      }
    }
  }

  void setInterval(int _interval)
  {
    interval = _interval;
    pause->setInterval(interval);
    edtInterval->update(String(interval));
    meter->setLimits(0, interval);
  }
  void setUploadInterval(int i)
  {
    uploadInterval = i;
    pauseUpload->setInterval(uploadInterval);
  }

  int getInterval() { return interval; }

//##################################################################
//##################        L O G       D A T A   ##################
//##################################################################
  bool logData()
  {
    bool r = false;
    if ((now() > 1000000000)) // aqui decia si combo value =! 0 se lo saque para poder tomar snaps
    {
      bool first = true;
#ifdef ESP8266
      if (LittleFS.exists("/capturas/" + fileName))
#endif
#ifdef ESP32
        if (SPIFFS.exists("/capturas/" + fileName))
#endif
        {
          first = false;
        }
      File tempLog;
#ifdef ESP8266
      tempLog = LittleFS.open("/capturas/" + fileName, "a+"); // Write the time and the temperature to the csv file
#endif
#ifdef ESP32
      tempLog = SPIFFS.open("/capturas/" + fileName, "a+"); // Write the time and the temperature to the csv file
#endif
      if (first)
      {
        tempLog.print("Time,");
        for (int i = 0; i < index; i++)
        {
          tempLog.print(inputArray[i]->name);
          Serial.println("Logging column "+inputArray[i]->name);
          if ((i != index - 1) || (indexF != 0))
            tempLog.print(",");
        }
        for (int i = 0; i < indexF; i++)
        {
          tempLog.print(variableArray[i]->name);
          if (i != indexF - 1)
          {
            tempLog.print(",");
          }
        }
        tempLog.println("");
      }
      tempLog.print(String(now()) + ",");
      if (index > 0)
      {
        for (int i = 0; i < index; i++)
        {
          tempLog.print(String(inputArray[i]->value));
          // chart->update(inputArray[i]->value);
          if ((i != index - 1) || (indexF != 0))
            tempLog.print(",");
          // Serial.print(inputArray[i]->name);
          // Serial.print(":::");
          // Serial.println(inputArray[i]->getMqtt());
        }
      }

      if (indexF > 0)
      {
        for (int i = 0; i < indexF; i++)
        {
          float temp = variableArray[i]->value;
          tempLog.print(String(temp));
          if (indexF - 1 != i)
            tempLog.print(",");
        }
      }
      tempLog.println("");
      tempLog.close();
      //#ifndef debug
      Serial.println("Data Logged on file " + fileName);
    }
    return r;
  }
 
  //              Atencion si falta una de los metodos static se produce un Error que no se detecta y no hay HTML

  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e == edtInterval)
      interval = edtInterval->value;
    pause->setInterval(interval);
    if (e == btnSnap)
      logData();


    if (e == btnSave)
    {
#ifdef ESP8266
      LittleFS.rename("/capturas/" + fileName, "/capturas/Capture-" + String(now()).substring(6) + ".csv"); //+String(day())+"-"+String(month())+"-"+String(hour())+"-"+String(minute())+"-"+String(second())  );
#endif
#ifdef ESP32
      SPIFFS.rename("/capturas/" + fileName, "/capturas/Capture-" + String(now()).substring(6) + ".csv");
#endif
    };
//     if (e == btnView)
//     {
//       javaQueue.addJsonInner(id, "redirect=graphic.html?file=/capturas/" + fileName);
//     }
//     if (e == btnDelete)
//     {
// #ifdef ESP8266
//       if (LittleFS.exists("/" + fileName))
//       {
//         LittleFS.remove("/" + fileName);
// #endif
// #ifdef ESP32
//         if (SPIFFS.exists("/" + fileName))
//         {
//           SPIFFS.remove("/" + fileName);
// #endif
//         }
//       }
      if (e == btnUpload)
      {
        itWorked = uploadToServer();
      }

      return "";
    }
    void setTimer(bool state) { chkTimer->update(state); }
    void setDeviation(bool state) { chkDeviation->update(state); }
    static int partial;
    //###################################
    // UPLOAD TO SERVER
    //###################################
    bool uploadToServer()
    {
      // partial = 0;
      String payload;
      bool success = true;
      String a = MapFile::lookUpMap("serverName", "/settings.set");
      // String a="http://192.168.0.109/capturas/fileUpload.php?fileName=" + fileName;
      // String a = "http://michel.atarraya.dev/capturas/fileUpload.php?fileName=" + fileName;
      if (a == "")
        a = "michel.atarraya.ai";
      a = "http://" + a + "/capturas/fileUpload.php?fileName=" + fileName;
      a += "&ip=" + WiFi.localIP().toString();

// a="http://httpbin.org/post?filename=test.csv";  // this server is for testing POST requests
// a="http://michel.atarraya.dev/capturas/test.php";
//#ifdef ESP8266
      if (FILE_SYS.exists("/capturas/" + fileName))
      {
        success = false;
        File f = FILE_SYS.open("/capturas/" + fileName, "a+");
//#endif
//#ifdef ESP32
//        if (SPIFFS.exists("/capturas/" + fileName))
//        {
//          File f = SPIFFS.open("/capturas/" + fileName, "a+");
//#endif

          Serial.println(a);
          if (client.begin(*wifiClient, a)) // ,443,"/",sha1))  Atencion he leido que no se puede usar begin sin WiFiClient
          {

            Serial.println("Posting sensor data.");
            // client.print("POST ");
            // client.print("/fileUpload.php");
            // client.addHeader(" HTTP/1.1");
            // client.addHeader("Host","192.168.0.130");
            client.addHeader("User-Agent", fileName);
            client.addHeader("Connection", "keep-alive");
            client.addHeader("Content-Type", "application/x-www-form-urlencoded");
            // client.addHeader("Content-Length", "100");
            String data;

            f.seek(0);

            String header = f.readStringUntil('\n');
            data += header; //+"\n";
             Serial.println("header: -"+header+"-");
            f.seek(partial + header.length());
            while (f.available())
            {
              String line = (f.readStringUntil('\n'));
              data.concat(line + "\n");

                //Serial.println(line);
              if (data.length() > 500) // ATENCION CON ESTO LO HE CAMBIADO PORQUE NO FUNCIONA BIEN
              {
                partial += data.length() - header.length();
                Serial.println("Partial: "+String(partial));
                break;
              }
            }
            if (f.position() >= f.size())
            {
              partial = 0;
            }
            // Serial.println(data);
            f.close();

            if (data.length() > 0)
            {
              Serial.println("PostResult:" + String((int)client.POST(data)));
            }
            // Serial.println(' = Result');
            payload = client.getString(); // Get the response payload

            Serial.println("Data to send to server: " + data);
            Serial.println("Payload: " + payload);
            // Serial.println("- done");
            // client.begin(*wifiClient,"http://api.telegram.org/bot1950585553:AAFCxpKbaHP8yk0A-HJR0eYwJkHAh60t8dM/sendMessage?chat_id=1461925075&text="+
            //             id+" : " + name+" logged data.");
            //            client.GET();

            // disconnect when done
            Serial.println("Disconnecting from server...");
            // Serial.println("- bye!");
          }
          client.end(); // ULTIMA MODIFICACION estaba antes de la corchea antes
          if (partial != 0)
            Serial.println("Uploaded partial data, partial = " + String(partial));
          if ((payload.substring(0, 8) == "SUCCESS.") && (partial == 0))
          {
            FILE_SYS.remove("/capturas/" + fileName);
            success = true;
          }

        }
        return success;
      }

      
      bool uploadLiveData()
      {
        String a = MapFile::lookUpMap("serverName", "/settings.set");
        Serial.println("Found serverName" + a);
        // String a="http://192.168.0.109/capturas/fileUpload.php?fileName=" + fileName;
        // String a = "http://michel.atarraya.dev/capturas/fileUpload.php?fileName=" + fileName;
        if (a == "")
          a = "michel.atarraya.dev";
        a = "http://" + a + "/capturas/uploadLive";

        Serial.println(a);
        if (client.begin(*wifiClient, a)) // ,443,"/",sha1))  Atencion he leido que no se puede usar begin sin WiFiClient
        {

          Serial.println("Uploading live data.");
          // client.print("POST ");
          // client.print("/fileUpload.php");
          // client.addHeader(" HTTP/1.1");
          // client.addHeader("Host","192.168.0.130");
          // client.addHeader("User-Agent","TempLogger");
          // client.addHeader("Connection","keep-alive");
          client.addHeader("Content-Type", "application/x-www-form-urlencoded");
          String msg;
          if (index > 0)
          {
            for (int i = 0; i < index; i++)
            {
              msg += ((inputArray[i]->name));
              msg += "=";
              msg += (String(inputArray[i]->value));
              if ((i != index - 1) || (indexF != 0))
                msg += ("&");
            }
            Serial.println("PostResult:" + String((int)client.POST(msg)));
          }
        }
      }

      void loadDataFromFile()
      {
        firstRun = false;
        String s = MapFile::lookUpMap(id + "upl", "/status.sta");
        if (s != "")
          chkUpload->update(s);

        s = MapFile::lookUpMap(id + "chkTmr", "/status.sta");
        if (s == "1")
          chkTimer->update(1);
        else
          chkTimer->update(0);
        s = MapFile::lookUpMap(id + "chkDev", "/status.sta");
        if (s == "1")
          chkDeviation->update(1);
        else
          chkDeviation->update(0);

        s = MapFile::lookUpMap(id + "percentage", "/status.sta");
        if (s != "")
        {
          Serial.println("Status percentage=" + s);
          edtPercentage->update(s.toInt());
        }
        s = MapFile::lookUpMap("name", "/settings.set");
        if (s != "")
          fileName = s + ".csv";
        setInterval(interval);
        // Serial.println("Logger load from file: "+s);
      }
      void sendMqtt()
      {
        if (index > 0)
        {
          for (int i = 0; i < index; i++)
          {
            if (inputArray[i]->getMqtt())
            {
               Serial.println("Sending MQTT");
              inputArray[i]->sendMqtt();
            }
          }
        }
      }
    void printInputs(){
      for (int i = 0; i < index; i++)
        {
          Serial.print(inputArray[i]->getName());Serial.print(":");
          Serial.println(inputArray[i]->value);          
        }
        for (int i = 0; i < indexF; i++)
        {
          Serial.print(variableArray[i]->name); Serial.print(":");
          Serial.println(variableArray[i]->value);
      }
    }
    private:
      HTTPClient client;
      WiFiClient *wifiClient;
      String fileNameUpload = "defaultName.csv";
      loggableInterface *inputArray[30];
      // Output* outputArray[10];
      savedVariable *variableArray[10];
      float inputArrayLastSave[10];
      String names[10];
      int uploadInterval = 600;
      bool itWorked = true;
      // Label *label;
      Meter *meter;
      String f[2] = {"OFF", "ON"};
      String ff[2] = {"Timer", "Deviation"};
      Label *edtInterval;
      Button *btnSnap;
      //Button *btnDelete;
      Button *btnUpload;
      String fileName;
      SavedEdit *chkUpload;
      SavedEdit *chkTimer;
      SavedEdit *chkDeviation;
      SavedEdit *edtPercentage;
      int index = 0;
      int indexO = 0;
      int indexF = 0;
      Pause *pause;
      Pause *pauseUpload;
      int interval = 60;
      Button *btnSave;
      //Button *btnView;
      unsigned long lastUploadTry = 0;
      String sha1 = "BE:22:E9:99:11:41:F0:32:FC:DF:18:72:BC:8B:94:FA:86:24:25:B6";
      // Chart *chart;
    };
    int Logger::partial = 0;

    // ########################################
    //  Command IF
    // ########################################
    class IfCommand : public Commands
    {
    public:
      IfCommand(String s, Input *inLL, Input *inRR)
      {
        name = s;
        id = s;
        inR = inRR;
        inL = inLL;
      } //   ERROR ERROR ERROR ERROR ERROR  Serial.print("Created IfCommand "+name); };

      String getHtml()
      {
        String html = "<div  class='card'><h4>" + name + "</h4>" + inL->getHtml() + "  > " + inR->getHtml() + "<br>";
        // for ( int i=0; i<command_count; i++) {
        //   html+=commands[i]->getHtml();
        // }
        return html;
      }
      bool run() { return false; }
      String postCallBack(ElementsHtml *e, String postValue) { return ""; };
      void update(){};

    private:
      Input *inL;
      Input *inR;
    };

    // ########################################
    //  Command IF UNITARIO = Active Control
    // ########################################
    class ActiveControl : public Commands
    {
    public:
      // String name;
      Input *inputLeft;
      Input *inputRight;
      HardwareOutput *output;
      Input *inputEdit;
      ComboBox *op;
      String signs[3] = {"=", ">", "<"};

      ActiveControl(String n, Input *inL, String _op, Input *inR, HardwareOutput *out, Input *inpEdit)
          : inputLeft{inL}, inputRight{inR}, output{out}, inputEdit{inpEdit}
      {
        op = new ComboBox(id + "op", 3, signs, this);
        name = n;
        id = name;
        output = out;
        inputEdit->parent = this;
      };

      String getHtml()
      {
        String s = "<div class=' card' ";
        s += "id='";
        s += id;
        s += "'><h4>";
        s += name;
        s += "</h4> If ";
        s += inputLeft->getHtml();
        s += "  ";
        s += op->getHtml();
        s += "  ";
        s += inputRight->getHtml();
        s += "<br><center>  Then </center> ";
        s += "Output"; //(output->name);
        s += " =  ";
        s += inputEdit->getHtml();
        s += "</div>";
        return s;
      }
      void update()
      {
        inputEdit->update();
        inputLeft->update();
        inputRight->update();

        if (op->text == ">")
        {
          if (inputLeft->value > inputRight->value)
          {
            output->update(inputEdit->value);
          }
          else
            output->update(!inputEdit->value);
        }
        if (op->text == "=")
        {
          if (inputLeft->value == inputRight->value)
          {
            output->update(inputEdit->value);
          }
          else
            output->update(!inputEdit->value);
        }
        if (op->text == "<")
        {
          if (inputLeft->value < inputRight->value)
          {
            output->update(inputEdit->value);
          }
          else
            output->update(!inputEdit->value);
        }
      }
      bool run()
      {
        update();
        return true;
      }
      String postCallBack(ElementsHtml *e, String postValue) { return ""; }
    };

    class ActivePause : public ActiveControl
    {
    public:
      int value = 0;
      //     ActivePause (String n,Input* inL,String _op, Input* inR) {
      //     //ActivePause (String n) {
      //      inputLeft=inL;inputRight=inR;
      //       op = _op;
      //       name = n;
      //          id=name;
      //
      //    html="<div id='"+id+"'><h4>"+name+"</h4> If "+ inputLeft->getHtml() +"  "+ op + "  " + inputRight->getHtml()+ " Then Continue </div>";
      //    inputEdit->parent=this;
      //
      //    };
      void update()
      {
        value = 0;
        if (op->text = ">")
        {
          if (inputLeft->value > inputRight->value)
          {
            value = 1;
          }
        }
        if (op->text = "=")
        {
          if (inputLeft->value == inputRight->value)
          {
            value = 1;
          }
        }
      }
      bool run()
      {
        update();
        if (value == 0)
          return false;
        else
          return true;
      }
      String postCallBack(ElementsHtml *e, String postValue) { return ""; };
    };

    // ########################################
    //  Conditional Program
    // ########################################
    class ConditionalProgram : public Commands
    {
    public:
      ConditionalProgram(String _name, String _id, ElementsHtml *_inp, Program *_out)
      {
        name = _name;
        id = _id;
        inp = _inp;
        out = _out;
      }
      bool run()
      {
        if (!inp->value)
          out->run();
        return true;
      }
      void update() { inp->update(); }
      String getHtml()
      {
        String s = "<h4>" + name + "</h4>" + inp->getHtml() + out->getHtml();
        return s;
      }
      String postCallBack(ElementsHtml *e, String postValue) { return ""; }

    private:
      ElementsHtml *inp;
      Program *out;
    };

    // ########################################
    //  Command Keypad Control
    // ########################################
    // class savedVariable;
    class KeypadControl : public Commands
    {
    public:
      // KeyPad* keypad;
      KeypadControl(String n)
      {
        name = n;
        id = n;
        edit = new EditBox(name + "edt", "1234", "password", this);
        label = new Label(name + "lbl", "Locked", this);
        state = new savedVariable("keyPadState"); // Why Im saving it ? I think cause it shuts off when no connection
      }
      String getHtml()
      {
        label->update(state->value ? "Unlocked" : "Locked");
        String s = "<div><h4>";
        s += name;
        s += "</h4><center>";
        s += edit->getHtml();
        s += "</center><br>";
        s += label->getHtml();
        s += "</div>";
        return s;
      }
      bool run()
      {
        if (state->text == "")
        {
          state->setFile("/status.sta");
          state->update();
        }
        return state->value;
      }
      String postCallBack(ElementsHtml *e, String postValue)
      {
        if (edit->text == "1234")
        {
          state->update(true);
          label->update("Unlocked");
        }
        else
        {
          state->update(false);
          label->update("Locked");
        };
        return "";
      };
      void update() {}

    private:
      EditBox *edit;
      Label *label;
      String temp = "logStatus";
      savedVariable *state;
    };

    // ########################################
    //  Command TimeAlarms
    // ########################################
    class TimeAlarms : public Commands
    {
    public:
      // KeyPad* keypad;
      TimeAlarms(String n, Commands *_program)
      {
        name = n;
        id = n;
        program = _program;
        editHour = new EditBox(name + "edtH", "", "number", this);
        editMinute = new EditBox(name + "edtM", "", "number", this);
        label = new Label(name + "lbl", "Locked", this);
      }
      void setAlarm(int _hour, int _minute, int _second)
      {
        hours = _hour;
        minutes = _minute;
        seconds = _second;
        // Alarm.alarmRepeat(hour,minute,second, alarmTrigered);
      }
      String getHtml()
      {
        String s = "<div><h4>";
        s += name;
        s += "</h4><center>";
        s += editHour->getHtml();
        s += ":";
        s += editMinute->getHtml();
        s += "</center><br>";
        s += label->getHtml();
        s += program->getHtml();
        s += "</div>";
        return s;
      }
      bool run()
      {
        if ((minutes == minute()) && (hours == hour()))
          state = true;
        if (state)
        {
          bool aaa = (program->run());
          state = aaa;
          return aaa;
        }
        else
          return false;
      }
      // static void alarmTrigered(){state=true;}
      // bool run(){return true;}
      String postCallBack(ElementsHtml *e, String postValue)
      {
        if (e == editHour)
          hours = editHour->value;
        if (e == editMinute)
          minutes = editMinute->value;
        setAlarm(hours, minutes, seconds);
        return "";
      };
      void update(){};

    private:
      EditBox *editHour;
      EditBox *editMinute;
      Label *label;
      Commands *program;
      int hours = 0, minutes = 0, seconds = 0;
      bool state;
    };
    // bool TimeAlarms::state = false;
