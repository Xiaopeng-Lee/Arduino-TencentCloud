#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>



/* 连接您的WIFI SSID和密码 */
#define WIFI_SSID         "lp"
#define WIFI_PASSWD       "15556545898"

#define LED_PIN           2

/* 设备的三元组信息*/
#define PRODUCTID       "S0Y1P5IY1X"
#define DEVICENAME      "sl_1"
#define DEVICEPSK       "/tc3J/rW0c7sDZGjNJQCcQ=="

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       "S0Y1P5IY1X.iotcloud.tencentdevices.com"
#define MQTT_PORT         1883

#define CLIENT_ID         "ESP8266|securemode=3,timestamp=1234567890,signmethod=hmacsha1|"
#define MQTT_USRNAME      "S0Y1P5IY1Xsl_1;12010126;PVVVX;3875934966"
#define MQTT_PASSWD       "3e687ddd2ce1dafdc9b0e2661c2dbe615fb2b5391df26cd8b60b9d97764df225;hmacsha256"

#define TENCENTYUN_PROPERY_SUB         "$thing/down/property/S0Y1P5IY1X/"DEVICENAME             //设备属性订阅
#define TENCENTYUN_PROPERY_PUB         "$thing/up/property/S0Y1P5IY1X/"DEVICENAME              //设备属性上报


WiFiClient espClient;
PubSubClient  client(espClient);
unsigned long lastMs = 0;
unsigned char brightness = 0;
bool power_switch = 0;


void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);

    DynamicJsonDocument doc(length*2);
    // 重点3：反序列化数据
    deserializeJson(doc, payload);
 
  // 重点4：获取解析后的数据信息
  String Method = doc["method"].as<String>();
  if(Method == "control")
  {
      int c_brightness = doc["params"]["brightness"].as<int>();
      int c_power_switch = doc["params"]["power_switch"].as<int>();  

      brightness = c_brightness;
      power_switch = c_power_switch;

      
      if(brightness == 0)
      {
        power_switch = 0;
      }
      else
      {
        power_switch = 1;
      }

      
      if(power_switch == 1)
      {
          analogWrite(LED_PIN, brightness);
      }
      else
      {
          analogWrite(LED_PIN, 0);
      }

      

      
      // 通过串口监视器输出解析后的数据信息
      Serial.print("c_brightness = ");Serial.println(c_brightness);
      Serial.print("c_power_switch = ");Serial.println(c_power_switch);
      mqttIntervalPost();
  }
 


}

void wifiInit()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);   //连接WiFi
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
    Serial.println("Connected to AP");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());    
    Serial.print("espClient [");
    client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接WiFi之后，连接MQTT服务器 */
    
    client.setCallback(callback);
}


void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if (client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))

        {

            Serial.println("MQTT Connected!");
            client.subscribe(TENCENTYUN_PROPERY_SUB);

        }
        else
        {
            Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);
        }
    }
}


void mqttIntervalPost()
{
    char param[512];
//    char jsonBuf[512];
//    brightness ++;
//    if(brightness > 100)
//    {
//      brightness = 0;  
//    }
    sprintf(param, "{\"method\":\"report\",\"clientToken\":\"123\",\"params\":{\"power_switch\":%d, \"brightness\":%d}}", power_switch, brightness);
//    sprintf(jsonBuf, TENCENTYUN_PROPERY_PUB, param);
    Serial.println(param);
    boolean d = client.publish(TENCENTYUN_PROPERY_PUB, param);
    if(d){
      Serial.println("publish Temperature success"); 
    }else{
      Serial.println("publish Temperature fail"); 
    }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Demo Start");
  analogWriteRange(100);
  analogWrite(LED_PIN, brightness);
  wifiInit();

}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - lastMs >= 10000)
  {
    lastMs = millis();
    mqttCheckConnect(); 

    /* 上报 */
    mqttIntervalPost();

  }
  client.loop();
  

}
