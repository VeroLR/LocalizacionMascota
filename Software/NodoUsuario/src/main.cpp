/** @file main.ino
 * @brief Programa principal del nodo de usuario (receptor LoRa).
 *
 * Implementa el flujo principal del nodo receptor:
 * - Inicializa los periféricos: LCD, WiFi, servidor web y módulo LoRa.
 * - Recibe los payloads GNSS del nodo mascota vía LoRa.
 * - Decodifica las coordenadas y las muestra en la interfaz web.
 * - Gestiona la conectividad WiFi y el portal de configuración.
 *
 * Este firmware actúa como interfaz de usuario, mostrando la ubicación
 * recibida y ofreciendo opciones de configuración a través del navegador.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */


#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "hardware/watchdog.h"

#include "wifi_manager.h"
#include "lcd_utils.h"
#include "html_pages.h"
#include "lora_handler.h"
#include "gps_handler.h"

#define CONFIG_FILE "/wifi.config"

WebServer server(80);
bool pendingReset = false;
unsigned long pendingResetTime = 0;
static const float FREQ_LORA = 868.0;

void setup() {

  Serial.begin(115200);
  Serial.println("Iniciando...");

  if (!LittleFS.begin()) {
    showLCDMessage("Error al montar FS");
    return;
  }
  
  // ------------ Pantalla LCD -------------------------
  pinMode(21, OUTPUT);
  digitalWrite(21, LOW);
  delay(10);               // un breve retardo
  digitalWrite(21, HIGH);  // habilita

  confLCD();
  showLCDMessage("Cargando WiFi...");

  String ssid, pwd;
  bool conectado;

// ------------- CONEXIÓN WIFI --------------

// Si hay configuración guardada, intenta conectar
// Si falla o no hay, lanza modo AP para configuración
 conectado = initWiFiConnection(ssid, pwd);
 
/*  if (loadWiFiConf(ssid, pwd)) {
    conectado = tryConnectWiFi(ssid, pwd);
    if (conectado) {
      showLCDMessage("Conectado IP:\n" + WiFi.localIP().toString());
    } else {
      showLCDMessage("Fallo de WiFi.");
      delay(3000);
    }
  }

  if (!conectado) {
    startWiFiAP();
    delay(2000);
    showLCDMessage("Acceda a http://" + WiFi.softAPIP().toString());
  }*/

  // --------------------- LoRa ------------------------
  LORA_begin(868.0);
  LORA_startRx();

  // ------------- CARGA DE PÁGINAS WEB --------------

  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/wifimanager", HTTP_GET, []() {
    String html = generateScanNetworksHTML();
    server.send(200, "text/html", html);
  });
  
  server.on("/savedcredentials", HTTP_GET, []() {
    String html = generateCredentialsSavedHTML();
    server.send(200, "text/html", html);
  });

  server.on("/coords", HTTP_GET, []() {
    String html = generateCoordsHTML();
    server.send(200, "text/html", html);
  });

  server.on("/style.css", HTTP_GET, []() {
    File file = LittleFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/coords.txt", HTTP_GET, []() {
  GpsInfo gi; float rssi=0, snr=0;
  if (LORA_lastValidGPS(gi, &rssi, &snr) && gi.valid) {
    String qs = "lat=" + String(gi.lat, 6) + "&lon=" + String(gi.lon, 6) + "&z=18";
    server.send(200, "text/plain", qs);
  } else {
    server.send(200, "text/plain", "lat=40.4168&lon=-3.7038&z=15");
  }
});


  // Gestiona el POST tras realizar el submit en el formulario
  server.on("/submit", HTTP_POST, handleFormSubmit);
  server.begin();

}

void loop() {
  server.handleClient(); // Maneja las peticiones de los clientes

  LORA_rxTick();

  static uint32_t lastPrint = 0;
  GpsInfo gi; float rssi, snr;
  if (LORA_lastValidGPS(gi, &rssi, &snr) && gi.hhmmss != lastPrint) {
    lastPrint = gi.hhmmss;
    Serial.print("[RX] hhmmss="); Serial.print(gi.hhmmss);
    Serial.print(" lat="); Serial.print(gi.lat, 6);
    Serial.print(" lon="); Serial.print(gi.lon, 6);
    Serial.print(" RSSI="); Serial.print(rssi);
    Serial.print("dBm SNR="); Serial.print(snr);
    Serial.println("dB");
  }
  
  if (pendingReset && millis() - pendingResetTime > 5000) {  
  watchdog_reboot(0, 0, 0);
  }
}
