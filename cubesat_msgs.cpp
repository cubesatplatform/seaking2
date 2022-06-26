#include "cubesat.h"


#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
void resetFunc(){
  NVIC_SystemReset();
}
#else
void(* resetFunc) (void) = 0; //declare reset function @ address 0
#endif


void CSatellite::newMsg(CMsg &msg) {
  
  std::string sys=msg.getSYS();
  std::string act=msg.getACT();


  writeconsoleln("----------------------------------CSatellite::newMsg(CMsg &msg)");
  msg.writetoconsole();

  if(sys=="SAT") {    
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
    if(act=="MOUNTFS") mountFS();    
    if(act=="FORMATFS") formatFS();    
    if(act=="DELETEFILE") {std::string path=msg.getParameter("PATH");  deleteFile(path.c_str()); }   
    if(act=="DELETEFILES") {std::string path=msg.getParameter("PATH");  deleteFiles(path.c_str()); }   
    if(act=="WRITEFILE") {std::string path=msg.getParameter("PATH");  writeFile(path.c_str(),(const unsigned char *)msg.serialize().c_str(),msg.serialize().size()); }   
    if(act=="APPENDFILE") {std::string path=msg.getParameter("PATH"); std::string path1=msg.getParameter("PATH1");   appendFile(path.c_str(),path1.c_str()); }   
    if(act=="READFILE") {std::string str,path=msg.getParameter("PATH"); str=readFile(path.c_str()); CMsg m(str);  addTransmitList(m); }   
    if(act=="READFILEBINARY") {std::string str,path=msg.getParameter("PATH"); CMsg m; str=readFileBinary(path.c_str(),&m.byteVector);   addTransmitList(m); }   
    if(act=="LISTDIR"){std::string str,path=msg.getParameter("PATH");CMsg m=listDir(path.c_str());    addTransmitList(m);}
    }    


    else{      
      CSystemObject *psys=getSystem(sys.c_str(),"CSatellite::newMsg(CMsg &msg)");
      if(psys!=nullptr){             
        writeconsole("Sending new message to :");writeconsoleln(psys->Name());
         psys->newMsg(msg);       
        }    
      else
        writeconsoleln("Couldn't find system to send message to.");
    } 
}
