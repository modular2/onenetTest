#include "mbed.h"
#include "BC95B.h"
#define READ 0
#define WRITE 1
#define DISCOVER 2
#define OBSERVE 3
DigitalOut myled(PC_6);
BC95B oneNet(PB_6,PB_7,true);
DigitalOut Enable(PD_13);
DigitalOut RST(PD_12);
double temperature=0.8;
double min_value=-180;
double max_value=180;
//Signal Strength And bit error rate
int rssi,ber;
bool observe;
unsigned char hex_char(char c)
{
    if ('0' <= c && c <= '9') return (unsigned char)(c - '0');
    if ('A' <= c && c <= 'F') return (unsigned char)(c - 'A' + 10);
    if ('a' <= c && c <= 'f') return (unsigned char)(c - 'a' + 10);
  //  printf("Error Val=%d\n",c);
    return 0xFF;
}
double convertHextoDouble(char * buf,int len)
{ double result ;
    uint64_t temp;
   int i;
   
   temp=0;
   for (i=0;i<len;i++)
      temp=(uint64_t)(temp<<4)| hex_char(buf[i]);
      memcpy(&result,&temp,8);
      return result;
    }
    
int readValue(int objId, int insId, int resId,char *ValueString)
{   
  int ref,len,index,flg,valueType,result,insid; 
  insid=insId; 
       printf("read %d Value\n",resId);
       if ((insid==-1)||(resId==-1))
       {
           index=0;flg=0; result=1;ref=0;len=4;insid=0;
           sprintf(ValueString,"%0.1f",temperature);valueType=4;resId=5700;index=2;
           oneNet.Respond_Read(ref,result,objId,insid,resId,valueType,len,ValueString,index,flg); 
           sprintf(ValueString,"%0.1f",min_value);valueType=4;resId=5601;index=1;
           oneNet.Respond_Read(ref,result,objId,insid,resId,valueType,len,ValueString,index,flg); 
            sprintf(ValueString,"%0.1f",max_value);valueType=4;resId=5602;index=0;
            oneNet.Respond_Read(ref,result,objId,insid,resId,valueType,len,ValueString,index,flg);            
           }
       else
       {
       switch (resId)
       {
           case 5700:{sprintf(ValueString,"%0.1f",temperature);valueType=4;break;}
           case 5601:{sprintf(ValueString,"%0.1f",min_value);valueType=4;break;}
           case 5602:{sprintf(ValueString,"%0.1f",max_value);valueType=4;break;}
           } 
            index=0;flg=0;result=1;ref=0;len=4;
           oneNet.Respond_Read(ref,result,objId,insId,resId,valueType,len,ValueString,index,flg);    
         }
             
       return len;
    }
int writeValue(int objId, int insId, int resId,char *ValueString,int len)
{
    double analog;
    analog=convertHextoDouble(ValueString,len);
    printf("write %d value \n",resId);
    switch(resId)
    {
        case 5700:{min_value=analog;}
        case 5601:{max_value=analog;}
        case 5602:{temperature=analog;}
        }
    
   return len;
    }
// OneNet callback function
void eventProcess()
{ int EventCode;
  int objId,insId,resId;
  long msgId;
  char  ValueString[12];
  int len;
  EventCode =  oneNet.getEventCode();
  //printf("event Processing %d\n",EventCode);
 switch(EventCode)
 {
     case READ:{
         oneNet.getCurrentID(&msgId,&objId,&insId,&resId);
         len=readValue(objId, insId, resId,ValueString);
         oneNet.putCurrentValue(4,ValueString,len);
         break;
         }
     case WRITE:{
          oneNet.getCurrentID(&msgId,&objId,&insId,&resId);
          len=oneNet.getCurrentValue(ValueString);
         
          writeValue(objId, insId, resId,ValueString,len);
          break;
         }
         case DISCOVER:{
          //   printf("Discover callback\n");
             oneNet.getCurrentID(&msgId,&objId,&insId,&resId);
           //  memcpy(ValueString,"5700:5601:5602",14); 
             oneNet.putCurrentResNames(0,"5700;5601;5602",14);
             break;
             } 
        case OBSERVE:{
            oneNet.getCurrentID(&msgId,&objId,&insId,&resId);
            len=oneNet.getCurrentValue(ValueString);
            if (ValueString[0]==0) 
            {
             observe=false;
             printf("Cancel observe\n");}
               else {observe=true;
               printf("Enable observe\n");
             }
            }        
     }

}

int main() {
          int time_counter=0;
          int lifeTime=0;
          const char * ss; 
          int result;
          char temp[12];
            Enable=1;
             RST=0;
            RST=1;
            observe=false;
          printf("BC95B8 China Mobile ooneNet Demo \n");
          temperature=0.8;
          oneNet.attach(&eventProcess);
          if(oneNet.reset())
          printf("Rest OK\n");
          wait(4);
          ss=oneNet.get_firmware_version();
          printf("firmware  version =%s\n",ss);
                 ss=oneNet.get_imei();
          printf("IMEI=%s\n",ss);
                 ss=oneNet.get_imsi();
          printf("IMSI=%s\n",ss);
           ss=oneNet.get_imsi();
          printf("IMSI=%s\n",ss);
            oneNet.get_SignalStrength(&rssi,&ber);
           printf("signal strength: rssi=%d,ber=%d\n",rssi,ber);
           result=oneNet.create_suite();
          printf("create communication suite=%d\n",result);
        
          // +MIPLADDOBJ:<ref>,<objId>,<insCount>,<insBitmap>,< attrCount>,<actCount> 
           oneNet.create_object(result,3303,1,"1",3,0) ; //Attribute count=3    
            while(!oneNet.register_request(result,3600,600))
            {   
                wait(1);
             };
          printf("register request OK\n");
          observe=true;  
    while(1) {
      oneNet.recv();
        wait(0.5); // 200 ms
        myled = !myled; // LED is OFF
        if (observe)
        {
          time_counter++;
              if (time_counter>25)
              {
                  time_counter=0;
                  
                   temperature=temperature+1.5;
                   if (temperature>20) temperature=-20;
                  sprintf(temp,"%0.1f",temperature);
                  oneNet.notify_resource(0,3303,0,5700,4,4,temp,0,0);
                  }
        }
      lifeTime++;
      if (lifeTime==60)
      {
          lifeTime=0;
          oneNet.Update_Request(0,3600,0);
          }  
    }
}
