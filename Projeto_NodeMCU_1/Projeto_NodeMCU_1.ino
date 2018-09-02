//Projeto_NodeMCU_1
//Univates_2018_2_Vicente_Mauricio
//Sistemas_Microprocessados_Avançados_Dispositivos_para automação_residencial


//#include <IRremote.h>
//#include <IRremoteInt.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <stdlib.h>
#include <stdio.h>
#include <EEPROM.h>
#define MEM_ALOC_SIZE 64
#include <PubSubClient.h>
#define ID_MQTT "Projeto_1"
#define TOPIC_SUBSCRIBE ""

ESP8266WebServer server(80);
WiFiClient microsavancados;
PubSubClient client(microsavancados);

//IRsend irsend;

// Define nome da rede e senha a ser utilizado no SETUP CONFIGURAÇÃO
const char *ssid = "micros_1_A";
const char *password = "12345678";

//Variáveis internas
float temperatura;
char mensagem [100];
int isterese = 4;
char novaRede;
char novaSenha;
float setpoint = 25;
int estado = 0;
String Status = "Ar Desligado";

//Váriaveis do MQTT
const char* BROKER_MQTT = "m10.cloudmqtt.com"; // ip/host do broker
int BROKER_PORT = 15557; // porta do broker
const char *BROKER_USER = "fvhdnjss";
const char *BROKER_PASSWORD = "HWW0wyt9laS-";

