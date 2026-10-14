# Dados do CP2

Esta pasta guarda dados do carrinho e manifestos de origem. Mantenha cada CSV de sessão junto ao JSON que registra classe, origem, quantidade de medições e hash. A ferramenta `../tools/dataset.py` cria esses arquivos e prepara janelas para o Edge Impulse.

## Organização

- `synthetic/raw/`: sessões artificiais geradas para exercitar o fluxo;
- `synthetic/edge-impulse/`: janelas artificiais preparadas, manifesto e partições `training/` e `testing/`;
- `real/raw/`: destino recomendado para CSVs e metadados de sessões físicas;
- `real/edge-impulse/`: destino recomendado para as janelas preparadas com dados físicos.

As duas pastas `real/` ainda precisam ser criadas quando houver coleta. Não substitua ou edite os arquivos sintéticos para fazê-los parecer reais. A ferramenta verifica o hash e bloqueia origem simulada por padrão. Consulte [coleta](../docs/coleta.md) para o protocolo e os comandos.

O material atualmente presente tem 30 sessões sintéticas, 10 por classe, totalizando 6.000 medições brutas. O manifesto registra 600 janelas de 10 medições: 480 para treino e 120 para teste, separadas por sessão. Esses dados não são evidência de coleta física nem de desempenho acadêmico.
