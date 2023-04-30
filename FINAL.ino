#include <neotimer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 23
#define DHTTYPE DHT11
#define SENSOR_PIN  12 //DS18

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";
int valor_lido;
int analogSoloSeco = 2500; //VALOR MEDIDO COM O SOLO SECO
int analogSoloMolhado = 850; //VALOR MEDIDO COM O SOLO MOLHADO

//atuadores
int LAMP2 = 5;
int INDICADOR = 18;

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(SENSOR_PIN);
DallasTemperature sensors(&oneWire);

Neotimer tempar;
Neotimer tempsolo;
Neotimer umdar;
Neotimer umdsolo;

 WiFiManager wm;

void setup() {

    pinMode(34,INPUT);
    pinMode(27,OUTPUT); 
    pinMode (LAMP2, OUTPUT);
    pinMode (INDICADOR,OUTPUT);

    digitalWrite (INDICADOR,HIGH);
    digitalWrite(27,LOW);
    dht.begin();
    sensors.begin();
    Serial.begin(115200);
    //define o tempo de leitura dos sensores
    tempar.set(2000);
    tempsolo.set(5000);
    umdar.set(5000);
    umdsolo.set(10000);
       
    bool res;
    res = wm.autoConnect("ESP_TESTE",""); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {  
        Serial.println("connected...yeey :)");
    }
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    //conecta no broker quando liga o esp32
    while (!client.connected()) {
        Serial.println("Conectando com o Broker (MQTT)…");
        String clientId = "";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("Conectado com o Broker");
            digitalWrite (INDICADOR,LOW);
        } else {
            Serial.print("Falha na conexão. Erro: ");
            Serial.print(client.state());
            Serial.println("\n");
            delay(2000);
            digitalWrite (INDICADOR,HIGH);    
        }
    }
    Serial.println("Enviando status de conexão para o smartphone\n");
    Serial.println("Status de conexão OK\n");

    //sensores
    client.publish("estufa/conexao", "1");
    client.publish("estufa/umidade_ar", "1"); //dht11 
    client.publish("estufa/umidade_Solo", "1"); //fc_28 
    client.publish("estufa/temperatura_ar", "1");//dht11 
    client.publish("estufa/temperatura_solo", "1"); //ds18b20 

    //atuadores

    client.subscribe("casa/lamp2");
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
    
     if (strcmp(topic,"casa/lamp2") == 0){
        if (payload[0] == '0'){
            Serial.println("Desligando luz");
            digitalWrite(LAMP2, LOW);
        }
        if (payload[0] == '1'){
            Serial.println("Ligando luz");
            digitalWrite(LAMP2, HIGH);
        }
    }
    //Serial.println();
    Serial.println("----------------------------");
}

void loop() {
  sensoranalog();
    //reconectar caso a internet caia
   if (digitalRead(34) == HIGH) {
      Serial.println("Abertura Portal");
      digitalWrite(27,HIGH); //Acende LED 
      wm.resetSettings();       //Apaga rede salva anteriormente
      if(!wm.startConfigPortal("ESP32-TESTE", "") ){ //Nome da Rede e Senha gerada pela ESP
        Serial.println("Falha ao conectar"); //Se caso não conectar na rede mostra mensagem de falha
        delay(2000);
        ESP.restart(); //Reinicia ESP após não conseguir conexão na rede
      }
      else{       //Se caso conectar 
        Serial.println("Conectado na Rede!!!");
        ESP.restart(); //Reinicia ESP após conseguir conexão na rede 
      }
   }
   //reconecta na rede apos perder conexão
    while (!client.connected()) {
        Serial.println("Conectando com o Broker (MQTT)…");
        String clientId = "";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqttUser, mqttPassword )) {
            Serial.println("Conectado com o Broker");
            digitalWrite (INDICADOR,LOW);
            
        } else {
            Serial.print("Falha na conexão. Erro: ");
            Serial.print(client.state());
            Serial.println("\n");
            digitalWrite (INDICADOR,HIGH);
            delay(6000);
        }
  }            
     //topicos para envio de leitura MQTT
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
