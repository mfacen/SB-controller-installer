// ########################################
//  INPUT
// ########################################
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <EmonLib.h>
#endif

//#include <LittleFS.h>
//#include <Adafruit_ADS1015.h>
#include <OneWire.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
//#include <Q2HX711.h>

#include "DHTesp.h"

#include <Adafruit_ADS1X15.h>
//#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
// class ElementsHtml;
#include <../common-libs/hardwareInput.h>


class Input : public ElementsHtml, Subject
{

public:
  float factor = 1;
  String unit;
  String text = "";
  Output *output;

  // String getHtml() {};
  void setFactor(float _factor) { factor = _factor; }
  void setOutput(Output *out)
  {
    output = out;
    value = output->getValue();
  }
};

// ########################################
//  MODBUS DEVICE 
// ########################################
class Modbus_wrapper: public Input {
  public:
  uint8_t readInputValue(uint8_t fc, uint16_t address, uint16_t length) {
    // read analog input
    for (int i = 0; i < length; i++) {
        slave->writeRegisterToBuffer(i, input->value);
    }

    return STATUS_OK;
}
    Modbus_wrapper (int _slaveNumber,Input *_input) {
      //node.begin(SerialInterface);
      slaveNumber=_slaveNumber;
      //pinNumber = _pinNumber;
    //ModbusRTUServer.configureCoils(0x00,1);
    input=_input;
    slave = new Modbus ( 4 , 12);//12 es D6 en esp6266
    slave->begin(9600);
    //slave->cbVector[CB_READ_REGISTERS] = readInputValue;
    }
    void update(){
        Serial.println(String(millis()));
          //value= nodeRelays.readInputRegisters(slaveNumber,pinNumber,1);
          slave->writeRegisterToBuffer(0,input->value);
        Serial.println(String(millis()) + "read value:" +String(value));
        input->update();
    }
  private:
  Input *input;
    Modbus *slave;
  int slaveNumber,pinNumber;
};
// ########################################
//  BUTTON HTML
// ########################################

class Button : public Input
{
public:
  Button(String n, String t, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    parent = e;
    text = t;          // ERROR ERROR ERROR ERROR NO HACER ESTO Serial.println (t);
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    Serial.println("Button-Post-Call-Back" + id +" postValue="+postValue);

    if (parent && postValue)
      parent->postCallBack(this, postValue);

    // return ("console.log('postCallBack of " + name+" parent: "+parent->name+"'); ");
    return "";
  }
  String getHtml()
  {
    String s = "<button ";
    s += style;
    s += " type='button' width='40' id='";
    s += id;
    s += "' value ='";
    s += text;
    s += "' onclick=\"btnClickText('";
    s += id;
    s += "',this.value)\" >";
    s += text;
    s += "</button>\n";
    return s;
  }
  // String getHtml() { String s= "<button id='";s+=id;s+="'></button> ";javaQueue.addOnLoad("btn('"+id+"','"+text+"');\n"); return s;}

  // void addHtml() {
  //     htmlAdd("<button "); htmlAdd(style.c_str()); htmlAdd(" type='button' width='40' id='"); htmlAdd( id.c_str()); htmlAdd ("' value ='"); htmlAdd(text.c_str());
  //     htmlAdd("' onclick=\"btnClickText('");
  //     htmlAdd(id.c_str()); htmlAdd("',this.value)\" >");htmlAdd(text.c_str()); htmlAdd ("</button>\n");
  // }
  void update(String sss){ text = sss; update();}
  void update()
  {
   if (lastText!=text){
    String s = "var a='";
    s += docIdStr;
    s += id;
    s += "'); a.value='";
    s += text;
    s += "'; a.textContent='";
    s += text;
    s += "';";
    javaQueue.addJsonInner(id,"btnText="+text);
    lastText=text;
  }
  }
  private:
    String lastText;
  };


class ButtonPrompt : public Input
{

public:
  ButtonPrompt(String n, String t, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    parent = e;
    text = t;          // ERROR ERROR ERROR ERROR NO HACER ESTO /Serial.println (t);
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    Serial.println("Button-Post-Call-Back" + id + " : " + postValue);

    if (parent && postValue)
      //Serial.println(parent->name);
      parent->postCallBack(this, postValue);

    // return ("console.log('postCallBack of " + name+" parent: "+parent->name+"'); ");
    return "";
  }
  String getHtml()
  {
    String s = "<button ";
    s += style;
    s += " type='button' width='40' id='";
    s += id;
    s += "' value ='";
    s += text;
    s += "' onclick=\"handlePromptButton('";
    s += id;
    s += "',this.value)\" >";
    s += text;
    s += "</button>\n";
    return s;
  }

  void update(String sss)
  {
    text = sss;
    String s = "var a='";
    s += docIdStr;
    s += id;
    s += "'); a.value='";
    s += sss;
    s += "'; a.textContent='";
    s += sss;
    s += "';";
    javaQueue.add(s);
  };
};

// ########################################
//  EDIT BOX HTML
// ########################################

class EditBox : public Input
{
public:
  EditBox(String _id, String t, String _type, ElementsHtml *e = 0)
  {
    name = _id;
    text = t;
    id = _id;
    type = _type;
    value = text.toFloat();

   if (e!=0)
    parent = e; // ERROR ERROR ERROR ERROR NO HACER ESTO /Serial.println (t);
    //else
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!

  }

