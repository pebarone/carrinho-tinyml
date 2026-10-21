# Simulação do carrinho TinyML

## Firmware atual

O perfil padrão é `real`: projeto 1140184, Arduino EON float32 v2, biblioteca
1.0.2. O perfil `synthetic` continua disponível para comparação. O firmware
arma sozinho após 5 segundos; `S` desarma/cancela e `A` rearma.
A política completa está em [navegação autônoma](../docs/NAVEGACAO_AUTONOMA.md).

Execute da raiz do repositório:

```powershell
powershell -ExecutionPolicy Bypass -File cp2/tools/build.ps1 -Mode inference -Model real
powershell -ExecutionPolicy Bypass -File cp2/tools/build.ps1 -Mode collection
```

`wokwi.toml` aponta para `build/tinyml-real/`; `wokwi.collection.toml`, para
`build/collection/`. No navegador Wokwi, F1 → Upload Firmware permite carregar
`build/tinyml-real/carrinho.ino.merged.bin`. No [projeto digital](https://wokwi.com/projects/477821599079365633),
os LEDs representam apenas os sinais ENA/ENB, sem simular o carrinho mecânico.

## Cenários CLI

Requer Wokwi CLI e `WOKWI_CLI_TOKEN`. A licença do VS Code não substitui o
token do CLI. O script verifica esses requisitos antes de compilar.

```powershell
powershell -ExecutionPolicy Bypass -File cp2/tools/simulate.ps1 -Mode collection -Scenario collection
powershell -ExecutionPolicy Bypass -File cp2/tools/simulate.ps1 -Mode inference -Scenario free
powershell -ExecutionPolicy Bypass -File cp2/tools/simulate.ps1 -Mode inference -Scenario approach
powershell -ExecutionPolicy Bypass -File cp2/tools/simulate.ps1 -Mode inference -Scenario blocked
powershell -ExecutionPolicy Bypass -File cp2/tools/simulate.ps1 -Mode inference -Scenario emergency
```

`-Model real` é o padrão. Use `-Model synthetic` explicitamente para comparar.
Os cenários usam os banners atuais e aguardam a partida automática:

| Cenário | Distância | Verificação |
| --- | --- | --- |
| collection | 100 cm | Coleta com todos os sinais de motor desligados |
| free | 120 cm | Avanço, sentido dos motores e desarme |
| approach | 45 cm | Avanço lento estático sem travamento e desarme |
| blocked | 15 cm | Parada, ré, pausa, giro à direita, parada e desarme |
| emergency | 5 cm | Mesma manobra, sem avanço contra o alvo, e desarme |

O caso `approach` agora usa distância estática: não depende de `set-control`
do HC-SR04. A variação dinâmica pode ser ensaiada manualmente no navegador.
Logs e VCD ficam em `build/wokwi/`, com modo, modelo e cenário no nome.
Estes cenários atualizados precisam de execução no Wokwi CLI; nenhum resultado
CLI anterior é uma aprovação automática da revisão atual.

## Evidências históricas e limites

Os registros em [evidence/wokwi](../evidence/README.md) são ensaios manuais da
versão sintética anterior: PWM 160/110, sem ré/giro e parada por baixa
confiança. Não descrevem o comportamento atual nem validam o modelo real.

O diagrama digital liga ECHO diretamente à ESP32 somente para simulação.
Na montagem física, use o divisor e a alimentação em [hardware.md](../docs/hardware.md).
O Wokwi não verifica deslocamento, frenagem, ângulo, corrente ou autonomia.
A [montagem ilustrada](assembly/README.md) é um projeto separado; seus chips
customizados e o divisor analógico não tiveram validação dinâmica.
