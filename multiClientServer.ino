#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define arraylength 4
const char* Privatessid = "TestSoftAp";
const char* Privatepassword = "htirosh6";
const char* hostName = "TestIgnoreMe";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

int portNum = 80;
// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(portNum);
WiFiClient clients[arraylength];
bool clientFlags[arraylength] = {false, false, false, false};
char Wifissid[33] = "G5";
char WifiPassword[33] = "htirosh7";
bool isNeedToBeWifi = true;

void setup() {
  Serial.begin(9600);


  // prepare GPIO2 to wrire
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);

    WiFi.hostname("Test_Ignore_me");

  // make my own soft access point
  tryConnectWifi();

  if (WiFi.status() != WL_CONNECTED)
  {
    tryConnectWifi();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("\nconnected successfully");
     
    IPAddress myIP = WiFi.softAPIP();

    // Print the IP address
    Serial.print("\n");
    Serial.println(myIP);
    Serial.print("\n");
    Serial.println(WiFi.macAddress());
    Serial.print("\n");

    // Start the server
    Serial.print("\nStarting server ");
    server.begin();
    Serial.print("\nServer started on port: ");
    Serial.print(portNum);
    Serial.print("\n");
  }

  else {
    Serial.println("|seting up softAp|");
    setSoftAp();
  }
}

void loop() {
  // Check if a client has connected
  if (WiFi.status() != WL_CONNECTED && WifiPassword[0] != '\0' && isNeedToBeWifi)
  {
    tryConnectWifi();
    if (WiFi.status() != WL_CONNECTED) {

      WiFi.disconnect();
      setSoftAp();
      isNeedToBeWifi = false;
    }
  }
  
  checkClientConnection();
  checkNewClients();

  if (isThereAnyClient())
  {
    static bool SentLonelyMessage;

    handleClients();
    checkClientConnection();
  }

}


void checkNewClients() {
  WiFiClient client = server.available();
  if (client != 0)
  {
    Serial.print("\nnew client wants to connect\n");
    //check if there is any room for new client in clients array
    int allocAns = AllocateClient(client);
    if (allocAns == (-1))
    { Serial.print("\nToo bad I have no room for you \n");
      client.print("\nToo bad I have no room for you \n");
      client.stop();
    }
    else
    {
      Serial.print("\nclient allocated to room : ");
      Serial.print(allocAns);
    }
  }
}

void handleClients() {

  for (int i = 0; i < arraylength; i++)
  {
    if (clients[i].available())
    {
      String req = clients[i].readString();
      Serial.print("\ngot massage from client num ");
      Serial.print(i);
      Serial.println("\n");
      Serial.print(req);
      if (req.substring(0, 7).equals("trywifi")) //the trywifi command is like this "trywifi,"ssid":"password"
      {
        if (WiFi.status() == WL_CONNECTED)
          WiFi.disconnect();
        int mid = req.indexOf(':');
        (req.substring(8, mid)).toCharArray(Wifissid, 33);
        (req.substring(mid + 1, req.length())).toCharArray(WifiPassword, 33);
        isNeedToBeWifi = true;
        tryConnectWifi();
        if (WiFi.status() != WL_CONNECTED)
          setSoftAp();
      }
      if (req.substring(0, 7).equals("ap_mode"))
      {
        Serial.println("Starting AP by command");
        isNeedToBeWifi = false;
        WiFi.disconnect();
        setSoftAp();
      }
      String answer = handleReq(req);
      clients[i].print(answer);
      Serial.print("\nanswer is : ");
      Serial.print(answer);
    }
  }


}

String handleReq(String r) {
  String ans = "we recieved";
  ans += r;
  return r;
}


bool isThereAnyClient() {
  int ifconnected = false;
  for (int i = 0; i < arraylength; i++)
  {
    if (clientFlags[i] == true)
      ifconnected = true;
  }
  return ifconnected;
}

int  AllocateClient(WiFiClient C) {

  for (int i = 0; i < arraylength; i++)
  {
    if (clientFlags[i] == false)
    { clients[i] = C;
      clientFlags[i] = true;
      return i;
    }
  }
  return -1;
}


//update the trueth table based on real connection check
void checkClientConnection() {
  //Serial.println("Checking connection");
  for (int i = 0; i < arraylength; i++)
  {
    if (clientFlags[i] == true)
      if ((clients[i] != NULL && !clients[i].connected()) || clients[i]==NULL)
      {
       // clients[i]=NULL;
        clientFlags[i] = false;
        String temp = "\nclient ";
        temp += i;
        temp += " has left!";
        Serial.print(temp);
      }
      
    if (clients[i] != NULL && clients[i].connected())
    {
      clientFlags[i] = true;
    }
    //      String temp = "\nclient ";
    //      temp += i;
    //      temp += " has connected!";
    //      Serial.print(temp);
    //    }
  }
  return;
}


//trt to connect to wifi, nothing else
void tryConnectWifi() {
  if (WifiPassword[0] != '\0')
  {
    Serial.print("\nill try and connect to: ");
    Serial.print(Wifissid);
    Serial.print("\npassword ");
    Serial.print(WifiPassword);
    WiFi.softAPdisconnect (true);
    //connect to wifi
    WiFi.begin(Wifissid, WifiPassword);

    int counter = 0;
    while (WiFi.status() != WL_CONNECTED && counter < 20 ) //connect to wifi
    {
      Serial.print(".");
      counter += 1;
      delay(500);
    }
    if (WiFi.status() != WL_CONNECTED)
    { Serial.print("\nconnection to wifi has failed");
      WiFi.disconnect();
    }
    if (WiFi.status() == WL_CONNECTED)
    { Serial.print("\nconnection to wifi is a success");
      Serial.print("\nNew ip address is: ");
      Serial.print(WiFi.localIP());
    }

  }

}


//sets softAP
void setSoftAp() {
  server.stop();
  WiFi.softAPdisconnect (false);
  Serial.print("\nSetting up SoftAp");
  Serial.println(hostName);
  Serial.println(Privatessid);
  Serial.println(Privatepassword);
  WiFi.softAP(Privatessid, Privatepassword);
  WiFi.softAPConfig (local_IP, gateway, subnet);
  Serial.println(WiFi.softAPIP());
  server.begin();
}