  String getHtml()
  {
    String s;
    if (type == "checkbox")
      {s = " <label class='switch' float='right'>"
            "<input type='checkbox' id='" ;
            s+=id + "' ";
            s+=(value)?"checked":"";
             s+=" onchange=\"btnClickText('" + id + "',(this.checked))\"> " +
            "<span class='toggle'></span></label> ";}//+ name;}
       //     { s= "<input type='checkbox' id='"+id+"' onchange=\"btnClickText('"+id+"',(this.value))\">";}

    else
    {
      s = s + F(" <input ");
      s += style;
      s += F(" type='");
      s += type;
      s += F("' id='");
      s += id;
      s += F("' value='");
      s += text;
      s += F("' onchange=\"btnClickText('");
      s += id;
      // if (type == "checkbox") {
      //   s += F("',(this.checked))\" "); // Esto es javascript this, en este caso sería el objeto que hizo la llamada a btnClickText
      //   s += F(" checked=");
      //   s += value?"true":"false";
      //   s += ">";
      // //if  ( type=="checkbox" )  s += " <span class='toggle'></span></label></div>";

      // }
      //  else
      s = s + F("',(this.value))\">");
      // if (type != "number")
      // s = s + text;

      s = s + F("");// </input>
    }
    return (s);
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if(type=="checkbox"){
      if (postValue=="true") postValue = "1";
      if (postValue=="false") postValue = "0";
    }
    text = postValue;
    value=postValue.toFloat(); // Aqui en vez de llamar update(postValue) para evitar doble llamada.

    if (parent)
      {parent->postCallBack(this, postValue);
            //Serial.println(id+":"+postValue);
            }
    return "";
  }

  void disable()
  {
    javaQueue.add(docIdStr + id + "').setAttribute('disabled','disabled');");
  }
  void appendText(String t)
  {
    text += t;
    update();
  }
  void setText(String t) { text = t; }
  void update(String t)
  {
    //Serial.println("EditBox->Update(String)" + id +","+t);
    text = t;
    value=t.toFloat();
    update();
  }
  void deleteChar()
  {
    text = text.substring(0, text.length() - 1);
    update();
  }
  void update(float f) { update(String(f)); }
  void update()
  {
    if ( (text != lastValue))
    {
      value = text.toFloat();
      if (text == "true")
        value = 1;
      lastValue = text;
      //Serial.println(value);
      
      if (type == "checkbox")
      {
        javaQueue.addJsonInner(id, ((value) ? "chkBxON" : "chkBxOFF") );
      }
      else
      {
        javaQueue.addJsonInner(id,"value="+text);
      }
    }
  }

private:
  String type = "";
  String lastValue = "*&^";
};

// ########################################
//  Saved Edit
// ########################################
class SavedEdit : public ElementsHtml
{
public:
  SavedEdit(String n, String _id, String _file = "/status.sta",String _type="text",ElementsHtml* _parent=NULL)
  {
    name = n;
    id = _id;
    edit = new EditBox(id + "edt", "", _type, this);
    file = _file;
    parent = _parent;
    nw = new char [id.length() + 1];
    id.toCharArray(nw, sizeof(id));

  }
  String getHtml()
  {
    edit->setStyle(style );
    edit->update(preferences.getString(nw));
    String str;
    str +=name +" "+ edit->getHtml();
    return str;
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e == edit)
    {
      if (postValue=="true") postValue = "1";
      if (postValue=="false") postValue ="0";
      value = postValue.toFloat();
      preferences.putString(nw,postValue);

    }
      if (parent)
        parent->postCallBack(this, postValue);

    return "";
  }
  void update()
  {
    if (firstRun)
    {
      text=preferences.getString(nw);
        value = text.toFloat();

      Serial.print("First Run Update Edit "+name+" = ");
      Serial.println(text+" "+String(preferences.freeEntries()));
      firstRun = false;
    }
    edit->update(text);
  }
  String getText (){return String(value);}
  void update(String s)
  {
    preferences.putString(nw,s);
    edit->update(s);
    value = s.toFloat();
    text=s;
  }
  void update(float f)
  {
    edit->update(f);
    value = f;
    preferences.putString(nw,String(f));
    text=String(f);
  }

  // static std::vector<SavedEdit *> list; // esta lista estatica la he creado para hacerles init() a todas las
  // static void updateAll()
  // {
  //   for (auto *s : list)
  //   {
  //     s->update();
  //   }
  // } //  variables que he creado.
  // static void add(SavedEdit *var)
  // {
  //   list.push_back(var);
  // }

private:
  char *nw;
  EditBox *edit;
  String file;
  String text;
};
//std::vector<SavedEdit *> SavedEdit::list;

// ########################################
//  Combo Box HTML
// ########################################
class ComboBox : public Input
{
public:
  String *fields;
  ComboBox(String n, int s, String _fields[], ElementsHtml *e = 0)
  {
    name = n;
    text = "";
    id = n;
    fields = _fields;
    fieldsCount = s;
    value = 0;
    savedVariable = new SavedVariable (id);
    //SavedVariable::add(savedVariable); // STATIC adds this variable to the list so it can init() correctly

    if (e != 0)
      parent = e; // ERROR ERROR ERROR ERROR NO HACER ESTO /Serial.println (t);
    //else
      pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !! 
                        // AQUI LO HE CAMBIADO SOLO REGISTRA SI NO TIENE PARENT !
  }

