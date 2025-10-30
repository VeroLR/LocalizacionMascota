/** @file gps_handler.cpp
 * @brief Implementación del periférico GNSS (TinyGPS++) para el nodo de la mascota.
 *
 * Funcionalidad principal:
 * - Inicializa el enlace serie con el receptor GNSS (NMEA).
 * - Alimenta el parser TinyGPS++ con las tramas entrantes.
 * - Expone la última estampa válida (lat, lon, hhmmss, valid).
 * - Serializa/parsea el payload binario compacto (13 B) usado en LoRa.
 *
 * @note La hora reportada es UTC. El formato binario es little-endian.
 *
 * @author Verónica
 * @date 23/07/2025
 */

#include "gps_handler.h"
#include <SoftwareSerial.h>
#include <math.h>
#include <string.h>

// ----------------- Configuración pines -----------------
static const uint8_t GPS_RX_PIN = 5;   // Pico RX <- TX del GPS
static const uint8_t GPS_TX_PIN = 4;   // Pico TX -> RX del GPS 

// ----------------- Estado interno -----------------------
static TinyGPSPlus gps;
static SoftwareSerial gpsSerial(GPS_RX_PIN, GPS_TX_PIN);  

/**
 * \brief Inicializa SoftwareSerial hacia el receptor GNSS.
 */
bool GPS_begin(uint32_t baud) {
  gpsSerial.begin(baud);
  return true;
}

/**
 * \brief Lee los mensajes recibidos y los pasa al parser NMEA.
 */
void GPS_update() {
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }
}

/**
 * \brief Comprobación mínima de validez (posición + hora).
 */
bool GPS_hasFix() {
  return gps.location.isValid() && gps.time.isValid();
}

/**
 * \brief Extrae el mensaje actual; si los datos no son válidos, devuelve valid=false.
 */
GpsInfo GPS_getInfo() {
  GpsInfo info {0,0,0,false};
  if (gps.location.isValid() && gps.time.isValid()) {
    info.lat = gps.location.lat();
    info.lon = gps.location.lng();
    info.hhmmss = (uint32_t)(gps.time.hour()*10000UL +
                             gps.time.minute()*100UL +
                             gps.time.second());
    info.valid = true;
  }
  return info;
}

/**
 * \brief Decodifica 13 B a GpsInfo; exige fix_flag==1.
 */
bool GPS_parsePayload(const uint8_t* in, size_t len, GpsInfo& out) {
  if (!in || len != 13) return false;
  if (in[0] != 1) return false;  // si quieres aceptar “no fix”, elimina esta línea

  uint32_t hhmmss = 0;
  int32_t latFixed = 0, lonFixed = 0;

  memcpy(&hhmmss,   &in[1], 4);
  memcpy(&latFixed, &in[5], 4);
  memcpy(&lonFixed, &in[9], 4);

  out.hhmmss = hhmmss;
  out.lat    = ((double)latFixed) / 100000.0;
  out.lon    = ((double)lonFixed) / 100000.0;
  out.valid  = true;
  return true;
}
