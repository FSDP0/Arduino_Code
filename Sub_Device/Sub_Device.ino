// Sub Device 2 : Arduino Uno Board Code

#include <DHT.h>        // DHT11 온습도 센서 라이브러리 불러옴
#include <Adafruit_GFX.h> 
#include <Adafruit_PCD8544.h> 
#include <SoftwareSerial.h>

#define DHTPIN 2     
#define DHTTYPE DHT11    

#define BT_Rx 4
#define BT_Tx 5

#define Vo A0
#define V_LED 3

#define sampling_time 280
#define delta_time 40
#define sleep_time 9680

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial BTserial(BT_Rx,BT_Tx);

float dust_measure;
float dust_density;

Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 11, 12);  
// pin 8 - 시리얼 클럭 출력핀 (SCLK)
// pin 9 - 시리얼 데이터 출력핀 (DIN)
// pin 10 - 데이터/커맨드 설정핀 (D/C)
// pin 11 - LCD칩 설정 핀 (CS/CE)
// pin 12 - LCD 리셋 핀 (RST)

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
  BTserial.begin(9600);
  
  dht.begin();      
  
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
    String send_data = "";
  
    int humi = dht.readHumidity();
    int temp = dht.readTemperature();
    
    dust_calc();
    
    send_data += "dust2=";
    send_data += (String)dust_density;
    send_data += "&temp2=";
    send_data += (String)temp;
    send_data += "&humi2=";
    send_data += (String)humi;

    char buf[send_data.length() + 1]= { 0 };
    
    send_data.toCharArray(buf,send_data.length()+1);


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

    for(int i=0; i < sizeof(buf) ;i++){
      BTserial.write(buf[i]);
    }

    delay(5000);
}
