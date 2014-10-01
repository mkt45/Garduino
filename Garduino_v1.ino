/*
 * Garduino Time sketch
 *
 */
 
 #include <Time.h>
 #include <TimeAlarms.h>
 #include <Ethernet.h>
 #include <EthernetUdp.h>
 #include <SPI.h>
  

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
// NTP Servers:
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov

const int timeZone = -3;     // Fuso horário
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

//int t;

EthernetUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

 
int moistureSensor=A0;  // Indica porta 0 analógica para o medidor de umidade A1.
int moisture_val;


int Rele=2;  //Indica porta 2 para o relé.

void digitalClockDisplay(){     // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  //Serial.write(" ");
  //Serial.write(day());
  //Serial.write(" ");
  //Serial.write(month());
  //Serial.write(" ");
  //Serial.write(year());
  Serial.println();
   }


void printDigits(int digits){
  // utility function for clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
   }

void Ligar(){
   Serial.print("Agua Ligada");
   Serial.println();
   digitalWrite(Rele, HIGH);
   }


void Desligar(){
   Serial.print("Agua Desligada");
   Serial.println();
   digitalWrite(Rele, LOW);
   }   
   
   
void Irrigar() {      
    if (moisture_val>260){
      Ligar();
      Alarm.delay(10000);    
      Desligar();
    } else {
      Serial.print("Umidade OK");
      Serial.println();
    } 
}

  time_t t=now();

  time_t prevDisplay = 0; // when the digital clock was displayed
  


 void setup()
 {
   Serial.begin(9600);
 //  setTime(14,00,0,2,11,14); //set time hora,minutos, segundo, mês, dia, ano
 //  Alarm.alarmRepeat(21,26,30, Irrigar);  // medir e irrigar de dia
 //  Alarm.alarmRepeat(02,00,10, Irrigar);  // medir e irrigar de noite
   pinMode (Rele,OUTPUT);                  // indica o pino do Rele como saída.
 
   while (!Serial) ; // Needed for Leonardo only
   delay(250);
   Serial.println("Buscando Informação de Horário");
   if (Ethernet.begin(mac) == 0) {
    // no point in carrying on, so do nothing forevermore:
       while (1) {
       Serial.println("Sem Conexão");
       delay(10000);
       }
    }
    Serial.print("O IP é ");
    Serial.println(Ethernet.localIP());
    Udp.begin(localPort);
    Serial.println("Esperando sincronização");
    setSyncProvider(getNtpTime);
}

 
 void loop()
 {
//   lcd.begin(16,2);       //declara o tipo de monitor (16 por 2)
//   lcd.setCursor(0,0);    //coloca a posição do primeiro texto
//   lcd.write("Umidade:");
//   lcd.setCursor(12,0);
//   lcd.print(moisture_val);
   Alarm.delay(1000);  // para funcionar código é necessário o Alarm. no código senão não funciona.

//   digitalClockDisplay();
   
   moisture_val=analogRead(moistureSensor);  //read the value 
   Serial.print("Umidade igual a: ");
   Serial.print(moisture_val);
   Serial.println(); 
   
   
   int waterHour = 8;
   int waterMinute = 0;
   int waterSecond = 0;
   bool watered = false;

     if(hour() == waterHour && minute() == waterMinute && second() == waterSecond && !watered)
     {
       Irrigar();
       watered = true;
     } else {
       watered = false;
       Serial.println("Moist OK");
     }
   
//   lcd.begin(16,2);       //declara o tipo de monitor (16 por 2)
//   lcd.setCursor(0,1);    //coloca a posição do primeiro texto na segunda linha
   

   if (timeStatus() != timeNotSet) {
     if (now() != prevDisplay) { //update the display only if time has changed
       prevDisplay = now();
       digitalClockDisplay();  
     }
   }

}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  //Serial.println("Atualizando hora");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      //Serial.println("Recebendo atualizacao");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  //Serial.println("Sem resposta :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
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
