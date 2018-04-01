/*
  Web client

 This sketch connects to a website (http://www.google.com)
 using a WiFi shield.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the WiFi.begin() call accordingly.

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the WiFi.begin() call accordingly.

 Circuit:
 * WiFi shield attached

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */


#include <SPI.h>
#include <SD.h>
#include <WiFi101.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
const int chipSelect = 4;
int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "songbird.backyardbrains.com";    // name address for Google (using DNS)
//char server[] = "172.20.10.5";
String server_name(server);
float voltage = 0.00;
String data_name = "voltage = ";
String filename = "BIRD0.WAV";
String post_data = "";
char buffer[20];
char header[44];
int buf_size = 1000;
byte content1[1000];
char c;
//char *content;

int read_count = 0;
File audio_file;
// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  //SPI.setClockDivider(1);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
  Serial.println("tring post data");
  postData();
  Serial.println("stop post data");
}

void postData() {
	// Combine yourdatacolumn header (yourdata=) with the data recorded from your arduino
	// (yourarduinodata) and package them into the String yourdata which is what will be
	// sent in your POST request
	Serial.println("Initializing SD card...");

  // see if the card is present and can be initialized:
	if (!SD.begin(chipSelect)) {
	  Serial.println("Card failed, or not present");
	  // don't do anything more:
	  return;
	}
	Serial.println("card initialized.");

	// Check existance of file
	if(!SD.exists(filename)){
	  Serial.println("No file: " + filename);
	}
	audio_file = SD.open(filename, FILE_READ);
  	//	read_count++;
	//audio_file.read(header, 44);
	//audio_file.seek(44);
  int content_len = audio_file.size()-44;
  char content[content_len];
  //audio_file.read(content1, 44);
	//audio_file.read(content, audio_file.size()-44);	
	//audio_file.close();
//	String temp_content(content1);
//	String temp_header(header);
//	post_data = "{\"header\":\""+temp_header+"\", \"content\":\"" + temp_content+ "\"}";
  //post_data = "{\"header\":\"gps\",\"content\":1351824120}";

	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
//	root["header"] = temp_header;
//	root["content"] = temp_content;
  Serial.println("data to send :");
//	root.printTo(Serial);
	//Serial.println(post_data);
	Serial.println("\nStarting connection to server..."); 	
	// If there's a successful connection, send the HTTP POST request
	if (client.connect(server, 80)) {
		Serial.println("Connecting successfully");
		
    int client_count = 10240;
    //int filesize = 1024*200;
    int filesize = audio_file.size();
    int num_slices = filesize/buf_size;
    //content1[64] = '\0';
    Serial.println(num_slices);
    int slice_left = filesize%buf_size;
    Serial.println(slice_left);
		// EDIT: The POST 'URL' to the location of your insert_mysql.php on your web-host
		client.println("POST /api HTTP/1.1");
		
		// EDIT: 'Host' to match your domain
		client.println("Host: " + server_name);
		client.println("User-Agent: Arduino/1.0");
		client.println("Connection: close");
		//client.println("Content-Type: text/plain");
    	client.println("Content-Type: text/plain");
		client.print("Content-Length: ");
		client.println(filesize);
		client.println();
    	//client.println(post_data);
    	//client.println();
    int count= 0;
		while(num_slices>0){
        Serial.println(num_slices);
        audio_file.read(content1, buf_size);
        client.write(content1,buf_size);
        num_slices--;
		}
    audio_file.read(content1,slice_left);
    client.write(content1,slice_left);
    client.println();
    Serial.println(count);
		audio_file.close();
		//root.printTo(client);
    	//client.println();
    	//root.printTo(Serial);
		//client.println(post_data); 
	} 
	else {
		// If you couldn't make a connection:
		Serial.println("Connection failed");
		Serial.println("Disconnecting.");
		client.stop();
	}

}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
	
	if (Serial.available()){
		//Serial.println("Transfering command...");
		
		//while (Serial.available()){
		//	char c = Serial.read();
		//	ret += c;
		//}
		//Serial.println("receive command: " + ret);
		//if(ret == "post data"){
			postData();
		//}

	}
  // if the server's disconnected, stop the client:
  if(client.available()){
    //String ret = "";
    Serial.println("server response:");
    while(client.available()){
    char c = client.read();
    Serial.print(c);
    }
  }
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}






