  String getHtml()
  {
    String s = "<select ";
    s += style;
    s += " id='";
    s += id;
    s += "' onchange=\"btnClickText('";
    s += id;
    s += "', this.selectedIndex)\">\n";
    for (int i = 0; i < fieldsCount; i++)
    {
      s += (i == value) ? "<option selected value='" : "<option value='";
      s += fields[i];
      s += "'>";
      s += fields[i];
      s += "</option>";
    }
    s += "</select>";
    return s;
  }
  void update()
  {
        if (firstRun)
    {
      //variable->update();
      value = savedVariable->value;
      //Serial.print("First Run Update Edit "+name+" = ");
      //Serial.println(variable->value);
      firstRun = false;
    }
    if (value != lastValue)
    {
      text = fields[(int)value];
      lastValue = value;
      String s = docIdStr;
      s += id;
      s += "').selectedIndex=";
      s += String(value);
      s += ";";
      javaQueue.add(s); //+ "'; console.log('"+name+" update value="+String(value)+"');");
    }
  }
  void update(int i)
  {
    value = i;
    text = fields[(int)value];
    String s = docIdStr;
    s += id;
    s += F("').selectedIndex='");
    s += String(i);
    s += "';";
    javaQueue.add(s);
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    value = postValue.toInt();
    update();
    if (parent)
      parent->postCallBack(this, postValue);
    return ""; // docIdStr + id + "').innerHTML='" + text + "';";
  }

private:
  int fieldsCount;
  int lastValue = -1;
  SavedVariable *savedVariable;
};

class I2C_Msg
{
public:
  static String sendI2CCommand(int address, String command)
  {

    delay(600);
    Serial.println("Sendig I2C command: " + command);
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
    Serial.println("Sent msg to EZOOO: " + command + " on address "+String(address));

    delay(600); // let the EZO rest a little...
    if (command != "Sleep")
    {
      Wire.requestFrom(address, 14);
      char result = Wire.read();
      // Serial.println(result);
      //(result == 1) ? msg = "Mensaje " + String(command) + "  recibido." : msg = "Error de comunicacion.";

      while (Wire.available())
      {
        char chr = char(Wire.read());
        // if ( chr = char(1) ) chr = 44;
        if (chr == 0)
          break;
        resp = resp + String(chr); // Aqui he cambiado !!!! Antes no decia String pero me comenzo a dar error.
      }
    }

    //Serial.println("Respuesta=  " + String(resp));       // print the character
    msg = String(resp);
    return msg;
  }
  static String readProbeCalibration()
  {
    String msg = "<link rel='stylesheet' type='text/css' href='style.css'><h1>Calibration readings</h1><br>Oxygen Probe Temperature: ";
    String resp = sendI2CCommand(DO_I2C_ADDRESS, "T,?");
    Serial.println("OD T Cal: " + String(resp));
    msg += resp.substring(resp.indexOf(",") + 1, resp.length()) + "<br>Salinity Probe Temperature: ";
    resp = sendI2CCommand(EC_I2C_ADDRESS, "T,?");
    Serial.println("EC T Cal: " + String(resp));
    msg += resp.substring(resp.indexOf(",") + 1, resp.length()) + "<br>Oxygen Probe Salinity";
    delay(10);
    // TDS_temp_cal = resp.substring(resp.indexOf(",") + 1, resp.length()).toFloat();
    resp = sendI2CCommand(DO_I2C_ADDRESS, "S,?");
    Serial.println("OD S Cal: " + String(resp));
    msg += resp.substring(resp.indexOf(",") + 1, resp.length()) + "<br>PH Probe Temperature: ";
    resp = sendI2CCommand(PH_I2C_ADDRESS, "T,?");
    Serial.println("PH T Cal: " + String(resp));
    msg += resp.substring(resp.indexOf(",") + 1, resp.length()) + "<br>PH Probe Cal Points: ";
    resp = sendI2CCommand(PH_I2C_ADDRESS, "Cal,?");
    Serial.println("PH T Cal: " + String(resp));
    msg += resp.substring(resp.indexOf(",") + 1, resp.length()) + "<br>";
    // OD_salinity_cal = resp.substring(resp.indexOf(",") + 1, resp.length()).toFloat();
    return msg;
  }
};

class Dsb18B20 : public HardwareInput
{
public:
  int pin;
  unsigned long lastSuccesfulReading;
  static DallasTemperature *tempSensors;
  static OneWire *oneWire;
   static bool tempRequested;
  static unsigned long lastTemperatureRequest;
  static int intervalTemperature;
static void initSensors(int pin){
    oneWire = new OneWire(pin);                   // Set up a OneWire instance to communicate with OneWire devices
    tempSensors = new DallasTemperature(oneWire); // Create an instance of the temperature sensor class
    tempSensors->setWaitForConversion(false);     //  dont block the program while the temperature sensor is reading
    tempSensors->begin();
    Serial.println("Found "+String(tempSensors->getDeviceCount())+" dsb18b20.");

}
static float updateSensors( int device_number){
    if (!tempRequested)
    {
      tempSensors->requestTemperatures(); // Request the temperature from the sensor (it takes some time to read it)
      tempRequested = true;
      lastTemperatureRequest = millis();
      //Serial.println("temp requested");
    }
    if ((millis() - lastTemperatureRequest > intervalTemperature) && tempRequested)
    {
    //Serial.println("Found "+String(tempSensors->getDeviceCount())+" dsb18b20.");
      tempRequested = false;
       return tempSensors->getTempCByIndex(device_number); // Get the temperature from the sensor
      
    }

}
// static DeviceAddress* getAddress(int index){ 
//   DeviceAddress *d;
//   tempSensors->getAddress(d,index);
//   return d;}
  Dsb18B20(int _pin , int _device_number = 0)
  {
    pin = _pin;
   // unit = "&#8451;"; // html code for ^C
    // descriptor = "Dsb18B20";
    pinMode(pin, INPUT_PULLUP);
    device_number = _device_number;
    Dsb18B20::initSensors(pin);
    value=-1;
  }
  //~Dsb18B20(ElementsHtml::deleteElement(this));


