/** @file wifi_manager.cpp
 * @brief Implementación de la gestión de conexión WiFi y configuración de red.
 *
 * Este módulo controla la conexión del nodo a una red WiFi y el almacenamiento
 * de credenciales en memoria persistente (LittleFS). Permite:
 * - Cargar y guardar SSID/contraseña.
 * - Intentar conexión en modo estación (STA).
 * - Activar un punto de acceso (AP) para configuración manual.
 * - Manejar el formulario HTML de envío de credenciales.
 *
 * Integra feedback visual mediante la pantalla LCD y coordinación con el servidor WebServer.
 *
 * @note Compatible con el stack WiFi del núcleo RP2040 de Raspberry Pi Pico W.
 *
 * @author Verónica Lechón Rodríguez 
 * @date 23/07/2025
 */

#include "hardware/structs/resets.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include <LittleFS.h>
#include <WebServer.h>
#include "lcd_utils.h"
#include "html_pages.h"
#include "hardware/watchdog.h"

#define CONFIG_FILE "/wifi.config"
#define AP_SSID     "WiFiConfig"
#define AP_PASS     "12345678"

// =================== Persistencia de credenciales ===================

/**
 * @brief Guarda SSID y password (dos líneas) en CONFIG_FILE.
 * @warning Texto plano: válido para prototipo (documentar limitación en TFG).
 */
bool saveWiFiConf(const String &ssid, const String &pwd) {
  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) return false;
  file.println(ssid);
  file.println(pwd);
  file.close();
  return true;
}

/**
 * @brief Carga SSID y password de CONFIG_FILE.
 * @return true si existe y el SSID no es vacío.
 */
bool loadWiFiConf(String &ssid, String &pwd) {
  if (!LittleFS.exists(CONFIG_FILE)) return false;
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) return false;
  ssid = file.readStringUntil('\n'); ssid.trim();
  pwd  = file.readStringUntil('\n'); pwd.trim();
  file.close();
  if (ssid.length() == 0) return false;
  return true;
}

// =================== Conexión STA / AP ===================

/**
 * @brief Conexión inicial: intenta STA; si falla, levanta AP y guía por LCD al usuario.
 */
bool initWiFiConnection(String &ssid, String &pwd) {
  bool conectado = false;

  if (loadWiFiConf(ssid, pwd)) {
    conectado = tryConnectWiFi(ssid, pwd);
    if (conectado) {
      showLCDMessage("Conectado IP:\n" + WiFi.localIP().toString());
      return true;
    } else {
      showLCDMessage("Fallo de WiFi. Intente de nuevo.");
      delay(3000);
    }
  }

  // Activa modo AP para portal de configuración
  startWiFiAP();

  // Mensaje con SSID/clave del AP
  showLCDMessage("Red: WiFiConfig Clave: 12345678");

  // Espera asociar al menos 1 cliente para mostrar IP del portal
  const unsigned long TIMEOUT_WAIT_ASSOC = 120000UL;
  unsigned long t0 = millis();
  bool associated = false;

  while ((millis() - t0) < TIMEOUT_WAIT_ASSOC) {
    int nStations = WiFi.softAPgetStationNum();
    if (nStations > 0) {
      associated = true;
      break;
    }
    delay(250);
  }

  if (associated) {
    showLCDMessage("Acceda a http://" + WiFi.softAPIP().toString());
    delay(1200);
  } else {
    showLCDMessage("AP listo: WiFiConfig");
    delay(1200);
  }
  return false;
}

/**
 * @brief Intento de conexión STA con timeout.
 */
bool tryConnectWiFi(const String &ssid, const String &pwd) {
  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  delay(150);
  WiFi.mode(WIFI_OFF);
  delay(150);
  WiFi.mode(WIFI_STA);
  delay(150);

  WiFi.begin(ssid.c_str(), pwd.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000UL) {
    showLCDMessage("Conectando...");
    delay(500);
  }

  return (WiFi.status() == WL_CONNECTED);
}

/**
 * @brief Levanta AP con SSID/clave fijos (prototipo).
 */
void startWiFiAP() {
  WiFi.disconnect(true);
  WiFi.softAPdisconnect(true);
  delay(150);
  WiFi.mode(WIFI_OFF);
  delay(150);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  showLCDMessage("Conectese a WiFiConfig");
  delay(3000);
}

// =================== Formulario ===================

/**
 * @brief POST /submit: guarda credenciales, redirige y programa reinicio.
 *
 * Campos esperados: "ssid", "password".
 * Si la operación es exitosa → /savedcredentials.
 */
void handleFormSubmit() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String pwd  = server.arg("password");

    // Validación mínima (evita vacíos/espacios en blanco)
    ssid.trim(); pwd.trim();
    if (ssid.length() == 0) {
      server.send(400, "text/plain", "SSID vacio");
      return;
    }

    if (saveWiFiConf(ssid, pwd)) {
      server.sendHeader("Location", "/savedcredentials");
      server.send(303, "text/plain", "Redirigiendo...");

      // Feedback en LCD y reinicio diferido (gestiona el .ino)
      showLCDMessage("Credenciales OK. Reiniciando...");
      pendingReset = true;
      pendingResetTime = millis();
    } else {
      server.send(500, "text/plain", "Error al guardar configuracion.");
    }
  } else {
    server.send(400, "text/plain", "Datos incompletos");
  }
}