#include "TimeLib.h"
//#include <ArduinoModbus.h>

class Input;
//class ComboBox;
// ########################################
//  HARDWARE OUTPUT
// ########################################
class HardwareOutput
{
public:
  float factor = 1;
  float value;
  float lastValue;
  //String id;
  virtual void update();
  virtual void update(float f);
};
class FakeOutput: public HardwareOutput
{
  public:
  FakeOutput(){};
  void update(){}
  void update(float f){value = f;}
};
class Output : public ElementsHtml
{

public:
  String stateStr;
  //float value=0;
  String unit;
  bool invertedLogic = true;
  virtual void update(float newValue){value=newValue;};
  String getHtml() { return ""; }
  //void toogle(){ value? value=0:value=1;}
};

//Output operator++ () { value++;}
// ########################################
//  LABEL
// ########################################
#ifdef ESP8266 
#include <umm_malloc/umm_malloc.h> // for computing memory fragmentation
#endif

class Label : public Output
{
public:
  String text = "Output:Label";

  Label(String n, String t, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    text = t;
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String getHtml()
  {
    String s = F("<span ");
    s += style;
    s += F(" id='");
    s += id;
    s += F("'>");
    s += text;
    s += F("</span>");
    return s;
  }
  //void addHtml(){ String s= "<span id='";s+=id;s+= "'>";s+=text;s+="</span>"; htmlAdd(s.c_str()); }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }
  void update(String newValue)
  {
    {
      text = newValue;
      value = text.toFloat();
      update();
    }
  }
  void update() { 
    //if (lastText!=text){
    javaQueue.addJsonInner(id,text);
    lastText = text;
    value = text.toFloat();
    //Serial.println("UPDATING LABEL "+id+" value: "+text);
  //}
  }
  void update(float newValue) { update(String(newValue, 2));  }
  void append(String appendValue)
  {
    text += appendValue;
    update();
  }
protected:
  String lastText = "&^%$"; // es protected porque la usa LblFreeHeap que deriva de Label
private:
  int textSize = 1;
};

const size_t block_size = 8;
size_t getTotalAvailableMemory()
{
  //umm_info(0, 0);
  return ESP.getFreeHeap();//.freeBlocks * block_size;
}

size_t getLargestAvailableBlock()
{
  #ifdef ESP8266
  umm_info(0, 0);
  return ESP.getMaxFreeBlockSize();//.maxFreeContiguousBlocks * block_size;
  #else
  return 0;
  #endif
  

}

class LabelFreeHeap : public Label
{
public:
  using Label::Label;
  void update()
  {
    value = ESP.getFreeHeap();
    int percentage = 100 - getLargestAvailableBlock() * 100.0 / getTotalAvailableMemory();
    text = "Free Heap: "+String(value)+" - "+String(percentage)+
                                "% - MaxFreeBlock: "+String(getLargestAvailableBlock());
    if (text!=lastText){
    
    javaQueue.addJsonInner(id,text);
    lastText=text;
  }
  }
  private:
};


    String printDigits(int digits)
            {
                // utility for digital clock display: prints preceding colon and leading 0
                String s = String(":");
                if (digits < 10)
                    s += String('0');
                s += String(digits);
                return s;
            }
            
class TimeLabel : public Output
{
  String text = "L";

public:
  TimeLabel(String n, String t, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    text = t;
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
    String s = "var now = new Date(); btnClickText('";
    s += id;
    s += "',now.getTime()/1000);";
    #ifdef ESP8266    //     El ESP32 se resetea cuando va al handleBotonClick
    javaQueue.addOnLoad(s);
    #endif
  }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }

