#include <TimeLib.h>


//System definitions
// wspp - water sensor power pin, wssp - water sensor signal pin
#define wspp 7
#define wssp A0
//prpp - photoresistor power pin, prsp - photoresistor signal pin
#define prpp 13
#define prsp A1
//InfraRed led
#define IRlightpin 12
//wppp - water pump power pin, pt - pump time
#define wppp 8
//RGB led
#define redleg 11
#define greenleg 10
#define blueleg 9
//buttons
#define button 2
#define day_button_up 3
#define day_button_down 4
#define pump_button_up 6
#define pump_button_down 5

//Variables
int maxlight = 0;
int minlight = 0;
int lightTrigger=0;
int waterLevel = 1;
int pump_time=3500;
int dayinterval=1;
const int empty=1;
const int low=2;
const int full=3;
const int err=0;
int day_n=0;
int last_pump_day=0;
int hour_n=0;
int last_pump_hour=0;
int min_n=0;
int last_pump_min=0;
int sec_n=0;
int last_pump_sec=0;
int cur_sec=0;
int prev_sec=0;


void setup() {

  pinMode(IRlightpin,OUTPUT);
  pinMode(wspp,OUTPUT);
  digitalWrite(wspp,LOW);
  pinMode(wssp,INPUT);
  pinMode(prpp,OUTPUT);
  pinMode(prsp,INPUT);
  pinMode(wppp,OUTPUT);
  digitalWrite(wppp,LOW);
  pinMode(redleg,OUTPUT);
  pinMode(greenleg,OUTPUT);
  pinMode(blueleg,OUTPUT);
  pinMode(button,INPUT);
  pinMode(day_button_up,INPUT);
  pinMode(day_button_down,INPUT);
  pinMode(pump_button_up,INPUT);
  pinMode(pump_button_down,INPUT);
  
  Serial.begin(9600);
  
  systemSetUp();
  

}

void loop() {
  
  
    waterLevel = waterCheck(readWaterLevel());
    RGBledState(waterLevel);
    lightCheck(readLightVal());
    timeToPump(waterLevel);  
    menualPump(waterLevel);

    //when buttons not connected there is a bug
    //need to fix
    //buttonsActions();

}


//******************************Water Level System Check Functions******************************
int RGBledState(int wl){

  //Full-Green, Low-Yellow, Empty-Red
  //Rapid-Blinks

  if(wl==full){
    //green
    analogWrite(redleg,0);
    analogWrite(greenleg,255);
    analogWrite(blueleg,0);

    return 1;
  }
  else if(wl==low){
    //yellow
    analogWrite(redleg,255);
    analogWrite(greenleg,255);
    analogWrite(blueleg,0);

    return 1;
  }
  else if(wl==empty){
    //red
    analogWrite(redleg,255);
    analogWrite(greenleg,0);
    analogWrite(blueleg,0);

    return 0;
  }

  
  return 0; 

  
  
}
int waterCheck(int val){
  
  static int maxval = val;
  int minval=maxval/6;
  int midval=maxval/2;
    
  int section = maxval/3;
  int lowrange = section;
  int midrange = section+section;
  int highrange = section+section+section;
  
  
  //Serial.print("Water Level:  ");
  
  if(val>=midrange){
    //Serial.println("Full");
    return full;
  }
  else if(val<midrange && val>=minval){
  //Serial.println("Low");
    return low;
  }
  else if(val<minval || val==0){
    //Serial.println("Empty");
    return empty;
  }
  else{
    //Serial.println("Error");
    return err;
  }
  
  
    
}
int readWaterLevel(){
  int val=0;
  digitalWrite(wspp,HIGH);
  delay(10);
  val = analogRead(wssp);
  
  digitalWrite(wspp,LOW);
  return val;

  
}
//******************************Time Masurement Functions***************************************
int timeConverter(){
  //Sign in the curren time in days, hours, minutes and seconds
  //counting from the start of the system
  //days (1-31), hours (0-23), minutes (0-59), seconds (0-59)

  day_n = day();
  hour_n = hour();
  min_n = minute();
  sec_n = second();
  
  
  return 1;
    
}
void pumpTimeStamp(){

  timeConverter();

  last_pump_day = day_n;
  last_pump_hour = hour_n;
  last_pump_min = min_n;
  last_pump_sec = sec_n;

  Serial.print("Pump Time Stamp:   ");
  Serial.print(last_pump_day);Serial.print(":");
  Serial.print(last_pump_hour);Serial.print(":");
  Serial.print(last_pump_min);Serial.print(":");
  Serial.println(last_pump_sec);
  return;
}

