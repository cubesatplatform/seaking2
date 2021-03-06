#include "cubesat.h"

void CSatellite::loop() {  
    lcount++;

    MsgPump();  
    pstate->loop();     
    state_core.loop();   
    if(pstate->outOfTime()){
      if(pstate->Name()=="LOWPOWER")
        goNormalState();
      else
        goLowPowerState();
    }
  }

CSatellite::CSatellite() {
  Name("SAT");
	pstate = &state_normal;
	}


void CSatellite::newState(CMsg &msg) {    
  std::string s=msg.getVALUE();
  if(s.size()>1){
    CStateObj *tmpstate=(CStateObj *)getSystem(s.c_str(),"CSatellite::newState(CMsg &msg)");

    if((tmpstate!=NULL)&&(tmpstate!=pstate)){  //Don't reset if you are already in that state        
      pstate->exit();
      pstate=tmpstate;
      pstate->stateMsg(msg);  //Passes parameters of what you want the state to do
      pstate->enter();
    }
  }
}

void CSatellite::addState(CMsg &msg){
  std::string s=msg.getVALUE();
  long lmax=msg.getParameter("MAXTIME",3*TIMEORBIT);
  if(s.size()>1){
    CStateObj *tmpstate=(CStateObj *)getSystem(s.c_str(),"CSatellite::addState(CMsg &msg)");
    CStateObj *pSO=new CStateObj();
    pSO->Name(s);
    pSO->setForever(false);
    pSO->setMaxTime(lmax);
  }  
}



void CSatellite::addSystem(CMsg &msg) {
  std::string strstate=msg.getParameter("STATE");
  std::string strsystem=msg.getParameter("SYSTEM");

  CSystemObject *psys=getSystem(strsystem.c_str(),"addSystem(CMsg &msg)  System");
  CStateObj *pstate=(CStateObj *)getSystem(strstate.c_str(),"addSystem(CMsg &msg)  State");
  
  if((psys!=nullptr)&&(pstate!=nullptr)){    
     pstate->addSystem(psys);    
    }
}

void CSatellite::stats(){  
  CMsg msg;
  msg.setTABLE("STATS");
  msg.setParameter("TIME",getTime());
  
  msg.setParameter("RESETS",_restartcount);
  msg.setParameter("BURNS",state_deployantenna._burncount);
  msg.setParameter("DTBL",state_detumble._detumblecount);
  
  msg+=state_core.stats();  
  msg+=pstate->stats();  

  addTransmitList(msg);    
}

