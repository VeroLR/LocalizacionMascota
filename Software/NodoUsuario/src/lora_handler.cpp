/** @file lora_handler.cpp
* @brief Implementación de las funciones SX1262 (RadioLib) para recepción de payload GNSS por LoRa.
*
* Este módulo implementa las siguientes funciones:
* - Inicializa el transceptor SX1262 (vía RadioLib)
* - Arranca la recepción continua
* - Atiende la ISR de “paquete recibido”
* - Expone la última estampa GNSS válida decodificada desde un payload binario de 13 B.
*
* @author Verónica Lechón Rodríguez
* @date 23/07/2025
*/

#include <RadioLib.h>
#include <string.h>
#include "lora_handler.h"

// --- Pines RP2040 (SPI0 = SPI) ---
#define LORA_SCK        18
#define LORA_MISO       16
#define LORA_MOSI       19
#define LORA_SS         17
#define LORA_RST        22
#define LORA_DIO1       20
#define LORA_BUSY       28
#define LORA_TX_ENABLE  27
#define LORA_RX_ENABLE  26

// Longitud máxima que procesamos (por seguridad)
static const int LORA_MAX_READ = 64;

// Instancia RadioLib (SX1262 sobre SPI0)
// Nota: Module(SS, DIO1, RST, BUSY, spi, spiSettings)
SX1262 radio = new Module(LORA_SS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI, RADIOLIB_DEFAULT_SPI_SETTINGS);

// --------- Estado mínimo del receptor ----------
/** Flag levantado en ISR cuando hay un paquete pendiente. */
static volatile bool s_rxFlag = false;
/** Última estampa GNSS válida decodificada. */
static GpsInfo s_lastGps = {0,0,0,false};
/** Métricas RF del último paquete recibido. */
static float   s_lastRssi = 0.0f;
static float   s_lastSnr  = 0.0f;

// En ESP8266/ESP32 se fija el atributo de ISR; en RP2040 no es necesario.
#if defined(ESP8266) || defined(ESP32)
  ICACHE_RAM_ATTR
#endif
static void onPacketISR() {
  s_rxFlag = true;
}

/**
 * \brief Inicializa el SX1262 con la configuración LoRa indicada.
 */

bool LORA_begin(float freqMHz) {
  // Configura los pines físicos del bus SPI0 (RP2040)
  SPI.setSCK(SCK);
  SPI.setTX(MOSI);
  SPI.setRX(MISO);
  SPI.begin();

  pinMode(LORA_SS, OUTPUT);
  digitalWrite(LORA_SS, HIGH);

  // begin(freq, BW[kHz], SF, CR, syncWord, power[dBm], preamble, tcxo, useRegLDO=false)
  // Ajustes: BW=125 kHz, SF=9, CR=4/7, syncWord=0x12 (privado), 14 dBm, preámbulo 8
  int st = radio.begin(freqMHz, 125.0, 9, 7, 0x12, 14, 8, 0, false);
  if (st != RADIOLIB_ERR_NONE) return false;

  // Control de RF switch (enable RX/TX)
  radio.setRfSwitchPins(LORA_RX_ENABLE, LORA_TX_ENABLE);

  return true;
}

/**
 * \brief Registra la ISR de recepción y entra en modo RX.
 * \return true en caso de éxito.
 */
bool LORA_startRx() {
  radio.setPacketReceivedAction(onPacketISR);
  return radio.startReceive() == RADIOLIB_ERR_NONE;
}

/**
 * \brief Debe llamarse con frecuencia desde loop() para procesar paquetes.
 */
void LORA_rxTick() {
  // Salida rápida si no hay evento de recepción
  if (!s_rxFlag) return;

  // Clear del flag (race mínimo; suficiente para este caso)
  s_rxFlag = false;

  // Determina longitud de paquete
  int16_t len = radio.getPacketLength();
  if (len <= 0) len = LORA_MAX_READ;
  if (len > LORA_MAX_READ) len = LORA_MAX_READ;

  uint8_t buf[LORA_MAX_READ];
  int st = radio.readData(buf, len);

  if (st == RADIOLIB_ERR_NONE) {
    // Métricas del paquete actual
    s_lastRssi = radio.getRSSI();  // dBm
    s_lastSnr  = radio.getSNR();   // dB

    // Filtra sólo el payload GNSS de 13B con fix=1
    if (len == 13 && buf[0] == 1) {
      GpsInfo gi{};
      if (GPS_parsePayload(buf, 13, gi) && gi.valid) {
        s_lastGps = gi;
      }
      // Si falla parse, preserva s_lastGps anterior
    }
    // Otros tamaños/formatos se ignoran sin tocar s_lastGps
  }

  // Rearma la recepción continuamente
  radio.startReceive();
}

/**
 * \brief Devuelve la última estampa GNSS válida y métricas RF asociadas.
 */
bool LORA_lastValidGPS(GpsInfo& out, float* rssi_dBm, float* snr_dB) {
  if (!s_lastGps.valid) return false;
  out = s_lastGps;
  if (rssi_dBm) *rssi_dBm = s_lastRssi;
  if (snr_dB)   *snr_dB   = s_lastSnr;
  return true;
}
