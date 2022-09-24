#include <MQTT.h>
//#include <PubSubClient.h>


class HardwareInput// : public loggableInterface
{
public:
  float factor = 1;
  float value = 0;
  virtual void update();
  float getValue(){return value;}
   void update(float f){ if (value!=f) {value=f;}}
   float getSecondValue() { return 0; }
   int typeID = 0;
};


// ########################################
//  Analog IN
// ########################################
class AnalogIn : public HardwareInput
{
public:
  AnalogIn(int _pin)
  {
    pin = _pin; 
    
  }
  void update()
  {
    if (firstRun)
    {
      value = analogRead(pin);
      //Serial.println("Reading analog pin "+String(pin)+" value:"+String(value));
    }
  }

private:
  int pin;
  bool firstRun = true;
};


void onMqttMsg ( String &topic, String &payload){

}
// ########################################
//  MQTT Device
// ########################################
class MQTT_Device: public HardwareInput {

  public:
    float ff;
   MQTT_Device (String s ) { id = s;}
    void begin ( String m, int p , String _topic) {
        //mqtt.setOptions(10000, true, 15000);  // Keep alive, timeout, etc.
        mqtt.begin(m.c_str(),p,wifiClient) ;

        Serial.println(mqtt.connect(id.c_str(),"pi","pass"));
        mqtt.subscribe(_topic);
        float &_value = value;
        // mqtt.onMessage([this](String &topic, String &payload){ // this en el capture es para pasar el objeto y sus miembros
        //   value=payload.toFloat();
        //   Serial.println("Received new Value by MQTT "+id+" : "+String(value));
        // });
        //Serial.println("MQTT STARTED FROM OBJECT "+id+" subscribed to "+_topic);
      }
    void update(){
      if (lastValue!=value){
        lastValue = value;
      }
        //Serial.println("MQTT STATUS "+ String(mqtt.connected()) + ":" + String(value));
    }

  void update(float v){value = v;}

    MQTTClient mqtt;
  private:
    String id;
    WiFiClient wifiClient;
//    PubSubClient mqtt(&wifiClient);
    bool loaded = false;
    float lastValue;
};


// ########################################
//  Modbus Device
// ########################################
class Modbus_Device: public HardwareInput {

  public:

   Modbus_Device (String _id,int _slaveID ) { id = _id;value=-1;slaveID=_slaveID;list.push_back(this);}
   
    void update(){
      #ifdef ESP32
      modbus.readHoldingRegisters(slaveID,17,1);
      #endif
      //if (lastValue!=value){
            // int error= 1;//(!nodeRelays.readHoldingRegisters(17,1,slaveID));//17 es A0 en esp8266
            //     if(!error)      {value=0;//(nodeRelays.getResponseBuffer(0));
            //                       Serial.println("Success writen to modbus, value = "+String(value));
            //                     //nodeRelays.clearResponseBuffer();
            //                     }
            //     else         Serial.println("Error: "+String(error));
        //Serial.println("Serial sent from RS485 device "+ id+ ":" + String(value));
      //}
    }

  void update(float v){value = v;}
  int getSlaveId (){return slaveID;}
  static void checkSlaveIDs(int _id,float _val){
    Serial.printf("Size %02x",list.size());
    Serial.println("IOOIHOIHOIHJO");
    for (unsigned int i = 0; i < list.size(); i++)
    {
      Serial.println(list[i]->id);
      if (list[i]->getSlaveId() == _id)
      {
         list[i]->update(_val);
         Serial.println("Updating from modbus "+list[i]->id);
      };
    };
    
    }
  private:
    String id;
    int slaveID;
    
//    PubSubClient mqtt(&wifiClient);
    float lastValue;
    static std::vector <Modbus_Device*> list;
};
std::vector<Modbus_Device *> Modbus_Device::list;


