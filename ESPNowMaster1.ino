#include <WifiEspNow.h>
#include <WifiEspNowBroadcast.h>

#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

//Gpios that we are going to read (digitalRead) and send to the Slaves
//It's important that the Slave source code has this same array
//with the same gpios in the same order
uint8_t gpios[] = {23, 2};

//In the setup function we'll calculate the gpio count and put in this variable,
//so we don't need to change this variable everytime we change
//the gpios array total size, everything will be calculated automatically
//on setup function
int gpioCount;
const int alpha = 0.8;
uint8_t valOld = 0;
//Slaves Mac Addresses that will receive data from the Master
//If you want to send data to all Slaves, use only the broadcast address {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
//If you want to send data to specific Slaves, put their Mac Addresses separeted with comma (use WiFi.macAddress())
//to find out the Mac Address of the ESPs while in STATION MODE)
uint8_t macSlaves[][6] = {
  //To send to specific Slaves
  //{0x24, 0x0A, 0xC4, 0x0E, 0x3F, 0xD1}, {0x24, 0x0A, 0xC4, 0x0E, 0x4E, 0xC3} //Or to send to all Slaves
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //Calculation of gpio array size:
  //sizeof(gpios) returns how many bytes "gpios" array points to.
  //Elements in this array are of type uint8_t.
  //sizeof(uint8_t) return how many bytes uint8_t type has.
  //Therefore if we want to know how many gpios there are,
  //we divide the total byte count of the array by how many bytes
  //each element has.
  gpioCount = sizeof(gpios)/sizeof(uint8_t);

  //Puts ESP in STATION MODE
  WiFi.mode(WIFI_STA);

  //Shows on the Serial Monitor the STATION MODE Mac Address of this ESP
  Serial.print("Mac Address in Station: "); 
  Serial.println(WiFi.macAddress());

  //Calls the function that will initialize the ESP-NOW protocol
  InitESPNow();

  //Calculation of the size of the slaves array:
  //sizeof(macSlaves) returns how many bytes the macSlaves array points to.
  //Each Slave Mac Address is an array with 6 elements.
  //If each element is sizeof(uint8_t) bytes
  //then the total of slaves is the division of the total amount of bytes
  //by how many elements each MAc Address has
  //by how much bytes each element has.
  int slavesCount = sizeof(macSlaves)/6/sizeof(uint8_t);

  //For each Slave
  for(int i=0; i<slavesCount; i++){
    //Criamos uma variável que irá guardar as informações do slave
    esp_now_peer_info_t slave;
    //Informamos o canal
    slave.channel = CHANNEL;
    //0 para não usar criptografia ou 1 para usar
    slave.encrypt = 0;
    //Copia o endereço do array para a estrutura
    memcpy(slave.peer_addr, macSlaves[i], sizeof(macSlaves[i]));
    //Adiciona o slave
    esp_now_add_peer(&slave);
  } 
  esp_now_register_send_cb(OnDataSent);
   
  //Para cada pino que está no array gpios
  for(int i=0; i<gpioCount; i++){
    //Colocamos em modo de leitura
    pinMode(gpios[i], INPUT);
  }
 
  //Chama a função send
  delay(100);
  send();
}
void InitESPNow() {
  //If the initialization was successful
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  //If there was an error
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

//Function that will read the gpios and send
//the read values to the others ESPs
void send(){
  //Array that will store the read values
  uint8_t package = ReadSensorData();
  package = AlphaBeta(package);
  char macStr[18];
  uint8_t macAddr[] = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};
  esp_err_t result = esp_now_send(macAddr, (uint8_t*) &package, sizeof(package));
  Serial.print("Send Status: ");
  //If it was successful
  if (result == ESP_OK) {
    Serial.println("Success");
  }
  //if it failed
  else {
    Serial.println("Error");
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  //Copies the receiver Mac Address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Prints it on Serial Monitor
  Serial.print("Sent to: "); 
  Serial.println(macStr);
  //Prints if it was successful or not
  Serial.print("Status: "); 
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
  //Sends again
  delay(10); 
  send();
}

uint8_t ReadSensorData(){
  uint8_t sensorValueA3 = uint8_t(analogRead(A6));
  return sensorValueA3;
}

uint8_t AlphaBeta(uint8_t value)
{
  value = alpha*value + (1-alpha)*valOld;
  valOld = value;
}
void loop() {
  // put your main code here, to run repeatedly:
  delay(500); 
}
