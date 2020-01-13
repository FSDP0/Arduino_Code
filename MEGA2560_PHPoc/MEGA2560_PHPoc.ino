// Main Device Arduino Mega2560 Board Code

#include <DHT.h>        // DHT11 온습도 센서 라이브러리 불러옴
#include <SPI.h>
#include <Phpoc.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h> // 미세먼지 센서 라이브러리
#include <Adafruit_PCD8544.h> // 미세먼지 센서 라이브러리 불러옴

#define DHTPIN 2            //DHT핀을 2번 핀으로 정의 (DHT라이브러리의 매개변수로 사용),(DATA핀)
#define DHTTYPE DHT11      //DHT타입을 DHT11로 정의 (DHT라이브러리의 매개변수로 사용)

#define sub1_rx 10        //
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

DHT dht(DHTPIN, DHTTYPE);  //DHT설정 = dht(디지털2, dht11)  (객체생성이라 보면 됨)

char server_name[]="13.125.219.211";

float dust_measure;
float dust_density;
  
Adafruit_PCD8544 display = Adafruit_PCD8544(8, 9, 10, 11, 12);    //GLCD 설정
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
  dust_density = (dust_measure - 0.17) / 0.005;     // 센서 초기값 약 0.17
}

void setup()   
{
  Serial.begin(9600);        // 시리얼 통신을 사용하기 위해 보드레이트를 9600으로 설정합니다.
  sub_1.begin(9600);
  sub_2.begin(9600); 
  
  dht.begin();                //DHT 온습도 센서의 정보 전송을 시작
  
  Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);

  pinMode(Vo,INPUT);
  pinMode(V_LED,OUTPUT);   // ledPower를 출력 단자로 설정합니다.
  
  display.begin();            //GLCD의 출력 시작
  display.setContrast(50);    //디스플레이 대조 설정(반전정도 설정)
  display.clearDisplay();     //디스플레이 클리어(디스플레이 전에 반드시 전의 것을 지워줘야 함)

  display.setTextSize(1);           //텍스트 사이즈 설정
  display.setTextColor(BLACK);      //텍스트 색 설정
  display.setCursor(0,0);           //커서 위치 설정 (글이 적힐 부분 좌표 설정)
  display.print("dust: ");          //초기 출력
  display.print("");                  //초기 출력
  display.setCursor(0,15);          //커서 위치 설정 (글이 적힐 부분 좌표 설정)
  display.println("temperature: "); //초기 출력
  display.print("");                  //초기 출력
  display.setCursor(0,30);          //커서 위치 설정 (글이 적힐 부분 좌표 설정)
  display.println("humidity: ");    //초기 출력
  display.print("");                  //초기 출력
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

    display.clearDisplay();   //Apaga o buffer e o display
    display.setTextSize(1);   
    display.setTextColor(BLACK,WHITE);  
    display.setCursor(0,0);  //Seta a posicao do cursor
    display.println("dust: ");
    display.print(dust_density);
    display.setCursor(0,15);  //Seta a posicao do cursor
    display.println("temperature: ");
    display.print(temp);
    display.setCursor(0,30);  //Seta a posicao do cursor
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
