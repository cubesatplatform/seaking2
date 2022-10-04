#pragma once

#include "defs.h"
#include "sdfs.h"
#include "csfilenames.h"

#include <messages.h>

#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)   
  
#else
  #include <system_gps.h>
#endif  


#include <state_detumble.h>
#include <state_deployantenna.h>
#include <state_payload.h>
#include <state_adcs.h>
#include <ceps.h>
#include <system_reactionwheel.h>
#include <system_magtorquer.h>
#include <system_temperature.h>
#include <mdrive.h>
#include <fhmotor.h>

#include <system_imu.h>
#include <system_delay.h>
#include <system_relay.h>

#include <radio.h>
#include <messages.h>
#include <system_mgr.h>
#include <system_irarray.h>
#include <phone.h>



#define POWERKEY  "POWERKEY"


class CSatellite:public CSystemObject {
  public:
  CMsg msgCounts;
  	std::list<CSystemObject*> coresystems;

  int _restartcount=0;  
	unsigned long lcount=0;
  
  CStateObj state_core;
  CStateObj state_normal;        
  CStateObj state_lowpower;   
  
  CEPS Power;
  CIMU IMUSPI;

#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7) 
  

  
  CRadio Radio2; 
  
  
#else 
  #define Radio2 Radio  
  CGPS gps;
#endif  

  CADCSState state_adcs;     
  CDeployAntennaState state_deployantenna;  
  CDetumbleState state_detumble;  
  CPayloadState state_payload;
  CRW RW;
  CMagTorquer MT;
  CPhone Phone;  
  CMDrive MagX;
  CMDrive MagY;
  CMDrive MagZ;

  CMotorController MotorX;
  CMotorController MotorY;
  CMotorController MotorZ;
  
  CTemperatureObject TempX1,TempX2,TempY1,TempY2,TempZ1,TempZ2, TempOBC;
  
  CIRArray IRX1,IRX2,IRY1,IRY2,IRZ1,IRZ2;

  CSystemMgr Mgr;
  CRadio Radio;   
  CIMU IMUI2C;   
  
  CDelay Delay;
  CRelay Relay;
  
  
  CStateObj* pstate;
	//std::bitset<10> satflag;  //initialized to 0  enum some flags HEALTHCHECK,TEMP, etc    satflag[HEALTHCHECK]

	CMessages MSG;  
	CSatellite();

	void newState(CMsg &msg);
  void addState(CMsg &msg);
  void newMsg(CMsg &msg);
 
	void setup();  
  void readCounts();
  void sendCounts();
  void writeCounts();
  void readSysMap();
  void sendBeacon();  
	void loop();
	void stats();
  void temp();
	void MsgPump();
  void updateRadios(CMsg &msg);
  void addSystem(CMsg &msg);

  
  void callCustomFunctions(CMsg &msg); 

};

CSatellite* getSatellite();
