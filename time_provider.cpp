#include "time_provider.h"
#include <WiFiNINA.h>
#include <WiFiClient.h>

static bool timeSynced = false;
unsigned long startupTs = 0;

static unsigned long t0_epoch  = 0;
static unsigned long t0_millis = 0;

static int _mon(const String& m){
  if(m=="Jan")return 1; if(m=="Feb")return 2; if(m=="Mar")return 3;
  if(m=="Apr")return 4; if(m=="May")return 5; if(m=="Jun")return 6;
  if(m=="Jul")return 7; if(m=="Aug")return 8; if(m=="Sep")return 9;
  if(m=="Oct")return 10; if(m=="Nov")return 11; if(m=="Dec")return 12;
  return 0;
}
static bool _leap(int y){ return (y%4==0 && y%100!=0) || (y%400==0); }
static unsigned long _epoch(int Y,int M,int D,int h,int mi,int s){
  static const int md[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  long days=0, y1=Y-1;
  long leaps=(y1/4-1969/4)-(y1/100-1969/100)+(y1/400-1969/400);
  days += (Y-1970)*365L + leaps;
  for(int m=1;m<M;m++) days += (m==2 && _leap(Y))?29:md[m-1];
  days += (D-1);
  return days*86400UL + h*3600UL + mi*60UL + s;
}

static unsigned long httpEpochOnce(){
  WiFiClient c;
  c.setTimeout(2000);
  if(!c.connect("google.com", 80)) { c.stop(); return 0; }
  c.print("HEAD / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n");

  unsigned long t0 = millis();
  while(c.connected() && millis()-t0 < 2500){
    String line = c.readStringUntil('\n');
    line.trim();
    if(line.length()==0) break;
    if(line.startsWith("Date:")){
      String v=line.substring(5); v.trim();
      int comma=v.indexOf(','); if(comma>=0) v=v.substring(comma+1);
      v.trim();
      int sp1=v.indexOf(' '), sp2=v.indexOf(' ',sp1+1), sp3=v.indexOf(' ',sp2+1), sp4=v.indexOf(' ',sp3+1);
      if(sp1<0||sp2<0||sp3<0||sp4<0){ c.stop(); return 0; }
      int DD=v.substring(0,sp1).toInt();
      int MM=_mon(v.substring(sp1+1,sp2));
      int YYYY=v.substring(sp2+1,sp3).toInt();
      String t=v.substring(sp3+1,sp4);
      int c1=t.indexOf(':'), c2=t.indexOf(':',c1+1);
      if(c1<0||c2<0){ c.stop(); return 0; }
      int HH=t.substring(0,c1).toInt();
      int MI=t.substring(c1+1,c2).toInt();
      int SS=t.substring(c2+1).toInt();
      unsigned long e=_epoch(YYYY,MM,DD,HH,MI,SS); 
      c.stop();                                 
      return e;
    }
  }
  c.stop();                     
  return 0;
}

static bool trySyncOnce(){
  unsigned long t = WiFi.getTime();
  if (t < 1700000000UL) t = httpEpochOnce();
  if (t >= 1700000000UL){
    t0_epoch  = t;
    t0_millis = millis();
    startupTs = t;
    timeSynced = true;
    return true;
  }
  return false;
}

unsigned long getTimestamp(){
  if (!timeSynced) return 0UL;
  unsigned long dt = (millis() - t0_millis) / 1000UL;
  return t0_epoch + dt;
}

void waitForValidTime(unsigned long minEpoch, uint16_t maxTries, uint16_t delayMs){
  for(uint16_t i=0;i<maxTries;++i){
    if (timeSynced || trySyncOnce()) break;
    delay(delayMs);
  }
}