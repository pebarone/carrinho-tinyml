# Dados do CP2

Esta pasta guarda dados do carrinho e manifestos de origem. Mantenha cada CSV de sessão junto ao JSON que registra classe, origem, quantidade de medições e hash. A ferramenta `../tools/dataset.py` cria esses arquivos e prepara janelas para o Edge Impulse.

## Organização

- `synthetic/raw/`: sessões artificiais geradas para exercitar o fluxo;
- `synthetic/edge-impulse/`: janelas artificiais preparadas, manifesto e partições `training/` e `testing/`;
- `real/raw/`: destino recomendado para CSVs e metadados de sessões físicas;
- `real/edge-impulse/`: destino recomendado para as janelas preparadas com dados físicos.

As duas pastas `real/` ainda precisam ser criadas quando houver coleta. Não substitua ou edite os arquivos sintéticos para fazê-los parecer reais. A ferramenta verifica o hash e bloqueia origem simulada por padrão. Consulte [coleta](../docs/coleta.md) para o protocolo e os comandos.

O material atualmente presente tem 30 sessões sintéticas, 10 por classe, totalizando 6.000 medições brutas. O manifesto registra 600 janelas de 10 medições: 480 em `training/` e 120 em `testing/`, separadas por sessão. O teste de 120 janelas é o conjunto independente mais forte disponível neste conjunto, mas ainda é sintético.

No Edge Impulse, 20% das 480 janelas de treino foram automaticamente reservadas para validação (96 janelas). Essa divisão interna não mantém sessões inteiras juntas; use-a para acompanhar o treino, não para afirmar generalização entre sessões. Os resultados int8 e float32 nessa validação foram 100%, mas não substituem dados reais. A avaliação no `testing/` reservado registrou 100% nas variantes int8 e float32.

O Edge Impulse registrou 100% de accuracy em int8 e float32 no teste reservado de 120 janelas (40 por classe), separado por sessão. A validação interna de 96 janelas também ficou em 100%, mas foi selecionada automaticamente entre as janelas de treino sem agrupar por sessão. As métricas completas estão em `../evidence/edge-impulse/metrics.json`.

Não há coleta física documentada em `real/`. Para a entrega acadêmica, ainda é preciso coletar medições reais, preservar sessões e avaliar com sessões completas separadas do treino. Os resultados atuais caracterizam apenas o conjunto sintético.
