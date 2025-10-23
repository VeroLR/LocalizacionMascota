/** @file main.ino
 * @brief Programa principal del nodo de la mascota (GNSS + LoRa TX + WiFi/LCD).
 *
 * Flujo principal:
 * - Inicializa GNSS, LoRa (TX), WiFi/AP y LCD.
 * - Actualiza continuamente el parser GNSS.
 * - Construye payloads de 13 B (fix, hhmmss, lat*1e5, lon*1e5) y los transmite por LoRa
 *   con temporización periódica (p.ej., cada N segundos).
 * - Muestra estado por LCD y expone portal web para configuración WiFi.
 *
 * @note La temporización por `SS % PERIOD == 0` es válida para PERIOD < 60;
 *       para periodos mayores, usar planificador por `millis()`.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#include <Arduino.h>
#include "gps_handler.h"
#include "lora_handler.h"

static uint8_t payload[13];

// ----------------- Configuración -----------------
static const uint32_t GPS_BAUD = 9600;
/**
 * \brief Segundos entre envíos.
 * \warning Válido con esta lógica sólo para PERIOD < 60
 */
static const uint16_t PERIOD   = 10;   // 10->10 s

// ----------------- Estado -----------------
static uint32_t lastSentHHMMSS = 0;
static bool txInProgress = false;

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println("\n[GPS TEST] Arrancando...");

  bool ok = GPS_begin(GPS_BAUD);
  Serial.println(ok ? "GPS OK" : "GPS FAIL");

  if (!LORA_begin(868.0)) {
    Serial.print("[LoRa] INIT FAIL, code ");
    Serial.println(LORA_lastState());
    // En el prototipo seguimos ejecutando para poder ver los logs de GPS
  } else {
    Serial.println("[LoRa] INIT OK");
  }
}

void loop() {
  // 1) Actualizar GPS siempre (alimentar parser NMEA)
  GPS_update();

  // 2) Cerrar TX previa si terminó (una sola vez por paquete)
  if (txInProgress && LORA_isTxDone()) {
    if (LORA_lastState() == RADIOLIB_ERR_NONE) {
      Serial.println("[LoRa] TX OK");
    } else {
      Serial.print("[LoRa] TX FAIL, code ");
      Serial.println(LORA_lastState());
    }
    LORA_finishTx();          // limpieza explícita
    txInProgress = false;
  }

  // 3) Si hay fix válido (posición + hora)
  if (GPS_hasFix()) {
    GpsInfo info = GPS_getInfo();

    if (info.valid && !txInProgress) {
      // Evita doble envío en el mismo segundo
      bool nuevoSegundo = (info.hhmmss != lastSentHHMMSS);

      // === Temporización basada en SS (segundo dentro del minuto) ===
      // Enviar cuando SS % PERIOD == 0
      // LIMITACIÓN: sólo correcto para PERIOD < 60
      uint8_t ss = info.hhmmss % 100;     // extrae SS de HHMMSS
      if (nuevoSegundo && (ss % PERIOD == 0)) {
        size_t len = GPS_buildBinaryPayload(info, payload, sizeof(payload));
        if (len == 13) {
          // Dump HEX (debug)
          Serial.print("[Payload HEX] ");
          for (size_t i = 0; i < len; i++) {
            if (payload[i] < 16) Serial.print('0');
            Serial.print(payload[i], HEX);
            Serial.print(' ');
          }
          Serial.println();

          // Verificación de simetría encode/decode (debug)
          GpsInfo check;
          if (GPS_parsePayload(payload, len, check)) {
            Serial.print("[Check] hhmmss="); Serial.print(check.hhmmss);
            Serial.print(" lat="); Serial.print(check.lat, 6);
            Serial.print(" lon="); Serial.println(check.lon, 6);
          }

          // Transmitir por LoRa (asíncrono)
          if (LORA_startTx(payload, len)) {
            txInProgress = true;
            lastSentHHMMSS = info.hhmmss;
            Serial.println("[LoRa] TX started");
          } else {
            Serial.print("[LoRa] startTx FAILED, code ");
            Serial.println(LORA_lastState());
          }
        }
      }
    }
  }

  delay(1);
}
