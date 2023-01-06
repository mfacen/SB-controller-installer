// ########################################
//  CLARIFICADOR
// ##################################
class Clarificador : public ElementsHtml
{
public:
  Clarificador(String n, String _id, HardwareOutput *_bomba, HardwareOutput *_ev, Logger *_logger = NULL)
  { // int pinBomba , int pinElectrovalvula ) {
    name = n;
    id = _id;
    bomba = _bomba; // new DigitalOutput ( pinBomba, id+"bomba", true );
    electroV = _ev; // new DigitalOutput ( pinElectrovalvula, id+"ev", true );
    bombaTimer = new GenericTimer(id+"_Bomba", id + "bmb", bomba);
    valvulaPanel = new GenericOutputPanel("Electro Valve", id + "_EV", "", electroV, false,this,false);
    onoffswitch = new SavedEdit("on/off",id + "edt","/status.sta", "checkbox", this);
    //running = new savedVariable(id + "running");
    //running->setFile("/status.sta");
    logger = _logger;
    
  }
  String getHtml()
  {
    return "<div class=' card '><h4>" + 
    name + "</h4><fieldset>" + 
    onoffswitch->getHtml() +
     "<div class='card'>" + 
     bombaTimer->getHtml() + 
     valvulaPanel->getHtml() + 
     "</div></fieldset></div>";
  }
  String postCallBack(ElementsHtml *e, String postValue)
  {
    if (e == onoffswitch)
    {
      // onoffswitch->value== !onoffswitch->value;  
      valvulaPanel->update(!onoffswitch->value);
      onoffswitch->value ? bombaTimer->enable() : bombaTimer->stop();
      //Serial.println(running->value);
      value = onoffswitch->value;
    }

    return "";
  }
  void update()
  {
    if (firstRun)
    {
      onoffswitch->update();
      valvulaPanel->update(onoffswitch->value);
      //logger->addInput(valvulaPanel);    // El problema esta en el MQTT !!!!! Ahora lo he apagado en footer linea 274
      logger->addInput(bombaTimer);
      //Serial.println(logger->name);
      firstRun = false;
    onoffswitch->value ? bombaTimer->enable() : bombaTimer->stop();
    }
    if (!onoffswitch->value) bombaTimer->stop();
    bombaTimer->update();
    valvulaPanel->update(!onoffswitch->value);
    if (bombaTimer->value != lastValue) {logger->logData();lastValue=bombaTimer->value;}
         // Serial.println(onoffswitch->value);

  }
private:
  HardwareOutput *bomba;
  HardwareOutput *electroV;
  GenericTimer *bombaTimer;
  GenericOutputPanel *valvulaPanel;
  SavedEdit *onoffswitch;
  //savedVariable *running;
  Logger *logger;
  float lastValue=0;
};

// ########################################
//  FEEDER
// ##################################

class Feeder : public ElementsHtml
{
public:
  Feeder(String n, String _id, HardwareOutput *pin_Air, HardwareOutput *pin_Feed, Set *_blower, Logger *log)
  {
    name = n;
    id = _id;
    logger = log;
    blower = _blower;
    relayFeed = pin_Feed;
    relayAir = pin_Air;
    btnStart = new Button(id + "bS", "Start", this);
    btnReset = new Button(id + "bR", "Reset", this);
    info = new Label(id + "lblInfo", "", this);
    dispensed = new SavedVariable(id + "Dispensed");
    running = new SavedVariable(id + "running");
    millisStart = new SavedVariable(id + "millisStart");
    feedNumber = new SavedVariable(id + "feedNumber");
    feedTime = new SavedVariable(id + "feedTime");
    iddleTime = new SavedVariable(id + "iddleTime");
    state = new SavedVariable(id + "state");
    // setFeed = new GenericTimer ( "Feed", relayFeed );
    weight = new SavedEdit("Weight", id + "wEdt");
    interval = new SavedEdit("Interval", id + "Int");
    rate = new SavedEdit("Rate", id + "rt");
    servings = new SavedEdit("Servings", id + "srv");
    airTime = new SavedEdit("Air Time", id + "at");
    air = new Set("Air", id + "Air", relayAir);
    feed = new Set("Feed", id + "Feed", relayFeed);
  }
  void setLogger()
  {
    logger->addSavedVariable(dispensed);
    logger->addSavedVariable(state);
  }

