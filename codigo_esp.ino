/**
 * @file Bebedouro.ino
 * Criado por: Saulo Aislan
 * @brief Firmware para controlar o motor bomba do bebedouro para gato.
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define PIN_MOTOR 4 // Pino D2 do Wemos

const char* ssid     = "Bebedouro"; // Nome da rede
const char* password = "123456789"; // Senha da rede

// AsyncWebServer objeto na porta 80
AsyncWebServer server(80);

bool buttonStatus = false; // Habilitando ou não o botão na página
bool motorStatus = false; // Status do motor bomba
uint8_t paramMotorStatus = 0;
uint8_t controlMode = 0;
uint16_t interval = 60; // Intervalo de acionamento do motor no modo automático (timeControl) em minutos

/* Prototipo da funcao */
void withOutControl(void);
void controlManual(void);
void timeControl(int interval);
String processor(const String& var);

/**
 * @brief Função que aciona a bomba, sem controle de tempo ou manual, acionamento constante
 */
void withOutControl(void){
  digitalWrite(PIN_MOTOR, HIGH);
  buttonStatus = false;
}

/**
 * @brief Função para o controle manual da bomba, habilitando o botão na página
 */
void controlManual(void) {
  Serial.println("Controle Manual");
  buttonStatus = true;
}

/**
 * @brief Função de acionamento por tempo do motor bomba
 * @param int Intervalo em minutos de desligada para ligada 
 */
void timeControl(int i_interval){
  i_interval = i_interval*60*1000;
  digitalWrite(PIN_MOTOR, LOW);
  delay(i_interval);
  digitalWrite(PIN_MOTOR, HIGH);
  delay(15000); // Tempo que vai permanecer ligada 15s
}

/**
 * @brief Função para o controle manual da bomba,
 * recebe o status recebido e aciona o motor bomba
 * @param AsyncWebServerRequest ponteiro para o request HTTP_GET
 */
void manualControl(AsyncWebServerRequest* request) {
  bool st;

  if (request->hasParam("st"))
    st = request->getParam("st")->value().toInt();
  else
    st = 0;

  if(st){
    digitalWrite(PIN_MOTOR, HIGH);
    Serial.println("Motor: ON");
    request->send(200, "text/html", SendHTML()); 
  } else {
    digitalWrite(PIN_MOTOR, LOW);
    Serial.println("Motor: OFF");
    request->send(200, "text/html", SendHTML());     
  }
}

/**
 * @brief Função que cria a página (HTML) de configuração
 * @return String com o HTML da página
 */