  void update()
  {
    float reading = Dsb18B20::updateSensors(device_number);
    //Serial.printf("temp: ",reading);
     if (reading>-200) {value =reading;lastValue=value;lastSuccesfulReading = millis();}
     else if ( millis() - lastSuccesfulReading > 10000 ) value=reading;//lastValue ;
     // Serial.println("Requesting device number"+String (value));
  }
  float getTempAndBlock()
  {
    tempSensors->setWaitForConversion(true); //  block the program while the temperature sensor is reading
    tempSensors->begin();
    tempSensors->requestTemperatures();
    return value = tempSensors->getTempCByIndex(device_number);
  }
  void setDeviceNumber(int n){device_number = n;}
private:
  float lastValue=-1;
  int device_number = 0;
};
DallasTemperature *Dsb18B20::tempSensors;
OneWire *Dsb18B20::oneWire;
bool Dsb18B20::tempRequested;
unsigned long Dsb18B20::lastTemperatureRequest;
int Dsb18B20::intervalTemperature=5000;
// ########################################
//  CALIBRATION MODULE   ##################
// ########################################
class calibration_module : ElementsHtml
{
public:
  calibration_module(String _id, ElementsHtml *e)
  {
    id = _id;
    child = e;
    btnCalLow = new ButtonPrompt((child->id + ("btnLow")).c_str(), "Cal Low", this);
    btnCalHigh = new ButtonPrompt(child->id + "btnHigh", "Cal High", this);
    btnResetCal = new Button(child->id + "btnReset", "Reset Cal", this);
  }
  String getHtml() { return btnCalHigh->getHtml() + btnCalLow->getHtml()+btnResetCal->getHtml(); }

  void init(float newData)
  {
    //Serial.println("Looking for "+child->id+"refHigh");
    //Serial.println(MapFile::lookUpMap(child->id + "refHigh", "/status.sta"));
    if (MapFile::lookUpMap(child->id + "refHigh", "/status.sta") != "")
      refHigh = (MapFile::lookUpMap(child->id + "refHigh", "/status.sta")).toFloat();
    if (MapFile::lookUpMap(child->id + "refLow", "/status.sta") != "")
      refLow = (MapFile::lookUpMap(child->id + "refLow", "/status.sta")).toFloat();
    if (MapFile::lookUpMap(child->id + "rawHigh", "/status.sta") != "")
      rawHigh = (MapFile::lookUpMap(child->id + "rawHigh", "/status.sta")).toFloat();
    if (MapFile::lookUpMap(child->id + "rawLow", "/status.sta") != "")
      rawLow = (MapFile::lookUpMap(child->id + "rawLow", "/status.sta")).toFloat();
  }
  float calculate(float v)
  {
    if (firstRun)
    {
      firstRun = false;
      init(0);
    }
    value = v;
    float refRange = refHigh - refLow;
    float rawRange = rawHigh - rawLow;
    if (rawRange != 0 && refRange !=0 )
    {
    //Serial.println("Calculating Calibration "+String(rawHigh)+":"+String(refHigh)+":"+String(rawRange));
      return   ((((v - rawLow) * refRange) / rawRange) + refLow);
    }
    else
      return v;
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    float newData = postValue.toFloat();
    if (e == btnCalLow)
    {
      refLow = newData;
      rawLow = value;
      //Serial.println("refLow: "+ String(refLow)+"    rawLow: " + String(rawLow));
      MapFile::saveMap(child->id + "refLow", String(refLow), "/status.sta");
      MapFile::saveMap(child->id + "rawLow", String(rawLow), "/status.sta");
    }
    if (e == btnCalHigh)
    {
      refHigh = newData;
      rawHigh = value;
      //Serial.println("refHigh: "+ String(refHigh)+"    rawHigh: " + String(rawHigh));
      MapFile::saveMap(child->id + "refHigh", String(refHigh), "/status.sta");
      MapFile::saveMap(child->id + "rawHigh", String(rawHigh), "/status.sta");
    }
     if (e == btnResetCal)
    {
      refHigh = 0;
      rawHigh = 0;
      refLow = 0;
      rawLow = 0;
      //Serial.println("refHigh: "+ String(refHigh)+"    rawHigh: " + String(rawHigh));
      MapFile::saveMap(child->id + "refHigh", String(refHigh), "/status.sta");
      MapFile::saveMap(child->id + "rawHigh", String(rawHigh), "/status.sta");
      MapFile::saveMap(child->id + "refLow", String(refLow), "/status.sta");
      MapFile::saveMap(child->id + "rawLow", String(rawLow), "/status.sta");
    }   
    return "";
  }
  void update() {}
  void update(float f) {}

private:
  String id;
  ButtonPrompt *btnCalLow;
  ButtonPrompt *btnCalHigh;
  Button *btnResetCal;
  ElementsHtml *child;
  float refHigh = 0;
  float  refLow=0;
  float rawHigh=0;
  float rawLow=0;
};


// ########################################
//  Analog IN
// ########################################
class AnalogIn1 : public Input
{
public:
  int pin;
  AnalogIn1(int _pin, String _name, String _unit = "mV", float _factor = 1, bool _emon = false)
  {
    pin = _pin;
    name = _name;
    id = _name;
    factor = _factor;
    unit = _unit;
    label = new Label(name + "lbl", "", this);
    label->setStyle(" class='numInp';");
    btnCalLow = new ButtonPrompt(id + "btnLow", "Cal Low", this);
    btnCalHigh = new ButtonPrompt(id + "btnHigh", "Cal High", this);
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }

  String getHtml()
  {
    String s = " <div class=' card '";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += name;
    s += "</h4>";
    s += "<br>";
    s += label->getHtml();
    s += "<br>";
    s += unit;
    s += "<br>";
    s += btnCalLow->getHtml();
    s += btnCalHigh->getHtml();
    s += "</div>";
    return s;
  }
  void setFactor(float f) { factor = f; }
  void setZero(int z) { zero = z; }

  void update()
  {
    getReading();
    label->update(String(value, 1));
    //Serial.println(value);
    if (output)
      output->update(value);
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e == btnCalLow)
    {
      refLow = postValue.toFloat();
      rawLow = value;
      //Serial.println("refLow: " + String(refLow) + "    rawLow: " + String(rawLow));
      MapFile::saveMap(id + "refLow", String(refLow), "/status.sta");
      MapFile::saveMap(id + "rawLow", String(rawLow), "/status.sta");
    }
    if (e == btnCalHigh)
    {
      refHigh = postValue.toFloat();
      rawHigh = value;
      //Serial.println("refHigh: " + String(refHigh) + "    rawHigh: " + String(rawHigh));
      MapFile::saveMap(id + "refHigh", String(refHigh), "/status.sta");
      MapFile::saveMap(id + "rawHigh", String(rawHigh), "/status.sta");
    }
    update();
    return "";
  }
  void setAds(ADS1115_Component *a, int ch)
  {
    ads = a;
    ADSchannel = ch;
  }
  void getReading()
  { // value = (analogRead(pin)-zero) * factor;    // Zero es porque siempre marca un poquito el analog in
    if (firstRun)
    {
      firstRun = false;
    }
    float refRange = refHigh - refLow;
    float rawRange = rawHigh - rawLow;
    if (rawRange == 0)
      rawRange = 0.01;
    if (ads)
    {
      average.addValue(ads->readChannel(ADSchannel) * factor);
      value = ((((average.getAverage() - rawLow) * refRange) / rawRange) + refLow);
    }
    else
    //Serial.println(analogRead(pin));
    {
      average.addValue(analogRead(pin) * factor);
      value = ((((average.getAverage() - rawLow) * refRange) / rawRange) + refLow);
      // value = (analogRead(pin) * factor);
    }
    // if (value<0) value = 0;
    // Serial.println(value);
  }

private:
  Label *label;
  ButtonPrompt *btnCalLow;
  ButtonPrompt *btnCalHigh;
  ADS1115_Component *ads;
  int ADSchannel = 0;
  int zero = 0;
  float refLow = 0, refHigh = 1023, rawLow = 0, rawHigh = 1023;
  boolean firstRun = true;
  AverageModule average;
};

#ifdef ESP8266
// ########################################
//  VCC IN
// ##################################
// ADC_MODE(ADC_VCC); // CAMBIAR ESTO SI NECESITO ANALOG IN