  String getHtml()
  {
    String s = "<span id='";
    s += id;
    s += "'>";
    s += text;
    s += "</span>";
    return s;
  }
  // void addHtml(){ String s= "<span id='"; s+=id;s+= "'>";s+=text;s+="</span>"; htmlAdd( s.c_str()) ;  }
  void update(uint32_t t)
  {
    tt = t;
    value = t;
    String s = F("var now = new Date(");
    s += String(t);
    s += "*1000);if(now){ ";
    s += docIdStr;
    s += id;
    s += F("').innerHTML=now.toString();}");
    //javaQueue.add(s);
    //javaQueue.addJsonInner(id,now.toString());
  }
  void update()
  {
    String s ;//= docIdStr;
    time_t localtime (now()-LOCAL_TIME_OFFSET*3600); //Aqui ajusto para la hora local
    //s += id;
    //s += F("').innerHTML='");
    s += String(day(localtime));
    s += "/";
    s += String(month(localtime));
    s += "/";
    s += String(year(localtime));
    s += " - ";
    s += String(hour(localtime));
    s += printDigits(minute(localtime));
    s += printDigits(second(localtime));
    //s += "';";
    javaQueue.addJsonInner(id,s);

  }
  void update(float t) {}
  void update(String ss)
  {
    javaQueue.addJsonInner(id,ss);
  }

private:
  long tt;
};
// ########################################
//  TABLE
// ########################################
class Table : public Output
{
public:
  String text = "Table Title";
  Table(String n, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String getHtml()
  {
    String s = "<div id='";
    s += id;
    s += "div'><h5>";
    s += text;
    s += "</h5><table id='";
    s += id;
    s += F("'><tr><td></td><td></td></tr></table></div>");
    return s;
  }
  //void addHtml(){ String s= "<div id='";s+=id;s+="div'><h5>";s+=text;s+="</h5><table id='";s+=id;s+= "'><tr><td></td><td></td></tr></table></div>"; htmlAdd( s.c_str());  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    else
      return "";
  }
  //void update ( String newValue ) { text= newValue; javaQueue.add(docIdStr + id + "').innerHTML='<h5>"+text+"</h5>';");}
  void setTitle(String sss) { text = sss; }
  void addRow(String newRow) { javaQueue.add(docIdStr + id + F("').insertRow().insertCell(0).innerHTML='") + newRow + "';"); }
  // void makeTable(String csv){ javaQueue.add(docIdStr + id + "') = makeTable(csv);"); }
  void makeTable(String csv)
  {

    String content = F("<table id='tableDir'><thead><tr><th>Peso (Kg)</th><th>Distancia (cm)</th></tr></thead>");
    int maxIndex = csv.length() - 1;
    int index = 0;
    int next_index;
    String data_word;
    do
    {

      next_index = csv.indexOf(',', index);
      data_word = csv.substring(index, next_index);
      content += F("<tr><td onclick='clickedCell(this)'>");
      content += data_word + "</td>";
      index = next_index + 1;
      next_index = csv.indexOf(',', index);
      data_word = csv.substring(index, next_index);

      content += "<td>" + data_word + "</td></tr>";
      //Serial.println(data_word);
      index = next_index + 1;
    } while ((next_index != -1) && (next_index < maxIndex));

    content += "</table>";
    javaQueue.add(docIdStr + id + "div').innerHTML=\"" + content + "\";"); // Atencion si Content ya contiene semicolons simples no puedo usarlo aqui !!!
  }
  void changeCell(String newText, int indexRow, int indexColumn)
  {
    String s = F("var createdText=document.createTextNode('");
    s = s + newText;
    s = s + "');";
    s += docIdStr;
    s += id;
    s += F("').getElementsByTagName('tbody')[0].getElementsByTagName('tr')[");
    s += String(indexRow);
    s += F("].getElementsByTagName('td')[");
    s += String(indexColumn);
    s += F("].childNodes[0]=createdText;");
    javaQueue.add(s); //    insertRow().insertCell("+String(indexY)+").innerHTML='"+newText+"';");
  }
  void update() {}
  void update(float newValue)
  {
    value = newValue;
    update();
  }

private:
};