  String getHtml()
  {
    return "<div class=' card '><h4>" + name + "</h4><fieldset>" +
           weight->getHtml() + " gr<br>" +
           interval->getHtml() + " h<br>" +
           rate->getHtml() + " gr/s<br>" +
           servings->getHtml() + " u<br>" +
           airTime->getHtml() + " s<br>" +
           btnStart->getHtml() +"<br>"+
           btnReset->getHtml() +"<br>"+
           info->getHtml() +
           "<div class='row'>"+air->getHtml() +
           feed->getHtml() + blower->getHtml() + "</div></fieldset>"+
           "</div>";
  }

  String postCallBack(ElementsHtml *e, String postValue)
  {
    String r = "";
    if (e == btnStart)
    {
      running->update(1);
      millisStart->update(String(millis()));
      Serial.println(servings->name + "->value=" + String(servings->value));
      Serial.println(airTime->name + "->value=" + String(airTime->value));
      feedNumber->update(servings->value);
      feedTime->update((weight->value / servings->value) / rate->value);
      dispensed->update(0);
      iddleTime->update((interval->value * 3600 / servings->value) - feedTime->value);
      Serial.println("iddleTime: " + String(iddleTime->value) + "Feed Time: " + feedTime->value);
      startDispensing();
    }
    if (e == btnReset)
    {
      running->update(0);
      state->update(STATE_DONE);
      air->update(0);
      feed->update(0);
      dispensed->update(0);
      feedNumber->update(0);
    }
    return "";
  }
  void update()
  {
    if (firstRun)
    {
      setLogger();
      weight->update();
      interval->update();
      rate->update();
      servings->update();
      airTime->update();
      firstRun = false;
      millisStart->update(millis()); // Resetear millisStart ????????????????????? Directamente hacer variable normal
      air->update(0);
      feed->update(0);
      servings->value?
        dispensed->update((servings->value - feedNumber->value) * weight->value / servings->value):
        dispensed->update(0);
    }
    //  { weight->update();interval->update();rate->update();running->update();servings->update();
    //             state->update();airTime->update();feedNumber->update();feedTime->update(); iddleTime->update();firstRun=false;}
    if (running->value)
    {
      // Serial.println("Running");
      if (state->value == STATE_DISPENSING)
      {
        if (millis() - millisStart->text.toDouble() > feedTime->value * 1000 * (feedNumber->value==1?1.5:1)) // hace que la ultima sea mas larga
        {
          stopDispensing();
          dispensed->update(dispensed->value + weight->value / servings->value);
        }
      }
      if (state->value == STATE_AIR_ON)
      {
        // air->update(HIGH);
        if (millis() - millisStart->text.toDouble() > airTime->value * 1000)
        {
          stopAir();
          feedNumber->update(feedNumber->value - 1);
          logger->logData();
          if (feedNumber->value == 0)
          {
            Serial.println(id + " Done Feeding...");
            running->update(0);
            state->update(STATE_DONE);
          }
        }
      }
      if (state->value == STATE_IDDLE)
      {
        if (millis() - millisStart->text.toDouble() > iddleTime->value * 1000)
        {
          startDispensing();
        }
      }
    }
    // dispensed->update(23);

    air->update();
    feed->update();
    info->update("State:" + String(state->value) + " <br>Feeds Left:" + String(feedNumber->value ) +
                 "<br>Dispensed: " + String(dispensed->value) + " <br>FeedTime: " + feedTime->text + "<br>" +
                 "Iddle Time: " + String(iddleTime->value) + "<br>");
  }


