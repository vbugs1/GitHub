//Projeto_NodeMCU_1
//Univates_2018_2_Vicente_Mauricio
//Sistemas_Microprocessados_Avançados_Dispositivos_para automação_residencial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <stdlib.h>
#include <stdio.h>
#include <EEPROM.h>
#define MEM_ALOC_SIZE 64
#include <PubSubClient.h>
//#include <IRsend.h>

ESP8266WebServer server(80);

float temperatura;
char mensagem [100];
char setpoint = 27;
char novaRede;
char novaSenha;

const char* BROKER_MQTT = "m10.cloudmqtt.com"; // ip/host do broker
int BROKER_PORT = 15557; // porta do broker
const char *BROKER_USER = "fvhdnjss";
const char *BROKER_PASSWORD = "HWW0wyt9laS-";

/*---------------------------------------------------------------------------*/
/*prototypes*/
void initPins();
void initSerial();
void initWiFi();
void initMQTT();
void setup();
void setup_configuracao();
void configura ();
void salvar ();
void setup_operacao();
void callback(char* topic, byte* payload, unsigned int length);
void loop_operacao();
void loop();
void liga();
void desliga();

#define ID_MQTT "Projeto_1"
#define TOPIC_SUBSCRIBE ""

WiFiClient microsavancados;
PubSubClient client(microsavancados);

/*---------------------------------------------------------------------------*/
/*SETUP*/

void setup()
{
  // IRsend irsend(4)
  pinMode (D1, INPUT);
  pinMode (A0, INPUT);

  delay(1000);
  Serial.begin(115200);
  Serial.println();


  if (digitalRead(D1) == 1) {
    Serial.println("Configuração");     //Cria rede para configurar conexão no IP 192.168.
    setup_configuracao ();
  } else {
    Serial.println("Operação");        //Conecta na rede configurada
    setup_operacao();
  }
}

/*---------------------------------------------------------------------------*/
/*CRIA REDE*/

void setup_configuracao () {

  const char *ssid = "micros_1_A";
  const char *password = "12345678";

  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", configura);
  server.begin();
  Serial.println("HTTP server started");
}

void loop_configuracao() {
  server.handleClient();
}

/*---------------------------------------------------------------------------*/
/*LÊ DADOS DIGITADOS*/

void configura () {
  //MODO CONFIGURAÇÃO - PARA ENTRADA DE NOME DA REDE, SENHA E SET POINT DE OPERAÇÃO
  server.send(200, "text/html",
              "<html>\n"
              "<body>\n"
              "<form action=\"/salvar\">\n"
              "    Rede name: <br>\n"
              "    <input type=\"text\" name=\"NomeRede\" value=\"Informe a Rede\">\n"
              "    <br> Senha name: <br>\n"
              "    <input type=\"text\" name=\"SenhaRede\" value=\"Informe a senha\">\n"
              "    <br> Setpoint name: <br>\n"
              "    <input type=\"text\" name=\"SetPoint\" value=\"SetPoint de temperatura\">\n"
              "    <br><br>\n"
              "    <input type=\"submit\" value=\"Submit\">\n"
              "</form>\n"
              "<p>If you click the \"Submit\" button, the form - data will be sent to a page called \"/salvar\".</p>\n"
              "</body>\n"
              "</html>"
             );
}
/*---------------------------------------------------------------------------*/
/*SALVA DADOS DE ENTRADA*/

void salvar () {
  //SALVA NOME DA REDE E SENHA

  Serial.println(novaRede);
  Serial.println(novaSenha);
  Serial.println(setpoint);

  server.send(200, "text/html", "<p>ok, senha salva</p>");

  EEPROM.begin(MEM_ALOC_SIZE);
  EEPROM.write(0, novaRede);
  EEPROM.write(1, novaSenha);
  EEPROM.write(2, setpoint);
  EEPROM.end();
}

/*---------------------------------------------------------------------------*/
/*SETUP OPERAÇÃO*/

