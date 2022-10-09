#include "cubesat.h"


#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
void resetFunc(){
  NVIC_SystemReset();
}
#else
void(* resetFunc) (void) = 0; //declare reset function @ address 0
#endif



void CSatellite::callCustomFunctions(CMsg &msg)  {  //Used to be NewMsg
  CStateObj::callCustomFunctions(msg);
  
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
  
  if(act=="UPDATERADIOS") updateRadios(msg);
  
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



void CSatellite::stats(){  
  CMsg msg=getDataMap(std::string("SAT")); 
  msg.setParameter("TIME",getTime());
    
  msg+=state_core.stats();  
  msg+=pstate->stats();  
  addTransmitList(msg);      
}

void CSatellite::readCounts() {
  CMsg msgCounts;
  msgCounts.deserializeFile(SATCOUNTS_FILE);
  addDataMap(std::string("SAT"),msgCounts);

  msgCounts.writetoconsole();
  addTransmitList(msgCounts);   
}


void CSatellite::sendCounts() {  
  CMsg msg=getDataMap(std::string("SAT")); 
  addTransmitList(msg);   
}


void CSatellite::writeCounts() {  
  CMsg msgCounts=getDataMap(std::string("SAT")); 
  
  std::string fn=msgCounts.serializeFile(SATCOUNTS_FILE);
  addTransmitList(msgCounts);   
}