void CSatellite::setup() {    //Anything not in a loop must be setup manually  or have setup done automatically when called


  state_adcs.Name("ADCS");
  state_adcs.setMaxTime(3*TIMEORBIT);
  state_adcs.availablesystems["MT"] = true;
  state_adcs.availablesystems["MAGX"] = true;
  state_adcs.availablesystems["MAGY"] = true;
  state_adcs.availablesystems["MAGZ"] = true;
  state_adcs.availablesystems["MOTORX"] = true;
  state_adcs.availablesystems["MOTORY"] = true;
  state_adcs.availablesystems["MOTORZ"] = true;

  state_core.Name("CORE");
  state_core.setForever();

  state_normal.Name("NORMAL");
  state_normal.setMaxTime(NORMALMAXTIME);
  state_normal.availablesystems["GPS"] = true;
  state_normal.availablesystems["DATAREQUEST"] = true;
  state_normal.onEnter["ENABLESENSORS"]=true;
  state_normal.onExit["DISABLESENSORS"]=true;

  state_deployantenna.Name("DEPLOY");
  state_deployantenna.setMaxTime(10000);
  state_deployantenna.onEnter["ENABLEBURNWIRE"]=true;
  state_deployantenna.onExit["DISABLEBURNWIRE"]=true;


  state_adcs.Name("ADCS");
  state_adcs.setMaxTime(2*TIMEORBIT);
  state_adcs.onEnter["ENABLE65V"]=true;
  state_adcs.onEnter["ENABLEMBLOGIC"]=true;
  state_adcs.onExit["DISABLE65V"]=true;
  state_adcs.onExit["DISABLEMBlogic"]=true;

  
  state_lowpower.Name("LOWPOWER");
  state_lowpower.setMaxTime(LOWPOWERMAXTIME);
  state_lowpower.onEnter["DISABLE65V"]=true;
  state_lowpower.onEnter["DISABLEBURNWIRE"]=true;
  state_lowpower.onEnter["DISABLESENSORS"]=true;
  state_lowpower.onEnter["DISABLEPHONE"]=true;

  
  Radio.Name("RADIO");
  Radio.setTransmitter(true);
  Radio.setReceiver(false);


  #if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
    Radio2.Name("RADIO2");
    Radio2.setTransmitter(false);
    state_core.addSystem(&Radio2);
  #endif


  state_core.addSystem(&Radio);  
  state_core.addSystem(&Mgr);    

//  state_normal.addSystem(&MotorX);  
  
  state_payload.addSystem(&Phone);  
  //state_core.addSystem(&RW);    //RW needs to be in core because if you are running it you cant switch states and turn it off
  //state_lowpower.addSystem(&Delay);   //May be a bad idea!!!


  IMUI2C.Name("IMUI2C");   
  IMUSPI.Name("IMUSPI");

  IMUI2C.remap();
  IMUSPI.remap();
  
  IRX1.Name("IRX1");
  IRX2.Name("IRX2");
  IRY1.Name("IRY1");
  IRY2.Name("IRY2");
  IRZ1.Name("IRZ1");
  IRZ2.Name("IRZ2");
  
  MagX.Name("MAGX");
  MagY.Name("MAGY");
  MagZ.Name("MAGZ");
  
  MotorX.Name("MOTORX");
  MotorY.Name("MOTORY");
  MotorZ.Name("MOTORZ");

  MotorX.config(MOTOR_X_SPEED,MOTOR_X_FG,MOTOR_X_DIR);
  MotorY.config(MOTOR_Y_SPEED,MOTOR_Y_FG,MOTOR_Y_DIR);
  MotorZ.config(MOTOR_Z_SPEED,MOTOR_Z_FG,MOTOR_Z_DIR);


  TempX1.Name("TEMPX1");
  TempX2.Name("TEMPX2");
  TempY1.Name("TEMPY1");
  TempY2.Name("TEMPY2");
  TempZ1.Name("TEMPZ1");
  TempZ2.Name("TEMPZ2");
  TempOBC.Name("TEMPOBC");  

     
  IRX1.config(IRARRAY_ADDRESS_X1,&Wire);
  IRX2.config(IRARRAY_ADDRESS_X2,&Wire);
  IRY1.config(IRARRAY_ADDRESS_Y1,&Wire1);
  IRY2.config(IRARRAY_ADDRESS_Y2,&Wire1);
  IRZ1.config(IRARRAY_ADDRESS_Z1,&Wire2);
  IRZ2.config(IRARRAY_ADDRESS_Z2,&Wire2);

  IMUI2C.config(IMUADDRESS1,&Wire);

  TempOBC.config(TEMP_OBC,&Wire);
  TempX1.config(TEMP_X1,&Wire);
  TempX2.config(TEMP_X2,&Wire);
  TempY1.config(TEMP_Y1,&Wire1);
  TempY2.config(TEMP_Y2,&Wire1);
  TempZ1.config(TEMP_Z1,&Wire2);
  TempZ2.config(TEMP_Z2,&Wire2);

  MagX.config(MAG_ADDRESS_X,&Wire2);  
  MagY.config(MAG_ADDRESS_Y,&Wire2);  
  MagZ.config(MAG_ADDRESS_Z,&Wire2);

/*             
  state_detumble.addSystem(&IMUI2C);   
  state_detumble.addSystem(&IMUSPI);   
  state_detumble.addSystem(&MT);
      
  state_adcs.addSystem(&RW);
  state_adcs.addSystem(&IMUI2C);   
  state_adcs.addSystem(&IMUSPI);
  
  state_payload.addSystem(&Phone);
  
  state_core.addSystem(&Radio2);
  state_core.addSystem(&Power);
  state_core.setup();  
  */

 mountFS();
 delay(100);
readCounts();  
_restartcount++;
writeCounts();  
}

void CSatellite::readCounts() {
  msgCounts.deserializeFile(SATCOUNTS_FILE);

  _restartcount=msgCounts.getParameter("RESTARTS",_restartcount);
  state_deployantenna._burncount=msgCounts.getParameter("BURNS",state_deployantenna._burncount);
  state_detumble._detumblecount=msgCounts.getParameter("DETUMBLES",state_detumble._detumblecount);

  
  msgCounts.writetoconsole();
  addTransmitList(msgCounts);   
}


