/** @file html_pages.h
 * @brief Declaraciones del módulo de generación de páginas HTML para el portal web.
 *
 * Define las funciones que generan dinámicamente el contenido HTML del servidor embebido.
 * Las plantillas se almacenan en LittleFS y se sirven a través del objeto global WebServer.
 * Incluye la interfaz de configuración WiFi y la visualización de datos de posición.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#ifndef HTML_PAGES_H
#define HTML_PAGES_H

#include <Arduino.h>
#include <WebServer.h>

// Servidor HTTP global (definido en el .ino o en otro módulo)
extern WebServer server;

/**
 * \brief Genera la página de selección de red WiFi.
 * \details Carga /wifimanager.html de LittleFS y sustituye el marcador
 *          %%OPTIONS%% por <option>…</option> según el escaneo WiFi.
 * \return HTML completo o mensaje de error embebido.
 */
String generateScanNetworksHTML();

/**
 * \brief Genera la página de confirmación de credenciales guardadas.
 * \return HTML de /savedcredentials.html o mensaje de error embebido.
 */
String generateCredentialsSavedHTML();

/**
 * \brief Genera la página de visualización de coordenadas de la mascota.
 * \return HTML de /coords.html o mensaje de error embebido.
 */
String generateCoordsHTML();

#endif