  void startDispensing()
  {
    Serial.println(id + " StartDispensing()");
    // relayFeed->update(HIGH);
    feed->update(HIGH);
    millisStart->update(String(millis()));
    state->update(STATE_DISPENSING);
    logger->logData();
    startAir();
  }
  void startAir()
  {
    temp_interval = logger->getInterval();
    logger->setInterval(1);
    air->update(HIGH);
    blower->update(HIGH);
    Serial.println(id + " Start Air()");
  }
  void stopAir()
  {
    air->update(LOW);
    blower->update(LOW);
    state->update(STATE_IDDLE);
    logger->logData();
    Serial.println(id + " Stop Air()");
  }
  void stopDispensing()
  {
    Serial.println(id + " Stop Dispensing");
    feed->update(LOW);
    // flushMillisStart = millis();
    state->update(STATE_AIR_ON);
    logger->logData();
    logger->setInterval(temp_interval);
    millisStart->update(String(millis()));
  }
private:
  HardwareOutput *relayFeed;
  HardwareOutput *relayAir;
  Set *blower;
  SavedVariable *dispensed; // used for Output to Logger
  Button *btnStart;
  Button *btnReset;
  Logger *logger;
  Label *info;
  SavedEdit *weight;
  SavedEdit *interval;
  SavedEdit *rate;
  SavedEdit *servings;
  SavedEdit *airTime;
  SavedVariable *running;
  SavedVariable *millisStart;
  SavedVariable *feedNumber;
  SavedVariable *feedTime;
  SavedVariable *iddleTime;
  SavedVariable *state;
  int temp_interval;
  Set *air, *feed;
  enum states
  {
    STATE_IDDLE,
    STATE_DISPENSING,
    STATE_AIR_ON,
    STATE_DONE
  };
};

//#######################################################################
//                SPEED CONTROL
//#######################################################################

class Speed_Control : public ElementsHtml
{
public:
  Speed_Control(String _name, String _id, HardwareOutput *_hardwareOutput, HardwareInput *_hardwareInput, Logger *_logger)
  {
    logger = _logger;
    name = _name;
    id = _id;
    vent_bars = _hardwareInput;
    pressure = new GenericInputPanel( id + "mbar", "mBar", vent_bars,true,true,this,false);
    //oxigen = new GenericOutputPanel("Oxigeno", id + "ox", "mg/l", oxi,this);
    speedCtrl = _hardwareOutput;
    fakeOut = new FakeOutput();
    speedCtrlPanel = new GenericOutputPanel(id+"_speed", id + "spd", "%", speedCtrl,false,this,false);
    tmrVenturi = new GenericTimer("Venturi Timer", id + "tVen", fakeOut,this);
    edtMinOxy = new EditBox(id + "MinOxy", "MinimumOxygen", "number");
    edtSetting = new SavedEdit("Set_"+id, id + "edtSet", "/status.sta", "number");
    edtSetting->style = " class='numInp' ";
    myPID = new PID(&input, &output, &setpoint, .05, .01, 0, DIRECT);
    vfd_names[0]="Soyan";vfd_names[1]="Mollom";
    cmbVFD_Type = new ComboBox ( id+"vfdType",2,vfd_names,this);
    //PID_GUI = new PID_Module ( id+"pid", myPID );
        //logger->addInput(pressure);

  }
  void setUpLogger()
  {
    logger->addInput(speedCtrlPanel);
    //logger->addInput(oxigen);
    logger->addInput(edtSetting);
    //Serial.println("Press id:" +pressure->id);
    logger->addInput(pressure);
  }
  GenericInputPanel* getPressure(){return pressure;}
  void init()
  {
    myPID->SetMode(AUTOMATIC);
    myPID->SetOutputLimits(0, 100);
    //oxigen->update(6);
    // pressure->update(10);
  }
  String getHtml()
  {
    String s = "<div class=' card '><h4>" + String(name) + "</h4><fieldset>";
    s += pressure->getHtml();
    s += edtSetting->getHtml();
    s += speedCtrlPanel->getHtml();
    s += "VFD Type: "+cmbVFD_Type->getHtml();
    //s += PID_GUI->getHtml();
    s += tmrVenturi->getHtml();
    //s += oxigen->getHtml();
    // s+=edtMinOxy->getHtml();
    s += "</fieldset></div>";
    return s;
  }
  void update()
  {
    if (firstRun)
    {
      setUpLogger();
      firstRun = false;
      setpoint = vent_bars->value;
      edtSetting->update();
      vfd->setType(cmbVFD_Type->value);
    }
    tmrVenturi->update();
    pressure->update();
    speedCtrlPanel->update();
    vent_bars->update();
    if ((!tmrVenturi->value) )// && (oxigen->value > 4))
    {
      input = vent_bars->value;
      setpoint = edtSetting->value;
      myPID->Compute();
      // Serial.print(name);
      // Serial.print(": ");
      //  Serial.print(input);
      // Serial.print(": ");
      //  Serial.print(setpoint);
      // Serial.print(": ");
      //  Serial.print(output);
      //  Serial.println();
      speedCtrlPanel->update(output);
    }
    else
      speedCtrlPanel->update(100);
  }
  String postCallBack(ElementsHtml *e, String postValue ) {
        Serial.println("postCallBack SpeedControl "+postValue);

     if (e==cmbVFD_Type && vfd!=nullptr) vfd->setType(postValue.toInt());
     return "";
  }
  void setVfd (ModbusVFD *_vfd){vfd=_vfd;}
  ElementsHtml *searchById(String _id)
  {
    if (_id == pressure->id)
      return pressure;
    
    else if (_id == speedCtrlPanel->id )  // este en realidad no hace falta, solo para los que 
      return speedCtrlPanel;              // hacen update en a travez del rs485
    return NULL;
  }
  String returnDebugValue(){ return String(pressure->value)+"-"+String(speedCtrlPanel->value);}
  //float getPressure(){return pressure->value;}

