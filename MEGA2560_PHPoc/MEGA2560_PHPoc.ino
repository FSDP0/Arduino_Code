// Main Device Arduino Mega2560 Board Code

#include <DHT.h>    
#include <SPI.h>
#include <Phpoc.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define DHTPIN 2         
#define DHTTYPE DHT11     

#define sub1_rx 10     
#define sub1_tx 11

#define sub2_rx 12
#define sub2_tx 13

#define Vo A0
#define V_LED 3

#define sampling_time 280
#define delta_time 40
#define sleep_time 9680
 
#define DATA_SIZE 20
#define PORT 3000

SoftwareSerial sub_1(sub1_rx,sub1_tx);
SoftwareSerial sub_2(sub2_rx,sub2_tx);

DHT dht(DHTPIN, DHTTYPE); 

char server_name[]="xxx.xxx.xxx.xxx"; // Server IP Address

float dust_measure;
float dust_density;
  
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 11, 12);   
// pin 8 - 시리얼 클럭 출력핀 (SCLK)
// pin 9 - 시리얼 데이터 출력핀 (DIN)
// pin 10 - 데이터/커맨드 설정핀 (D/C)
// pin 11 - LCD칩 설정 핀 (CS/CE)
// pin 12 - LCD 리셋 핀 (RST)

PhpocClient client;

void dust_calc(){
  digitalWrite(V_LED,LOW);
  delayMicroseconds(sampling_time);
  dust_measure = analogRead(Vo);
  
  delayMicroseconds(delta_time); 
  digitalWrite(V_LED,HIGH);
  
  delayMicroseconds(sleep_time);

  dust_measure = dust_measure * 5.0 / 1024.0;
  dust_density = (dust_measure - 0.17) / 0.005;    
}

void setup()   
{
  Serial.begin(9600);      
  sub_1.begin(9600);
  sub_2.begin(9600); 
  
  dht.begin();             
  
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);

  pinMode(Vo,INPUT);
  pinMode(V_LED,OUTPUT);   
  
  display.begin();          
  display.setContrast(50);  
  display.clearDisplay();    

  display.setTextSize(1);          
  display.setTextColor(BLACK);      
  display.setCursor(0,0);         
  display.print("dust: ");         
  display.print("");                
  display.setCursor(0,15);         
  display.println("temperature: "); 
  display.print("");                
  display.setCursor(0,30);         
  display.println("humidity: ");  
  display.print("");               
  display.display();               
}

void loop()
{
  String data="";
 
  int humi = dht.readHumidity();
  int temp = dht.readTemperature();

  dust_calc();

  data+="dust1=";
  data+=(String)dust_density;
  data+="&temp1=";
  data+=(String)temp;
  data+="&humi1=";
  data+=(String)humi;

  data+="&";

  while(sub_1.available()){
    data+=(char)sub_1.read();
  }

  data+="&";

  while(sub_2.available()){
    data+=(char)sub_2.read(); 
  }

    display.clearDisplay();  
    display.setTextSize(1);   
    display.setTextColor(BLACK,WHITE);  
    display.setCursor(0,0);  
    display.println("dust: ");
    display.print(dust_density);
    display.setCursor(0,15); 
    display.println("temperature: ");
    display.print(temp);
    display.setCursor(0,30);  
    display.println("humidity: ");
    display.print(humi);
    display.display();

  while(client.available() > 0){
      char c=client.read();
      Serial.print(c);
  }
  
  if(!client.connected()){
      Serial.println("client_disconnected....");
      client.stop();
      delay(5000);

      if(client.connect(server_name,PORT)){
          Serial.println("Connected to server");
          Serial.println("Sending to Server: ");
          Serial.print("POST /device HTTP/1.1");

          client.println("POST /device HTTP/1.1");                 
          client.print("Host: ");
          client.println(server_name);
          client.println("Content-Type: application/x-www-form-urlencoded");
          client.println("Connection: close");
          client.println("User-Agent: Arduino/1.0");
          client.print("Content-Length: ");
          client.println(data.length());
          client.println();
          
          client.print(data);  // data to post
          client.println();
        }else
          Serial.println("connection failed");
    }
}
