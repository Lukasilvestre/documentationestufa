#include <neotimer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 23
#define DHTTYPE DHT11
#define SENSOR_PIN  12


const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
int valor_lido;
int analogSoloSeco = 2500; //VALOR MEDIDO COM O SOLO SECO
int analogSoloMolhado = 850; //VALOR MEDIDO COM O SOLO MOLHADO

//atuadores
int LAMP1 = 21;
int LAMP2 = 5;
int LAMP3 = 27;
int LAMP4 = 15;

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

Neotimer tempar;
Neotimer tempsolo;
Neotimer umdar;
Neotimer umdsolo;

void setup() {

    pinMode (LAMP1, OUTPUT);
    pinMode (LAMP2, OUTPUT);
    pinMode (LAMP3, OUTPUT);
    pinMode (LAMP4, OUTPUT);
    dht.begin();
    sensors.begin();
    Serial.begin(115200);

    tempar.set(2000);
    tempsolo.set(5000);
    umdar.set(5000);
    umdsolo.set(10000);
       
    WiFiManager wm;


    bool res;
    res = wm.autoConnect("ESP_TESTE","12345678"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");

    } 
    else {  
        Serial.println("connected...yeey :)");
    }
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    while (!client.connected()) {
        Serial.println("Conectando com o Broker (MQTT)…");
        String clientId = "";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("Conectado com o Broker");
        } else {
            Serial.print("Falha na conexão. Erro: ");
            Serial.print(client.state());
            Serial.println("\n");
            delay(2000);
            
        }
    }

    Serial.println("Enviando status de conexão para o smartphone\n");
    Serial.println("Status de conexão OK\n");

    //sensores
    client.publish("estufa/conexao", "1");
    client.publish("estufa/umidade_ar", "1"); //dht11 ok
    client.publish("estufa/umidade_Solo", "1"); //fc_28 ok
    client.publish("estufa/temperatura_ar", "1");//dht11 ok
    client.publish("estufa/temperatura_solo", "1"); //ds18b20 n ok

    //atuadores
    client.subscribe("casa/lamp1");
    client.subscribe("casa/lamp2");
    client.subscribe("casa/lamp3");
    client.subscribe("casa/lamp4");



}

void sensoranalog(){
  pinMode(4,INPUT);
  float hs = analogRead(4);     
  
}

void callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Mensagem recebida do tópico: ");
    Serial.print(topic);

       Serial.print(": ");

    for (int i = 0; i < length; i++) {

        Serial.print((char)payload[i]);
    }

    Serial.println("\n");
    
    if (strcmp(topic,"casa/lamp1") == 0)
    {
        if (payload[0] == '0')
        {
            Serial.println("Desligando luz");
            digitalWrite(LAMP1, LOW);
        }

        if (payload[0] == '1')
        {
            Serial.println("Ligando luz");
            digitalWrite(LAMP1, HIGH);         
        }
    }

     if (strcmp(topic,"casa/lamp2") == 0)
    {
        if (payload[0] == '0')
        {
            Serial.println("Desligando luz");
            digitalWrite(LAMP2, LOW);
        }

        if (payload[0] == '1')
        {
            Serial.println("Ligando luz");
            digitalWrite(LAMP2, HIGH);
        }
    }
     if (strcmp(topic,"casa/lamp3") == 0)
    {
        if (payload[0] == '0')
        {
            Serial.println("Desligando luz");
            digitalWrite(LAMP3, LOW);
        }

        if (payload[0] == '1')
        {
            Serial.println("Ligando luz");
            digitalWrite(LAMP3, HIGH);
        }
    }
     if (strcmp(topic,"casa/lamp4") == 0)
    {
        if (payload[0] == '0')
        {
            Serial.println("Desligando luz");
            digitalWrite(LAMP4, LOW);
        }

        if (payload[0] == '1')
        {
            Serial.println("Ligando luz");
            digitalWrite(LAMP4, HIGH);
        }
    }

    Serial.println();
    Serial.println("----------------------------");

}

void loop() {
    delay(1000);
     
    char MsgtempsoloMQTT[10];
    char MsgUmidadeMQTT[10];
    char MsgUmidadeSoloMQTT[10];
    char MsgTemperaturaMQTT[10];
    
    if(tempsolo.repeat()){       
      float tempC=0;
      sensors.requestTemperatures();       
      tempC = sensors.getTempCByIndex(0);
      sprintf(MsgtempsoloMQTT,"%f",tempC);
      client.publish("estufa/temperatura_solo", MsgtempsoloMQTT);
    }  
      
    if(umdsolo.repeat()){       
      valor_lido = map(analogRead(4),analogSoloMolhado,analogSoloSeco,100,0);
      sprintf(MsgUmidadeSoloMQTT,"%d",valor_lido);
      client.publish("estufa/Umidade_Solo", MsgUmidadeSoloMQTT);
    }
    
    if(umdar.repeat()){        
      float h = dht.readHumidity();
      sprintf(MsgUmidadeMQTT,"%f",h);
      client.publish("estufa/umidade_ar", MsgUmidadeMQTT);
    }

    if(tempar.repeat()){       
      float t = dht.readTemperature();//em Celcius   
      sprintf(MsgTemperaturaMQTT,"%f",t);
      client.publish("estufa/temperatura_ar", MsgTemperaturaMQTT);
    }
 
    client.loop();
}