// ########################################
//  IMAGE
// ########################################

class Image : public Output
{
public:
  Image(String n, String t, ElementsHtml *e = 0)
  {
    name = n;
    id = n;
    url = t;
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  String getHtml()
  {
    String s = F("<img heigth='");
    s += String(heigth);
    s += F("' width='");
    s += String(width);
    s += "' id='";
    s += id;
    s += "' src='";
    s += url;
    s += "'>";
    return s;
  }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }
  void update(String newValue)
  {
    if (newValue != lastValue)
    {
      url = newValue;
      String s = docIdStr;
      s += id;
      s += "').src='";
      s += url;
      s += "';";
      javaQueue.add(s);
      lastValue = newValue;
    }
  }
  void update(float f) {}
  void update() {}
  void setWidth(int w) { width = w; }
  void setHeigth(int h) { heigth = h; }

protected:
  String url = "";
  int width = 40;
  int heigth = 40;
  String lastValue = "";
};

class wifiIcon : public Image
{
  String getHtml()
  {
    String s = F("<img heigth='");
    s += String(heigth);
    s += F("' width='");
    s += String(width);
    s += "' id='";
    s += id;
    s += "' src='/images/wifi_signal_0.png'>";
    return s;
  }
};
// ########################################
//  Graphic
// ########################################

class Graphic : public Output
{
public:
  Graphic(String ss, ElementsHtml *e = 0)
  {
    name = ss;
    id = ss;
    parent = e;
  }
  String getHtml() { return "<div><canvas id='" + id + "' heigth='200'></canvas></div><script>var xPos=0;</script>"; }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }
  void update(float newValue)
  {
    String str;
    str += "var canvas = ";
    str += docIdStr;
    str += id;
    str += F("');\n"
             "ctx=canvas.getContext('2d');\n"
             "ctx.strokeStyle = 'blue';\n"
             "ctx.beginPath();\n"
             "ctx.arc(xPos*4,canvas.height-2 - ");
    str += String(newValue * 5);
    str += F(", 2, 0 , 2*Math.PI );\n"
             "ctx.fillStyle = 'blue';\n"
             "ctx.fill();\n"
             "ctx.stroke();\n"
             "ctx.closePath();\n"
             "xPos++; if ( xPos*4>canvas.width ) {  ctx.clearRect(0, 0, canvas.width, canvas.height);   xPos = 0;     generateCanvas();   }"
             "function generateCanvas(){ var canvas = document.getElementById('");
    str += id;
    str += F("');  ctx=canvas.getContext('2d');"
             "  ctx.font = '10px serif';  ctx.fillStyle = 'blue';    ctx.fillText('");
    str += name;
    str += F("', 20, 15);    ctx.stroke();} ");
    javaQueue.add(str);
    //xPos++; if(xPos==
  }
  void update(String sss) { update(sss.toInt()); }
  void update() {}
  // void update( float newValue) {value=newValue;update();}

private:
  //int xPos=0;
};


// ########################################
//  Digital Output
// ########################################

class DigitalOutput : public HardwareOutput
{
public:
  DigitalOutput(int _pin, String _id, bool inverted = false)
  {
    pin = _pin;
    invertedLogic = inverted;
    //id=_id;
  }
  void update(String sss) { update(sss.toInt()); };
  void update(float newValue)
  {
    //Serial.println("Updating "+id+" Value: " + String(newValue));
    if (firstRun)
    {
      firstRun = false;
      pinMode(pin, OUTPUT);
    }
    value = newValue;
    if (value > 1)
      value = 1;
    if (value != 0)
    {
      digitalWrite(pin, invertedLogic ? HIGH : LOW);
    }
    if (value == 0)
    {
      digitalWrite(pin, invertedLogic ? LOW : HIGH);
    }
  }
  void update()
  {
    update(value);
  }
  

private:
  int pin;
  bool invertedLogic;
  bool firstRun = true;
};

