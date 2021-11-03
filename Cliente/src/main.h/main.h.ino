//bibliotecas para o shield ethernet
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h> // A comunicacao NTP e feita utilizando UDP
#include <DS3231.h>      //Biblioteca para manipulação do DS3231
//biblioteca necessária para executar o watchdog e reiniciar o arduino
#include <avr/wdt.h> 


// ------------ CONFIGURACOES NTP ------------------

//GMT do Brasil (-3)
short gmt = -3;

//porta padrao do ntp
unsigned int localPort = 8888;

//servidor ntp padrao
char timeServer[] = "a.ntp.br";

//A hora no NTP vem nos primeiros 48bytes da mensagem 
const int NTP_PACKET_SIZE = 48;

//Um buffer para armazenar as mensagens
byte packetBuffer[ NTP_PACKET_SIZE];

//Uma instancia UDP permitirá a comunicação utilizando este protocolo
EthernetUDP Udp;


// ------------- FIM DAS CONFIGURACOES NTP ----------


// ------------- CONFIGURACOES DO RTC ---------------

DS3231  rtc(SDA, SCL);              //Criação do objeto do tipo DS3231

//um struct para ajudar na obtencao das horas

const uint8_t TOQUE_NORMAL = 0;
const uint8_t RESET = 1;

struct Tempo {
  uint8_t   hour;
  uint8_t   min;
  uint8_t   tempo_toque;
  uint8_t   tempo_pausa;
  uint8_t   repeticoes;
  uint8_t   tipo;
};

//Um objeto de hora para ajudar nas comparacoes
Tempo horaAux = {0, 0};

//Numero de horarios
const unsigned short int tamH = 19;

//Lista de Horarios Padrao
Tempo horarios[tamH];

// ------------- FIM DAS CONFIGURACOES DO RTC -------

//porta digital onde o relay esta ligado
int relay = 7;

//Uma variavel auxiliar para que o arduino nao precise usar a funcao sleep
// e acabe atrasando o toque pois essa funcao nao eh precisa
unsigned short minuto_toque = 60;


void setup() {
  
  //preparar o arduino para, caso seja necessário, executar um RESET
  init_reset();
  
  //inicializar serial
  init_serial();

  //inicializar porta da sineta
  init_relay();

  //inicializar relogio RTC
  init_clock();
  
  //inicializar rede
  init_network();

  //obter hora via ntp
  obtain_current_time(); 

  //inicializar toques padrao
  init_toque_array();

  Serial.println("Configuracao inicial finalizada.");
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Verificando se a hora atual eh uma hora de toque.");
  Serial.print("Hora atual: ");
  Serial.println(rtc.getTimeStr());

  horaAux = {rtc.getTime().hour, rtc.getTime().min};

  for (unsigned short i = 0; i < tamH; i++) {
    if (horaAux.hour == horarios[i].hour && horaAux.min == horarios[i].min && minuto_toque != horaAux.min) {

      switch (horarios[i].tipo) {
        case TOQUE_NORMAL:
          tocar(&horarios[i]);
          minuto_toque = horaAux.min;
        break;
        case RESET:
          //resetar
          reset();
        break;
      }
    }
  }
}

//inicializa a rede do arduino utilizando DHCP
void init_network() {
  byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  if (Ethernet.begin(mac) == 0){
    Serial.println("Nao foi possivel obter um endereco IP via DHCP");
    Serial.println("Continuando sem IP.");
  } else {
    //mostrar IP
    Serial.print("IP = ");
    Serial.println(Ethernet.localIP());
  }
}

//inicializa a comunicacao serial do arduino
void init_serial() { 
  Serial.begin(9600);
}