void CSatellite::sendCounts() {  
  addTransmitList(msgCounts);   
}



void CSatellite::writeCounts() {  
  msgCounts.setSYS("satcounts");
  msgCounts.setParameter("RESTARTS",_restartcount);
  msgCounts.setParameter("BURNS",state_deployantenna._burncount);
  msgCounts.setParameter("DETUMBLES",state_detumble._detumblecount);
  std::string fn=msgCounts.serializeFile(SATCOUNTS_FILE);
  addTransmitList(msgCounts);   
}


void CSatellite::MsgPump() {
	//Gets messages receieved from radio, pushes to message list and then pumps them out
  CMsg msg;
	MSG.moveReceived();
  int count=0;
  while(  MSG.MessageList.size()){
    writeconsoleln("MsgPump()");
    count++;
    if(count>20)
      break;
    msg = MSG.MessageList.front();
    MSG.MessageList.pop_front();
    if(msg.Parameters.size()){      
      newMsg(msg);   //Satellite      
    }
	}

	MSG.MessageList.clear();//Probable make sure messages have all been processed.  I think they will as only thing that can add messages should be the loop
}


void CSatellite::updateRadios(CMsg &msg){  
    std::string transmitter=msg.getParameter("TRANSMITTER","");
    std::string sleeper=msg.getParameter("SLEEP","");
    writeconsoleln("updateRadios(CMsg &msg)");
    writeconsoleln(transmitter);

    if(transmitter.size()){
      if(transmitter=="RADIO") {
          Radio.setTransmitter(true);
          Radio2.setTransmitter(false);
      }
      if(transmitter=="RADIO2") {        
          Radio.setTransmitter(false);
          Radio2.setTransmitter(true);
      }    
    }

    if(sleeper.size()){
      if(sleeper=="RADIO") {
          Radio.sleep(true);        
      }
      if(sleeper=="RADIO2") {        
          Radio2.sleep(true);
      }    
    }
}


void CSatellite::readSysMap(){
  writeconsoleln("SysMap List:");
  std::string str;
  std::string strName="SYSMAP";
  
  for(auto s:SysMap){
    writeconsoleln(s.first);    
    str+=s.first;
    if(str.size()>150){
      CMsg m;
      m.setLOG("SYSTEMS");
      m.setParameter(strName,str);    
      writeconsoleln(str);
      addTransmitList(m);      
      str="";      
    }
  }
  
  CMsg m;
  m.setLOG("SYSTEMS");
  m.setParameter(strName,str);    
  writeconsoleln(str);
  addTransmitList(m);
}

void CSatellite::sendBeacon(){  
  CMsg msg;
  msg.setParameter("STATE",getSatState());
  msg.setParameter("TIME",getTime());

  CEPS *pEPS=(CEPS *)getSystem("EPS");
  
  if(pEPS!=NULL){
    CMsg m=pEPS->m;
    
    msg.setParameter("BatteryVolt",m.getParameter("BatteryVolt"));
    msg.setParameter("BatteryCrnt",m.getParameter("BatteryCrnt"));
    msg.setParameter("BatTemp1",m.getParameter("BatTemp1"));
    msg.setParameter("XVolt",m.getParameter("XVolt"));
    msg.setParameter("YVolt",m.getParameter("YVolt"));
    msg.setParameter("ZVolt",m.getParameter("ZVolt"));
  }


  CRadio *pRadio=(CRadio *)getSystem("RADIO");
  CRadio *pRadio2=(CRadio *)getSystem("RADIO2");
  if(pRadio!=NULL){
    msg.setParameter("R_STATE",pRadio->State());
  }
  
  if(pRadio2!=NULL){
    msg.setParameter("R2_STATE",pRadio2->State());
  }

  CTemperatureObject *pTemp=(CTemperatureObject *)getSystem("TEMPOBC","TEMPOBC");  
  
  if(pTemp!=NULL) {    
    pTemp->setup();    
    pTemp->loop();
    msg.setParameter("TEMPOBC",pTemp->getTemp()); 
  }
  addTransmitList(msg);
}
