/** @file lcd_utils.cpp
 * @brief Implementación de las funciones de control de la pantalla LCD (HD44780 vía I²C).
 *
 * Este módulo se encarga de:
 * - Inicializar la pantalla LCD conectada al bus I²C.
 * - Mostrar mensajes informativos sobre el estado del sistema, como IP o errores.
 * - Gestionar la presentación en una pantalla de 16x2 caracteres.
 *
 * Utiliza la librería `hd44780_I2Cexp` de Bill Perry y el bus `Wire1` (GP2–GP3)
 * de la Raspberry Pi Pico W.
 *
 * @note Se recomienda mantener la longitud de los mensajes inferior a 32 caracteres
 *       para evitar recortes.
 *
 * @author Verónica Lechón Rodríguez
 * @date 23/07/2025
 */

#include "lcd_utils.h"
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Dirección y dimensiones del LCD
#define I2C_ADDR    0x3F
#define LCD_COLUMNS 16
#define LCD_ROWS    2

// Instancia global del display
static hd44780_I2Cexp lcd;

/**
 * @brief Configuración inicial del bus I²C y del LCD.
 *
 * - Configura los pines GP2 (SDA1) y GP3 (SCL1) para `Wire1`.
 * - Inicializa el objeto `hd44780_I2Cexp`.
 * - Enciende la retroiluminación.
 *
 * @warning Si se usan otros pines o bus I²C (Wire0),
 *          es necesario ajustar la función.
 */
void confLCD() {
  Wire1.setSDA(2);
  Wire1.setSCL(3);
  Wire1.begin();

  // Redirigir la instancia global Wire al bus 1 (para librerías que lo usan internamente)
  extern TwoWire Wire;
  Wire = Wire1;

  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.backlight();
}

/**
 * @brief Muestra un mensaje en la pantalla LCD (máx. 2 líneas).
 *
 * Divide el texto en dos partes: la primera línea (16 caracteres)
 * y, si hay más texto, una segunda línea con el resto.
 *
 * @param message Cadena de texto a mostrar.
 */
void showLCDMessage(const String &message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message.substring(0, LCD_COLUMNS));

  if (message.length() > LCD_COLUMNS) {
    lcd.setCursor(0, 1);
    lcd.print(message.substring(LCD_COLUMNS, LCD_COLUMNS * 2));
  }
}