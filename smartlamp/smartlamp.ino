#include "DHT.h"

// Defina os pinos de LED e LDR
// Defina uma variável com valor máximo do LDR (4000)
// Defina uma variável para guardar o valor atual do LED (10)
#define ledPin 26
#define ldrPin 12
#define DHT_PIN 33
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);
float humidity, temperature;
int ledValue = 10;

// Faça testes no sensor ldr para encontrar o valor maximo e atribua a variável ldrMax
int ldrMax = 4096;
//int ldrMax = 4063;

String minhaString;
String comando;
String valor;


void setup() {
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(DHT_PIN, INPUT);
    analogSetAttenuation(ADC_11db);
    analogWrite(ledPin, map(ledValue, 0, 100, 0,255));
    Serial.printf("SmartLamp Initialized.\n");
    dht.begin();
    //processCommand("GET_LDR", "");
}

// Função loop será executada infinitamente pelo ESP32
void loop() {
    //Obtenha os comandos enviados pela serial 
    //e processe-os com a função processCommand
  if(Serial.available() != 0){
    minhaString = Serial.readStringUntil('\n');
    comando = "";
    valor = "";  
    for(int i = 0; i < minhaString.length(); i++){
      if(minhaString[i] != ' '){
        comando += minhaString[i];
      }else{
        valor += minhaString.substring(i+1);
        break;
      }
    }
    processCommand(comando,valor);
  }
}

void processCommand(String command,String valor) {
    // compare o comando com os comandos possíveis e execute a ação correspondente
    if(command == "SET_LED"){
      ledUpdate(valor);
    }
    else if(command == "GET_LED"){
     Serial.print("RES GET_LED ");
     Serial.println(ledValue); 
    }else if(command == "GET_LDR"){ //LDR
      int valor_ldr = ldrGetValue();
      Serial.print("RES GET_LDR ");
      Serial.println(valor_ldr);
    }else if(command == "GET_TEMP") {
      temperature = dht.readTemperature(); 
      Serial.print("RES GET_TEMP ");
      Serial.println(temperature);
    }else if(command == "GET_HUM") {
      humidity = dht.readHumidity();
      Serial.print("RES GET_HUM ");
      Serial.println(humidity);
    }else{
      Serial.println("ERR Unknown command");
    }
}

// Função para atualizar o valor do LED
void ledUpdate(String valor) {
    // Valor deve convertar o valor recebido pelo comando SET_LED para 0 e 255
    // Normalize o valor do LED antes de enviar para a porta correspondente      
      ledValue = valor.toInt();
      if(ledValue >= 0 && ledValue <= 100){
        int value = map(ledValue, 0, 100, 0, 255);
        analogWrite(ledPin,value);     
        Serial.println("RES SET_LED 1");   
      }else{
        Serial.println("RES SET_LED -1");
      }
}

// Função para ler o valor do LDR
int ldrGetValue() {
    // Leia o sensor LDR e retorne o valor normalizado entre 0 e 100
    // faça testes para encontrar o valor maximo do ldr (exemplo: aponte a lanterna do celular para o sensor)       
    // Atribua o valor para a variável ldrMax e utilize esse valor para a normalização
    int analogValue = analogRead(ldrPin);
    return analogValue = map(analogValue, 0, ldrMax, 0, 100);
    //return analogValue = map(analogValue, 32, ldrMax, 0, 100);
}