class VccIn : public Input
{
public:
  VccIn(String n, float factor = 1)
  {
    id = n;
    name = "Bateria";
    unit = "Volts";
    // descriptor = "VIN";
    label = new Label(id + "lbl", "", this);
    label->setStyle(" class='numInp'"); //   Esto lo hago desde el CSS es una clase
    pushElement(this);                  // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String getHtml()
  {
    String s = " <div ";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += name;
    s += "</h4><br>";
    s += label->getHtml();
    s += "<br>";
    s += unit;
    s += "</div>";
    return s;
  }
  void update()
  {
    value = (ESP.getVcc() / 1000.00) * factor;
    label->update(String(value, 1));
    if (output)
      output->update(value);
  } // por ahora lo convierto a volts * 10.

private:
  Label *label;
};
#endif

// ########################################
//  GenericTimer
// ##################################
class GenericTimer : public ElementsHtml
{
public:
  GenericTimer(String _name, String _id, HardwareOutput *o = 0, ElementsHtml *e = 0)
  {
    name = _name;
    id = _id;
    output = o;
    edt_Shedule = new EditBox(id + "edt", "", "text", this);
    edt_OnTime = new SavedEdit("On Time:",id + "edtOn", "", "time", this);
    edt_OffTime = new SavedEdit("Off Time",id + "edtOff", "", "time", this);
    chkState = new SavedEdit("on/off",id + "chkS", "/status.sta","checkbox", this);
    chkMode = new SavedEdit("Timer / Hours",id + "chkM", "/status.sta","checkbox", this);
    edt_Shedule->setStyle (" class='numInp'");
    strTime = new SavedVariable(id + "eTi");
    strTime->setFile("/status.sta");
    parent=e;
    label = new Label (id+"lbl","",this);
  }
  String getHtml()
  {
    return "<div class='card'><h4>" + name + "</h4>"+
    chkState->getHtml()+chkMode->getHtml()+"Time Schedule" +
             edt_Shedule->getHtml() + edt_OnTime->getHtml() + edt_OffTime->getHtml()+
             "<br>Status: "+label->getHtml() +
             "</div>";

  }
  void update()
  {
     //Serial.println(id+" : "+String(running));
    if (firstRun)
    {
      strTime->update();
      edt_Shedule->update(strTime->text);
      firstRun = false;
      output->update(0);
      index = 0;
      chkState->update();
      chkMode->update();
      edt_OnTime->update();
      edt_OffTime->update();
      running = (bool) chkState->getValue();
      lastCheck = millis();
    }
    if (running)
    {
      if (!chkMode->value) {     //  MODE SCHEDULE 
          
          if ((millis() - lastCheck) > (((strTime->text.substring(index, strTime->text.indexOf('-', index)))).toInt()-1) * 1000)
          {
            value = !value;
            //if (index=0) value = 0;
            output->update(value);
            index = strTime->text.indexOf('-', index) + 1;
            if (index < 0){
              index = 0;
              output->update(0);
            }
            // if (index == 0)
            //   output->update(0);
            // if (id== "csbmbedt")
             Serial.println("index: "+String(index)+" text: "+((strTime->text.substring(index,strTime->text.indexOf('-',index))).toInt()) +
                             " value: "+String(value));
            lastCheck = millis();
          }
      }
      else {                      // MODE TIME OF DAY
            bool reversed=false;
           int onTime = String(edt_OnTime->getText().substring(0,2)+edt_OnTime->getText().substring(3,5)).toInt();
           int offTime = String(edt_OffTime->getText().substring(0,2)+edt_OffTime->getText().substring(3,5)).toInt();
           int nowTime = String(String(hour())+(minute()<10?"0":"")+String(minute())).toInt();
           reversed = (onTime > offTime); // in case onTime < offTime
              //Serial.println(edt_OnTime->getText());
          if (timeStatus() == timeSet) {
            if (( nowTime>onTime) &&  (nowTime<=offTime) )
                {
                output->update(!reversed); value = !reversed;
                //Serial.println("true");
              }
              else {
                output->update(reversed); value = reversed;
                   //           Serial.println("false");

              }
          }
          else {
            label->update("Time not Set");
          }
      }  //  end of if chkMode == Timer
    } //        END OF IF running
    else
    {
      //     if ( (millis()-lastCheck)  > (offTime->value*1000)) {
      output->update(0);
      //       lastCheck = millis();
      //     }
    }
    //if (lastValue!=output->value){
      label->update(  (output->value ? "ON" : "OFF") );
      lastValue=output->value;
    //}
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e == edt_Shedule)
    {
      strTime->update(postValue);
      output->update(0);
      index = 0; // if any change then reset index
      lastCheck=millis();
    }
    if (e == chkState)
    {
      Serial.println(postValue);
      if (postValue=="1") {running = true; lastCheck = millis();}
      else {stop();}
    }
    if (e==chkMode){
      lastCheck=millis();
      index=0;
    }
    return "";
  }
  void enable()
  {
    running = true;
    index=0;
    //value = 1;
  }
  void stop()
  {
    running = false;
    output->update(0);
    value = 0;
    index=0;
  }
  void update(float f)
  {
    output->update(f);
    value = f;
  }
  void update(String sss) { edt_Shedule->update(sss); }

private:
  Label *label;
  String unit = "";
  HardwareOutput *output;
  SavedVariable *strTime;
  EditBox *edt_Shedule;
  SavedEdit *edt_OnTime;
  SavedEdit *edt_OffTime;
  SavedEdit * chkState;
  SavedEdit * chkMode;
  unsigned long lastCheck;
  int index = 0;
  bool running = true;
  float lastValue=0.1;
};


// ########################################
//  GenericInputPanel
// ##################################
class GenericInputPanel : public Input
{
public:
  GenericInputPanel(String n, String u, HardwareInput *h = NULL,
                    bool use_calibration = false,
                    bool use_Average = false,
                     ElementsHtml *e = NULL,
                     bool _allow_name_change=true)
  {
    id = n;
    unit = u;
    name = n;
    input = h;
    label = new Label(id + "lbl", "", this);
    label->setStyle(" class='numInp'"); //   Esto lo hago desde el CSS es una clase
    btnID = new ButtonPrompt((id + ("btnID")).c_str(), "Change ID", this);
    columnName = new SavedVariable (id+"Name");
    // pushElement(this);          // Los elementos basicos se registran solos en el AllHTMLElemens !!
    if (use_calibration)
      cal = new calibration_module(id + "Cal", this);
    if (use_Average)
      avg = new AverageModule();
      allow_change_name=_allow_name_change;
    // chart = new Chart ( "chart"+id,this);
  }
  void setAllowChangeName(bool b){allow_change_name=b;}
  String getHtml()
  {
    String s = " <div class='card'";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += id;
    s += "</h4>";
    if (allow_change_name) s+= btnID->getHtml();
    s += label->getHtml();
    s += "<br>";
    s += unit;
    // s+= chart->getHtml();
    s += "<br>";
    if (cal)
      s += cal->getHtml();
    s += "</div>";
    return s;
  }
  void update()
  {
    if (firstRun){
      firstRun = false;
      columnName->update();
      if (columnName->text!="" && columnName->text!=" ") {id=columnName->text;name=id;}
      else columnName->update(id);
    }
    if (input)
    {
      input->update();
      value = input->value;
      //Serial.println("Before CAL"+String(value));
      cal ? value = cal->calculate(value) : value = value;
      //Serial.println("Before AVG"+String(value));
      avg ? value = avg->addValue(value) : value = value;// NO SE QUE PASA AQUI RESETEA
      //Serial.println("After AVG"+String(value));
    }
    label->update(value);
    // chart->update(value);
  }
  void update(float v)
  {
     value = (v);   //Creo que no tiene sentido hacerle update(float)
    label->update(value);
    // input->update(value);
  }
  void update(String v)
  {
     value = (v.toFloat());
    label->update(value);
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e==btnID){
      columnName->update(postValue);
      id = columnName->text;
      name = id;
    }
    return "";
  }
  float getSecondValue() { return input->getSecondValue(); }
  void setSecondValue() { secondValueBool = true; };

