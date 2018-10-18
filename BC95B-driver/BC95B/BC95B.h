/* BC95BInterface Example
  
 */

#ifndef BC95B_H
#define BC95B_H

#include "ATCmdParser.h"

/** BC95BInterface class.
    This is an interface to a BC95B radio.
 */
class BC95B
{
protected:
void (*_callback)(void); 
public:
    BC95B(PinName tx, PinName rx, bool debug=false);
    bool init(void);
    const char * get_firmware_version(void);
    const char * get_imei();
    const char * get_imsi();
    bool BC95B::get_SignalStrength (int* rssi,int* ber);
    int   create_suite();
    bool  create_object(int ch,const int name,int insCount,const char * bitmap,int attrCount,int actCount);
    bool  register_request(int ref,int lifetime,int timeout);
    //bool  respond_discover(int ref,int result ,int length,char *valuestring); 
    bool  delete_object(int ref,const char * objid);
    bool  deregister(int ref);
    bool notify_resource(int ref,int objid,int ins,int resid,int valueType,int len,const char * value,int index,int flg);
    bool  Update_Request(int ref,int lifetime,int flg);
    bool Respond_Read(int ref,int result,int objId,int insId,int resId,int valueType,int len,const char * value,int index,int flg);
    bool reset(void);
    void attach(void (*func)(void));   
    bool recv(void);
    void setTimeout(uint32_t timeout_ms);
    int getEventCode();
    void getCurrentID(long *msgId,int *objID,int *insId,int *resId);
    int  getCurrentValue(char *buf);
    int  putCurrentValue(int type,char *buf,int length);
    int  putCurrentResNames(int type,char *buf,int length);
private:
     int ref,objId,insId,resId,valueType,len,index,flag,result;
     int curr_objId,curr_insId,curr_resId,currCode,currType;
    long curr_msgId;
    long msg,obsmsg;
    char value[32];
    char resNames[32];
    int valueLength;
    int resLength;
    UARTSerial _serial;
    ATCmdParser _parser;
      char buffer[64];
     bool _fail;
    void _read_handler();
    void _write_handler();
    void _obs_handler();
    void _discover_handler();
};

#endif
