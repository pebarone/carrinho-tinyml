# Dados do CP2

Esta pasta guarda dados do carrinho e manifestos de origem. Mantenha cada CSV de sessão junto ao JSON que registra classe, origem, quantidade de medições e hash. A ferramenta `../tools/dataset.py` cria esses arquivos e prepara janelas para o Edge Impulse.

## Organização

- `synthetic/raw/`: sessões artificiais geradas para exercitar o fluxo;
- `synthetic/edge-impulse/`: janelas artificiais preparadas, manifesto e partições `training/` e `testing/`;
- `real/raw/`: destino recomendado para CSVs e metadados de sessões físicas;
- `real/edge-impulse/`: destino recomendado para as janelas preparadas com dados físicos.

A campanha real está em `real/raw/` e `real/edge-impulse-v1/`. Não substitua ou edite os arquivos sintéticos para fazê-los parecer reais. A ferramenta verifica o hash e bloqueia origem simulada por padrão. Consulte [coleta](../docs/coleta.md) para o protocolo e os comandos.

O material atualmente presente tem 30 sessões sintéticas, 10 por classe, totalizando 6.000 medições brutas. O manifesto registra 600 janelas de 10 medições: 480 em `training/` e 120 em `testing/`, separadas por sessão. O teste de 120 janelas é o conjunto independente mais forte disponível neste conjunto, mas ainda é sintético.

No Edge Impulse, 20% das 480 janelas de treino foram automaticamente reservadas para validação (96 janelas). Essa divisão interna não mantém sessões inteiras juntas; use-a para acompanhar o treino, não para afirmar generalização entre sessões. Os resultados int8 e float32 nessa validação foram 100%, mas não substituem dados reais. A avaliação no `testing/` reservado registrou 100% nas variantes int8 e float32.

O Edge Impulse registrou 100% de accuracy em int8 e float32 no teste reservado de 120 janelas (40 por classe), separado por sessão. A validação interna de 96 janelas também ficou em 100%, mas foi selecionada automaticamente entre as janelas de treino sem agrupar por sessão. As métricas completas estão em `../evidence/edge-impulse/metrics.json`.

A campanha física começou em `real/`: três capturas exploratórias em `calibration/`, 15 sessões rotuladas em `raw/` e duas tentativas de aproximação preservadas em `review/`. O lote real tem 464 medições (462 válidas), que renderam 44 janelas em `edge-impulse-v1/`: 35 de treino e 9 de teste, com sessões inteiras separadas. As referências de 20 e 40 cm foram estimadas pelo usuário, portanto não representam calibração com régua. O projeto Edge Impulse real é 1140184; os dados sintéticos e suas métricas permanecem separados. Consulte [o diário](real/diario-coleta.md) e [as evidências](../evidence/real-training-v1/README.md). O lote real já foi treinado e avaliado em sessões separadas: 8/9 janelas no teste. Novas campanhas devem ampliar objetos, ambientes e distâncias, incluindo posições estáticas entre 20 e 60 cm; preserve um novo teste independente. As métricas sintéticas acima são apenas históricas.
