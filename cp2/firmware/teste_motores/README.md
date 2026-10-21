# Teste isolado dos dois motores

Sketch de bancada para ESP32 DevKitC V4 / WROOM e L298N, independente do
sensor e do TinyML. Substitui temporariamente o programa gravado na placa;
o firmware original continua em `../carrinho/`.

Inicia parado. Pela serial a 115200 baud, `A` inicia a repeticao:
frente por 1000 ms, motores desabilitados por 300 ms, tras por 1000 ms,
motores desabilitados por 300 ms. `S` para e desarma a qualquer momento.
Os dois canais recebem habilitacao total. O sentido fisico depende da
polaridade de cada motor. Nao ha encoder: 1 segundo nao garante 360 graus.

## Ligacoes para testar com USB

Monte com USB e pilhas desconectados e suspenda as rodas antes de testar.

- Retire os jumpers ENA, ENB e 5V_EN da L298N.
- ESP32 5V (USB) -> somente 5V da logica da ponte.
- Pilhas positivo -> somente 12V/Vs da ponte; nunca ao 5V da ESP32.
- Negativo das pilhas, GND da ponte e GND da ESP32 unidos.
- ENA -> GPIO25; IN1 -> GPIO26; IN2 -> GPIO27.
- ENB -> GPIO33; IN3 -> GPIO32; IN4 -> GPIO21.
- Motor A -> OUT1/OUT2; motor B -> OUT3/OUT4.

Nao alimente 12V/Vs pelo USB ou pelo pino 5V da ESP32. Sem alimentacao
em Vs, o programa alterna os sinais, mas nao pode girar os motores.
Para esta verificacao, o HC-SR04 nao e utilizado e pode ficar desconectado.

## Compilar e gravar

Na raiz do repositorio, substitua COM4 se a porta mudar:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build/teste-motores cp2/firmware/teste_motores
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM4 --input-dir build/teste-motores cp2/firmware/teste_motores
```

Abra o monitor serial em 115200 baud. Confira a alimentacao separada de Vs
antes de enviar `A`. Envie `S` ao encerrar. Reset ou nova alimentacao
sempre deixam o teste desarmado.