// ########################################
//  Modbus Relay
// ########################################

class ModbusRelay: public HardwareOutput{
  public:
   ModbusRelay(int _slaveID, int _relayNumber) { 
     gSlaveID = _slaveID;
     relayNumber = _relayNumber;
     list.push_back(this);
   }
    void update(){
      #ifdef ESP32
      if (lastValue != value ){
      //unsigned long tmr = millis();
      //Serial.println("Writing to Modbus Relay "+String(relayNumber));
      //nodeRelays.writeSingleRegister(relayNumber,(value?0x0100:0x0200), gSlaveID );
          modbus.writeSingleHoldingRegister(gSlaveID,relayNumber,(value?0x0100:0x0200));

      //Serial.println("Elapsed time for RS485 com to RelayBard: "+String(millis()-tmr));
      lastValue = value;
      }
      #endif
    }
    void update (float v){value=v;update();}
    static void requestData(int _slaveID){
      modbus.readHoldingRegisters(_slaveID,0,16);
      Serial.println("Sending Relay Data Request");
    }
    static void checkData(uint8_t* data){
      Serial.println("Checking for Modbus Relay discrepancies");
          for (unsigned int i = 0; i < list.size(); i++){
             if ( data[(list[i]->relayNumber-1) * 2 +1  ] != list[i]->value) {  // si es igual hay discrepancia ya que en la respuesta 0=ON
                list[i]->lastValue=-1;list[i]->update();
                Serial.println("Discrepancia en  Relay numero "+String(list[i]->relayNumber));
             };

          }

    }
  private:
    int relayNumber;
    int gSlaveID;
    static std::vector <ModbusRelay*> list;
};
std::vector<ModbusRelay *> ModbusRelay::list;

// ########################################
//  Modbus VFD
// ########################################

class ModbusVFD: public HardwareOutput {
  public:
    ModbusVFD ( int _slaveID , int _type=0){ slaveID=_slaveID;type=_type;}
    void setType(int t){type=t;}
    void update(float v){ value = v ;update();}

    void update(){
            #ifdef ESP32

      if (lastValue!=value){
        //Serial.println("Trying to write to VFD modbus value=" + String(value));
        unsigned long tmr = millis();

        if (value==0) {
        running = false;
        if (type==VFD_Types::SOYAN_SVD)
          //nodeRelays.writeSingleRegister(0x1000,5, slaveID );
            modbus.writeSingleHoldingRegister(slaveID,0x1000,5);
        if (type==VFD_Types::MOLLOM_B20)
            modbus.writeSingleHoldingRegister(slaveID,0x2000,1);
          //nodeRelays.writeSingleRegister(0x2000,1, slaveID );
        }
        else {
          if (type==VFD_Types::SOYAN_SVD)
            //nodeRelays.writeSingleRegister(0x2000,value * 100  ,slaveID);
            modbus.writeSingleHoldingRegister(slaveID,0x2000,value*100);
          if (type==VFD_Types::MOLLOM_B20)
            //nodeRelays.writeSingleRegister(0x2001,value * 100  ,slaveID);
            modbus.writeSingleHoldingRegister(slaveID,0x2001,value*100);

        if (!running){
          if (type==VFD_Types::SOYAN_SVD)
            //nodeRelays.writeSingleRegister(0x1000,1,slaveID );
            modbus.writeSingleHoldingRegister(slaveID,0x1000,1);
          if (type==VFD_Types::MOLLOM_B20){
            //nodeRelays.writeSingleRegister(0x2000,0x12,slaveID );
            modbus.writeSingleHoldingRegister(slaveID,0x2000,0x12);
            //if (!nodeRelays.readHoldingRegisters(0x2100,1,slaveID))
            //Serial.println(nodeRelays.getResponseBuffer(0));
          }
          //running=true;
        }
        }
        //     Serial.println(String (type)+" Elapsed time for RS485 com to VFD: "+String(millis()-tmr));

        lastValue = value;
      }
      #endif
    }
  private:
    int slaveID;
    float lastValue=-1;
    float running=false;
    int type=0;
};
// class modbusVFD_html : public ElementsHtml {
//   public:
//     modbusVFD_html (String _id , ModbusVFD *m){vfd=m;id=_id;
//       panel=new GenericOutputPanel (id,id,"",vfd,false,this,false);}
//     void update(){vfd->update();value=vfd->value;}
//     void update(float v){vfd->update(v);}
//     String getHtml(){
//       return panel->getHtml();
//     }
//   private:
//     ModbusVFD * vfd;
//     GenericOutputPanel *panel;
// };
// ########################################
//  Modbus LED
// ########################################