void setup_operacao() {
  //CONECTA A REDE WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin("micros_2", "12345678");        // pré-configuração rede wifi para conexão servidor

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  client.setServer(BROKER_MQTT, BROKER_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", BROKER_USER, BROKER_PASSWORD)) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

  client.subscribe("temperatura");
  client.subscribe("setpoint");

}

/*---------------------------------------------------------------------------*/
/*Fução placa MCU2 para controle da saída D1*/
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  int i;
  char value[100];

  Serial.print("Message:");
  for (i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    value[i] = (char)payload[i];
  }
  value[i] = '\0';

  //if (topic == "temperatura") {
  float temp = atof(value);    //converte uma cadeia de caracteres em um valor de ponto flutuante de precisão dupla.

  Serial.print("TEMP: ");
  Serial.println(temperatura);        // imprime temperatura serial
  Serial.print("SETPOINT: ");
  Serial.println(setpoint);

  if (temp > setpoint) {
    liga ();
  }

  if (temp < setpoint)
  {
    desliga();
  }

  Serial.println();
  Serial.println("-----------------------");
  //}
}

/*---------------------------------------------------------------------------*/ //Medição temperatura MCU1
/*LOOP OPERAÇÃO*/

void loop_operacao() {

  client.loop();

  float temperatura = (analogRead(A0) * 330.0) / 1023.0;    //Lê pino A0 e converte mV para ºC
  sprintf(mensagem, "%f", temperatura);       //converte valor de temperatura (float) to char
  client.publish("temperatura", mensagem);    //publica na tag valor de temperatura para o servidor
  delay(2000);
}

/*---------------------------------------------------------------------------*/
/*LOOP*/

void loop() {

  //MQTT.loop();

  if (digitalRead(D1) == 1) {
    loop_configuracao();
  } else {
    loop_operacao();
  }
}

/*---------------------------------------------------------------------------*/
/*Fução placa MCU2 liga/desliga da saída D1*/
void liga() {
  Serial.print("LIGA");/*

  uint16_t liga[] = {4600, 4250, 750, 1450, 750, 400, 700, 1450, 700, 1500, 700, 400, 700, 450, 700, 1500, 700, 450, 700, 450, 700, 1550, 700, 450, 700, 450, 700, 1500, 700, 1550, 700, 450, 700, 1550, 700,
                     1500, 700, 450, 650, 450, 650, 1550, 650, 1550, 650, 1500, 650, 1550, 650, 1500, 650, 500, 650, 1550, 650, 1550, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500,
                     650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 5150, 4450, 4350, 650, 1550, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 1550, 650,
                     500, 650, 500, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 500, 650, 1600, 650, 1550, 650, 500,
                     650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 500, 600, 500, 600, 1600, 650, 1550, 650, 500, 600, 500, 600, 1600, 650, 1550, 650, 1600, 650, 1600, 650
                    };

  irsend.sendRaw(desliga, sizeof(desliga) / sizeof(desliga[0]), 38);  // Send a raw data capture at 38kHz.*/
}

void desliga () {
  Serial.print("DESLIGA");

  /*
    uint16_t desliga[] = {4600, 4250, 750, 1450, 750, 400, 700, 1450, 700, 1500, 700, 400, 700, 450, 700, 1500, 700, 450, 700, 450, 700, 1550, 700, 450, 700, 450, 700, 1500, 700, 1550, 700, 450, 700, 1550, 700,
                          450, 700, 1550, 700, 1500, 700, 1550, 650, 1550, 650, 450, 650, 1550, 650, 1500, 650, 1550, 650, 450, 650, 500, 650, 450, 650, 500, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 500,
                          650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 5150, 4450, 4350, 650, 1550, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 1550, 650,
                          500, 650, 500, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 500,
                          650, 500, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 600, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1600, 650
                         };  // SAMSUNG B24D7B84

    irsend.sendRaw(desliga, sizeof(desliga) / sizeof(desliga[0]), 38);  // Send a raw data capture at 38kHz.
  */
}
