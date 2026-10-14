# Parte 1 — projetos Wokwi e dados

## Estado dos dados

Não há leitura de uma placa física nesta entrega. Os arquivos `leituras_brutas.csv` e `dados.csv` desta pasta foram extraídos de `monitor_serial_wokwi.txt`, um trecho de 20 janelas consecutivas efetivamente observadas no Monitor Serial do projeto Wokwi público. O parser validou as 200 leituras ADC e as 20 features (10 da classe 0 e 10 da classe 1) antes de gravar os CSVs. Trata-se de simulação, não de medição física. O treinamento já realizado no Edge Impulse usou o conjunto sintético anterior, não estes CSVs Wokwi.

## Circuito simulado

Os dois projetos usam `wokwi-photoresistor-sensor` com `rl10=15` kΩ e `gamma=0.6`, ligado ao ESP32 DevKit V1:

| Pino do fotoresistor | ESP32 |
| --- | --- |
| VCC | 3V3 |
| GND | GND |
| AO | VP / GPIO36 (ADC1_CH0) |
| DO | sem conexão |

O sketch cru imprime uma leitura ADC de 12 bits a cada 100 ms. O sketch de pré-processamento agrega 10 leituras e limita o valor a 0–4095. O sketch de captura em execução usou intervalo de 5 ms entre leituras para obter o trecho serial; portanto, sua janela dura aproximadamente 50 ms. O módulo Wokwi reduz a tensão AO quando a iluminação aumenta; por isso, para manter `0=escuro` e `1=claro`, a feature é `1 - média/4095`. O firmware classifica com limiar `feature = 0.45`. Essa inversão é específica do módulo simulado; o sketch de bancada continua refletindo o divisor físico documentado no relatório.

## Executar e capturar

1. Abra um dos projetos dentro de `wokwi/` no editor Wokwi e copie o `diagram.json` e o sketch da pasta correspondente. Se o editor nomear o arquivo como `sketch.ino`, use o conteúdo do arquivo `.ino` correspondente.
2. Inicie a simulação e observe a saída no Monitor Serial a 115200 baud.
3. Para cada sketch, obtenha amostras em baixa iluminância e em alta iluminância usando o controle de lux do componente durante a simulação. Para cruzar o limiar deste circuito, use valores próximos de 1 lux e 1000 lux. Descarte a saída de inicialização e não inclua linhas que não sejam leituras.
4. Para reproduzir a entrega, copie um trecho completo do Monitor Serial para um arquivo de texto dentro de `deliverables/parte1`. O arquivo `monitor_serial_wokwi.txt` registra 20 janelas consecutivas da execução observada, incluindo a transição entre 5 e 1000 lux.
5. Execute `python deliverables/parte1/wokwi/processar_captura.py deliverables/parte1/monitor_serial_wokwi.txt`. O script aceita o protocolo sem índices (`tipo,valor,label`; `raw,<adc>,`; `feature,<feature>,<label>`) e também a variante indexada. Valida o ADC, a sequência dos índices quando disponíveis, cada janela de 10 leituras, a média, o limiar e a presença das duas classes. Só então grava `leituras_brutas.csv` e `dados.csv` nesta pasta.
6. Registre no relatório que os dados vieram do Wokwi. Não confunda esses CSVs com o conjunto sintético de 199 amostras usado no treinamento do Edge Impulse.

Na captura, as janelas estáveis mostraram ADC 2844 em 5 lux (feature 0,3055, escuro) e ADC 354 em 1000 lux (feature 0,9136, claro). A janela de transição inclui nove valores 2844 e um valor 354, produzindo feature 0,3663. Os extremos observados nesse trecho são 354 e 2844; eles não são os extremos físicos do sensor.