class ModbusLed: public HardwareOutput {
  public:
    ModbusLed ( int _slaveID ){ slaveID=_slaveID;}
    void update(float v){ value = v ;update();}
    void update(){

      //int r=nodeRelays.writeSingleCoil(2,on,slaveID ); // GPIO 2 es el built in led
      #ifdef ESP32
                  modbus.writeSingleHoldingRegister(slaveID,0x02,on);
      #endif
      //Serial.println("trying to send to Modbus Led: value:"+String(on)+" result: "+String(r));
      on = !on;
      ;
      
      //Serial.println(ModbusRTUClient.coilWrite(slaveID, 0x0000, value));
    }
  private:
    int slaveID;
    bool on=false;
};
// ########################################
//  PWM
// ########################################
class PWM : public HardwareOutput
{
public:
  PWM(int _period , DigitalOutput * _output)
  {
    period = _period;
    previousMillis = 0;
    output = _output;
  }
  void update (float f){}
  void update()
  {
    unsigned long currentMillis = millis();
    bool result;
    if ((currentMillis - previousMillis >= (period / max) * level) && level!=max)
    {
     // consoleLog("PWM",String(currentMillis)+":"+String(previousMillis));
      result = false;
    }
    else
    {
      result = true;
    }

    if ((currentMillis - previousMillis) > period)
    {
      previousMillis = currentMillis;
      //result=true;
    }
    output->update(result);
  }
  void setLevel(float i) { level = i; }

private:
  unsigned long previousMillis;
  int period = 0;
  float level = 0; // level goes from 0ne to max;
  int max = 10;
  DigitalOutput *output;
};

// ########################################
//  BINARY OUTPUT
// ########################################
class BinaryOutput: public HardwareOutput {
  public:
    BinaryOutput( int _pin1,int _pin2, int _pin3, ElementsHtml *e = 0){ 
      d1 = new DigitalOutput ( _pin1 , "pin1");
      d2 = new DigitalOutput ( _pin2 , "pin2");
      d3 = new DigitalOutput ( _pin3 , "pin3");
    }
    void update(float v){
       d1->update( ((int)v) & 1 );
       d2->update( ((int)v) & 2 );
       d3->update( ((int)v) & 4 );
       //Serial.println("Update BinaryOutput v="+String(v)+" : v&4= "+String((int)v&4));
       value = v;
    }
    void update(){update(value);}
    void update(String s){update(s.toFloat());}
  private:
    DigitalOutput *d1;
    DigitalOutput *d2;
    DigitalOutput *d3;
    
};



// ########################################
//  RELAY OUTPUT
// ########################################

class RelayOutput : public DigitalOutput
{
  using DigitalOutput::DigitalOutput; // inherit the constructor
public:
  // invertedLogic = true;
};

// ########################################
//  GAUGE
// ########################################

