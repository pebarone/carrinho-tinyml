# CP1 threshold reference firmware

This is a reconstructed reference sketch created in this workspace for comparing a simple distance-threshold controller with the TinyML controller in CP2. It is not claimed as a recovered or historical CP1 firmware artifact.

The sketch uses an HC-SR04 and L298N with two motors: above 40 cm it drives forward, above 20 cm through 40 cm it slows down, and at 20 cm or less it stops. A reading below 8 cm is treated as an immediate emergency stop; an invalid reading also stops the motors. The ESP32 starts disarmed: send `A` to arm and `S` to stop and disarm. Serial output is `timestamp,distance_cm,valid,action` at 115200 baud.

Build for an ESP32 DevKit V1 with:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 cp1/firmware/carrinho_threshold
```