void timeToPump(int wl){
  int rti = 0;;

  timeConverter();
  rti = day_n-last_pump_day;   
  if(rti){
     if(last_pump_hour==hour_n && last_pump_min==min_n && last_pump_sec==sec_n){
      activateWaterPump(wl);
    }
  }

 // !!!!!!!System Check!!!!! DO NOT ACTIVATE!!!!! unless you need to check this function in min
//  rti = min_n-last_pump_min;   
//  if(rti==dayinterval){
//    if(last_pump_sec==sec_n){
//      activateWaterPump(wl);
//    }
//  }

  
}
//******************************Water Pump System  Functions************************************
void activateWaterPump(int wl){

if(wl==low || wl==full){
  pumpTimeStamp();
  digitalWrite(wppp,HIGH);
  delay(pump_time); //pump time as defined
  digitalWrite(wppp,LOW);
  
}
  
  return;
  
}
void menualPump(int wl){
  
  if(digitalRead(button)==HIGH){
    if(wl==empty){
      ledWarning();
      return;
    }
    timeConverter();
    activateWaterPump(wl);
  }
  return;
}
//******************************Light Sensor System*********************************************
void lightCheck(int val){
  if(val>=lightTrigger){
    digitalWrite(IRlightpin,HIGH);
  }
  else{
    digitalWrite(IRlightpin,LOW);
  }
}
int readLightVal(){
  int val=0;
  
  digitalWrite(prpp,HIGH);
  delay(10);
  val = analogRead(prsp);
  if(val>maxlight){
    //new max lightval
    maxlight=val;
  }
  else if(val<minlight){
    minlight=val;
  }

  //Calculate light trigger point
  lightTrigger = (maxlight-minlight);
  lightTrigger = lightTrigger/6;
  lightTrigger= lightTrigger*5;
  lightTrigger = lightTrigger+minlight;

//  Serial.print("max: ");Serial.print(maxlight);
//  Serial.print(" trigger: ");Serial.print(lightTrigger);
//  Serial.print("  min: ");Serial.println(minlight);

  digitalWrite(prpp,LOW);
  return val;
  
}
//******************************System Functions************************************************
void ledWarning(){
int i=0;

  while(i<5){
    analogWrite(redleg,255);
    analogWrite(greenleg,0);
    analogWrite(blueleg,0);

    delay(100);
    analogWrite(redleg,0);
    analogWrite(greenleg,0);
    analogWrite(blueleg,0);

    delay(100);
    
    i++;


    
  }

  return;
}
int systemSetUp(){


  int w_val =0;
  int l_val;
  int i=0;
  
  while(digitalRead(button)==LOW){}
  w_val = readWaterLevel();
  l_val = readLightVal();
  maxlight = minlight = l_val;
  Serial.print("max water level: ");Serial.print(w_val);
  Serial.print("  max light level:  ");Serial.println(l_val);
  
  while(i<3){
    
  analogWrite(redleg,255);
  analogWrite(greenleg,0);
  analogWrite(blueleg,0);
    
    delay(200);
    
    analogWrite(redleg,0);
  analogWrite(greenleg,0);
  analogWrite(blueleg,0);
    
    delay(200);
  
    i++;
  
  }
  
  i=0;
  while(i<3){
    
  analogWrite(redleg,0);
  analogWrite(greenleg,255);
  analogWrite(blueleg,0);
    
    delay(200);
    
    analogWrite(redleg,0);
  analogWrite(greenleg,0);
  analogWrite(blueleg,0);
    
    delay(200);
  i++;
  
  }
  
  activateWaterPump(full);
  
  
  
  return 1;

  
} 
void systemStatus(){

  //last pump
  Serial.print("Last Pump:   ");
  Serial.print(last_pump_day);Serial.print(":");
  Serial.print(last_pump_hour);Serial.print(":");
  Serial.print(last_pump_min);Serial.print(":");
  Serial.println(last_pump_sec);


  //days interval between pumps
  Serial.print("Days between pumps:  ");
  Serial.print(dayinterval);
  Serial.println(" [days]");

  //pump work time for one pump
  Serial.print("Secons per pump:  ");
  Serial.print(pump_time/1000);
  Serial.println(" [seconds]");

  return;
  
}
void buttonsActions(){

  int flag=0;
  
  if(digitalRead(day_button_up)==HIGH){
     if(dayinterval==30){
        return;
      }
    dayinterval=dayinterval+1;
    flag=1;
    }
    else if(digitalRead(day_button_down)==HIGH){
      if(dayinterval==1){
        return;
      }
    dayinterval=dayinterval-1;
    flag=1;
    }
    else if(digitalRead(pump_button_up)==HIGH){
      if(pump_time==7000){
        return;
      }
    pump_time=pump_time+1000;
    flag=1;
    }
    else if(digitalRead(pump_button_down)==HIGH){
      if(pump_time==1000){
        return;
      }
    pump_time=pump_time-1000;
    flag=1;
    }

    if(flag){
      systemStatus();
      delay(500);
    }
    


    return;
  
}
