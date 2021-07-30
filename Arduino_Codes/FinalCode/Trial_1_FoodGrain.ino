#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>;
#include <ThingSpeak.h>;
#define RL 10.0 // the value of RL is 10K
float m = -0.3376; //Slope 
float b = 0.7165; //Y-Intercept 
#define R0 2.85


// replace with your channel’s thingspeak API key,
const char * myWriteAPIKey = "3HKBAA2JV1U0VCUQ";
unsigned long myChannelNumber = 1405250; //Replace it with your channel ID
const char* ssid = "Annuadi_2 Ghz";
const char* password = "@Annuadi2000";
const char* server = "api.thingspeak.com";
const char *host = "maker.ifttt.com";
const char *privateKey = "fUOOLv2y_-OeLl7srsFTHW8cOm4GQUb54ibihByRAnO";
double gas;
#define Gas_Pin A0

#define DHTPIN D4 // CONNECT THE DHT11 SENSOR TO PIN D4 OF THE NODEMCU
DHT dht(DHTPIN, DHT11); //CHANGE DHT11 TO DHT22 IF YOU ARE USING DHT22
WiFiClient client;
void setup() {
Serial.begin(9600);
dht.begin();
ThingSpeak.begin(client);
WiFi.begin(ssid, password);
pinMode(Gas_Pin,INPUT);
Serial.println();
Serial.println();
Serial.print("Connecting to ");
Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED) {
delay(200);
Serial.print(".");
}
Serial.println("");
Serial.println("WiFi connected");
}
int ppm1() {  
  float sensor_volt; //Define variable for sensor voltage 
  float RS_gas; //Define variable for sensor resistance  
  float ratio; //Define variable for ratio
  int sensorValue = analogRead(A0); //Read analog values of sensor  
  sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage 
  RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas
  ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air
  
  float ppm_log = (log10(ratio)-b)/m; //Get ppm value in linear scale according to the the ratio value  
  float ppm = pow(10, ppm_log); //Convert ppm value to log scale 
  Serial.print("PPM = ");
  Serial.println(ppm);
  ThingSpeak.writeField(myChannelNumber, 3,ppm, myWriteAPIKey);
  delay(200);
  return(ppm);  
}


void loop() {
 float t = dht.readTemperature();
 delay(200);
ThingSpeak.writeField(myChannelNumber, 1,t, myWriteAPIKey);
Serial.print("Temp = ");
Serial.println(t);
float h = dht.readHumidity();
delay(2000);
ThingSpeak.writeField(myChannelNumber, 2,h, myWriteAPIKey);
Serial.print("Humidity = ");
Serial.println(h);

gas = ppm1();
delay(2000);

if (h > 45){
  send_event("temp_increase_overlimit");
}


   if (isnan(h) || isnan(t)|| isnan(gas)) {
   Serial.println("Failed to read from DHT sensor!");
    }
}

void send_event(const char *event)
{
  Serial.print("Connecting to ");
  Serial.println(host);  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("Connection failed");
    return;
  }  
  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  while(client.connected())
  {
    if(client.available())
    {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    } else {
      // No data yet, wait a bit
      delay(50);
    };
  } 
  Serial.println();
  Serial.println("closing connection");
  client.stop();
}
