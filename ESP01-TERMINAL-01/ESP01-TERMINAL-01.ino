#include <string.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* _ssid     = "WIFI NAME";
const char* _password = "WIFI PASSWORD";
const char* _privateKey = "64 CHARS IOT PRIVATE KEY";
const String _hostname = "IOT_LUZ_CUARTO";
const String _enabledIps = "192.168.1.2|192.168.1.10|192.168.1.11|192.168.1.12||192.168.1.13";
bool encendido = false;
bool esperar = false;
String _current_token = "";

// response values
const String _apagado = "{\"response\": \"apagado\"}";
const String _encendido = "{\"response\": \"encendido\"}";
const String _ok = "{\"response\": \"ok\"}";
const String _404 = "{\"error\": 404}";
const String _401 = "{\"error\": 401}";
const String _type_json = "application/json";
const String _type_html = "text/html";

ESP8266WebServer server;

void setup() {
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(0, HIGH);
  
  wifiTryConnect();
  
  Serial.begin(9600);  // initialize serial for debugging
  //Serial.println("");
  //Serial.print("IP Address: ");
  //Serial.println(WiFi.localIP());
  
  server.on("/", home);
  server.on("/accion/encender", encender);
  server.on("/accion/apagar", apagar);
  server.on("/accion/estado", estado);
  server.on("/token", token);
  server.begin();
 
  _current_token = generateToken();
  //ESP.wdtEnable(20000);
}

void loop() {
  // si no tenemos cliente chao.
  
/*  if(encendido) {
    digitalWrite(0, LOW);
    digitalWrite(2, LOW);
  } else {
    digitalWrite(0, HIGH);
    digitalWrite(2, HIGH);
  }
  
  if(esperar) {
    esperar = false;
    delay(1000);
  }
  */
  if(wifiTryConnect()) {
    server.handleClient();
  }
  delay(250);
}

bool wifiTryConnect() {

  if(WiFi.status()== WL_CONNECTED) {
    return true;
  }

  WiFi.hostname(_hostname);
  WiFi.begin(_ssid, _password);
  
  int i = 0;
  while(WiFi.status()!=WL_CONNECTED && i < 60)
  {
    delay(500);
    i++;
  }
  return false;
}

bool verifyToken() {
    String name = server.argName(0);
    String value = server.arg(0);
    
    String ip = server.client().remoteIP().toString();
    if(_enabledIps.indexOf(ip) == -1) {
      return false;
    }
    
    if(name.indexOf("token") != -1 && _current_token.indexOf(value) != -1) {
      return true;
    }
    return false;
}

bool verifyKey() {
    String name = server.argName(0) ;
    String value = server.arg(0) ;
    
    // verify ip sender.
    String ip = server.client().remoteIP().toString();
    if(_enabledIps.indexOf(ip) == -1) {
      return false;
    }
    // verify token
    if(name.indexOf("key") != -1 && value.indexOf(_privateKey) != -1) {
      return true;
    }
    return false;
}


void home() {
  server.send(200,_type_json, _ok);
  delay(50);
}

void estado() {
  if(verifyToken()) {
    if(encendido) {
      server.send(200,_type_json, _encendido);
    } else {
      server.send(200,_type_json, _apagado);
    }
  } else {
    server.send(401,_type_json, _401);
  }
  
  delay(50);
}

void encender() {
  if(verifyToken()) {
    encendido = true;
    server.send(200,_type_json, _ok);
    comandoEncender(0x01);
  } else {
    server.send(401,_type_json, _401);
  }
  
  delay(100);
  esperar = true;
}

void apagar() {
  if(verifyToken()) {
    encendido = false;
    server.send(200,_type_json, _ok);
    comandoApagar(0x01);
  } else {
    server.send(401,_type_json, _401);
  }
  
  delay(100);
  esperar = true;
}

void comandoEncender(byte dir) {
  Serial.write(0xA0);
  Serial.write(0x01);
  Serial.write(0x01);
  Serial.write(0xA2);
  delay(50);
  
}

void comandoApagar(byte dir) {
  Serial.write(0xA0);
  Serial.write(0x01);
  Serial.write(0x00);
  Serial.write(0xA1);
  delay(50);
}

void token() {
  if(verifyKey()) {
    _current_token = generateToken();
    String _result = "{\"response\": \"";
    _result += _current_token;
    _result += "\"}";
    server.send(200,_type_json, _result);
  } else {
    server.send(401,_type_json, _401);
  }
  
  delay(50);
}

String generateToken() {
  String _validChars = "abcdefghABCDEFGH1234567890";
  String _result = "";
  for(int i = 0; i<64; i++){
    _result += _validChars[random(25)];
  }

 _current_token = _result;
  return _result;
}



// End Hash Methods.