// ########################################
//  EZO_Oxygen
// ########################################
class EZO_Oxygen : public HardwareInput
{

public:
  void update()
  {
    if (!O2Requested)
    { 

      Wire.beginTransmission(DO_I2C_ADDRESS); // transmit to device OD probe   ( takes 600ms )   DECIA 33 ANTES
      Wire.write("R");            //  I2C address=   34 old,   97   new
      Wire.endTransmission();     // stop transmitting        O2Requested=true;
      lastO2Request = millis();
      //Serial.println("O2-Requested");
      O2Requested = true;
    }
    if ((millis() - lastO2Request > intervalO2) && O2Requested)
    {
      //Serial.println("Into Loop");
      Wire.requestFrom(DO_I2C_ADDRESS, 10); // request 10 bytes from OD

      if (Wire.available())
      {
        char response = Wire.read(); // receive a byte as character
        if (response == 1)
        {
          String resp;
          while (Wire.available())
          {
            resp = resp + char((Wire.read()));// Aqui he cambiado !!!! Antes no decia String pero me comenzo a dar error.
          }
          value = avg.addValue(resp.toFloat());  // get the average value of oxygen
          //value = resp.toFloat();
          //Serial.println("Response :"+resp);
          oxygenPercent = resp.substring(resp.indexOf(",") + 1, resp.length()).toFloat();
          //Serial.print("DO: ");Serial.println(value);
        }
      }
      O2Requested = false;
    }
  }
  float getSecondValue()
  {
    return oxygenPercent;
    Serial.println("*");
  }

private:
  boolean firstRun = true;
  bool O2Requested = false;
  unsigned long lastO2Request;
  int intervalO2 = 700;
  float oxygenPercent;
  AverageModule avg;
  //int i2cAddress = 33; // 97   El jellyfish nuevo usa otras direcciones
};

// ########################################
//  EZO_EC
// ########################################
class EZO_EC : public HardwareInput
{

public:
  void update()
  {
    if (!ECRequested)
    {
      //Wire.begin(3,2);
      Wire.beginTransmission(EC_I2C_ADDRESS); // transmit to device OD probe   ( takes 600ms )  AQUI DECIA 34
      Wire.write("R");            // I2c Address:           34 - viejo  ----   100 nuevo
      Wire.endTransmission();     // stop transmitting        O2Requested=true;
      lastECRequest = millis();
      //Serial.println("EC-Requested");
      ECRequested = true;
    }
    if ((millis() - lastECRequest > intervalEC) && ECRequested)
    {
      Wire.requestFrom(EC_I2C_ADDRESS, 10); // request 10 bytes from OD

      if (Wire.available())
      {
        //Serial.println("EC-Response");

        char response = Wire.read(); // receive a byte as character
        if (response == 1)
        {
          String resp;
          while (Wire.available())
          {
            resp = resp + char(Wire.read()); // Aqui he cambiado !!!! Antes no decia String pero me comenzo a dar error.
          }
          //Serial.println(resp);
          siemmens = resp.toInt();

          value = resp.substring(resp.indexOf(",") + 1, resp.length()).toFloat();
          //Serial.println("EC VALUE: "+String(value));
        }
      }
      ECRequested = false;
    }
  }

private:
  boolean firstRun = true;
  bool ECRequested = false;
  unsigned long lastECRequest;
  int intervalEC = 700;
  float siemmens = 0;
};
// ########################################
//  EZO_PH
// ########################################
class EZO_PH : public HardwareInput
{

public:
  void update()
  {
    if (!PHRequested)
    {
      //Wire.begin(3,2);
      Wire.beginTransmission(PH_I2C_ADDRESS); // transmit to device OD probe   ( takes 600ms )
      Wire.write("R");            // sends instruction byte
      Wire.endTransmission();     // stop transmitting        O2Requested=true;
      lastPHRequest = millis();
      //Serial.println("EC-Requested");
      PHRequested = true;
    }
    if ((millis() - lastPHRequest > intervalPH) && PHRequested)
    {
      Wire.requestFrom(PH_I2C_ADDRESS, 10); // request 10 bytes from OD

      if (Wire.available())
      {
        //Serial.println("EC-Response");

        char response = Wire.read(); // receive a byte as character
        if (response == 1)
        {
          String resp;
          while (Wire.available())
          {
            resp = resp + char(Wire.read()); // Aqui he cambiado !!!! Antes no decia String pero me comenzo a dar error.
          }
          siemmens = resp.toInt();
//Serial.println(resp);
          value = resp.substring(resp.indexOf(",") + 1, resp.length()).toFloat();
          //Serial.println("EC VALUE: "+String(value));
        }
      }
      PHRequested = false;
    }
  }

private:
  boolean firstRun = true;
  bool PHRequested = false;
  unsigned long lastPHRequest;
  int intervalPH = 900;
  float siemmens = 0;
};

