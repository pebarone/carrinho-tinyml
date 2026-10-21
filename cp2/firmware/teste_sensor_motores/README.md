# Sensor e motores pelo monitor serial

Abra `teste_sensor_motores.ino` no Arduino IDE, selecione ESP32 Dev Module
(ESP32 DevKitC V4 / WROOM), a porta da placa e grave. O sketch e independente,
usa apenas Arduino.h e nao precisa de biblioteca TinyML. Abra o Monitor Serial
em **115200 baud**. Pode selecionar Nova linha, Ambos NL e CR ou Sem final de linha.

## Comandos

| Comando | Acao dos dois motores |
| --- | --- |
| W ou w | Frente por 1 segundo e parada automatica |
| S ou s | Tras por 1 segundo e parada automatica |
| X ou x | Para e cancela movimento pendente |

Repetir a mesma direcao renova o prazo de 1 segundo. Trocar o sentido inclui
uma pausa minima de 300 ms com a ponte desabilitada. Um comando desconhecido
tambem para. Reset inicia parado. Os enables usam potencia total, sem PWM.
O atendimento da serial pode aguardar ate 25 ms pela leitura do ECHO.

**Neste sketch S significa RE, nao parada como no teste anterior.**
O sensor apenas imprime informacoes; nao para o motor diante de obstaculos.
Nao ha Wi-Fi, TinyML ou controle de angulo. Um segundo nao garante uma volta.

## Leituras a cada 250 ms

```text
tempo_ms,distancia_cm,echo_us,sensor,motores
250,20.0,1166,OK,PARADO
500,NA,0,SEM_ECHO,PARADO
```

Essas linhas sao exemplos, nao medicoes reais. `OK` indica distancia entre
2 e 400 cm; `SEM_ECHO` indica timeout; `FORA_DA_FAIXA` indica pulso recebido
fora dessa faixa. A coluna echo_us ajuda a diagnosticar a leitura bruta.

## Ligacoes com o PC

Faca a montagem com USB e pilhas desligados. Retire 5V_EN, ENA e ENB da L298N.

- USB alimenta a ESP32; 5V da ESP32 -> VCC do sensor e 5V da logica da ponte.
- Pilhas positivo -> somente 12V/Vs da ponte; negativo -> GND comum.
- Una GND da ESP32, sensor, ponte e pilhas.
- Nao ligue 12V/Vs ao 5V da ESP32/USB para alimentar motores.
- TRIG -> GPIO23.
- Conforme os seis resistores de 330 ohms informados pelo usuario:

```text
ECHO -- 330 -- 330 -- 330 --+-- 330 -- 330 -- 330 -- GND
                          |
                        GPIO22
```

Esse divisor tem 990 ohms de cada lado. Se ECHO atingir 5V, o GPIO recebe
aproximadamente 2,5V; com alimentacao de 3,3V isso fica proximo do limiar
minimo garantido de nivel alto (0,75 x VDD). Falhas de leitura exigem conferir
alimentacao, emendas e sinal; o divisor nao regula a alimentacao do sensor.
Nunca conecte ECHO diretamente ao GPIO.

- ENA -> 25; IN1 -> 26; IN2 -> 27.
- ENB -> 33; IN3 -> 32; IN4 -> 21.
- Motor A -> OUT1/OUT2; motor B -> OUT3/OUT4.

O sentido do motor B (OUT3/OUT4) esta invertido no codigo para compensar a
montagem fisica informada: W usa IN3 LOW / IN4 HIGH; S usa IN3 HIGH / IN4 LOW.
Mantenha os fios desse motor como estao; nao os inverta tambem.

Para verificar apenas o sensor, deixe Vs/12V sem alimentacao e envie X.
Para movimento, suspenda as rodas e use alimentacao dos motores nas pilhas,
mantendo o positivo delas separado do 5V do USB.

## Verificacao pela CLI

Na raiz do repositorio:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build/teste-sensor-motores cp2/firmware/teste_sensor_motores
```

A gravacao manual substitui o firmware anterior da placa; os demais sketches
permanecem preservados no repositorio.

Compilacao conferida em 14/10/2026 com Arduino CLI 1.4.1 e core ESP32 3.3.11:
274.192 bytes de flash e 22.132 bytes de RAM estatica. Este sketch ainda nao
foi gravado nem validado fisicamente; a gravacao foi deixada para o usuario.

## Roteiro de verificacao apos gravar

1. Reinicie: motores parados e cabecalho CSV no monitor serial.
2. Com Vs desligado, mova um alvo diante do sensor: confira distancia e OK.
3. Com sensor sem ECHO, confira NA/SEM_ECHO e continuidade da serial.
4. Com alimentacao correta de Vs e rodas suspensas, envie W e depois S:
   ambos os motores devem inverter, cada comando parando apos 1 segundo.
5. Envie W e imediatamente S: deve haver pausa antes da inversao.
6. Durante movimento ou pausa, envie X: deve parar e cancelar a inversao.
7. Envie um comando desconhecido: deve parar e informar comando invalido.
