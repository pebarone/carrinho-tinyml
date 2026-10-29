# Testes, problemas e resultados

## Testes realizados

| Data | Teste | Resultado | Evidência |
| --- | --- | --- | --- |
| 13/10 | Simulação Wokwi: coleta a 100 cm, `livre`, `bloqueado`, aproximação, emergência a 5 cm, desarme | Leituras ~100,8 cm a 10 Hz; ações corretas em todos os cenários | [cp2/evidence/wokwi/](../cp2/evidence/wokwi/) · [resumo](../cp2/evidence/README.md) |
| 14/10 | Primeira gravação na ESP32 física | Gravação íntegra, firmware inicia e responde a `S` | [physical-setup-v1](../cp2/evidence/physical-setup-v1/README.md) |
| 14/10 | Motores isolados: frente 1 s, pausa, ré 1 s em loop | Sequência e tempos corretos na serial | [motor-test-v1](../cp2/evidence/motor-test-v1/README.md) |
| 14/10 | Sensor real: alvos a ~20, ~40 e ~80 cm | 81 leituras válidas por captura, intervalo de 100 ms | [calibration/](../cp2/data/real/calibration/) · [diário](../cp2/data/real/diario-coleta.md) |
| 14/10 | Inferência real na placa (int8) | 120 leituras válidas, 11 inferências | [on-device.json](../cp2/evidence/real-training-v1/on-device.json) |
| 14/10 | Inferência → motores | Os dois motores avançaram, reduziram e pararam conforme a classe | [motor-response.json](../cp2/evidence/real-training-v1/motor-response.json) |
| 14/10 | Partida autônoma sem enviar `A` | `auto_armed`, 3 manobras ré/giro após `bloqueado`, avanço após `livre` | [autonomous-v2.json](../cp2/evidence/real-training-v1/autonomous-v2.json) |
| 15/10 | Aproximação de obstáculo com ajuste de PWM | PWM 202 a 31 cm, 190 a 25 cm, 180 a 20 cm; parada + ré + giro a 19,7 cm; 16 manobras; rodas confirmadas pelo grupo | [navigation-distance-v3.json](../cp2/evidence/real-training-v1/navigation-distance-v3.json) |
| 18/10 | Modelo float32 no PC + assertions C++ da navegação + 8 testes Python | 8/9 no teste reservado; todos os testes passaram | [navigation-float32-v4](../cp2/evidence/navigation-float32-v4/README.md) |
| Final | Carrinho no chão, com pilhas, desviando de obstáculo | Ver vídeo | [media/video_carrinho.mp4](../media/video_carrinho.mp4) |

### Testes automatizados

```powershell
python -m pip install -r requirements.txt
python -m unittest discover -s cp2/tests -v
powershell -ExecutionPolicy Bypass -File cp2/tests/test_policy.ps1
```

`test_policy.ps1` compila assertions C++ da política de CP1 e CP2: parada
por distância, PWM proporcional, manobra, desarme, scores inválidos e estouro
de `millis()`.

## Problemas encontrados e correções

| Problema | Causa | Correção |
| --- | --- | --- |
| ECHO do HC-SR04 em 5 V no GPIO de 3,3 V | Sensor alimentado em 5 V | Divisor com resistores de 330 Ω (≈ 3,0 V no GPIO 22) |
| Motor B girava ao contrário | Polaridade da montagem | Sentido invertido em `motors.h` (IN3/IN4); fios mantidos |
| Carrinho parado ao desconectar do PC | Firmware iniciava desarmado e esperava `A` pela serial | Partida automática 5 s após ligar |
| Motores sem força / partida inconsistente | Pilhas caíram de 5,5 V para 3,8 V sob carga; regulador da L298N instável com Vs baixo | Lógica separada no 5 V do USB (jumper 5V_EN removido); pilhas reservadas aos motores e mantidas carregadas |
| Velocidade lenta igual à de cruzeiro | PWM lento e cruzeiro ambos em 255 | PWM 255 / 220 / 180 e redução proporcional entre 40 e 20 cm |
| Ré dependia de o modelo dizer `bloqueado` com alta confiança | Manobra acoplada à classe | Manobra disparada pela distância direta ≤ 20 cm |
| Modelo int8 errava longe do obstáculo | Entrada saturava acima de ~61,9 cm na quantização | Exportação float32 do mesmo treinamento |
| Carrinho travava parado entre 41 e 54 cm | Confiança < 0,80 em distâncias estáticas intermediárias | Avanço cauteloso (PWM 180) com classe conhecida e baixa confiança |
| Uma aproximação classificada como `livre` no teste | Poucos dados (9 janelas de teste) | Limite de 20 cm independente do modelo garante a parada |

## Resultado final

- No vídeo final, o carrinho anda sozinho pelo piso com as pilhas e, quando
  um obstáculo é colocado à frente do sensor, recua e muda de direção antes
  de seguir — ver o [vídeo](../media/video_carrinho.mp4) e as
  [fotos](06-evidencias-uso.md).
- Modelo TinyML: 85,71% na validação interna e **88,89% (8/9)** no teste
  reservado ([relatório CP2](../cp2/docs/entrega.md)).
- Firmware final: 953.667 bytes de flash e 49.420 bytes de RAM estática.

### Limitações conhecidas

- O sensor só mede à frente; a ré não verifica obstáculos atrás.
- Sem encoders, o giro de 450 ms não garante um ângulo fixo.
- A lógica ainda depende de 5 V por USB (powerbank); pilhas só nos motores.
