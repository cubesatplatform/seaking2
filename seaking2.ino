#include "cubesat.h"

#include <map>
#include <list>
#include <bitset>

#include <sdfs.h>


#if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
  #include "mbed.h"
#else
  #define Wire2 Wire
#endif

#ifdef TTGO1
#include <axp20x.h>
#include <boards.h>
#endif


////////////////////////  DONT WRITE TO SERIAL PORT BEFORE ITS DECLARED  -----   NO WRITING TO CONSOLE IN CONSTRUCTORS OF SYSTEMS THAT ARE BELOW ----------------------////////////////////////////////////////////////
////////////////////////REACTIONWHEEL #include <mbed.h>  IS CAUSE OF SPI ISSUES ---- Never include that!  ////////////////////////////////

#define WATCHDOGWAIT 32760  //This is the Max the Portenta can do

std::map<std::string,CSystemObject *> SysMap;


CSatellite sat;

#define IAM "ADR1"

std::string CSystemObject::_IAM=IAM;
std::string CSystemObject::_defaultTO="ALL";
unsigned long CSystemObject::_lastLowPowerMsg=0;
CMessages* getMessages() { return &sat.MSG; }
CSatellite* getSatellite() { return &sat; }
std::string getSatState() { return sat.pstate->Name(); }


void setup() { 
  sat.Name(IAM);
  Serial.begin(115200);
  long tt=getTime();
  while (!Serial){
    if(getTime()>tt+10000)
      break;
  }
  Wire.begin();                                             //NEED to turn I2C ON   Otherwise CRASH
  Wire1.begin();
  Wire2.begin();
   
  #ifdef TTGO1
   initBoard();   
  #endif 
  #ifdef MYESP32
//  esp_task_wdt_init(WDT_TIMEOUT, true); // panic so ESP32 restarts
//  esp_task_wdt_add(NULL); //add current thread to WDT watch
  #endif
  #if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
  //Super IMPORTANT  Initialize SPIs so not floating!!!!
  /*
  //////////// WARNING   DOESNT WORK WITH PIN ALIASES   NO IDEA WHY
  digitalWrite(R1_NSS,HIGH);
  digitalWrite(R2_NSS,HIGH);
  digitalWrite(IMU_CS,HIGH);
*/

  digitalWrite(PC_13,HIGH);
  digitalWrite(PI_4,HIGH);
  digitalWrite(PJ_8,HIGH);
  //mbed_reset_reboot_count();
  mbed::Watchdog::get_instance().start(WATCHDOGWAIT); 

  #endif
  }

void mysetup() {   
  
  while (!Serial&&(getTime()<10000))  ;
  writeconsoleln("Starting...");
  delay(1000);
  #if defined(TTGO) || defined(TTGO1)
   Wire.begin(21,22);
  #endif 
  
  Wire.setClock(400000); 
  Wire1.setClock(400000); 
  Wire2.setClock(400000); 
  delay(1000);  
  
  sat.setup();           
} 



//Check to make sure time isnt getting too big and overflows (~50days), reset

void superSleepRadio(){
  if (sat.pstate->Name()!="LOWPOWER")
    return;
  sat.Radio.sleep(true);
  sat.Radio2.sleep(true);
  delay(10000);
  sat.Radio.sleep(false);
  sat.Radio2.sleep(false);
}


void loop() {    
  mysetup(); 
  unsigned long count=0;
  CMsg msg;
  msg.setSYS("BASE");
  msg.setACT("This is Major Tom to Ground Control");   
  sat.addTransmitList(msg);


  while(1){   

    sat.loop();  
    count++; 
    #if defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_PORTENTA_H7_M7)
      if(count%WATCHDOG_LOOP_COUNT==0) mbed::Watchdog::get_instance().kick();   
    #endif 
 
   //   superSleepRadio();
        
  }      
}