// ########################################
//  BMP180
// ########################################
class Bmp180 : public HardwareInput
{

public:  //           Doesnt have a constructor, uses Wire by default
  enum PRESSURE_UNIT { HPA,MMHG,PSI,mBAR};
  Bmp180 (int _unit){unit=_unit;}
  void update()
  {
    if (firstRun)
    {
      Wire.begin(SDA,SCL);
      
      Serial.println("TryingToStart BMP180");
      if (bmp.begin()){Serial.println("Started BMP180");firstRun = false;}
      else Serial.println(" BMP180 not found");
      
    }
    if (!firstRun) { value = bmp.readPressure();
    switch (unit){
      case PRESSURE_UNIT::HPA:
        value = value; // hPa
        break;
      case PRESSURE_UNIT::MMHG:
        value = .00750063 * value; // mmHg
        break;
      case PRESSURE_UNIT::PSI:
        value = 0.01450377 * value; // Psi
      case PRESSURE_UNIT::mBAR:
        value = 0.01 * value; // Bar
        } 

    }

    //Serial.println(value);
  }
  float getSecondValue() { if(firstRun){return 0;}else{return bmp.readTemperature();} }
  void setUnit(int _unit){unit=_unit;}
protected:
  boolean firstRun = true;
  Adafruit_BMP085 bmp;
  int unit=PRESSURE_UNIT::MMHG;
};

class Bmp180Temp : public HardwareInput
{
public:
  Bmp180Temp(Bmp180 *_bmp){bmp = _bmp;}
  void update()
  {
    if (firstRun)
    {
      firstRun = false;
      
    }
    value = bmp->getSecondValue(); // hPa = hectoPascales
                                   //  mmHg = .750062 * KPa
  }
private:
  Bmp180 *bmp;
  bool firstRun=true;
};


// ########################################
//  HX711
// ########################################
class HX711Component : public HardwareInput
{

public:


      void setup(int dataPin, int clockPin) {
   Dout = dataPin;
    Clock = clockPin;
  pinMode(Clock, OUTPUT); // initialize digital pin 4 as an output.(clock)
  digitalWrite(Clock, HIGH); 
  delayMicroseconds(100);   //be sure to go into sleep mode if > 60µs
  digitalWrite(Clock, LOW);     //exit sleep mode*/
  pinMode(Dout, INPUT);  // initialize digital pin 5 as an input.(data Out)
    firstRun = false;
      }
  void update()
  { 
      //Serial.println(digitalRead(Dout));
    if (digitalRead(Dout) == LOW ) {
     Weight =0;
    // pulse the clock pin 24 times to read the data
    for (char i = 0; i<24; i++)
    {
      digitalWrite(Clock, HIGH);
      delayMicroseconds(2);
      Weight = Weight <<1;
      if (digitalRead(Dout)==HIGH) Weight++;
      digitalWrite(Clock, LOW);
    }
    // set the channel and the gain factor (A 128) for the next reading using the clock pin (one pulse)
    digitalWrite(Clock, HIGH);
    Weight = Weight ^ 0x800000;
    digitalWrite(Clock, LOW);

      Serial.println(Weight);
    
    if (useAverage) {
      avg.addValue(Weight);
      value = avg.getAverage();
        }    
    else { value = Weight;}
    }
    // if (firstRun)

    // {
    //   firstRun = false;   // todavia no se si hay que setear esto cada vez o otro sensor que use WIRE setea los pins diferentes
    //   hx711.begin(dataPin,clockPin);
    // }
    //if (hx711.is_ready()) value = hjhux711.read_average(); // 
    
    ;
  }

protected:
  boolean firstRun = true;
  long Weight = 0;
  AverageModule avg;
  bool useAverage = false;
  //HX711 hx711;
  //Q2HX711 *hx711;//                      =HX711(PIN_DOUT,PIN_SCK);
  int Dout =SDA;
  int Clock= SCL;
};

