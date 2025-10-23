/** @file html_pages.cpp
 * @brief Implementación de la generación de páginas HTML dinámicas del portal web.
 *
 * Este módulo genera y sirve las páginas HTML necesarias para la interfaz de usuario
 * del nodo receptor, incluyendo:
 * - Listado de redes WiFi disponibles.
 * - Confirmación de credenciales almacenadas.
 * - Visualización de coordenadas GNSS recibidas.
 *
 * Las páginas se cargan desde el sistema de ficheros LittleFS y se envían mediante
 * el servidor web integrado (WebServer).
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#include "html_pages.h"
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>


/// @brief Genera la página web de la lista de redes escaneadas
String generateScanNetworksHTML() {
  File file = LittleFS.open("/wifimanager.html", "r");
  if (!file) return "Error cargando HTML";

  String html = file.readString();
  file.close();

  int n = WiFi.scanNetworks();
  String options;

  // Escaneo de redes WiFi disponibles
  if (n == 0) {
    options = "<option disabled>No se encontraron redes</option>";
  } else {
    for (int i = 0; i < n; i++) { // Busca y sitúa el nombre de los ssid encontrados en los tags HTML
      String ssid = WiFi.SSID(i);
      ssid.replace("\"", "&quot;");
      options += "<option value=\"" + ssid + "\">" + ssid + "</option>";
    }
  }

  html.replace("%%OPTIONS%%", options); // Para servirlo adecuadamente en un HTML dinámico
  return html;
}

/// @brief Genera la pantalla de aviso de "Credenciales guardadas"
String generateCredentialsSavedHTML() {
  File file = LittleFS.open("/savedcredentials.html", "r");
  if (!file) return "<p>Error cargando página savedcredentials.html</p>";

  String html = file.readString();
  file.close();
  return html;
}

/// @brief Genera la página que muestra las coordenadas de la mascota
String generateCoordsHTML(){
  File file = LittleFS.open("/coords.html", "r");
  if (!file) return "<p>Error cargando página coords.html</p>";

  String html = file.readString();
  file.close();
  return html;
}