  FakeOutput *fakeOut;
private:
  GenericInputPanel *pressure;
  HardwareInput *vent_bars;
  GenericOutputPanel *oxigen;
  HardwareOutput *speedCtrl;
  GenericOutputPanel *speedCtrlPanel;
  GenericTimer *tmrVenturi;
  EditBox *edtMinOxy;
  SavedEdit *edtSetting;
  Logger *logger;
  PID *myPID;
  ComboBox *cmbVFD_Type;
  //PID_Module *PID_GUI;
  double input, output, setpoint;
    String vfd_names[2]; 
  ModbusVFD *vfd;
};

//#######################################################################
//                TEMPERATURE CONTROL
//#######################################################################
class TempControl : public ElementsHtml
{
public:
  TempControl(String _id, String _name,int pin_rly, int pin_tempProbe, Logger *_logger)
  {
    id = _id;
    name = _name;
    relay = new DigitalOutput(pin_rly, id + "rly");
    tempProbe = new Dsb18B20(pin_tempProbe);
    edtSet = new SavedEdit("Setpoint", id + "Set", "/status.sta", "number");
    pwm = new PWM(60000, relay);
    set = new Set("relay", "rly", relay);
    myPID = new PID(&input, &output, &setpoint, .5, .0005, .5, DIRECT);
    pid_module = new PID_Module("pid", myPID);
    logger = _logger;
    panOut = new GenericOutputPanel ("Output",id+ "pnlOut", "%");
  }
  void loop(){
    myPID->Compute();
  }
  void update()
  {
    if (firstRun)
    {
      myPID->SetMode(AUTOMATIC);
      myPID->SetOutputLimits(0, 60);
      edtSet->update();
      pid_module->updateTuning();
    }
    tempProbe->update();
    pwm->update();
    input = tempProbe->value;
    setpoint = edtSet->value;
    //myPID->Compute();
    if (output < .1)
      pwm->setLevel(0);
    else
      pwm->setLevel(output);
    panOut->update(String(output));
    lastCheck = millis();
    set->update(relay->value);
    //Serial.println(input);
    //Serial.println(setpoint);
    //Serial.println(output);
  }
  String getHtml()
  {
    String s = "<div class='card'><h4>" + name + "<h4><fieldset>";
    s += set->getHtml();
    s += edtSet->getHtml();
    s+= panOut->getHtml();
    s += pid_module->getHtml();
    s += "</fieldset></div>";
    return s;
  }

private:
  DigitalOutput *relay;
  Dsb18B20 *tempProbe;
  SavedEdit *edtSet;
  GenericOutputPanel *panOut;
  GenericEditPanel *edtSeting;
  PWM *pwm;
  Set *set;
  unsigned long lastCheck;
  double input, output, setpoint;
  PID *myPID;
  PID_Module *pid_module;
  Logger *logger;
};