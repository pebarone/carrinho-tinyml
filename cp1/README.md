# CP1 — Carrinho-robô

A entrega do CP1 está documentada a partir do [README principal](../README.md),
com o vídeo do carrinho funcionando e a documentação em [`docs/`](../docs/):

| Tema | Documento |
| --- | --- |
| Requisitos, MVP, backlog, planejamento e evolução | [01-requisitos-planejamento.md](../docs/01-requisitos-planejamento.md) |
| Chassi e adaptações | [02-projeto-mecanico.md](../docs/02-projeto-mecanico.md) |
| Hardware, ligações e fotos da montagem | [03-hardware-eletronica.md](../docs/03-hardware-eletronica.md) |
| Software | [04-software.md](../docs/04-software.md) |
| Testes e resultados | [05-testes-resultados.md](../docs/05-testes-resultados.md) |
| Evidências e instruções de uso | [06-evidencias-uso.md](../docs/06-evidencias-uso.md) |

<img src="../media/fotos/carrinho-final-topo.jpg" alt="Carrinho finalizado" width="360">

## Esta pasta: MVP por limiar de distância

O primeiro firmware do carrinho usa regras fixas sobre a leitura do HC-SR04,
sem modelo de IA. Foi a base sobre a qual o firmware final (em
[`cp2/firmware/carrinho/`](../cp2/firmware/carrinho/)) acrescentou TinyML,
partida automática e a manobra de ré e giro.

| Distância medida | Ação | PWM |
| --- | --- | ---: |
| > 40 cm | avança | 160 |
| > 20 cm e ≤ 40 cm | reduz a velocidade | 110 |
| ≤ 20 cm, < 8 cm (emergência) ou leitura inválida | para | 0 |

- Hardware: ESP32 DevKit, HC-SR04 (ECHO com divisor resistivo), L298N com
  jumpers ENA/ENB removidos e dois motores DC TT. Mesmos pinos do firmware
  final (TRIG 23, ECHO 22, ENA 25, IN1 26, IN2 27, ENB 33, IN3 32, IN4 21).
- O carrinho inicia desarmado: envie `A` pela serial para armar e `S` para
  parar e desarmar.
- Saída serial a 115200 baud: `timestamp,distance_cm,valid,action`.

| Arquivo | Conteúdo |
| --- | --- |
| [firmware/carrinho_threshold/carrinho_threshold.ino](firmware/carrinho_threshold/carrinho_threshold.ino) | Leitura do sensor, comandos `A`/`S` e acionamento dos motores |
| [firmware/carrinho_threshold/threshold_policy.h](firmware/carrinho_threshold/threshold_policy.h) | Regra distância → ação |
| [tests/threshold_policy_compile.cpp](tests/threshold_policy_compile.cpp) | Verificações da regra em tempo de compilação |

### Compilar

A partir da raiz do repositório:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 cp1/firmware/carrinho_threshold
```

Os testes da regra rodam junto com os do CP2:
`powershell -ExecutionPolicy Bypass -File cp2/tests/test_policy.ps1`.
