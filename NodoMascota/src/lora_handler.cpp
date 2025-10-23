/** @file lora_handler.cpp
 * @brief Implementación de las funcionalidades de SX1262 (RadioLib) para transmisión LoRa.
 *
 * Este módulo:
 * - Inicializa el transceptor SX1262 con BW=125 kHz, SF=9, CR=4/7, Ptx=14 dBm.
 * - Gestiona el RF switch (RX/TX enable) y el bus SPI del RP2040.
 * - Lanza transmisiones asíncronas (startTransmit) y atiende la ISR de fin de TX.
 * - Expone utilidades para conocer el estado final y finalizar TX explícitamente.
 *
 * @note El sync word usado es 0x12 (privado). La salida se ajusta a la banda EU 868 MHz.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#include "lora_handler.h"
#include <RadioLib.h>

//-------------- Configuración pines SX1262 ------------------
// Raspberry Pi Pico (SPI0 por defecto): NSS=17, DIO1=20, RST=22, BUSY=28
#define LORA_NSS  17
#define LORA_DIO1 20
#define LORA_RST  22
#define LORA_BUSY 28
#define LORA_RXEN 26
#define LORA_TXEN 27

//--------------- Instancias RadioLib ----------------------
// SX1262 sobre SPI0 con SPI settings por defecto del core.
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_RST, LORA_BUSY, SPI, RADIOLIB_DEFAULT_SPI_SETTINGS);

//--------------- Variables de estado -----------------------
/** Flag levantado por la ISR de fin de paquete TX. */
static volatile bool transmittedFlag = false;
/** Último estado devuelto por RadioLib. */
static int transmissionState = RADIOLIB_ERR_NONE;

//----------------- ISR fin de paquete ----------------------
/**
 * \brief Callback de RadioLib cuando termina la transmisión.
 */
static void onPacketSentISR() {
  transmittedFlag = true;
}

/**
 * \brief Inicializa bus SPI0, pines de la radio y configuración LoRa.
 *
 * Parámetros usados en radio.begin():
 *  - BW = 125.0 kHz
 *  - SF = 9
 *  - CR = 7  (equivale a 4/7)
 *  - syncWord = 0x12 (privado)
 *  - power = 14 dBm
 *  - preambleLen = 8
 *  - tcxoVoltage = 0 (sin TCXO)
 *  - useRegulatorLDO = false
 */
bool LORA_begin(float freqMHz) {
  // SPI0 (SCK=GP18, MOSI=GP19, MISO=GP16) desde el core
  SPI.setSCK(SCK);
  SPI.setTX(MOSI);
  SPI.setRX(MISO);
  SPI.begin();

  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);

  int state = radio.begin(freqMHz, 125.0, 9, 7, 0x12, 14, 8, 0, false);
  if (state != RADIOLIB_ERR_NONE) {
    transmissionState = state;
    return false;
  }

  // Control de RF switch discreto (enable RX/TX)
  radio.setRfSwitchPins(LORA_RXEN, LORA_TXEN);

  // Registrar ISR de fin de TX
  radio.setPacketSentAction(onPacketSentISR);

  transmittedFlag = false;
  transmissionState = RADIOLIB_ERR_NONE;
  return true;
}

/**
 * \brief Lanza transmisión asíncrona (non-blocking).
 * \details Valida puntero/longitud y delega en \c radio.startTransmit().
 *          El resultado final se detecta por ISR (transmittedFlag).
 */
bool LORA_startTx(const uint8_t* payload, size_t len) {
  if (!payload || len == 0 || len > 256) {
    transmissionState = RADIOLIB_ERR_INVALID_PAYLOAD;
    return false;
  }
  transmittedFlag = false;
  transmissionState = radio.startTransmit(payload, len);
  return (transmissionState == RADIOLIB_ERR_NONE);
}

/**
 * \brief Indica si la ISR de fin de transmisión ya se disparó.
 */
bool LORA_isTxDone() {
  return transmittedFlag;
}

/**
 * \brief Último estado devuelto por RadioLib (\c RADIOLIB_ERR_xxx).
 */
int LORA_lastState() {
  return transmissionState;
}

/**
 * \brief Finaliza transmisión (bloqueante corta) y marca TX hecho.
 * \note Útil para secuencias que requieran asegurar fin antes de cambiar modo.
 */
void LORA_finishTx() {
  radio.finishTransmit();
  transmittedFlag = true;
}
