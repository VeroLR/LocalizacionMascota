# Sistema de localización de mascotas basado en LoRa

Bienvenido/a a la documentación técnica del **nodo de la mascota**:
- GNSS (TinyGPS++)
- LoRa SX1262 (RadioLib)

## Módulos
- gps_handler — Adquisición de datos GNSS y construcción de payload
- lora_handler — Transmisión LoRa (TX)

> Formato de payload (13 B, little-endian): `[fix:1][hhmmss:4][lat*1e5:4][lon*1e5:4]`.
