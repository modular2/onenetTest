/* BC95B oneNet Application 
* reference document:Quectel_BC95_OneNET_Application_Note_V1.2.pdf
* By modular-2 team 2018/7/26
*/

#include "BC95B.h"

#define   BC95B_DEFAULT_BAUD_RATE   9600
#define READ 0
#define WRITE 1
#define DISCOVER 2
#define OBSERVE 3
BC95B::BC95B(PinName tx, PinName rx, bool debug)
    : _serial(tx, rx, BC95B_DEFAULT_BAUD_RATE), 
      _parser(&_serial),
      _fail(false)
{
    _serial.set_baud( BC95B_DEFAULT_BAUD_RATE );
    _parser.debug_on(debug);
    _parser.set_delimiter("\r\n");
     _parser.oob("+MIPLWRITE:", callback(this, &BC95B::_write_handler));
     _parser.oob("+MIPLREAD:", callback(this, &BC95B::_read_handler));
     _parser.oob("+MIPLOBSERVE:", callback(this, &BC95B::_obs_handler));
    _parser.oob("+MIPLDISCOVER:", callback(this, &BC95B::_discover_handler));
}

const char * BC95B::get_firmware_version()
{   
        if (!(_parser.send("AT+MIPLVER?")
        && _parser.recv("+MIPLVER:%s\n",buffer)
        && _parser.recv("OK"))) {
        return NULL;
    }
   return buffer;
    
}
const char * BC95B::get_imei()
{   
        if (!(_parser.send("AT+CGSN=1")
        && _parser.recv("+CGSN:%s\n",buffer)
        && _parser.recv("OK"))) {
        return NULL;
    }
   return buffer;
    
}
const char * BC95B::get_imsi()
{   
        if (!(_parser.send("AT+CIMI")
        && _parser.recv("%s\n",buffer)
        && _parser.recv("OK"))) {
        return NULL;
    }
   return buffer;
    
}
bool BC95B::get_SignalStrength(int* rssi,int* ber)
{   
        if (!(_parser.send("AT+CSQ")
        && _parser.recv("+CSQ:%d,%d\n",rssi,ber)
        && _parser.recv("OK"))) {
        return false;
    }
   return true;
    
}
int BC95B::create_suite()
{   int result;
        if (!(_parser.send("AT+MIPLCREATE")
        && _parser.recv("+MIPLCREATE:%d",&result)
        && _parser.recv("OK"))) {
        return -1;
    }
   return result;
    
}
bool BC95B::create_object(int ch,const int name,int insCount,const char * bitmap,int attrCount,int actCount)
{
   
        if (!(_parser.send("AT+MIPLADDOBJ=%d,%d,%d,\"%s\",%d,%d",ch,name,insCount,bitmap,attrCount,actCount)
        && _parser.recv("OK"))) {
        return false;
    }
   return true; 
   }
bool  BC95B::register_request(int ref,int lifetime,int timeout)
 {
         if (!(_parser.send("AT+MIPLOPEN=%d,%d,%d",ref,lifetime,timeout)
        && _parser.recv("OK") 
        && _parser.recv("+MIPLEVENT: 0,1") 
        && _parser.recv("+MIPLEVENT: 0,2") 
        && _parser.recv("+MIPLEVENT: 0,4") 
        && _parser.recv("+MIPLEVENT: 0,6")))
         {
        return false;
    }
   return true; 
     } 
  bool  BC95B::Update_Request(int ref,int lifetime,int flg)
 {
         if (!(_parser.send("AT+MIPLUPDATE=%d,%d,%d",ref,lifetime,flg)
        && _parser.recv("OK")))
         {
        return false;
    }
   return true; 
     }    
/*bool BC95B::respond_discover(int ref,int result ,int length,char *valuestring)
{
         if (!(_parser.send("AT+MIPLDISCOVERRSP=%d,%s,%d,%d,%S",ref,msg,result,length,valuestring)
        && _parser.recv("OK"))){
            return false;
            } 
            return true;
    } */
bool BC95B::delete_object(int ref,const char * objid)
{
        if (!(_parser.send("AT+MIPLDELOBJ=%d,%s",ref,objid)
        && _parser.recv("OK"))){
            return false;
            } 
            return true;
    } 
bool BC95B::deregister(int ref)
{
        if (!(_parser.send("AT+MIPLCLOSE=%d",ref)
        && _parser.recv("OK"))){
            return false;
            } 
            return true;
    } 
bool BC95B::notify_resource(int ref,int objid,int ins,int resid,int valueType,int len,const char * value,int index,int flg)
{  
if (!(_parser.send("AT+MIPLNOTIFY=%d,%d,%d,%d,%d,%d,%d,%s,%d,%d", ref,obsmsg,objid,ins,resid,valueType,len,value,index,flg)
        && _parser.recv("OK"))){
            return false;
            } 
            return true;
    }
                 
bool BC95B::init(void)
{   
    if (!_parser.recv("Bootup Completed!!!"))
    return false;
    return true;
      }
bool BC95B::reset(void)
{
        if (!(_parser.send("AT+NRB")
        &&_parser.recv("REBOOTING"))) {
        return false;
    }  
  return true;
}
 
