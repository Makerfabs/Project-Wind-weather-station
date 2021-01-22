#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h>
#include "Adafruit_STMPE610.h"
#include "frame1.h"
#include "frame2.h"
#include "frame3.h"
#include "frame4.h"
#include "frame5.h"
#include "frame6.h"
#include "frame7.h"
#include "time.h"

TFT_eSPI tft = TFT_eSPI();

#define TFT_CS 15
#define STMPE_CS 2
#define SPI_ON_TFT digitalWrite(TFT_CS, LOW)
#define SPI_OFF_TFT digitalWrite(TFT_CS, HIGH)
#define STMPE_ON digitalWrite(STMPE_CS, LOW)
#define STMPE_OFF digitalWrite(STMPE_CS, HIGH)

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

Adafruit_STMPE610 touch = Adafruit_STMPE610(STMPE_CS);



const char *ssid = "Makerfabs";
const char *password = "20160704";
const char *mqtt_server = "test.mosquitto.org"; //MQTT SERVER ADDRESS

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char *ntpServer = "120.25.108.11";
const long gmtOffset_sec = 8 * 60 * 60; //China+8
const int daylightOffset_sec = 0;

struct tm timeinfo;

int windspeed = 0;

//==========================================================================================


void setup_wifi()
{

    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
} //CONNECT WiFi

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    String payload_str;
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        payload_str = payload_str+(char)payload[i];
    } 
    Serial.println();

    int i = 0;
    String topic_str = "";
    while (topic[i])
    {
      topic_str = topic_str + topic[i];
      i++;
      }
// DISPLAY THE MESSAGE
    Serial.println(topic_str);
    tft.setTextColor(TFT_ORANGE);
    if ( topic_str == "/public/TEST/makerfabs-T" )
    {
      tft.fillRect(232,15,66,30,TFT_BLACK);
      tft.drawString(payload_str,233, 15, 4);
      Serial.println("1");
      }
    else if ( topic_str == "/public/TEST/makerfabs-H" )
    {
      tft.fillRect(232,45,66,30,TFT_BLACK);
      tft.drawString(payload_str,233, 45, 4);
      }
    else if ( topic_str == "/public/TEST/makerfabs-G" )
    {
      tft.fillRect(15,47,180,30,TFT_BLACK);
      tft.drawString(payload_str,15, 47, 4);
      }
     else if ( topic_str == "/public/TEST/makerfabs-W" )
    {
      char payload_c[10];
      payload_str.toCharArray(payload_c,payload_str.length()+1);
      float payload_f;
      payload_f = atof(payload_c);
      if ( payload_f >0 && payload_f <2) windspeed = 1;
      else if ( payload_f >=2 && payload_f <4) windspeed = 2;
      else if ( payload_f >=4 ) windspeed = 3;
      else windspeed = 0;
      Serial.println(windspeed);    
      tft.fillRect(13,97,180,38,TFT_BLACK);
      tft.drawString(payload_str,15, 97, 6);
      }
    
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "mqttx_0931857d";      // CLIENT_ID
        clientId += String(random(0xffff), HEX); 
        //尝试连接
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // 
           // client.publish("/public/TEST/Makerfabs-B", "hello world"); //PUBLISH
            //
            client.subscribe("/public/TEST/makerfabs-T"); //SUBSCRIBE
            client.subscribe("/public/TEST/makerfabs-H");
            client.subscribe("/public/TEST/makerfabs-W");
            client.subscribe("/public/TEST/makerfabs-G");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
          
            delay(5000);
        }
    }
}


void setup() {
    Serial.begin(115200);

    pinMode(TFT_CS, OUTPUT);
    pinMode(STMPE_CS, OUTPUT);
    STMPE_OFF;
    SPI_OFF_TFT;
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    
    STMPE_ON;
    if (!touch.begin())
    {
        Serial.println("STMPE not found!");
        while (1)
            ;
    }
    STMPE_OFF;

    setup_wifi();
    client.setServer(mqtt_server, 1883); //MQTT DEFAULT PORT IS 1883
    client.setCallback(callback);

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println(F("Alread get npt time."));    
    
    SPI_ON_TFT;
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    tft.drawRect(2,2,316,236, TFT_GREEN);  
    tft.drawFastVLine(200,2, 236,TFT_GREEN);
    tft.setTextColor(TFT_ORANGE);
    tft.drawFastHLine(2,23, 198,TFT_GREEN);
    tft.drawString("GPS:",19,32,2);  //GPS
    
  
     //WIND
    //tft.drawFastHLine(2,83, 198,TFT_GREEN);
    tft.drawString("m/s:",15,72,4);
    tft.drawString("T:",205,15,4);  //TEM
    tft.drawString("C",295,15,4);
    //tft.drawFloat(0.1, 1, 240, 15, 4);
    //tft.drawFastHLine(200,40, 118,TFT_GREEN);
    tft.drawString("H:",205,45,4);  //HUM
    tft.drawString("%",295,45,4);
    //tft.drawFastHLine(200,70, 118,TFT_GREEN);
    tft.fillCircle(255,175,20,TFT_RED);  //BUTTON
    tft.drawCircle(255,175,23,TFT_WHITE);
  

}

//==========================================================================================
void loop() {
 
  if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 5000)
    {
        lastMsg = now;
        ++value;
        snprintf(msg, 50, "{\"index\":\"%ld\"}", value);
        Serial.print("Publish message: ");
        Serial.println(msg);             
        //client.publish("/public/TEST/makerfabs", msg); 
    }
    
    SPI_OFF_TFT;
    STMPE_ON; 
    uint16_t x, y;
    uint8_t z;
    if (touch.touched())
    {
       int pos[2] = {0, 0};
        delay(50);      // delay for SPI receive the data
        while (!touch.bufferEmpty())
        {
            
            touch.readData(&x, &y, &z);
            pos[0] = x * 240 / 4096;
            pos[1] = y * 320 / 4096;                        
        }        
        //delay(10);
        if (pos[1] > 220 && pos[1] < 300 && pos[0] >20 && pos[0] <90)
            {
                client.publish("/public/TEST/makerfabs-B", "buzzerON");
                Serial.println("Publish message: on!!!"); 
            }
        
      }
      STMPE_OFF;
      SPI_ON_TFT;

    if(windspeed == 0) tft.pushImage(100,135,100,100,frame1);  //windmill
    else if (windspeed == 1)  displayimage(100);
    else if (windspeed == 2)  displayimage(60);
    else if (windspeed == 3)  displayimage(20); 
       
    displaytime();
  
} // Loop back and do it all again
//==========================================================================================


void displayimage(int t)
{
  tft.pushImage(100,135,100,100,frame1);
  delay(t);
  tft.pushImage(100,135,100,100,frame2);
  delay(t);
  tft.pushImage(100,135,100,100,frame3);
  delay(t);
  tft.pushImage(100,135,100,100,frame4);
  delay(t);
  tft.pushImage(100,135,100,100,frame5);
  delay(t);
  tft.pushImage(100,135,100,100,frame6);
  delay(t);
  tft.pushImage(100,135,100,100,frame7);
  }

void displaytime()
{
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");        
    }
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    int sec = timeinfo.tm_sec;
    char time_str[10];
    sprintf(time_str, "%02d:%02d:%02d", hour, min, sec);
    tft.fillRect(14,9,100,24,TFT_BLACK); 
    tft.setTextColor(TFT_ORANGE);
    tft.drawString(time_str,15,10,4);
}
