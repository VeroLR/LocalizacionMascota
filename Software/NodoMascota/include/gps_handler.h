/** @file gps_handler.h
 * @brief Declaraciones del módulo GNSS (estructura GpsInfo y funciones públicas).
 *
 * Define la estructura estándar `GpsInfo` y las funciones para:
 * - Iniciar el receptor GNSS.
 * - Actualizar el parser NMEA.
 * - Comprobar disponibilidad de fix.
 * - Obtener la estampa actual.
 * - Empaquetar y desempaquetar el payload binario (13 B) para LoRa.
 *
 * @note Precisión típica con escala lat/lon·1e5 ≈ 1 m.
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
 * \brief Construye payload binario de 13B (1B=fix, 4B=HHMMSS, 4B=lat*1e5, 4B=lon*1e5).
 * \return Devuelve 13 si OK, 0 si no hay fix o buffer insuficiente.
 */
size_t GPS_buildBinaryPayload(const GpsInfo& info, uint8_t* out, size_t outSize);

/**
 * \brief Decodifica un payload de 13B a GpsInfo.
 * \return  Devuelve true si OK.
 */
bool GPS_parsePayload(const uint8_t* in, size_t len, GpsInfo& out);