// ########################################
//  DSB 18B20 IN
// ########################################
class Dsb18b20a : public HardwareInput{
  public:
  Dsb18b20a ( int pin){
    pinMode(pin, INPUT_PULLUP);
    oneWire = new OneWire(pin);                   // Set up a OneWire instance to communicate with OneWire devices
    tempSensors = new DallasTemperature(oneWire); // Create an instance of the temperature sensor class
    tempSensors->setWaitForConversion(false);     //  dont block the program while the temperature sensor is reading
    tempSensors->begin(); }
     void update()
  {
    if (!tempRequested)
    {
      tempSensors->requestTemperatures(); // Request the temperature from the sensor (it takes some time to read it)
      tempRequested = true;
      lastTemperatureRequest = millis();
      //Serial.println("temp requested");
    }
    if ((millis() - lastTemperatureRequest > intervalTemperature) && tempRequested)
    {
      //Serial.println(tempSensors->getDeviceCount());
      value = tempSensors->getTempCByIndex(0) * factor; // Get the temperature from the sensor
      tempRequested = false;
    //value = tempSensors->getTempCByIndex(0);
    }
  }
  float getTempAndBlock()
  {
    tempSensors->setWaitForConversion(true); //  block the program while the temperature sensor is reading
    tempSensors->begin();
    tempSensors->requestTemperatures();
    return value = tempSensors->getTempCByIndex(0);
  }
protected:
  boolean firstRun = true;
    int pin;
  DallasTemperature *tempSensors;
  OneWire *oneWire;
    bool tempRequested = false;
  unsigned long lastTemperatureRequest;
  int intervalTemperature = 1500;
};


//########################################
// DHT11 IN
//########################################

class Dht11: public HardwareInput {
    public:

    int pin;
   DHTesp  dht;
    Dht11(){};
    Dht11 ( int _pin , String  _name) {
      pin = _pin;
       dht.setup( pin, DHTesp::DHT11); // Connect DHT sensor to pin
    }

    void update() {
       if ( ( millis() - lastTemperatureRequest) > intervalTemperature ) {
        value = dht.getTemperature(); // Request the temperature from the sensor (it takes some time to read it)
        humidity = dht.getHumidity();
        lastTemperatureRequest = millis();
      }
    }

  float getValue(){return value;}
  float getHumidity(){return humidity;}
  
  //private:
  bool tempRequested = false;
  unsigned long lastTemperatureRequest=0;
  int intervalTemperature = 2000;
  float humidity=0;
};
class DhtHumidity :public Dht11 {
  public:
    DhtHumidity(Dht11 *_dht ){
      dht=_dht;
    }
    void update() {
       if ( ( millis() - lastTemperatureRequest) > intervalTemperature ) {
        value = dht->getHumidity();
        lastTemperatureRequest = millis();
      }
    }
  private:
    Dht11 *dht;
};
#ifdef ESP8266
// ########################################
//  Energy Monitor Component
// ########################################
class Energy_Monitor: public HardwareInput{
  public:
    Energy_Monitor(int _pin){ pin = _pin;}
    void update(){
      if (firstRun) { emon.current ( pin , 6 ); firstRun=false;}
      value = emon.calcIrms(1480);
      }  // este 1480 de que es ???
  private:
    EnergyMonitor emon;
    bool firstRun = true;
    int pin;
};
#endif

// ########################################
//  ADS1115 Component
// ########################################

