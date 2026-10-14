# Parte 2 — integração Edge Impulse

O projeto público é [tinyml-ldr-pedrobarone](https://studio.edgeimpulse.com/public/1120098/live). A tela pedida pela rubrica está em [Classifier](https://studio.edgeimpulse.com/public/1120098/live/impulse/1/learning/keras/3).

Antes da captura Wokwi atual, o conjunto sintético então salvo como `../parte1/dados.csv` foi enviado ao Edge Impulse; o projeto público registra 199 amostras. O arquivo local `../parte1/dados.csv` foi posteriormente regenerado a partir do log Wokwi e já não é aquele upload. A captura atual (`../parte1/monitor_serial_wokwi.txt`) foi feita no simulador e validada pelo parser, com 200 leituras brutas, 20 janelas e 10 amostras por classe. Não houve medição em placa física. Assim, as evidências oficiais abaixo descrevem o modelo treinado com o conjunto sintético enviado e não são métricas treinadas sobre a captura Wokwi atual.

## Evidências oficiais

- `classifier_edge_impulse.png` é o print da tela Classifier, com o nome do projeto, Accuracy 100,0%, Loss 0,31, a matriz de confusão e a variante Quantized (int8).
- `metricas.json` registra esses valores e também o detalhe público das variantes int8 e float32. No conjunto de validação, as duas acertam as 32 amostras: 13 da classe 0 e 19 da classe 1.
- `biblioteca_arduino_edge_impulse.zip` é a biblioteca `tinyml-ldr-pedrobarone_inferencing` do projeto 1120098. A exportação EON não contém um `model.h` isolado com os pesos; o modelo está em `src/tflite-model/tflite_learn_1120098_3_compiled.cpp`, referenciado por `src/model-parameters/model_variables.h`.
- `model.h`, nesta pasta, é um wrapper de compatibilidade: inclui o cabeçalho da biblioteca Edge Impulse e verifica em compilação o id do projeto, a entrada de uma feature, duas classes e a quantização int8. Pesos e implementação continuam na biblioteca exportada.

## Demonstração local

`gerar_entrega.py`, `metricas_demo_local.json`, `diagnostico_treino_local_sintetico.png` e `inferencia_ldr/modelo_local_demonstracao.h` formam uma demonstração independente. Ela usa dados sintéticos gerados pelo próprio script e um quantizador local. Não são o resultado, o print nem a exportação do Edge Impulse. O script não lê nem escreve arquivos da Parte 1.

`inferencia_ldr/inferencia_ldr.ino` usa apenas esse modelo local e não chama a biblioteca oficial.

## Inferência com a biblioteca oficial

`inferencia_edge_impulse/inferencia_edge_impulse.ino` é o exemplo de integração com o pacote exportado. O sketch inclui diretamente o cabeçalho da biblioteca oficial instalada; `model.h` é fornecido separadamente como wrapper de compatibilidade. A leitura do LDR Wokwi no GPIO36 é agregada em janelas de 10 amostras separadas por 100 ms, normalizada com `feature = 1 - média/4095` e passada a `run_classifier` como uma entrada. A saída serial prevista inclui as pontuações de cada classe e o rótulo predito. A fórmula invertida corresponde à polaridade do fotoresistor Wokwi; ela não indica medição nem calibração em placa física. A compilação e a execução desse sketch ainda não foram validadas nesta entrega.

Os valores lux/ADC apresentados nas métricas locais são uma estimativa didática idealizada. Eles não calibram o ADC do ESP32 nem substituem leituras do sensor no Wokwi.