// Rotinas
void configura ();
void salvar ();
void desliga ();
void liga ();
void espaco();

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                              SETUP GERAL
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void setup()
{

  // IRsend irsend(4)

  //Define entradas (D1 - modo oper.(L) conf. (H) && A0 - sensor de temperatura)
  pinMode (D1, INPUT);
  pinMode (A0, INPUT);

  //Inicia o monitor serial
  Serial.begin(115200);
  Serial.println();
  delay(1000);
  
  espaco();
  Serial.println("Sistemas Microprocessados Avançados");
  Serial.println("Trabalho nota 1");
  Serial.println("Mauricio Lorenzon && Vicente Bugs");
  espaco();


  //SETUP CONFIGURAÇÃO

  if (digitalRead(D1) == 1) {

    //Cria rede WiFi
    Serial.println("MODO: Configuração");
    espaco();

    //Cria rede com parametros pré definidos
    Serial.print("Configurando ponto de acesso...");
    WiFi.softAP(ssid, password);
    espaco();


    //Apresenta endereço de IP
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Endereço de IP: ");
    Serial.println(myIP);

    //Se acessar tal IP - direciona para a rotina "CONFIGURA"
    server.on("/", configura);
    server.begin();
    Serial.println("Servidor iniciado");
    espaco();
  }


  //SETUP OPERAÇÃO

  else {
    Serial.println("MODO: operação");

    //CONECTA A REDE WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin("Lorenzon", "piacentidez");

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi conectado");

    // Iniciado o servidor
    server.begin();
    Serial.println("Iniciando o servidor");

    // Apresenta o endereço de IP
    Serial.println(WiFi.localIP());

    client.setServer(BROKER_MQTT, BROKER_PORT);
    client.setCallback(callback);

    while (!client.connected()) {
      Serial.println("Conectado ao servidor MQTT...");

      if (client.connect("ESP8266Client", BROKER_USER, BROKER_PASSWORD)) {
        Serial.println("Conectado");

      } else {
        Serial.print("Falha na conexão ");
        Serial.print(client.state());
        delay(2000);

      }
      espaco();
    }

    client.subscribe("temperatura");
    client.subscribe("setpoint");
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                              FUNÇÃO LOOP
  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void loop() {

  //MQTT.loop();


  //Loop configuração

  if (digitalRead(D1) == 1) {
    server.handleClient();
  }


  //Loop operação

  else {
    client.loop();

    //Lê valor de temperatura do sensor conectado a porta A0
    float temperatura = (analogRead(A0) * 330.0) / 1023.0;    //Lê pino A0 e converte mV para ºC
    sprintf(mensagem, "%f", temperatura);                     //Converte valor de temperatura (float) to char

    //Publica no servidor o valor lido
    client.publish("temperatura", mensagem);                  //publica na tag 'temperatura' o valor de temperatura

    delay(2000);
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                  EM MODO CONFIGURAÇÃO - LÊ OS DADOS DIGITADOS (REDE, SENHA, SETPOINT)
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void configura () {
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


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                   SALVA OS DADOS INFORMADOS NA MEMÓRIA
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void salvar () {

  //Apresenta os dados informados
  Serial.println("Nome da rede: ");
  Serial.print(novaRede);
  Serial.println("Senha: ");
  Serial.print(novaSenha);
  Serial.println("SetPoint: ");
  Serial.print(setpoint);

  server.send(200, "text/html", "<p>ok, senha salva</p>");

  //Salva os dados na memória EEPROM
  EEPROM.begin(MEM_ALOC_SIZE);
  EEPROM.write(0, novaRede);
  EEPROM.write(1, novaSenha);
  EEPROM.write(2, setpoint);
  EEPROM.end();
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                FUNÇÃO QUE RECEBE OS VALORES PUBLICADOS NO SERVIDOR
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void callback(char* topic, byte* payload, unsigned int length) {

  //Converte a informação (ponteiro) para char
  int i;
  char value[100];
  for (i = 0; i < length; i++) {
    value[i] = (char)payload[i];
  }
  value[i] = '\0';


  //Verifica se valor recebido é de setpoint
  if (strcmp(topic, "setpoint") == 0 ) {
    setpoint = atof(value);                                    //converte char para float e salva valor em 'setpoint'

    //Apresenta novo valor de set point
    espaco();
    Serial.print("Novo SETPOINT: ");
    Serial.println(setpoint);
    espaco();
  }

  //Verifica se valor recebido é de temperatura
  if (strcmp(topic, "temperatura") == 0 ) {
    float temp = atof(value);

    //Apresenta valor de temperatura recebido
    Serial.print("SetPoint: ");
    Serial.println(setpoint);
    Serial.println("");
    Serial.print("Valor de temperatura recebido: ");
    Serial.println(temp);

    //Verifica se temperatura é maior que setpoint
    if (temp > (setpoint + (isterese / 2)) && estado == 0) {
      liga ();
      estado = 1;
      Status = "Ar ligado";
    }

    //Verifica se temperatura é maior que setpoint
    if (temp < (setpoint - (isterese / 2)) && estado == 1) {
      desliga();
      estado = 0;
      Status = "Ar desligado";
    }

    //Exibe Status de funcionamento do equipamento (EM FUNÇÃO DO COMANDO LIGA/DESLIGA)
    Serial.print ("Status: ");
    Serial.println(Status);
    espaco();
  }
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                  FUNÇÃO LIGA
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void liga() {
  Serial.print("teste LIGA");
  int khz = 38;
  /*
    unsigned sinalLiga[] = {4600, 4250, 750, 1450, 750, 400, 700, 1450, 700, 1500, 700, 400, 700, 450, 700, 1500, 700, 450, 700, 450, 700, 1550, 700, 450, 700, 450, 700, 1500, 700, 1550, 700, 450, 700, 1550, 700,
                          1500, 700, 450, 650, 450, 650, 1550, 650, 1550, 650, 1500, 650, 1550, 650, 1500, 650, 500, 650, 1550, 650, 1550, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500,
                          650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 5150, 4450, 4350, 650, 1550, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 1550, 650,
                          500, 650, 500, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 500, 650, 1600, 650, 1550, 650, 500,
                          650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 500, 600, 500, 600, 1600, 650, 1550, 650, 500, 600, 500, 600, 1600, 650, 1550, 650, 1600, 650, 1600, 650
                         };
  */
  //irsend.sendRaw(sinalLiga, sizeof(sinalLiga) / sizeof(sinalLiga[0]), khz);  // Send a raw data capture at 38kHz.
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                  FUNÇÃO DESLIGA
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void desliga () {
  Serial.print("teste DESLIGA");
  int khz = 38;
  /*
    unsigned sinalDesliga[] = {4600, 4250, 750, 1450, 750, 400, 700, 1450, 700, 1500, 700, 400, 700, 450, 700, 1500, 700, 450, 700, 450, 700, 1550, 700, 450, 700, 450, 700, 1500, 700, 1550, 700, 450, 700, 1550, 700,
                               450, 700, 1550, 700, 1500, 700, 1550, 650, 1550, 650, 450, 650, 1550, 650, 1500, 650, 1550, 650, 450, 650, 500, 650, 450, 650, 500, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 500,
                               650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1550, 650, 5150, 4450, 4350, 650, 1550, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 1550, 650,
                               500, 650, 500, 650, 1600, 650, 500, 650, 500, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 500, 650, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 1600, 650, 1550, 650, 1600, 650, 500, 650, 500, 650, 500,
                               650, 500, 650, 1550, 650, 500, 650, 500, 650, 1600, 650, 1600, 650, 1550, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 650, 500, 600, 1550, 650, 1600, 650, 1550, 650, 1600, 650, 1600, 650
                              };  // SAMSUNG B24D7B84
  */
  //  irsend.sendRaw(sinalDesliga, sizeof(sinalDesliga) / sizeof(sinalDesliga[0]), khz);  // Send a raw data capture at 38kHz.
}


void espaco() {
  Serial.println("--------------------------------------------");
  Serial.println("");
}
