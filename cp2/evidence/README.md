# Evidências do CP2

## Edge Impulse

Os registros em `edge-impulse/` cobrem o impulso, treino, validação e teste reservado:

| Arquivo | Registro |
| --- | --- |
| `01-impulse.jpg` | Configuração do impulso e entrada. |
| `02-training.jpg` | Treino e resultados exibidos pelo Edge Impulse. |
| `03-test-float32.jpg` e `04-test-int8.jpg` | Resultados do teste reservado. |
| `metrics.json` | Métricas completas de validação e teste int8/float32. |
| `training-metrics.json` | Registro anterior apenas da validação. |

A validação automática acertou 96/96 janelas sintéticas; o teste reservado, separado por sessão e com 40 exemplos por classe, acertou 120/120 em ambas as variantes. A validação automática usa 20% das janelas de treino sem agrupamento por sessão. Consulte [treinamento](../docs/treinamento.md) para os valores de loss e a interpretação dos resultados.

## Wokwi

A coleta serial do firmware está em `wokwi/01-collection.jpg` e `wokwi/collection-serial.txt`: alvo simulado de 100 cm, leituras válidas próximas de 100,8 cm a 10 Hz.

Os cenários abaixo foram executados manualmente no navegador do [projeto Wokwi público](https://wokwi.com/projects/477821599079365633) com `build/tinyml/carrinho.ino.merged.bin`. Os LEDs são indicadores dos sinais ENA/ENB simulados.

| Evidência | Condição | Predição/resultado |
| --- | --- | --- |
| `wokwi/02-free.jpg`, `wokwi/free-serial.txt` | 100 cm | `livre`, confiança 0,99609; ação 2, PWM 160, ENA/ENB ligados; inferência 2 ms |
| `wokwi/03-blocked.jpg`, `wokwi/blocked-serial.txt` | 15 cm | `bloqueado`, confiança 0,99609; ação 0, ENA/ENB desligados; inferência 3 ms |
| `wokwi/06-approach.jpg`, `wokwi/approach-transition-serial.txt` | transição de 80,7 para 40,3 cm | `aproximando`, confiança 0,99609; ação 1, PWM 110; inferência 2 ms |
| `wokwi/05-low-confidence.jpg`, sequência em `wokwi/approach-serial.txt` | leitura estática a 40,3 cm | scores: `aproximando` 0,53516, `bloqueado` 0,07812, `livre` 0,38672; confiança máxima abaixo de 0,80, ação 0 (parada segura) |
| `wokwi/04-emergency.jpg`, `wokwi/emergency-after-motion-serial.txt` | emergência a 5 cm, seguida de leitura que pede movimento | sinais ENA/ENB continuam desligados |
| `wokwi/07-disarmed.jpg`, `wokwi/disarmed-serial.txt` | comando serial `S` | registro `# disarmed`; simulação termina parada em estado seguro |

`wokwi/emergency-serial.txt` também registra o cenário de emergência. A coleta e as verificações acima são execuções manuais no navegador; não atestam aprovação dos cenários automatizados pelo Wokwi CLI. Em particular, a automação CLI de aproximação permanece experimental e não verificada.

## Compilação e limites

O firmware de inferência compilou com Arduino CLI 1.4.1 e core ESP32 3.3.11, usando 304.915 bytes de flash e 24.560 bytes de RAM estática. O ZIP oficial e reproduzível fica em `cp2/model/edge-impulse-arduino-v1.zip`.

Todos os resultados Edge Impulse atuais usam dados sintéticos; o Wokwi também é simulado. Ainda faltam dados reais do grupo e demonstração/teste do carrinho montado. As simulações não demonstram desempenho físico.