class ADS1115_Component : public HardwareInput
{
public:
  char writeBuf[4];
  ADS1115_Component(String s , int pinData, int pinClock )
  {
    dataPin = pinData;
    clkPin = pinClock;
  } // initialize ADS1115 16 bit A/D chip}
  void update(){value=readChannel(0);};
  float readChannel(int channel)
  {
    if (firstRun)
    {
      firstRun = false;
      //Wire.begin (D6,D7); // Esto es para el Jellyfish
      //Wire.begin(D1,D2);
      Wire.begin(dataPin,clkPin); // aqui decia D6,D7
      writeBuf[0] = 1;          // config register is 1
      writeBuf[1] = 0b11000010; // 0xC2 single shot off Aqui hay que cambiar para aceptar el correcto
      // bit 15 flag bit for single shot
      // Bits 14-12 input selection:
      // 100 ANC0; 101 ANC1; 110 ANC2; 111 ANC3
      // Bits 11-9 Amp gain. Default to 010 here 001 P19
      // 000: FS= ±6.144V(1)     100: FS= ±0.512V       001: FS= ±4.096V(1)
      // 101: FS= ±0.256V        010: FS= ±2.048V(default)
      // 110: FS= ±0.256V        011: FS= ±1.024V111: FS= ±0.256V
      
      // Bit 8 Operational mode of the ADS1115.
      // 0 : Continuous conversion mode
      // 1 : Power-down single-shot mode (default)

      writeBuf[2] = 0b10000101; // bits 7-0  0x85
      // Bits 7-5 data rate default to 100 for 128SPS
      // Bits 4-0  comparator functions see spec sheet.

      // setup ADS1115
          // ASD1115
      // set config register and start conversion
      // ANC1 and GND, 4.096v, 128s/s
            //ads.setRate(ADS1115_RATE_8);
      //ads.setMultiplexer(ADS1115_MUX_P0_NG);
      //ads.setMode(ADS1115_MODE_SINGLESHOT);
      Wire.beginTransmission(0x48); // ADC
      Wire.write(writeBuf[0]);
      Wire.write(writeBuf[1]);
      Wire.write(writeBuf[2]);
      Wire.endTransmission();

    }
    writeBuf[0] = 0;              // pointer
    Wire.beginTransmission(0x48); // DAC
    Wire.write(writeBuf[0]);      // pointer
    Wire.endTransmission();

    Wire.requestFrom(0x48,2);
    writeBuf[1] = Wire.read(); //
    writeBuf[2] = Wire.read(); //
    Wire.endTransmission();

    // convert display results
    value = writeBuf[1] << 8 | writeBuf[2];

    //Serial.println(value);
    //return adsRead();
    return value;
  }

  float adsRead(){
    if (firstRun){
         // ads.setGain(GAIN_TWOTHIRDS) ;
     // ads.begin();
  

  }
    //value = (int16_t) ads.readADC_SingleEnded(0);

  }

  String postCallBack(ElementsHtml *e, String postValue) { return ""; }
  void setGain(adsGain_t gain) { 
    //ads.setGain(gain); }
  }
private:
  /////ADS1115 ads;
  //Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
  int dataPin;
  int clkPin;
  boolean firstRun = true;
};



// ########################################
//  ULTRA SOUND PROBE
// ########################################

class UltraSoundProbe : public HardwareInput
{
public:
  UltraSoundProbe(int _pinTrig, int _pinEcho)
  {
    trigPin = _pinTrig;
    echoPin = _pinEcho;
  }

  void update()
  {
    if (firstRun){    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);  digitalWrite(trigPin, LOW);firstRun=false;}
    else
    value = getDistance();


  }

  float getDistance()
  {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(trigPin, LOW);

    // Measure the response from the HC-SR04 Echo Pin

    long duration = pulseIn(echoPin, HIGH, 30000);

    // Determine distance from duration
    // Use 343 metres per second as speed of sound

    return (duration / 2) * 0.0349; // 0.0349 AIR , 0.1530 SEA WATER
  }

private:
  int trigPin;
  int echoPin;
  bool firstRun=true;
};
