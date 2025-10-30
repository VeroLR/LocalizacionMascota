/** @file lora_handler.h
* @brief Definición de las funciones SX1262 (RadioLib) para recepción de payload GNSS por LoRa.
*
* Este módulo define las siguientes funciones de inicialización del transceptor, la recepción
* continua, la activación del flag ISR de "paquete recibido" y almacena y expone la última
* estampa GNSS válida decodificada desde un payload binario de 13 B.
*
* @author Verónica Lechón Rodríguez
* @date 23/07/2025
*/


#pragma once
#include <RadioLib.h>
#include <Arduino.h>
#include "gps_handler.h"

/**
 * \brief Inicializa el SX1262 con la configuración LoRa indicada.
 * \param freqMHz Frecuencia central en MHz (p.ej., 868.1).
 * \return true si el radio quedó inicializado.
 * \details Fija BW=125 kHz, SF=9, CR=4/7, syncWord=0x12 (privado), power=14 dBm,
 * preámbulo=8 símbolos. Configura los pines del RF switch (RX/TX enable).
 */
bool LORA_begin(float freqMHz);

/**
 * \brief Registra la ISR de recepción y entra en modo RX.
 * \return true en caso de éxito.
 */
bool LORA_startRx();

/**
 * \brief Debe llamarse con frecuencia desde \c loop() para procesar paquetes.
 * \details Si el flag de ISR está activo, lee el paquete, actualiza RSSI/SNR
 * y, si su longitud es 13 B con fix=1, decodifica \c GpsInfo y lo almacena.
 * Rearma la recepción al final.
 */
void LORA_rxTick();

/**
 * \brief Devuelve la última estampa GNSS válida y métricas RF asociadas.
 * \param out Estructura \c GpsInfo de salida (sólo válida si la función devuelve true).
 * \param rssi_dBm (opcional) Puntero para RSSI del último paquete (dBm).
 * \param snr_dB   (opcional) Puntero para SNR del último paquete (dB).
 * \return true si existe una estampa previa válida, false en caso contrario.
 */
bool LORA_lastValidGPS(GpsInfo& out, float* rssi_dBm = nullptr, float* snr_dB = nullptr);
