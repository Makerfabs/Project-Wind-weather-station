#include <stdio.h>
#include <string.h>
#include<DHT.h>

#define DEBUG true
int PWR_KEY = 9;
int RST_KEY = 6;
int LOW_PWR_KEY = 5;

DHT  dht(3,DHT11);

#define buzzer 4

int pinInterrupt = 2; 
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1000;    // the debounce time; increase if the output flickers

int Count=0;
void onChange()
{
   if ( digitalRead(pinInterrupt) == LOW )
      Count++;
}


bool ModuleState=false;

void setup()
{
    pinMode(PWR_KEY, OUTPUT);
    pinMode(RST_KEY, OUTPUT);
    pinMode(LOW_PWR_KEY, OUTPUT);
    digitalWrite(RST_KEY, LOW);
    digitalWrite(LOW_PWR_KEY, HIGH);
    digitalWrite(PWR_KEY, HIGH);
    Serial1.begin(115200);
    digitalWrite(PWR_KEY, LOW);
    delay(3000);
    digitalWrite(PWR_KEY, HIGH);
    delay(10000);

    pinMode(buzzer, OUTPUT);
    digitalWrite(buzzer,HIGH);
    
    SerialUSB.begin(115200);
    //while (!SerialUSB)
    {
        ; // wait for serial port to connect
    }

    dht.begin();

    pinMode( pinInterrupt, INPUT_PULLUP);
    attachInterrupt( digitalPinToInterrupt(pinInterrupt), onChange, FALLING);

    ModuleState=moduleStateCheck();
    if(ModuleState==false)//if it's off, turn on it.
    {
      digitalWrite(PWR_KEY, LOW);
      delay(3000);
      digitalWrite(PWR_KEY, HIGH);
      delay(10000);
      SerialUSB.println("Now turnning the A9/A9G on.");
    }

    sendData("AT+CGATT=1", 3000, DEBUG);
    sendData("AT+CGDCONT=1,\"IP\",\"CMNET\"", 1000, DEBUG);    
    sendData("AT+CGACT=1,1", 1000, DEBUG);
    delay(500);
    sendData("AT+MQTTCONN=\"test.mosquitto.org\",1883,\"mqttx_0931852d34\",120,0", 1000, DEBUG);
    delay(100);
    sendData("AT+MQTTSUB=\"/public/TEST/makerfabs-B\",1,0", 1000, DEBUG);
    
    //sendData("AT+CIPSTART=\"TCP\",\"www.mirocast.com\",80", 2000, DEBUG);
    sendData("AT+GPS=1",3000, DEBUG);
    sendData("AT+GPSRD=0", 3000, DEBUG);
    SerialUSB.println("Maduino A9/A9G Test Begin!");
    digitalWrite(buzzer,HIGH);
}

unsigned long Couttime_t = 0;
unsigned long buzzertime = 0;  
int buzzer_cout = 0;

void loop()
{
  /*while (Serial1.available() > 0) {
    SerialUSB.write(Serial1.read());
    yield();
  }
  while (SerialUSB.available() > 0) {
    Serial1.write(SerialUSB.read());
    yield();
  }*/
//  subscribe callback  
 String sub_message = "";  
  long int subtime = millis();
  while (subtime+100>millis()){
        while(Serial1.available() > 0) {     
          char c = Serial1.read();
          sub_message += c;
        }
  }
  if (sub_message != "")
        SerialUSB.println(sub_message);
  if (sub_message.indexOf("buzzerON")>-1) {
        buzzer_cout = 1;
        buzzertime = millis();       
        digitalWrite(buzzer,LOW);
        SerialUSB.println("buzzer ON!");
  }
  if ((buzzer_cout == 1 )&& (millis()-buzzertime>3000) ) {    //buzzer ring 3S
        buzzer_cout = 0;       
        digitalWrite(buzzer,HIGH);
        SerialUSB.println("buzzer OFF!");
  }

      
// get wind speed
    
  char windspeek_str[8];
  float windspeek = 0; 
  if ((millis() - lastDebounceTime) >= debounceDelay) {
    int  currenTime = millis();        
    windspeek = Count*0.0875*debounceDelay/(currenTime-lastDebounceTime);
    lastDebounceTime = currenTime;
    Count=0;
    Couttime_t++;
    SerialUSB.print(windspeek);        
    SerialUSB.println("m/s");
  }

  //  publish 
  String topic_W = "AT+MQTTPUB=\"/public/TEST/makerfabs-W\",\""+(String)windspeek+"\",0,0,0";
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  String topic_T = "AT+MQTTPUB=\"/public/TEST/makerfabs-T\",\""+(String)t+"\",0,0,0";
  String topic_H = "AT+MQTTPUB=\"/public/TEST/makerfabs-H\",\""+(String)h+"\",0,0,0";
  if (Couttime_t%30 == 0) {
    //Couttime_t = 0;
    //sendData("AT+MQTTCONN=\"test.mosquitto.org\",1883,\"mqttx_0931852d34\",120,0", 1000, DEBUG);
    delay(100);
    sendData(topic_W, 1000, DEBUG);
    sendData(topic_T, 1000, DEBUG);
    sendData(topic_H, 1000, DEBUG);
    SerialUSB.println("Publish T and H");
  }

  if (Couttime_t > 60) {
    Couttime_t = 0;
    publishGPS();
  }
       

}

bool moduleStateCheck()
{
    int i = 0;
    bool moduleState=false;
    for (i = 0; i < 5; i++)
    {
        String msg = String("");
        msg = sendData("AT", 1000, DEBUG);
        if (msg.indexOf("OK") >= 0)
        {
            SerialUSB.println("A9/A9G Module had turned on.");
            moduleState=true;
            return moduleState;
        }
        delay(1000);
    }
    return moduleState;
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial1.println(command);
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (Serial1.available())
        {
            char c = Serial1.read();
            response += c;
        }
    }
    if (debug)
    {
        SerialUSB.print(response);
    }
    return response;
}

void publishGPS()
{
    
    String msg = String("");
    //sendData("AT+GPS=1", 3000, DEBUG);
    msg = sendData("AT+LOCATION=2", 1000, DEBUG); 
    SerialUSB.println(msg);
    if(msg.indexOf(",")>-1)
    {
      int a = msg.indexOf(",");
      char msg_c[30];
      msg.toCharArray(msg_c,msg.length()+1);
      String pub_msg = "";
      for(int i=1;i<7;i++) pub_msg = pub_msg + msg_c[16+i];
      pub_msg = pub_msg + ","; 
      for(int i=1;i<7;i++) pub_msg = pub_msg + msg_c[a+i]; 
      SerialUSB.println(pub_msg);
      sendData("AT+MQTTPUB=\"/public/TEST/makerfabs-G\",\""+pub_msg+"\",0,0,0", 2000, DEBUG);
      //sendData("AT+GPS=0", 1000, DEBUG);
      }
               
}
