#include "cubesat.h"


#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
void resetFunc(){
  NVIC_SystemReset();
}
#else
void(* resetFunc) (void) = 0; //declare reset function @ address 0
#endif




void CSatellite::callCustomFunctions(CMsg &msg)  {  //Used to be NewMsg
  
  std::string sys=msg.getSYS();
  std::string act=msg.getACT();

  writeconsoleln("----------------------------------callCustomFunctions(CMsg &msg) ---");
  msg.writetoconsole();
     
  if(act=="STATS") stats();
  if(act=="SYSMAP") readSysMap();
  if(act=="RESET") resetFunc();
  if(act=="SENDCOUNTS") sendCounts();   
  if(act=="READCOUNTS") readCounts();   
  if(act=="WRITECOUNTS") writeCounts();   
  if(act=="BEACON") sendBeacon();   
  if(act=="UPDATERADIOS") updateRadios(msg);
  if(act=="ADDSYSTEM") addSystem(msg);
  if(act=="ADDSTATE") addState(msg);
  if(act=="NEWSTATE") newState(msg);    

#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)  
  if(act=="MOUNTFS") mountFS();    
  if(act=="FORMATFS") formatFS();    
  if(act=="DELETEFILE") {std::string path=msg.getParameter("PATH");  deleteFile(path.c_str()); }   
  if(act=="DELETEFILES") {std::string path=msg.getParameter("PATH");  deleteFiles(path.c_str()); }   
  if(act=="WRITEFILE") {std::string path=msg.getParameter("PATH");  writeFile(path.c_str(),(const unsigned char *)msg.serialize().c_str(),msg.serialize().size()); }   
  if(act=="APPENDFILE") {std::string path=msg.getParameter("PATH"); std::string path1=msg.getParameter("PATH1");   appendFile(path.c_str(),path1.c_str()); }   
  if(act=="READFILE") {std::string str,path=msg.getParameter("PATH"); str=readFile(path.c_str()); CMsg m(str);  addTransmitList(m); }   
  if(act=="READFILEBINARY") {std::string str,path=msg.getParameter("PATH"); CMsg m; str=readFileBinary(path.c_str(),&m.byteVector);   addTransmitList(m); }   
  if(act=="LISTDIR"){std::string str,path=msg.getParameter("PATH");CMsg m=listDir(path.c_str());    addTransmitList(m);}
  if(act=="FILESIZE") {std::string str,path=msg.getParameter("PATH"); CMsg m=fileSize(path.c_str());   addTransmitList(m); }   
 #endif

/*
    else{      
      CSystemObject *psys=getSystem(sys.c_str(),"CSatellite::newMsg(CMsg &msg)");
      if(psys!=nullptr){             
        writeconsole("Sending new message to :");writeconsoleln(psys->Name());
         psys->newMsg(msg);       
        }    
      else
        writeconsoleln("Couldn't find system to send message to.");
    } 
    */
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

 
  CMsg m=getDataMap(POWERKEY);
  
  msg.setParameter("BatteryVolt",m.getParameter("BatteryVolt"));
  msg.setParameter("BatteryCrnt",m.getParameter("BatteryCrnt"));
  msg.setParameter("BatTemp1",m.getParameter("BatTemp1"));
  msg.setParameter("XVolt",m.getParameter("XVolt"));
  msg.setParameter("YVolt",m.getParameter("YVolt"));
  msg.setParameter("ZVolt",m.getParameter("ZVolt"));



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


void CSatellite::stats(){  
  CMsg msg;  
  msg.setParameter("TIME",getTime());
  
  msg.setParameter("RESETS",_restartcount);
  msg.setParameter("BURNS",state_deployantenna._burncount);
  msg.setParameter("DTBL",state_detumble._detumblecount);
  
  msg+=state_core.stats();  
  msg+=pstate->stats();  


  addTransmitList(msg);    
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
