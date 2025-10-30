/** @file lcd_utils.h
 * @brief Declaraciones del módulo de control de pantalla LCD (HD44780 I²C).
 *
 * Contiene las funciones para la inicialización y actualización de la pantalla LCD
 * utilizada para mostrar el estado del nodo receptor, como la dirección IP o mensajes
 * de configuración. Compatible con el controlador HD44780 y la interfaz I²C.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#ifndef LCD_UTILS_H
#define LCD_UTILS_H

#include <Arduino.h>

/**
 * \brief Inicializa la pantalla LCD por el bus I²C.
 *
 * Configura los pines SDA y SCL usados por `Wire1`
 * (GP2 → SDA1, GP3 → SCL1) e inicializa el controlador
 * `hd44780_I2Cexp`. Enciende la retroiluminación.
 *
 * \note Debe llamarse una única vez al arrancar el sistema
 *       (p. ej. en `setup()`).
 */
void confLCD();

/**
 * \brief Muestra un mensaje de texto en la pantalla LCD.
 *
 * Limpia la pantalla, coloca el cursor al inicio y escribe el texto
 * recibido. Si el mensaje supera el ancho de una línea (16 caracteres),
 * continúa en la segunda línea.
 *
 * \param message Cadena a mostrar (se trunca al tamaño del display).
 */
void showLCDMessage(const String &message);

#endif