bool BC95B::Respond_Read(int ref,int result,int objId,int insId,int resId,int valueType,int len,const char * value,int index,int flg)
{ 
if (!(_parser.send("AT+MIPLREADRSP=%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d ",ref,msg,result,objId,insId,resId,valueType,len,value,index,flg)
        && _parser.recv("OK"))){
            return false;
            } 
            return true;
    }   


/*
+MIPLWRITE--平台下发的写指令
+MIPLWRITE: <ref>,<msgId>,<objI d>,<insId>,<resId>,<valueType>,<len>,<value>,<flag>,<index> 
response:
AT+MIPLWRITERSP=<ref>,<msgId>,< result> 
*/


void BC95B::_write_handler()
{   
  if (!(_parser.recv("%d,%d,%d,%d,%d,%d,%d,",&ref,&msg,&objId,&insId,&resId,&valueType,&len)
     && _parser.read(value,len*2)
     &&_parser.recv(",%d,%d\n",&flag,&index))) {       
        return;
    }    
  
 //  printf("v=%.*s\n",len*2,value);
 currCode=WRITE;
 curr_objId=objId;
 curr_insId=insId;
 curr_resId=resId;
 valueLength=len*2;
    _callback();
    result=2;
    if (! _parser.send("AT+MIPLWRITERSP=%d,%d,%d",ref,msg,result))
   { 
   return;
   }
   
 }
 /*
 +MIPLREAD--平台下发的读指令
 +MIPLREAD: <ref>,<msgId>,<objId>,<insId>, <resId> 
 response:
   AT+MIPLREADRSP=<ref>,<msgId>, <result>[,<objId>,<insId>,<resId>,< valueType>,<len>,<value>,<index>, <flag>] 
 */
 void BC95B::_read_handler()
 { int valueType,len,index,flg;
      if (!_parser.recv("%d,%d,%d,%d,%d\n",&ref,&msg,&objId,&insId,&resId))         
         {
        return  ;
        }
  //getValue(objId,insId,resId);
 //"AT+MIPLREADRSP=%d,%s",ref,msgId,result,objId,insId,resId,valueType,len,value,index,flg)
 currCode=READ;
 curr_objId=objId;
 curr_insId=insId;
 curr_resId=resId;
 _callback();
  //  len=4;index=0;flg=0;result=1;valueType=currType;
  //  memcpy(value,"3.8",3);
 //    if (! _parser.send("AT+MIPLREADRSP=%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d ",ref,msg,result,objId,insId,resId,valueType,len,value,index,flg))
  // { 
 //  return;
 //  }
     }
/*
+MIPLOBSERVE: <ref>,<msgId>,<flag>,<objI d>,<insId>,<resId> 
AT+MIPLDISCOVERRSP=<ref>,<msgI d>,<result>[,<length>,<valuestring>] 
*/
void BC95B::_obs_handler()
{   
    if (!_parser.recv("%d,%d,%d,%d,%d,%d\n",&ref,&obsmsg,&flag,&objId,&insId,&resId)) {
        return;
    }
     currCode=OBSERVE; 
    curr_objId=objId;
    curr_insId=insId;
    curr_resId=resId;
     valueLength=1;
     value[0]=flag;
    _callback(); 
    result=1;
   if (! _parser.send("AT+MIPLOBSERVERSP=%d,%d,%d",ref,obsmsg,result))
     {
         return;
         } 
        
    }
void BC95B::_discover_handler()
{   
    if (!_parser.recv("%d,%d,%d\n",&ref,&msg,&objId)) {
        return;
    }
   // printf("discover\n");
    currCode=DISCOVER;
    curr_objId=objId;
    _callback();
    result=1;
   if (! _parser.send("AT+MIPLDISCOVERRSP=0,%d,%d,%d,\"%s\"",msg,result,resLength,resNames))
     {
         return;
         }  
    }
bool BC95B::recv()
{
     if (!_parser.process_oob()) {
            return false;
        }
    return true;    
        }

void BC95B::setTimeout(uint32_t timeout_ms)
{
    _parser.set_timeout(timeout_ms);
}

void BC95B::attach(void (*func)(void))
{
    _callback=func;

    }

  int BC95B::getEventCode()
  {  
  return currCode;
      }
  ;
    void BC95B::getCurrentID(long *msgId,int *objId,int *insId,int *resId)
    {
        *msgId=curr_msgId;
        *objId=curr_objId;
        *insId=curr_insId;
        *resId=curr_resId;
        };
    int  BC95B::getCurrentValue(char *buf)
    {
        memcpy(buf,value,valueLength);
       return valueLength;
        };
    int  BC95B::putCurrentValue(int type,char *buf,int length)
    {  
        valueLength= length ;
        currType=type;
         memcpy(value,buf,valueLength);
         return valueLength;
        };
  
  int  BC95B::putCurrentResNames(int type,char *buf,int length)
    { 
       // printf("reslength=%d\n",length);
        resLength= length ;
        currType=type;
         memcpy(resNames,buf,resLength);
         return resLength;
        };