private:
  Label *label;
  ButtonPrompt *btnID;
  String unit = "";
  SavedVariable *columnName;
  HardwareInput *input;
  calibration_module *cal;
  AverageModule *avg;
  bool secondValueBool = false;
  bool allow_change_name = true;
  // Chart *chart;
};
// ########################################
//  GENERIC OUTPUT PANEL   LO PUSE EN EL INPUT PORQUE NECESITO SEPARAR LOS HEADERS
// ########################################
class GenericOutputPanel : public Output
{

public:
  GenericOutputPanel( String _name,
                         String _id, 
                         String _unit,
                         HardwareOutput *out=NULL,
                          bool inverted = false,
                          ElementsHtml *_parent=NULL,
                          bool _allow_name_change=true)
  { /// Constructor
    id = _id;
    name = _name;
    unit = _unit;
    btnID = new ButtonPrompt((id + ("btnID")).c_str(), "Change ID", this);
    columnName = new SavedVariable (id+"Name");
    label = new Label("lbl" + id,"text", this);
    label->setStyle(" class='numInp' "); //   Esto lo hago desde el CSS es una clase
    value=0;
    invertedLogic = inverted;
    output = out;
    allow_name_change = _allow_name_change;
  };
  void update(){
    if (firstRun){
      firstRun = false;
      columnName->update();
      if (columnName->text!="" && columnName->text!=" ") {id=columnName->text;name=id;}
      else columnName->update(id);
    }
    label->update(value);
    };
  void update(String sss) { update(sss.toFloat()); };
  void update(float _value)
  {
    //Serial.println("Updating "+id+" Value: " + String(_value));
    value = _value;
    if (output!=NULL) output->update(value);
    label->update(value);
  }

  String getHtml()
  {
    String s = " <div class='card'";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += id;
    s += "</h4>";
    if (allow_name_change) s+= btnID->getHtml();
    s+="<br>";
    s += label->getHtml();
    s += "<br>";
    s += unit;
    s += "</div>";
    return s;
  }

  String postCallBack(ElementsHtml *e, String postValue) { 
    if (e==btnID){
      columnName->update(postValue);
      id = columnName->text;
    }
    
    return ""; 
          }

private:
  Label *label;
  HardwareOutput *output;
    ButtonPrompt *btnID;
  SavedVariable *columnName;
  bool allow_name_change = false;
};

class VFD_Panel: public GenericOutputPanel {
  public:

};
// ########################################
//  FakeInput
// ##################################
class FakeInput : public Input
{
public:
  FakeInput(String n)
  {
    label = new Label(n + "lbl", "0.00", this);
    value = 0;
    id = n;
    name = n;
  }
  String getHtml() { return label->getHtml(); }
  void update() {}
  void update(float v)
  {
    value = v;
    label->update(value);
  }
  void update(String v)
  {
    value = v.toFloat();
    label->update(v);
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    update();
    return "";
  }

private:
  Label *label;
  String unit = "";
};

// ########################################
//  GenericEditPanel
// ##################################
class GenericEditPanel : public Input
{
public:
  GenericEditPanel(String n, String u)
  {
    id = n;
    unit = u;
    name = n;
    edit = new EditBox(id + "lbl", "", "text", this);
    // edit->setStyle(" class='numInp'" );   //   Esto lo hago desde el CSS es una clase
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String getHtml()
  {
    String s = " <div  class=' card '";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += name;
    s += "</h4><br>";
    s += edit->getHtml();
    s += "<br>";
    s += unit;
    s += "</div>";
    return s;
  }
  void update() { text = edit->text; }
  // void update(float v){value=v;update();}
  String postCallBack(ElementsHtml *e, String postValue)
  {
    update();
    return "";
  }

private:
  // Label* label;
  String unit = "";
  EditBox *edit;
};

// ########################################
//  Digital IN
// ########################################

class DigitalIn : public Input
{
public:
  DigitalIn(int _pin, String _name)
  { // f es una funcion pasada como parametro
    pin = _pin;
    pinMode(pin, INPUT); ////   ESTO HE CAMBIADO
    name = _name;
    id = _name;
    unit = "n/a";
    label = new Label(name + "lbl", "", this); // addChild(label);
    img = new Image("img" + id, "power-button.jpg", this);
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }

  void attachInterruptSub(void (*f)()) { attachInterrupt(digitalPinToInterrupt(pin), f, FALLING); }

  String getHtml()
  {
    String s = " <div  class=' card '";
    s += style;
    s += " id='";
    s += id;
    s += "'><h6>";
    s += name;
    s += "</h6>";
    s += label->getHtml();
    s += img->getHtml();
    s += "</div>";
    return s;
  }

  void update()
  {
    value = digitalRead(pin);
    label->update(String(value));
    img->update((value == 0) ? "power-button.jpg" : "power-buttonON.jpg");
    if (output)
      output->update(value);
  }
  float getReading() { return digitalRead(pin); }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    update();
    return "";
  }

private:
  int pin;
  Label *label;
  Image *img;
};


