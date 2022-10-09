#include "cubesat.h"

CSatellite::CSatellite() {
  Name("SAT");
  pstate = &state_normal;
  }

void CSatellite::setup() {    //Anything not in a loop must be setup manually  or have setup done automatically when called
  state_core.Name("CORE");
  state_core.setForever();

  state_normal.Name("NORMAL");
  state_normal.setMaxTime(NORMALMAXTIME);
  state_normal.onEnter["ENABLESENSORS"]=true;
  state_normal.onExit["DISABLESENSORS"]=true;


  Radio2.Name("RADIO2");
  Radio2.setTransmitter(false);
  state_core.addSystem(&Radio2);

  state_adcs.Name("ADCS");
  state_adcs.setMaxTime(3*TIMEORBIT);
  
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

  IMUSPI.Name("IMUSPI");
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

  //  state_normal.addSystem(&MotorX);  

  state_payload.addSystem(&Phone);  
  //state_core.addSystem(&RW);    //RW needs to be in core because if you are running it you cant switch states and turn it off  

    
  #if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
    //mountFS();
  #endif
  
   
  state_lowpower.Name("LOWPOWER");
  state_lowpower.setMaxTime(LOWPOWERMAXTIME);
  state_lowpower.onEnter["DISABLE65V"]=true;
  state_lowpower.onEnter["DISABLEBURNWIRE"]=true;
  state_lowpower.onEnter["DISABLESENSORS"]=true;
  state_lowpower.onEnter["DISABLEPHONE"]=true;

  
  Radio.Name("RADIO");
  Radio.setTransmitter(true);
  Radio.setReceiver(false);


  state_core.addSystem(&Radio);  
  state_core.addSystem(&Mgr);    
  state_core.addSystem(&MSG);   
  state_core.addSystem(&Sch);

  Sch.initSat();  //Commands that are autoscheduled


  IMUI2C.Name("IMUI2C");   
  IMUI2C.remap();

  IMUI2C.config(IMUADDRESS1,&Wire);
  
  delay(100);
  readCounts();  
  _restartcount++;
  writeCounts();  
}


void CSatellite::loop() {  
    lcount++;   
    pstate->loop();     
    state_core.loop();   
    if(pstate->outOfTime()){
      if(pstate->Name()=="LOWPOWER")
        goNormalState();
      else
        goLowPowerState();
    }
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