String SendHTML(){
  String ptr = "<!DOCTYPE html><html>";
  ptr.concat(F("<head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">")); 
  ptr.concat(F("<title>Bebedouro para Gato</title>")); 
  ptr.concat(F("<style>html {font-family: Arial, Helvetica, sans-serif; display: inline-block; margin: 0px auto; text-align: center;}")); 
  ptr.concat(F("body{margin: 0;font-size: 1rem; color: #1e4659;background-color: #f2f2f2;} h3 {color: #444444;margin-bottom: 50px;}")); 
  ptr.concat(F(".topnav{overflow: hidden; background-color: #04448c; text-align: center; color: white;}")); 
  ptr.concat(F("form{border-radius: 5px;border-radius: 5px;padding: 20px;}")); 
  ptr.concat(F("input {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}")); 
  ptr.concat(F(".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}")); 
  ptr.concat(F(".button-on {background-color: #4caf50;}")); 
  ptr.concat(F(".button-off {background-color: #f44336;}")); 
  ptr.concat(F("table{border-collapse: collapse;} td{border: 1px solid #ffc107;padding:8px;}"));    
  ptr.concat(F("</style></head>")); 
  ptr.concat(F("<body><div class=\"topnav\"><h1>Bebedouro para Gato</h1></div>")); 
  ptr.concat(F("<div><form name=\"formMode\"><div style=\"border: 1px solid #ffc107;\"><div style=\"background-color: #ffc107; padding: 10px; font-weight: bold;\"><label for=\"mode\">Modos de operação</label></div>")); 
  ptr.concat(F("<table style=\"width: 100%;\"><tr><td style='width: 50%'><b>Modo atual:</b></td>")); 
  if(paramMotorStatus == 0)
  {
    ptr.concat(F("<td>-</td></tr>")); 
  } else if(paramMotorStatus == 1)
  {
    ptr.concat(F("<td>Ligada direto</td></tr>")); 
    buttonStatus = false;
  } else if(paramMotorStatus == 2)
  {
    ptr.concat(F("<td>Acionamento por intervalo de tempo</td></tr>")); 
    buttonStatus = false;
  } else if(paramMotorStatus == 3)
  {
    ptr.concat(F("<td>Acionamento manual</td></tr>"));
    buttonStatus = true;
  } else if(paramMotorStatus == 4)
  {
    ptr.concat(F("<td>Acionamento manual ou por intervalo de tempo</td></tr>")); 
    buttonStatus = true;
  }
  
  ptr.concat(F("<tr><td colspan=\"2\"><b>Escolha o Modo de operação:</b></td></tr>")); 
  ptr.concat(F("<tr><td colspan=\"2\"><select onchange=\"setmode()\" name=\"opMode\" id=\"mode\">")); 
  ptr.concat(F("<option value=\"-\">-</option>")); 
  ptr.concat(F("<option value=\"/mode?op=1\">Ligada</option>")); 
  ptr.concat(F("<option value=\"/mode?op=2\">Por tempo</option>")); 
  ptr.concat(F("<option value=\"/mode?op=3\">Manualmente</option>")); 
  ptr.concat(F("<option value=\"/mode?op=4\">Por tempo e manualmente</option></select></td></tr></table></form></div>")); 

  if(buttonStatus)
  {
    if(digitalRead(PIN_MOTOR) == LOW)
    {
      ptr.concat(F("<div><div style='background-color:#ffc107;padding: 10px;font-weight: bold;margin: auto;margin-bottom: 2%;width: 50%;'>Bebedouro: <span style='color: #f44336;'>Desligado</span></div><a class='button button-on' href=\"/bomba?st=1\">Ligar</a></div>"));
    } else {
      ptr.concat(F("<div><div style='background-color:#ffc107;padding: 10px;font-weight: bold;margin: auto;margin-bottom: 2%;width: 50%;'>Bebedouro: <span style='color: #4caf50;'>Ligado</span></div><a class='button button-off' href='/bomba?st=0'>Desligar</a></div>"));
    }
  } else {
    ptr.concat(F("<div><a class='button button-on' style='background-color:#c3c3c3;'> - </a></div>"));
  }

  ptr.concat(F("<script type=\"text/javascript\">function setmode(){"));
  ptr.concat(F("location=document.formMode.opMode.options[document.formMode.opMode.selectedIndex].value"));
  ptr.concat(F("}</script>"));
  ptr.concat(F("</body></html>"));
    
  return ptr;
}

/******************* Função em setup (setup) *********************/
void setup(){
  Serial.begin(115200);
  pinMode(PIN_MOTOR, OUTPUT);
  
  Serial.print("Access Point…");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", SendHTML()); 
  });

  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("op"))
    {
      paramMotorStatus = request->getParam("op")->value().toInt();
      controlMode = paramMotorStatus;
    }
    else
    {
      paramMotorStatus = 0;
    }
    Serial.print("Modo: ");
    Serial.println(paramMotorStatus);

    request->send(200, "text/html", SendHTML());
  });

  server.on("/bomba", HTTP_GET, manualControl);
  
  // Start server
  server.begin();
}

/******************* Função em loop (loop) *********************/
/**
 * @brief Máquina de estados para os modos de acionamento do motor bomba
 */
void loop(){
  switch(controlMode){
    case 1:
      withOutControl(); // Acionamento direto, motor ligado todo o tempo
      break;
    case 2:
      timeControl(interval); // Acionamento por tempo (minutos)
      break;
    case 3:
      controlManual(); // Acionamento manual
      break;
    case 4: // Caso 4 o motor poderá ser acionado manualmente e pelo tempo
      controlManual(); // Acionamento manual
      timeControl(interval); // Acionamento direto, motor ligado todo o tempo
      break;
    default:
      timeControl(30); // Acionamento por tempo (30 minutos)
  }
}