// ########################################
//  PID_Module               
// ########################################
//  Este modulo se adosa a un objeto PID existente y sirve para poder programar los parametros.
class PID_Module : public ElementsHtml {
  public:
    PID_Module ( String _id,PID *_pid  ){
      id = _id;
      name = _id;
      pid = _pid;
      edtP = new SavedEdit ( "kP",id + "edtP","/status.sta","number",this);
      edtI = new SavedEdit ( "kI",id + "edtI","/status.sta","number",this);
      edtD = new SavedEdit ( "kD",id + "edtD","/status.sta","number",this);
      //onOff = new SavedEdit ( "onOff",id + "edtOn","/status.sta","checkbox",this);
    }
    String getHtml () {
      String s="<div class='spdCtrl'><h4>"+id+"</h4>";
      s+=edtP->getHtml()+edtI->getHtml()+edtD->getHtml();
      s+="</div>";
      return s;
    }
   void  update () {
     if (firstRun) {
       edtP->update();edtI->update();edtD->update();
       updateTuning();
     }
     //consoleLog(id,String(pid->GetKp(),4));
     //consoleLog(id,String(pid->GetKi(),4));
     //consoleLog(id,String(pid->GetKd(),4));
     
   }
   void setPidParameters (){}
   void setP (double p){ edtP->update(p); updateTuning();}
   void setI (double i){ edtI->update(i);updateTuning();}
   void setD (double d){ edtD->update(d);updateTuning();}
   void updateTuning (){ pid->SetTunings(edtP->value,edtI->value,edtD->value); }
  String postCallBack(ElementsHtml *e, String postValue) {
    if (e==edtP || e==edtI || e==edtD ) updateTuning();
    //consoleLog(edtP->name,String(edtP->value));
  }
  private:
    PID *pid;
    SavedEdit *edtP, *edtI , *edtD;
    //Label *lblInp, *lblSet, *lblOut;
    //SavedEdit *onOff;
    String id;
};


// ########################################
//  KEYPAD HTML
// ########################################
class KeyPad : public Input
{ // Aqui lo he cambiado !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  Antes era input
public:
  int value = 0;
  String state = "ooo";
  EditBox *edt;
  Label *edtLabel;
  Button *buttons[11];

  KeyPad(String n)
  {
    name = n;
    id = n;
    edt = new EditBox(id + "Edit", "", "text", this);
    edtLabel = new Label(id + "Label", "Locked", this);
    //      addChild(edt);addChild(edtLabel);
    for (int i = 0; i < 10; i++)
    {

      buttons[i] = new Button(id + "btn" + String(i), String(i), this); // esto indica que tiene pariente
                                                                        //        addChild (buttons[i]);
                                                                        // yield();
    }
    buttons[10] = new Button(name, "delete", this);
  }
  String getHtml()
  {
    String s = "<div  class=' card '";
    s += style;
    s += " id='";
    s += id;
    s += "'><h4>";
    s += name;
    s += "</h4>\n\t";
    for (int i = 0; i < 11; i++)
    {
      s += buttons[i]->getHtml();
      if ((i == 2) || (i == 5) || (i == 8))
        s += "<br>";
    }
    s += edt->getHtml();
    s += edtLabel->getHtml();
    s += "</div>";
    return s;
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (postValue == "delete")
      edt->deleteChar();
    else
      edt->appendText(postValue);
    update();
    return ""; //( "console.log('postCallBack of "+name+"'); ");
  };
  void update()
  {
    (edt->text == "1234") ? value = 1 : value = 0; // updateDisplay();
    !value ? state = "Locked" : state = "Unlocked";
    edtLabel->update(state);
  };
};

// ########################################
//  Directory of captures
// ########################################

class DirCapture : public ElementsHtml
{
public:
  DirCapture(String s, String d)
  {
    id = s;
    name = s;
    dirName = d;
    // chart = new Chart ( "chart"+id,this);
  }

  String getHtml()
  {
    // Serial.println("DIRCAPTURE: " + dirName);
    String content = "Capturas:";
    bool flag = false;
    content = "<div id='tableDiv' class=' card '><h><table id='tableDir'><thead><tr><th>Nombre</th><th>memoria</th></tr></thead>";
#ifdef ESP8266      //  En esp8266 name es solo el NOMBRE en esp32 es todo el PATH
    Dir dir = LittleFS.openDir(dirName);
    while (dir.next())
    {
      entry = dir.openFile("r");
#endif
#ifdef ESP32
    File dir = SPIFFS.open(dirName);
    entry = dir.openNextFile();
    while (entry)
    {
#endif
      flag = true;
      String entryName = entry.name();
#ifdef ESP32
      entryName = (String(entryName)).substring(String(entryName).indexOf("/", 2) + 1);
#endif
      // content += "<tr><td>";content+= String(entry.name()) +"</td><td>" + entry.size() + "</td></tr>";
      content += "<tr><td onclick=\"btnClickText('" + id + "','" + String(entry.name()) + "')\">";
      content += (String(entryName)).substring(String(entryName).indexOf("/", 2) + 1);
      content += "</td><td>";
      content += String(entry.size());
      content += "</td><td><a href='javascript:;' onclick=\"handlePrompt('";
      content += entryName;
      content += "','New Name');\">Rename</a>";

      content += "<a href='/delete?file=/capturas/";
      content += entryName;
      content += "'>Delete</a>";

      content += "<a href='graphic.html?file=/capturas/";
      content += entryName;
      // content += "', '"+chart->id+"',50);\">Show</a>";
      content += "'>Show</a>";

      content += "<a href='/capturas/";
      content += entryName;
      content += "'>Download</a>";

      content += "<a href='javascript:;' onclick=\"handleUpload('";
      content += entryName;
      content += "');\">Upload</a>";

      content += "</td></tr>";
#ifdef ESP32
          entry = dir.openNextFile();
#endif
    }

    content += "</table>";
    // content += chart->getHtml();

//#endif
    content += "</div>";
    return flag ? content : "";
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    // chart->update();
    return "";
  };

private:
  String f[3] = {"DELETE", "DOWNLOAD", "UPLOAD"};
  // Chart *chart;
  File entry;
  String dirName;
};