class Gauge : public Output
{
public:
  bool firstRun = true;
  Gauge(String sss, String t, int minV = 0, int maxV = 100, ElementsHtml *e = 0)
  {
    name = t;
    id = sss;
    if (parent)
      parent = e;
    value = 0;
    label = new Label("lbl" + id, "0");
    String s = "var i=document.createElement('script');i.src='gauge.min.js';\n";
    s += "var i=document.createElement('script');i.src='gaugeScript.js';\n";
    s += "target = document.getElementById('";
    s += id;
    s += "'); window.";
    s += id;
    s += " = new Gauge(window.target).setOptions(window.opts);\n";
    s += "window." + id + ".maxValue=(" + String(maxV) + ");\n"; // set max gauge value
    s += "window." + id + ".minValue=(" + String(minV) + ");\n";
    javaQueue.addOnLoad(s);
  }

  String getHtml()
  {
    String s = "<div><h4>";
    s += name + "</h4><canvas id='";
    s += id;
    s += "' height='100' width='150'></canvas><br><center>";
    s += label->getHtml();
    s += "</div>";
    return (s);
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    else
      return "";
  }
  void update(int v)
  {
    value = v;
    update();
  }
  void update(float newValue)
  {
    value = newValue;
    update();
  }
  void update()
  {
    String str;
    // if (firstRun) {str+="window.";str+=id;str+=".setOptions(window.opts);"; firstRun=false;}
    //str+="console.log(window.opts);window.";
    str += "window.";
    str += id;
    str += ".set(";
    str += String(value);
    str += ");";
    label->update(value);

    javaQueue.add(str);
  }

private:
  Label *label;
};

// ########################################
//  Chart
// ########################################
class Chart : public Output
{
public:
  Chart(String nn, ElementsHtml *e = 0)
  {
    name = nn;
    id = nn;
    parent = e;
    String s; //="var i=document.createElement('script');i.src='gauge.min.js';\n";
    s += ("window." + id + "_ctx = document.getElementById('" + id + "_can').getContext('2d');");
    s += "createConfig('" + id + "');";
    s += id + " = new Chart(window." + id + "_ctx,config[checkConfig('" + id + "')]);\n"; //config[checkConfig("name")] devuelve el indice

    javaQueue.addOnLoad(s);
  }
  String getHtml()
  {
    String s = "<div id='wrapper' style='position:relative' width='30%' ><canvas id='";
    s += (id + "_can");
    s += "'></canvas></div>";
    //</div><button onclick=btnClickedPlus('"+id+"')>+</button>"
    //"<button onclick=btnClickedMinus('"+id+"')>-</button>"
    //"<button onclick=btnClickedForward('"+id+"')>>></button>"
    //"<button onclick=btnClickedBackward('"+id+"')><<</button></div>";
    return (s);
  }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }
  void update(float newValue)
  {
    String str;
    str += id;
    str += ".update("+String(newValue)+");console.log('";
    str += "CHART_UPDATE() " + id + "');\n";
    javaQueue.add(str);    
  }
  void update(String sss) { update(sss.toFloat()); }
  void update()
  {
    String str;
    str += id;
    str += ".update();console.log('";
    str += "CHART_UPDATE() " + id + "');\n";
    javaQueue.add(str);
  }
  // void update( float newValue) {value=newValue;update();}

private:
  //int xPos=0;
  boolean firstRun = true;
  String colors[8] = {"cyan", "red", "green", "blue", "magenta", "lightgray", "purple", "orange"};
  static int index;
};

