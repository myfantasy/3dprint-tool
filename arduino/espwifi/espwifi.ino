#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 


#define REQUEST_NEXT_WAIT_MS_REQUEST 2
unsigned long requestTimeoutValue = 1000;
unsigned long apConnectTimeoutValue = 60000;
 
String endl_ok_msg_responce = "ENDLROKS623746dy7382d6td6326--";
String endl_error_msg_responce = "ENDLRErrS623746dy7382d6td6326--";
String endl_msg_request = "ENDLRQ623746dy7382d6td6326--";


String host = "192.168.1.62";
int port = 80;
String path = "/ttt2";
String wifi_name = "DAP-136s0";
String wifi_pwd = "905";
int check_delay = 500;

bool getValue(String str, String key, String& value) {
  if (str.indexOf(key) == 0) {
    value = str.substring(key.length());
    return true;
  }
  return false;
}

void getInfo() {
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);  
  Serial.println(endl_ok_msg_responce);  
}

void setServerInfo(String f,char separator) {
  while (f.length() > 0) {
    int len = f.indexOf(separator);
    if (len < 0) {
      len = f.length();
    }
    String tmp;
    String str = f.substring(0, len);
    f = f.substring(len + 1); // cut string
    getValue(str, "host=", host);
    if (getValue(str, "port=", tmp)) {
      port = tmp.toInt();
    }
    getValue(str, "path=", path);
    getValue(str, "wifi_name=", wifi_name);
    getValue(str, "wifi_pwd=", wifi_pwd);
  }

  Serial.println(endl_ok_msg_responce);
}

void connectToAp() {
  unsigned long end = millis() + apConnectTimeoutValue;
  if (WiFi.status() == WL_CONNECTED) {    
    Serial.println(endl_ok_msg_responce);
    return;
  }

  WiFi.begin(wifi_name, wifi_pwd);
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    if (millis() >= end) {
      Serial.println("connect timeout");
      Serial.println(apConnectTimeoutValue);  
      Serial.println(endl_error_msg_responce);
      return;
    }
  }
  Serial.println(WiFi.localIP());  
  Serial.println(endl_ok_msg_responce);
}

void doPostRequest(String post_data) {
  WiFiClient client;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(-200); //Print HTTP return code -200
    Serial.println("NOT Connected, WIFI");
    Serial.println(endl_error_msg_responce);
  }

  if (!client.connect(host, port)) {
    Serial.println(-100); //Print HTTP return code -100
    Serial.println("NOT Connected, host");
    Serial.println(endl_error_msg_responce);
    return;
  }
 
  HTTPClient http; 
  http.setTimeout(5000); // 5 sec
  http.begin(client, host, port, path, false);
  int code = http.POST(post_data);
  if (code <100 || code >=400) {
    Serial.println(code); //Print HTTP return code 
    String payload = http.getString(); 
    Serial.println(payload); //Print request response payload 
    Serial.println(endl_error_msg_responce);
    return;
  } 
  String payload = http.getString(); 
  Serial.println(payload); //Print request response payload 
  http.end();
  
  
  Serial.println(endl_ok_msg_responce);
}

void processRequest(String f) {
  String str;
  String last;
  char separator;

  if (f.length() > 0 && f.substring(f.length()-1) == "\n") {
    f = f.substring(0, f.length()-1);
  }

  int len = f.indexOf('\n');
  if (len>0) {
    str = f.substring(0, len);
    last = f.substring(len + 1);
    separator = '\n';
  } else {
    len = f.indexOf(';');
    if (len>0) {
      str = f.substring(0, len);
      last = f.substring(len + 1);
      separator = ';';
    } else {
      str = f;
      last = "";
      separator = ';'; // ???
    }
  }

  len = last.indexOf(endl_msg_request);
  if (len == 0) {
    last = "";
  } else {
    if (len > 0) {
      last = last.substring(0, len-1); // (exclude separator and endl)
    }
  }

  if (str == "info") {
    getInfo();
    return;
  }
  if (str == "set settings") {
    setServerInfo(last, separator);
    return;
  }
  if (str == "ap connect") {
    connectToAp();
    return;
  }
  if (str == "post") {
    doPostRequest(last);
    return;
  }

  Serial.println("command not exists");
  Serial.println(endl_error_msg_responce);
}

String waitRequest(String stopWord1, String stopWord2, unsigned long addTimeout) { // добавить стоп контент
  unsigned long end = millis() + requestTimeoutValue + addTimeout;
  
  String res = "";
  bool msg_exists = Serial.available();

  while (millis() < end) {
    if (Serial.available()) {
      if (!msg_exists) {
        end = millis() + requestTimeoutValue + addTimeout;
        msg_exists = true;
      }
      res = res + Serial.readString();
      if (stopWord1 !="" && res.indexOf(stopWord1) >= 0) {
        break;
      }
      if (stopWord2 !="" && res.indexOf(stopWord2) >= 0) {
        break;
      }      
    } else {
      delay(REQUEST_NEXT_WAIT_MS_REQUEST);
    }
  }
  return res;
}




void setup() {
  Serial.begin(115200);
  /*
  Serial.println();
 
  WiFi.begin("DAP-1360", "90937125");
 
  Serial.print("Connecting");
  WiFi.mode(WIFI_STA);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
 
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  */
}




void loop() {
  String rq = waitRequest(endl_msg_request, "", 0);
  if (rq.length() == 0) {
    return; // exit
  }

  processRequest(rq);  
}
 
void loop2() {
  delay(2500);
  Serial.println(WiFi.status());
  WiFiClient client;
 
  if (client.connect(host, port))
  {
    Serial.println("Connected, host");
    // we are connected to the host!
  }
  else
  {
    Serial.println("NOT Connected, host");
    // connection failure
  }
 
  HTTPClient http; 
  http.setTimeout(5000);
  http.begin(client, host, port, path, false);
  int code = http.GET();
  Serial.println(code); //Print HTTP return code 
  String payload = http.getString(); 
  Serial.println(payload); //Print request response payload 
  http.end(); //Close connection Serial.println(); 
  Serial.println("closing connection"); 
}
