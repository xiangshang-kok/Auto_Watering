#define BLINKER_WIFI
#include <stdio.h>
#include <DS18B20.h>
#include <Blinker.h>
#include <time.h>
#include<Wire.h>
#include "DHT.h"
#define PCF8591 0x48
#define AIn0 0x40
#define AIn1 0x41
#define AIn2 0x42
#define AIn3 0x43
#define PUMP1_BUILTIN 12
#define PUMP2_BUILTIN 14
char auth[] = "58e2954e4f1f";//在BLINKER中设备的密钥
char ssid[] = "6514_lab";//局域网的名字
char pswd[] = "helloworld";

DS18B20 ds(2);
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0
#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
time_t now; //实例化时间
int last_watering_time1=0;
int last_watering_time2=0;

int wartering_level=40;
int watering_delay=10;
struct tm* timeInfo;


int soil_hum_value0 = 0;
int soil_hum_value1 = 0;
int count=0;
int count2=0;

float air_hum = 0;
float air_tem = 0;

uint8_t tem_address[8] = {40, 252, 58, 249, 20, 33, 1, 69};//有线温度传感器地址
uint8_t selected;
//uint8_t tem_address[8] = {40, 197, 116, 184, 41, 32, 1, 16};
float temp_data = 0.0;//定义土壤温度

//组件对象
BlinkerNumber TEMP("soil_tem"); //定义土壤温度键名，（来自app）
BlinkerButton Button1("pump_key1");
BlinkerButton Button2("pump_key2");

BlinkerText Text1("text_time");
BlinkerText Text2("text_time2");

BlinkerNumber AIR_TEM("air_tem");
BlinkerNumber AIR_HUM("air_hum");

BlinkerNumber WATER_LEVEL_DATA("water_level_data");
BlinkerNumber WATER_DELAY_DATA("water_delay_data");

BlinkerNumber SOIL_HUM1("soil_hum1");
BlinkerNumber SOIL_HUM2("soil_hum2");

#define Slider_1 "water_level"
BlinkerSlider Slider1(Slider_1);

#define Slider_2 "watering_delay"
BlinkerSlider Slider2(Slider_2);

float get_tem() {
  float tem = 0.0;
  selected = ds.select(tem_address);
  ds.setResolution(10);
  if (selected) {
    tem = ds.getTempC();
  } else {
    Serial.println("Device not found!");
  }
  return tem;
}

void slider1_callback(int32_t value)
{
    BLINKER_LOG("get slider value: ", value);
    wartering_level=value;
    
}

void slider2_callback(int32_t value)
{ 
    BLINKER_LOG("get slider2 value: ", value);
    watering_delay=value;
    
}

void button2_callback(const String & state) {
    BLINKER_LOG("get button state: ", state);
    if (state=="on") {
        digitalWrite(PUMP2_BUILTIN, HIGH);
        // 反馈开关状态
        Button2.print("on");
    } else if(state=="off"){
        digitalWrite(PUMP2_BUILTIN, LOW);
        // 反馈开关状态
        Button2.print("off");
    }
}


void button1_callback(const String & state) {
    BLINKER_LOG("get button state: ", state);
    if (state=="on") {
        digitalWrite(PUMP1_BUILTIN, HIGH);
        // 反馈开关状态
        Button1.print("on");
        // 反馈开关状态
        //Button1.print("on");
    } else if(state=="off"){
        digitalWrite(PUMP1_BUILTIN, LOW);
        Button1.print("off");
    }
}
void get_air_value() {
  delay(2000);
  air_hum = dht.readHumidity();
  air_tem = dht.readTemperature();
  if (isnan(air_hum) || isnan(air_tem))  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
}

