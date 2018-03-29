/*
 Thanks to Michael Margolis for UDPSendReceive.pde, I used to start
 
 Sineta - Unidade Remota de automacao da Sinal (URAS!)
 
 
 */


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 89, 1, 222);

unsigned int localPort = 4376;      // Porta IFRN (4376)
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
//char  ReplyBuffer[] = "OK";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

boolean debug = true;
int sineta = 9;


void setup() {
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);

  pinMode(9, OUTPUT);
  pinMode(A0, OUTPUT);
  //if (debug) {
    Serial.begin(9600);
 // }
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
    //Serial.print("Received packet of size ");
    //Serial.println(packetSize);
    //Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    /*for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }*/
    //Serial.print(", port ");
    //Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    //Serial.println("Contents:");
    //Serial.println(packetBuffer);
    
    char senha[9] = {'S','e','n','h','a','3','2','1','\0'};
    char senhaRecebida[9] = {'S','e','n','h','a','3','2','1','\0'};  
    char comando;
    
    //Recuperando a senha da mensagem
    for (int i = 0; i<8;i++){
      senhaRecebida[i] = packetBuffer[i];
    }
    
    //recuperando o comando
    comando = packetBuffer[9];
    
    Serial.print("senhaRecebida: -");
    Serial.print(senhaRecebida);
    Serial.print("- senha: -");
    Serial.print(senha);
    Serial.print("-");    
    jj
    Serial.print("comando: ");
    Serial.println(comando);
    if (strcmp( senha, senhaRecebida ) == 0) {
      Serial.println("Senhas Iguais");
      tratarComando(comando);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("OK");
      Udp.endPacket();
    } else {
     Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("ERR");
      Udp.endPacket();
      return;
    }
  }
  delay(10);
}

void tocar(int segundos) {
  digitalWrite(sineta, HIGH);
  delay(segundos*1000);
  digitalWrite(sineta, LOW);
}

void tratarComando(char comando) {
  switch (comando) {
    case '?':
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      if (sineta == 9) {
        Udp.write("Modo Producao");
      } else {
        Udp.write("Modo Teste");      
      }    
      Udp.endPacket();
      return;
      break;    
    case 's':
      tocar(2);
      break;
    case 'v':
      sineta = 9;
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("Modo Producao");
      Udp.endPacket();
      return;
      break;
    case 't':
     sineta = A0;
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("Modo Teste");
      Udp.endPacket();
      return;
      break;
    case 'a':
      tocar(5);
      delay(500);
      tocar(5);
      break;
    case 'p':
    {
      Serial.print("Modo Personalizado:");
    
      int duracao = (packetBuffer[11])- '0';
      int pausa = (packetBuffer[13])- '0';
      int repeticoes = (packetBuffer[15])- '0';
      Serial.print(duracao);
      Serial.print(":");
      Serial.print(pausa);
      Serial.print(":");      
      Serial.print(repeticoes);      
      for (int i = 0; i < repeticoes;i++){
        tocar(duracao);
        delay(pausa*1000);
      }
    }
      break;
    default: 
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write("Comando nao reconhecido");
      Udp.endPacket();
      return;
    break;
  }
}


/*
  Processing sketch to run with this example
 =====================================================
 
 // Processing UDP example to send and receive string data from Arduino 
 // press any key to send the "Hello Arduino" message
 
 
 import hypermedia.net.*;
 
 UDP udp;  // define the UDP object
 
 
 void setup() {
 udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
 //udp.log( true );     // <-- printout the connection activity
 udp.listen( true );           // and wait for incoming message  
 }
 
 void draw()
 {
 }
 
 void keyPressed() {
 String ip       = "192.168.1.177"; // the remote IP address
 int port        = 8888;    // the destination port
 
 udp.send("Hello World", ip, port );   // the message to send
 
 }
 
 void receive( byte[] data ) {      // <-- default handler
 //void receive( byte[] data, String ip, int port ) { // <-- extended handler
 
 for(int i=0; i < data.length; i++) 
 print(char(data[i]));  
 println();   
 }
 */