// ########################################
//  Dygraph
// ########################################
class Dygraph : public Output
{
public:
  Dygraph (String nn, ElementsHtml *e = 0)
  {
    name = nn;
    id = nn;
    parent = e;
    String s; //="var i=document.createElement('script');i.src='gauge.min.js';\n";
    s += ("window." + id + " = document.getElementById('" + id + "')");
    s += id + " = new Dygraph(window." + id + ",'test.csv');\n"; //config[checkConfig("name")] devuelve el indice

    javaQueue.addOnLoad(s);
  }
  String getHtml()
  {
    String s = "<div id='"+id+"' ></div>";
    //</div><button onclick=btnClickedPlus('"+id+"')>+</button>"
    //"<button onclick=btnClickedMinus('"+id+"')>-</button>"
    //"<button onclick=btnClickedForward('"+id+"')>>></button>"
    //"<button onclick=btnClickedBackward('"+id+"')><<</button></div>";
    return (s);
  }
  String postCallBack(ElementsHtml *e, String postValue) { return parent ? parent->postCallBack(this, postValue) : "null"; }
  void update(float newValue, int dataset = 0)
  {
  
  }
  void update(String sss) { update(sss.toFloat()); }
  void update()
  {
    String str;
    str += "console.log('";
    str += "DYGRAPH_UPDATE() " + id + "');\n";
    javaQueue.add(str);
  }
  // void update( float newValue) {value=newValue;update();}

private:
  //int xPos=0;
  boolean firstRun = true;
  String colors[8] = {"cyan", "red", "green", "blue", "magenta", "lightgray", "purple", "orange"};
  static int index;
};

// ########################################
//  METER
// ########################################

class Meter : public Output
{
public:
  Label *lblValue;
  Meter(String _id, String _name, String unit_, int _min, int _max, ElementsHtml *e = 0)
  {
    name = _name;
    unit = unit_;
    id = _id;
    max = max;
    min = min;
    lblValue = new Label("lbl" + id, "", this);
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
  }
  void setLimits(int _min, int _max)
  {
    min=_min;max=_max;
    javaQueue.addJsonInner(id,"max="+String(_max));
    javaQueue.addJsonInner(id,"min="+String(_min));
    //Serial.println("Setting Meter Limits"+String(_max));
  }
  String getHtml() { return "<meter id='" + id +
                             "' min='" + String(min) + "' max='" + String(max) + "'"+
                             "'></meter> " + unit; }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    else
      return "";
  }
  //void update ( String newValue ) { update(newValue.toInt());}
  void update()
  {
    javaQueue.addJsonInner(id,"value="+String(value));
  }
  void update(float newValue)
  {
    value = newValue;
    update();
  }

private:
  int max, min;
  String unit;
};

// ########################################
//  SLIDER
// ########################################

class Slider : public Output
{
public:
  Label *label;
  Slider(String n, String _name, String unit_, int min, int max, ElementsHtml *e = 0)
  {
    name = _name;
    unit = unit_;
    id = n;
    _max = max;
    _min = min;
    parent = e;
    pushElement(this); // Los elementos basicos se registran solos en el AllHTMLElemens !!
    label = new Label("lbl" + name, "", this);
  }
  String getHtml() { return "<div id='" + id + "'><h5>" + name + "</h5><input type='range' id='" + id + "Meter' min='" + String(_min) + "' max='" + String(_max) + "' onchange='btnClick(this.value)'>" + unit + label->getHtml() + "</div>"; }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (parent)
      return parent->postCallBack(this, postValue);
    value = postValue.toFloat();
    return "";
  }
  //void update ( String newValue ) { update(newValue.toInt());}
  void update()
  {
    javaQueue.addJsonInner(id,"value="+String((int)value));
    label->update(value);
  }
  void update(float newValue)
  {
    value = newValue;
    update();
  }

private:
  int _max, _min;
  String unit;
};

class valueStrategy
{
  virtual float update(float v) { return v; }
};

// ########################################
//  Probe Input
// ########################################
class OxygenCalculation
{
public:
  static float update(float tempCal, int salCal)
  {
    float ts = tempCal + 273.15;
    double result = a1 + a2 * (100 / ts) + a3 * log(ts / 100) + a4 * (ts / 100) + salCal * (b1 + b2 * ts / 100 + b3 * pow((ts / 100), 2));
    float value = exp(result);
    return value;
  }

private:
  constexpr static float a1 = -173.4292, a2 = 249.6339, a3 = 143.3483, a4 = -21.8492;
  constexpr static float b1 = -0.033096, b2 = 0.014259, b3 = -0.0017;
};
