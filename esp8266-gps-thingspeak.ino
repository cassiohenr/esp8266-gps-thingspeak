//DEFINIÇÕES PARA GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(4, 5);      //rx|tx

static void smartdelay(unsigned long ms);
byte month, day, hour, minute, second, hundredths;
int year;
unsigned long age;

//DEFINIÇÕES PARA ESP-01 - HTTPGET
#include <SoftwareSerial.h>
#define DEBUG true

String apiKey = "SUACHAVE";

SoftwareSerial ser(2,3);                                            // RX, TX
int TAG=0;
String DADO;


void setup() 
{  
  ss.begin(9600);
                  
  Serial.begin(9600); 
  ser.begin(9600);
  
  sendData("AT+CWMOD=1\r\n", 2000, DEBUG);                              // DEFINE PARA STA
  delay(1000);
  sendData("AT+RST\r\n", 2000, DEBUG);                                  // rst
  sendData("AT+CWJAP=\"SUAWIFI\",\"SENHA\"\r\n", 2000, DEBUG);          // Conecta a rede wireless
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 1000, DEBUG);                             // Mostra o endereco IP
  sendData("AT+CIFSR\r\n", 1000, DEBUG);                                // Configura para multiplas conexoes
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);     
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);                      // Inicia o web server na porta 80
  
  ser.println("AT+RST");                                                // reset ESP8266
 
}

void loop() 
{
  float flat, flon;

  gps.f_get_position(&flat, &flon, &age);
  Serial.print("Latitude: "); Serial.println(flat,6);
  Serial.print("Longitude: "); Serial.println(flon,6);
  Serial.print("Satélites: "); Serial.println(gps.satellites());
  Serial.print("HDOP: "); Serial.println(gps.hdop()/100);
  Serial.print("Velocidade: "); Serial.println(gps.f_speed_kmph());

  Serial.print("Time: "); 
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  char sz[32];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ", month, day, year, hour, minute, second);
  Serial.println(sz);
    
  Serial.println("-------------------------------");
  
  smartdelay(1000);

  
  esp_8266();
  TAG=gps.satellites();
}


//---------------------------------CONEXAO_TCP_IP+PACOTE_DAD0S--------------
void esp_8266()          
{
 
  //char buf[32];
  Serial.print(TAG);
  Serial.println("-->valor");
  String cmd = "AT+CIPSTART=\"TCP\",\"";             // TCP connection
  cmd += "184.106.153.149";                          // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
   
  if(ser.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
    
  String getStr = "GET /update?api_key=";              //Prepare GET string
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(TAG);
  getStr += "\r\n\r\n";

  
  cmd = "AT+CIPSEND=";                                  // send data length
  cmd += String(getStr.length());
  ser.println(cmd);

  if(ser.find(">"))
  {
    ser.print(getStr);
     Serial.println(getStr);
  }
  else
  {
    ser.println("AT+CIPCLOSE");                                                    //Alert user
    Serial.println("AT+CIPCLOSE");
  }
    
delay(20000);        
 
}
//-------------------------------------------------------------------------------

String sendData(String command, const int timeout, boolean debug)
{
                                                                        // Envio dos comandos AT para o modulo
  String response = "";
  ser.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (ser.available())
    {
      char c = ser.read(); 
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}


static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
