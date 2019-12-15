#include <MegunoLink.h>

#include <WifiEspNow.h>
#include <WifiEspNowBroadcast.h>

#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

XYPlot MyPlotXY;
TimePlot MyPlotTime;
Table MyTable;
// variables
bool sens_active_1, sens_active_2, sens_active_3 ,sens_active_4;
int sens_value_1, sens_value_2 , sens_value_3 , sens_value_4;


double v1 = 0, v2 = 0, v3 = 0, v4 = 0;
double x = 0, y = 0; 
int count1 = 0, count2 = 0, count3 = 0, count4 = 0;

// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  String Prefix = "Slave:";
  String Mac = WiFi.macAddress();
  String SSID = Prefix + Mac;
  String Password = "123456789";
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESPNow/Basic/Slave Example");
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);

  MyPlotTime.SetTitle("Sensors Plot");
  MyPlotTime.SetXLabel("Arduino Timer [S]");
  MyPlotTime.SetYLabel("Values");
  MyPlotTime.SetSeriesProperties("Sensor1", Plot::Magenta, Plot::Solid, 2, Plot::Square);
  MyPlotTime.SetSeriesProperties("Sensor2", Plot::Blue, Plot::Solid, 2, Plot::Square);
  MyPlotTime.SetSeriesProperties("Sensor3", Plot::Red, Plot::Solid, 2, Plot::Square);
  MyPlotTime.SetSeriesProperties("Sensor4", Plot::Black, Plot::Solid, 2, Plot::Square);
    
  MyPlotXY.SetTitle("Localization");
  MyPlotXY.SetXLabel("X [m]");
  MyPlotXY.SetYLabel("Y [m]");
  MyPlotXY.SetSeriesProperties("Localization", Plot::Red, Plot::NoLine, 3, Plot::Square);

}

// callback when data is recv from Master
void CalcCoord() {
  double x1 = v4/(v1+v4)*10; 
  double x2 = v3/(v2+v3)*10; 
  double y1 = v2/(v1+v2)*10; 
  double y2 = v3/(v3+v4)*10;
   Serial.print(x1); Serial.print(" ^ "); Serial.println(y1);
  x = (x1+x2)/2; 
  y = (y1+y2)/2;
}

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  //Serial.print("Last Packet Recv Data: "); Serial.println(*data);
  //Serial.println("");

  if (mac_addr[5] == 0x00) {
    v1 = double(data[0]);
  //  count1++;
  }
  else if (mac_addr[5] == 0xf0) {
    v2 = double(data[0]);
  //  count2++;
  }
  else if (mac_addr[5] == 0x38) {
    v3 = double(data[0]);
  //  count3++;
  }
  else if (mac_addr[5] == 0xf8) {
    v4 = double(data[0]);
   // count4++;
  }
  /*Serial.print(v1); Serial.print("  "); Serial.print(String(v2)); Serial.print("  ");
  Serial.print(v3); Serial.print("  "); Serial.println(v4);
  Serial.print(count1); Serial.print("  "); Serial.print(String(count2)); Serial.print("  ");
  Serial.print(count3); Serial.print("  "); Serial.println(count4);
if ((count1 == 1) && (count2 == 1) && (count3 == 1) && (count4 == 1)) {*/
    if ((v1 > 25) && (v2 > 25) && (v3 > 25) && (v4 > 25)) {
      CalcCoord();
      MyPlotXY.SendData("Localization", x,y);
      MyTable.SendData("X: ", x);
      MyTable.SendData("Y: ", y);
      MyTable.SetDescription("Emergency State", "SEISMIC DETECTED!");
     // Serial.print(x); Serial.print(" "); Serial.println(y);
      delay(10);
    }
    else MyTable.SetDescription("Emergency State", "Normal");

/*============================================================================================*/

    if ( v1 )    MyTable.SetDescription("Sensor State 1", "Noise detected!");
    else MyTable.SetDescription("Sensor State 1", "Idle");

    if ( v2 )   MyTable.SetDescription("Sensor State 2", "Noise detected!");
    else MyTable.SetDescription("Sensor State 2", "Idle");

    if ( v3 )    MyTable.SetDescription("Sensor State 3", "Noise detected!");
    else MyTable.SetDescription("Sensor State 3", "Idle");

    if ( v4 )    MyTable.SetDescription("Sensor State 4", "Noise detected!");
    else MyTable.SetDescription("Sensor State 4", "Idle");

    MyPlotTime.SendData(F("Sensor1"),v1);
    MyPlotTime.SendData(F("Sensor2"),v2);
    MyPlotTime.SendData(F("Sensor3"),v3);
    MyPlotTime.SendData(F("Sensor4"),v4);  

  //}count1 = 0; count2 = 0; count3 = 0; count4 = 0;
  /*
  if (mac_addr[5] == 0x00) {
    v1 = data[0];
    count1++;
    }
  if (count1 == 1) {
    if (v1>25) {
    Serial.println("Координата: "+ String(v1)); 
    }
    count1 = 0;
  }*/
}

void loop() {
  // Chill
}
