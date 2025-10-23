/** @file wifi_manager.h
 * @brief Declaraciones de funciones para la gestión WiFi y almacenamiento de credenciales.
 *
 * Define las funciones del módulo WiFi Manager, encargadas de gestionar el modo STA/AP
 * y la configuración del portal de acceso. Permite leer y guardar credenciales desde
 * LittleFS, iniciar conexiones automáticas o mostrar el portal web de configuración.
 *
 * Incluye variables globales que sincronizan el reinicio diferido del dispositivo tras
 * guardar nuevas credenciales.
 *
 * @author Verónica Lechón Rodríguez 
 * @date 23/07/2025
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>

// Servidor HTTP global (definido en otro módulo)
extern WebServer server;

// Señales de reinicio diferido (gestionadas en el bucle principal)
extern bool pendingReset;
extern unsigned long pendingResetTime;

/**
 * \brief Carga credenciales desde LittleFS.
 * \param ssid (out) SSID leído.
 * \param pwd  (out) Password leído.
 * \return true si existen y el SSID no está vacío.
 */
bool loadWiFiConf(String &ssid, String &pwd);

/**
 * \brief Guarda credenciales en LittleFS.
 * \param ssid SSID a guardar.
 * \param pwd  Password a guardar.
 * \return true si se escribe correctamente.
 * \note Para mayor robustez, la implementación usa escritura simple; opcionalmente
 *       puede cambiarse a escritura atómica (temp + rename).
 */
bool saveWiFiConf(const String &ssid, const String &pwd);

/**
 * \brief Intenta conexión inicial: STA si hay credenciales; si no o falla, sube AP.
 * \param ssid (in/out) SSID usado/resultado.
 * \param pwd  (in/out) Password usado/resultado.
 * \return true si quedó conectado en STA.
 */
bool initWiFiConnection(String &ssid, String &pwd);

/**
 * \brief Intenta conectar al AP indicado en modo estación (bloqueo con timeout).
 * \param ssid SSID objetivo.
 * \param pwd  Password.
 * \return true si WL_CONNECTED antes del timeout.
 */
bool tryConnectWiFi(const String &ssid, const String &pwd);

/**
 * \brief Levanta el punto de acceso para el portal de configuración.
 */
void startWiFiAP();

/**
 * \brief Manejador del POST de formulario (/submit): guarda y redirige.
 * \details En éxito: 303 → /savedcredentials y dispara pendingReset.
 */
void handleFormSubmit();

#endif
