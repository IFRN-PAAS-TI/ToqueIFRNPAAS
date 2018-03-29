//bibliotecas para o shield ethernet
#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h> // A comunicacao NTP e feita utilizando UDP
//As duas bibliotecas necessárias para manipular o RTC DS3231
#include <DS3231.h>      //Biblioteca para manipulação do DS3231


// ------------ CONFIGURACOES NTP ------------------
//porta padrao do ntp
unsigned int localPort = 8888;

//servidor ntp padrao
char timeServer[] = "10.94.0.155";

//A hora no NTP vem nos primeiros 48bytes da mensagem 
const int NTP_PACKET_SIZE = 48;

//Um buffer para armazenar as mensagens
byte packetBuffer[ NTP_PACKET_SIZE];

//Uma instancia UDP permitirá a comunicação utilizando este protocolo
EthernetUDP Udp;

// ------------- FIM DAS CONFIGURACOES NTP ----------


// ------------- CONFIGURACOES DO RTC ---------------

DS3231  rtc(SDA, SCL);              //Criação do objeto do tipo DS3231

// ------------- FIM DAS CONFIGURACOES DO RTC -------

//porta digital onde o shield de relogio esta ligado
int relogio = 13;

void setup() {
  //inicializar seiral - ATENÇÃO - Se a serial não se conectar o Arduino entrara
  //em loop nesta fase
  init_serial();

  //inicializar relogio RTC
  init_clock();
  
  //inicializar rede
  init_network();

  //obter hora via ntp
  obtain_current_time();

  
  
}

void loop() {
  // put your main code here, to run repeatedly:

}

//inicializa a rede do arduino utilizando DHCP
void init_network() {
  byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  if (Ethernet.begin(mac) == 0){
    Serial.println("Nao foi possivel obter um endereco IP via DHCP");
    Serial.println("Nao irei fazer mais nada.");
    while (true) {
      ; //Nao ha porque continuar, deixando o arduino em loop
    }
  }
  
  //mostrar IP
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());
}

//inicializa a comunicacao serial do arduino
void init_serial() { 
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}


//obtem a hora atual e aplica no vetor de toques
void obtain_current_time() {
  //inicializar socket udp
  Udp.begin(localPort);
  
  //enviar pacote udp pra servidor ntp
  Serial.println("Enviando requisicao NTP...");
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
    hours = (epoch  % 86400L) / 3600;
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
  }

  Ethernet.maintain();
  Serial.println("Não obti resposta NTP, finalizando.");
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


