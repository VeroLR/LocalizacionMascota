/** @file gps_handler.h
 * @brief Declaraciones del módulo GPS (TinyGPS++) para lectura y codificación de coordenadas.
 *
 * Define la estructura estandarizada de datos GNSS (`GpsInfo`) y las funciones
 * de inicialización, actualización y empaquetado de información geográfica.
 * Este módulo se utiliza tanto en el nodo transmisor como en el receptor para
 * garantizar compatibilidad en el formato del payload LoRa.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */


#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>

/**
 * \brief Estructura estandarizada para intercambiar datos GNSS.
 *
 * Campos mínimos para el caso de uso.
 *  - lat, lon en grados decimales (WGS84).
 *  - hhmmss UTC (6 dígitos empaquetados en uint32_t).
 *  - valid indica que lat/lon y hora son válidos y recientes.
 */
struct GpsInfo {
  double   lat;      ///< Latitud (grados decimales, WGS84).
  double   lon;      ///< Longitud (grados decimales, WGS84).
  uint32_t hhmmss;   ///< Hora UTC en formato HHMMSS (p.ej., 211507 = 21:15:07).
  bool     valid;    ///< true si posición y hora son válidas (ver GPS_hasFix()).
};

/**
 * \brief Inicializa el enlace serie con el receptor GNSS.
 * \param baud Baudrate del puerto NMEA (típico: 9600 o 38400).
 * \return true si el puerto queda inicializado.
 * \note Debe llamarse una vez en setup().
 */
bool GPS_begin(uint32_t baud);

/**
 * \brief Alimenta el parser NMEA con los bytes disponibles.
 * \details Debe llamarse de forma frecuente (loop()). Lee del SoftwareSerial
 * y pasa cada byte a TinyGPS++ (gps.encode()) hasta vaciar el buffer.
 */
void GPS_update();

/**
 * \brief Indica si hay un fix válido y hora válida.
 * \details Comprobación mínima: location.isValid() y time.isValid().
 * \return true si hay datos de posición y hora válidos (no necesariamente “fresh”).
 * \warning Si necesitas “datos recientes”, añade umbral de edad con TinyGPS++ (age()).
 */
bool GPS_hasFix();

/**
 * \brief Devuelve la estampa GNSS actual.
 * \return Estructura GpsInfo con lat, lon, hhmmss y valid. Si no hay datos válidos, valid=false.
 */
GpsInfo GPS_getInfo();

/**
 * \brief Decodifica un payload de 13 bytes en GpsInfo.
 * \param in  Puntero al payload.
 * \param len Longitud del payload (debe ser 13).
 * \param out Estructura de salida.
 * \return true si se decodifica correctamente y el fix_flag==1.
 * \note Si quieres aceptar “no fix”, relaja la comprobación del primer byte.
 */
bool GPS_parsePayload(const uint8_t* in, size_t len, GpsInfo& out);