int get_value(int port_num) {
  Wire.beginTransmission(PCF8591);
  switch (port_num) {
    case 0: {
        Wire.write(AIn0);
      }

    case 1: {
        Wire.write(AIn1);
      }

    case 2: {
        Wire.write(AIn2);
      }

    case 3: {
        Wire.write(AIn3);
      }
  }

  Wire.endTransmission();
  Wire.requestFrom(PCF8591, 1);
  return Wire.read();
}

void Watering_control(int sens1,int sens2){
  if (sens1<wartering_level){
    count++;
    if (count>30){
      digitalWrite(PUMP1_BUILTIN, HIGH);
       Blinker.delay(watering_delay*1000);
      digitalWrite(PUMP1_BUILTIN, LOW);
      // 反馈开关状态
      Button1.print("off");
      last_watering_time1=(timeInfo->tm_mon+1)*10000+timeInfo->tm_mday*100+timeInfo->tm_hour+timeInfo->tm_min*0.01;
      
      Blinker.delay(3000);
      count=0;
      }
    }

    if (sens2<wartering_level){
    count2++;
    if (count2>30){
      digitalWrite(PUMP2_BUILTIN, HIGH);
      Blinker.delay(watering_delay*1000);
      digitalWrite(PUMP2_BUILTIN, LOW);
      Button2.print("off");
      last_watering_time2=(timeInfo->tm_mon+1)*10000+timeInfo->tm_mday*100+timeInfo->tm_hour+timeInfo->tm_min*0.01;
      
      Blinker.delay(3000);
      
      // 反馈开关状态
      
      count2=0;
      }
    }
  
  }

void heartbeat()
{
  TEMP.print(temp_data);
}

void dataStorage()
{
  Blinker.dataStorage("chart_temp1", temp_data);
  Blinker.dataStorage("chart_hum0", soil_hum_value0);
  Blinker.dataStorage("chart_hum1", soil_hum_value1);
}


void setup() {
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  BLINKER_DEBUG.stream(Serial);
  Blinker.begin(auth, ssid, pswd);
  Blinker.attachHeartbeat(heartbeat);
  Blinker.attachDataStorage(dataStorage);
  
  pinMode(PUMP1_BUILTIN, OUTPUT);
  digitalWrite(PUMP1_BUILTIN, LOW);
  pinMode(PUMP2_BUILTIN, OUTPUT);
  digitalWrite(PUMP2_BUILTIN, LOW);
  Button1.attach(button1_callback);
  Button2.attach(button2_callback);
  Slider1.attach(slider1_callback);
  Slider2.attach(slider2_callback);
  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com");
}

void loop() {

  Blinker.run();
  temp_data = get_tem();
  get_air_value();

  soil_hum_value0 = 100-(get_value(0)-60)/0.6;
  soil_hum_value1 = 100-(get_value(1)-60)/0.6;
  Watering_control(soil_hum_value0,soil_hum_value1);
  Serial.println(temp_data);
  Serial.print("air_value****");
  Serial.println(air_tem);
  Serial.println(air_hum);

  Serial.println(soil_hum_value0);
  Serial.println(soil_hum_value1);
  Serial.print("count:");Serial.println(count);
  Serial.println("*****");
  
  now = time(nullptr);
  timeInfo = localtime(&now);
  Serial.println( timeInfo->tm_mon+1 );
  Serial.println(timeInfo->tm_hour);
  Serial.println(timeInfo->tm_min);
  Serial.println(wartering_level);
  Serial.println(watering_delay);
  TEMP.print(temp_data);
  AIR_TEM.print(air_tem);
  AIR_HUM.print(air_hum);
  WATER_LEVEL_DATA.print(wartering_level);
  WATER_DELAY_DATA.print(watering_delay);
  
  SOIL_HUM1.print(soil_hum_value0);
  SOIL_HUM2.print(soil_hum_value1);
  BLINKER_LOG("GET tempure is :", temp_data);
  Text1.print("Last Watering Time",last_watering_time1);
  Text2.print("Last Watering Time",last_watering_time2);
  Blinker.delay(5000);


}