//obtem a hora atual e aplica no vetor de toques
void obtain_current_time() {
  //inicializar socket udp
  Udp.begin(localPort);
  
  //enviar pacote udp pra servidor ntp
  Serial.println("Enviando requisicao NTP...");
  Serial.print("Servidor configurado: ");
  Serial.println(timeServer);
  sendNTPpacket(timeServer);

  // espere uma resposta chegar
  delay(5000);

  if (Udp.parsePacket()) {
    Serial.println("Resposta NTP obtida!");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Quantidade de segundos desde 01/01/1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Hora Unix = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    unsigned long hours = 0;
    unsigned long minutes = 0;
    unsigned long seconds = 0;

    // print the hour, minute and second:
    Serial.print("Seu UTC eh ");       // UTC is the time at Greenwich Meridian (GMT)
    hours = ((epoch  % 86400L) / 3600) + gmt;
    Serial.print(hours); // print the hour (86400 equals secs per day)
    Serial.print(':');
    minutes = (epoch % 3600) / 60;
    if (minutes < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print(minutes); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    seconds = epoch % 60;
    if (seconds < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(seconds); // print the second

    //Aplicar hora, minutos e segundos no shield de relogio
    rtc.setTime(hours, minutes, seconds);     // Set the time to 12:00:00 (24hr format)
    Serial.println(rtc.getTimeStr());
  } else {
    Serial.println("Não obti resposta NTP.");
    Serial.print("Hora atual do relógio: ");
    Serial.println(rtc.getTimeStr());
  }

  Ethernet.maintain();
  
}

//envia um pacote UDP para o servidor address, anota a resposta no objeto Udp
void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void init_clock() {
  rtc.begin();   //Configurando valores iniciais do RTC
  Serial.println("Relogio inicializado.");
}

void init_toque_array() {
  Serial.println("Inicializando conjunto de horarios padrao.");

  //FORMATO {HORA, MINUTO, TEMPO_DE_TOQUE, TEMPO_DE_PAUSA, 
  //         NUMERO_DE_REPETICOES, TIPO}
  
  //07:00
  horaAux = {7 , 0, 10, 2, 1, TOQUE_NORMAL};
  horarios[0] = horaAux;
  
  //07:45
  horaAux = {7 , 45, 10, 2, 1, TOQUE_NORMAL};
  horarios[1] = horaAux;

  //08:30
  horaAux = {8, 30, 20, 2, 1, TOQUE_NORMAL};
  horarios[2] = horaAux;

  //08:50
  horaAux = {8, 50, 20, 2, 1, TOQUE_NORMAL};
  horarios[3] = horaAux;

  //09:35
  horaAux = {9, 35, 10, 2, 1, TOQUE_NORMAL};
  horarios[4] = horaAux;

  //10:20
  horaAux = {10, 20, 20, 2, 1, TOQUE_NORMAL};
  horarios[5] = horaAux;

  //10:30
  horaAux = {10, 30, 20, 2, 1, TOQUE_NORMAL};
  horarios[6] = horaAux;

  //11:15
  horaAux = {11, 15, 10, 2, 1, TOQUE_NORMAL};
  horarios[7] = horaAux;

  //12:00
  horaAux = {12, 0, 10, 2, 1, TOQUE_NORMAL};
  horarios[8] = horaAux;

  //13:00
  horaAux = {13, 0, 10, 2, 1, TOQUE_NORMAL};
  horarios[9] = horaAux;
  
  //13:45
  horaAux = {13, 45, 10, 2, 1, TOQUE_NORMAL};
  horarios[10] = horaAux;

  //14:30
  horaAux = {14, 30, 20, 2, 1, TOQUE_NORMAL};
  horarios[11] = horaAux;

  //14:50
  horaAux = {14, 50, 20, 2, 1, TOQUE_NORMAL};
  horarios[12] = horaAux;

  //15:35
  horaAux = {15, 35, 10, 2, 1, TOQUE_NORMAL};
  horarios[13] = horaAux;

  //16:20
  horaAux = {16, 20, 20, 2, 1, TOQUE_NORMAL};
  horarios[14] = horaAux;

  //16:30
  horaAux = {16, 30, 20, 2, 1, TOQUE_NORMAL};
  horarios[15] = horaAux;

  //17:15
  horaAux = {17, 15, 10, 2, 1, TOQUE_NORMAL};
  horarios[16] = horaAux;

  //18:00
  horaAux = {18, 0, 10, 2, 1, TOQUE_NORMAL};
  horarios[17] = horaAux;

  //23:59 reiniciar arduino para evitar atrasos na hora
  horaAux = {23, 59, 0, 0, 0, RESET};
  horarios[18] = horaAux;

  Serial.println("Conjunto de horarios padrao inicializado.");
}

void tocar(int segundos) {
  digitalWrite(relay, LOW);
  delay(segundos*1000);
  digitalWrite(relay, HIGH);
}

void tocar(Tempo *t) {
  for (uint8_t i = 0; i < (*t).repeticoes; i++) {
    tocar((*t).tempo_toque);
    delay((*t).tempo_pausa * 1000);
  }
}

void init_relay() {
  Serial.println("Inicializando porta da sineta.");
  digitalWrite(relay, HIGH);
  pinMode(relay, OUTPUT);
}

void init_reset() {
  MCUSR = 0;  // clear out any flags of prior resets.  
  wdt_disable(); //ensure watchdog is disabled
}

void reset() {
  Serial.println("Um horario de RESET foi acionado.");
  Serial.println("Reiniciando arduino.");
  // reset the Arduino
  // esperar 60s para que o arduino nao entre em loop de reset
  delay(60000);
  wdt_disable();
  wdt_enable(WDTO_15MS);
  while (true) {}
}



