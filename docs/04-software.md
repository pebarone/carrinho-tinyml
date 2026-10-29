# Software

## Onde está o código

| Programa | Pasta | Uso |
| --- | --- | --- |
| **Firmware final** (TinyML + navegação autônoma) | [cp2/firmware/carrinho/](../cp2/firmware/carrinho/) | Gravado no carrinho |
| MVP por limiar (CP1) | [cp1/firmware/carrinho_threshold/](../cp1/firmware/carrinho_threshold/) | Referência de comparação |
| Teste isolado dos motores | [cp2/firmware/teste_motores/](../cp2/firmware/teste_motores/) | Bancada |
| Teste de sensor + motores pela serial | [cp2/firmware/teste_sensor_motores/](../cp2/firmware/teste_sensor_motores/) | Bancada |
| Ferramentas de coleta, build e simulação | [cp2/tools/](../cp2/tools/) | PC |
| Testes automatizados | [cp2/tests/](../cp2/tests/), [cp1/tests/](../cp1/tests/) | PC |

### Módulos do firmware final

| Arquivo | Responsabilidade |
| --- | --- |
| [carrinho.ino](../cp2/firmware/carrinho/carrinho.ino) | Laço principal: amostragem a 10 Hz, janela de 10 leituras, comandos `S`/`A`, partida automática |
| [config.h](../cp2/firmware/carrinho/config.h) | Pinos, distâncias-limite, PWMs e confiança mínima |
| [ultrasonic.h](../cp2/firmware/carrinho/ultrasonic.h) | Leitura do HC-SR04 (pulso TRIG, tempo de ECHO → cm, validação 2–400 cm) |
| [inference.h](../cp2/firmware/carrinho/inference.h) | Chamada `run_classifier` da biblioteca Edge Impulse |
| [policy.h](../cp2/firmware/carrinho/policy.h) | Classe + confiança → ação (avança, lento, parar) |
| [navigation.h](../cp2/firmware/carrinho/navigation.h) | Limites por distância, PWM proporcional e manobra parada → ré → giro |
| [motors.h](../cp2/firmware/carrinho/motors.h) | Sinais IN1–IN4 e PWM ENA/ENB da L298N |
| [network_status.h](../cp2/firmware/carrinho/network_status.h) | Wi-Fi e endpoint HTTP `/state` em tarefa FreeRTOS própria |

## Como o programa funciona

```text
HC-SR04 ─(10 Hz)→ janela de 10 distâncias ─→ modelo TinyML ─→ classe + confiança
                         │                                          │
                         └──── distância atual ──→ navigation.h ←───┘
                                                       │
                                               motors.h → L298N → motores
```

1. **Partida:** ao ligar, espera 5 s e arma sozinho (não precisa de PC).
2. **Sensor:** a cada 100 ms mede a distância. Leitura inválida (sem eco)
   para os motores imediatamente.
3. **Inferência:** a cada 10 leituras válidas (~1 s), o modelo classifica
   `livre`, `aproximando` ou `bloqueado`.
4. **Decisão:**

   | Situação | Ação | PWM |
   | --- | --- | ---: |
   | > 40 cm, `livre`, confiança ≥ 0,80 | avança | 255 |
   | > 40 cm, `aproximando`, confiança ≥ 0,80 | avança mais devagar | 220 |
   | > 40 cm, `bloqueado`, confiança ≥ 0,80 | para | 0 |
   | > 40 cm, confiança < 0,80 | avanço cauteloso | 180 |
   | 20–40 cm | reduz proporcionalmente | 220 → 180 |
   | ≤ 20 cm (qualquer classe) | para 150 ms → ré 300 ms → pausa → **giro à direita** 450 ms → pausa | — |

5. Depois da manobra, exige nova janela antes de avançar; se o obstáculo
   continua perto, repete a manobra.

### Controle dos motores

[motors.h](../cp2/firmware/carrinho/motors.h) traduz cada ação em sinais
para a L298N:

| Ação | Motor A (IN1/IN2) | Motor B (IN3/IN4)* | PWM |
| --- | --- | --- | --- |
| Frente | HIGH / LOW | LOW / HIGH | ENA = ENB = duty |
| Ré | LOW / HIGH | HIGH / LOW | ENA = ENB = duty |
| Giro à direita | HIGH / LOW (frente) | HIGH / LOW (ré) | ENA = ENB = duty |
| Parado | LOW / LOW | LOW / LOW | 0 |

\* Sentido do motor B invertido no código para compensar a montagem.

O PWM usa `ledcAttach` (1 kHz, 8 bits). Antes de trocar o sentido, os
enables vão a 0 e a manobra tem pausas, evitando inverter a ponte com os
motores ainda girando.

### Comunicação sem fio

Com `cp2/firmware/carrinho/wifi_config.h` preenchido (copiado de
[wifi_config.example.h](../cp2/firmware/carrinho/wifi_config.example.h),
ignorado pelo Git), a ESP32 conecta-se à rede em segundo plano e responde
em `http://<ip-do-carrinho>/state`:

```json
{"timestamp_ms":12345,"distance_cm":63.210,"sensor_valid":true,"armed":true,
 "class":"livre","confidence":0.92969,"action":2}
```

A rede roda em uma tarefa FreeRTOS no núcleo 0; clientes lentos não atrasam
sensor nem motores. Sem Wi-Fi configurado, o carrinho funciona normalmente.
Pela serial USB, `S` para e desarma, `A` rearma.

### Uso do sensor

- **Segurança direta:** a distância de cada leitura limita a velocidade e
  dispara a manobra de desvio a 20 cm — independe do modelo.
- **Contexto para o modelo:** a sequência de 10 distâncias permite distinguir
  obstáculo parado longe, obstáculo se aproximando e bloqueio.
- O modelo foi treinado com dados reais deste mesmo sensor
  ([projeto Edge Impulse público 1140184](https://studio.edgeimpulse.com/public/1140184/live);
  detalhes em [cp2/docs/entrega.md](../cp2/docs/entrega.md)).

## Compilar e gravar

Requisitos: Arduino CLI (ou Arduino IDE) com core ESP32. Na raiz do repositório:

```powershell
powershell -ExecutionPolicy Bypass -File cp2/tools/build.ps1 -Mode inference -Model real
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM4 --input-dir build/tinyml-real cp2/firmware/carrinho
```

No Arduino IDE: instale o ZIP
[edge-impulse-arduino-real-v1-float32.zip](../cp2/model/edge-impulse-arduino-real-v1-float32.zip)
(Sketch → Incluir Biblioteca → Adicionar .ZIP), abra
`cp2/firmware/carrinho/carrinho.ino`, selecione **ESP32 Dev Module** e grave.

MVP do CP1: `arduino-cli compile --fqbn esp32:esp32:esp32 cp1/firmware/carrinho_threshold`.
