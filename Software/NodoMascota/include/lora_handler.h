/** @file lora_handler.h
 * @brief Definición de las funciones de transmisión LoRa (SX1262/RadioLib) para el nodo de la mascota.
 *
 * Proporciona:
 * - `LORA_begin(freqMHz)`: inicialización del módem y parámetros físicos.
 * - `LORA_startTx(buf,len)`: inicio de transmisión asíncrona.
 * - `LORA_isTxDone()/LORA_lastState()`: consulta del estado de TX.
 * - `LORA_finishTx()`: cierre explícito de la transmisión.
 *
 * @warning Comprobar límites de duty-cycle según ETSI EN 300 220 (EU 868 MHz).
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#pragma once
#include <Arduino.h>
#include <RadioLib.h>

/**
 * \brief Inicializa el SX1262 con parámetros LoRa por defecto.
 * \param freqMHz Frecuencia central (MHz), p. ej. 868.1.
 * \return true si la radio quedó inicializada.
 */
bool LORA_begin(float freqMHz);

/**
 * \brief Inicia una transmisión asíncrona.
 * \param data Puntero a buffer a transmitir.
 * \param len  Longitud en bytes (1..256).
 * \return true si la orden de transmisión fue aceptada por la radio.
 * \note El resultado final puede consultarse con \c LORA_isTxDone() y \c LORA_lastState().
 */
bool LORA_startTx(const uint8_t* data, size_t len);

/**
 * \brief Indica si se ha completado la transmisión (ISR de fin de paquete).
 */
bool LORA_isTxDone();

/**
 * \brief Devuelve el último código de estado devuelto por RadioLib.
 * \return \c RADIOLIB_ERR_xxx
 */
int  LORA_lastState();

/**
 * \brief Finaliza la transmisión de forma explícita (llama a \c radio.finishTransmit()).
 * \details Útil como secuencia de limpieza si abortas o cambias de modo.
 */
void LORA_finishTx();
