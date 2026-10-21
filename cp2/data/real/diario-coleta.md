# Diario da campanha real v1

Data: 14/10/2026. Sensor HC-SR04, ESP32 DevKitC V4 / ESP32-D0WD-V3,
serial COM4 a 115200 baud, modo collection a 10 Hz. Motores sem alimentacao
durante a coleta. Materiais e distancias informados pelo usuario; nao ha
verificacao independente da montagem por parte do software.

Projeto de destino: [cp2-carrinho-real-v1, 1140184](https://studio.edgeimpulse.com/studio/1140184).
Projeto sintetico anterior 1139087 preservado separadamente.

## Conferencia exploratoria

- Alvos nominais de 20, 40 e 80 cm, um alvo plano (livro/caixa).
- O usuario esclareceu que 20 e 40 cm foram estimados por falta de espaco.
  Os metadados dessas duas capturas foram corrigidos para registrar a estimativa.
- Cada captura tem 81 leituras validas. Medianas respectivas: 14,560, 44,196 e
  68,377 cm; intervalos medianos de 100 ms, sem intervalos fora de 80–120 ms.
- Nao usar essas referencias estimadas como erro absoluto do sensor.
- CSVs/JSONs em calibration/, fora do lote de treino raw/.

## Sessoes rotuladas

| Arquivo | Classe | Cenario informado | Resultado e observacoes |
| --- | --- | --- | --- |
| raw/livre-s00.csv | livre | Alvo distante, mantendo a posicao anterior da conferencia nominal de 80 cm | 31 medicoes, 29 validas, 2 invalidas; 64,553–209,967 cm, mediana valida 103,895 cm. Revisar variacao/contexto antes de preparar. |
| raw/bloqueado-s00.csv | bloqueado | Usuario esclareceu que colocou a caixa bem perto | 31 medicoes validas; 10,290–10,307 cm, mediana 10,307 cm; sem intervalos irregulares. Antigo livre-s01, reclassificado pela confirmacao do cenario; CSV e hash preservados. |
| raw/livre-s02.csv | livre | Usuario confirmou caixa bem longe e parada | 31 medicoes, 31 validas, 0 invalidas; 58.396–59.991 cm, mediana 59.253 cm; 0 intervalos fora de 80–120 ms. |
| raw/livre-s03.csv | livre | Usuario autorizou nova captura de alvo distante apos pedido de reposicionamento | 31 medicoes, 31 validas, 0 invalidas; 43.681–44.676 cm, mediana 44.504 cm; 0 intervalos fora de 80–120 ms. |
| raw/livre-s04.csv | livre | Usuario autorizou nova captura apos pedido de afastar o alvo e manter parado | 31 medicoes, 31 validas, 0 invalidas; 82.714–86.916 cm, mediana 83.949 cm; 0 intervalos fora de 80–120 ms. |
| raw/livre-s05.csv | livre | Usuario autorizou captura apos pedido de nova posicao distante e parada | 31 medicoes, 31 validas, 0 invalidas; 102.077–126.430 cm, mediana 104.546 cm; 0 intervalos fora de 80–120 ms. |

| raw/aproximando-s01.csv | aproximando | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 30 medicoes, 30 validas; 26.480–53.542 cm; inicio 53.542, fim 26.480 cm. |
| raw/aproximando-s03.csv | aproximando | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 36.872–57.795 cm; inicio 57.452, fim 36.872 cm. |
| raw/aproximando-s04.csv | aproximando | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 29.447–40.525 cm; inicio 39.565, fim 29.447 cm. |
| raw/aproximando-s05.csv | aproximando | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 22.466–50.147 cm; inicio 49.032, fim 22.466 cm. |
| raw/aproximando-s06.csv | aproximando | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 31.762–61.894 cm; inicio 61.466, fim 31.762 cm. |
| raw/bloqueado-s01.csv | bloqueado | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 3.910–3.996 cm; inicio 3.996, fim 3.996 cm. |
| raw/bloqueado-s02.csv | bloqueado | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 12.417–12.434 cm; inicio 12.434, fim 12.434 cm. |
| raw/bloqueado-s03.csv | bloqueado | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 17.613–18.042 cm; inicio 17.630, fim 17.613 cm. |
| raw/bloqueado-s04.csv | bloqueado | Captura guiada continua; reposicionamento solicitado por avisos temporizados, sem confirmacao individual | 31 medicoes, 31 validas; 14.372–14.389 cm; inicio 14.389, fim 14.389 cm. |

O usuario nao informou nomes de operadores nem material exato do alvo.
Nao duplicar uma sessao nem dividi-la em arquivos para simular sessoes
independentes. As proximas capturas devem variar posicao, objeto ou percurso,
com rotulo definido pelo cenario antes de olhar o desempenho do modelo.

## Captura continua guiada

O usuario pediu continuidade sem confirmacoes individuais. Os avisos deram
8–10 segundos para reposicionar antes de cada captura. As sessoes usam
a mesma caixa e ambiente, com trajetorias/distancias variadas. A independencia
entre condicoes e limitada; o teste por sessao nao demonstra generalizacao
para novos objetos ou ambientes.

Tentativas preservadas fora de raw/:
- review/aproximando-s00.csv: Distancia variou pouco, com reversoes; nao correspondeu ao movimento continuo solicitado.
- review/aproximando-s02.csv: Distancia aumentou de aproximadamente 14 para 26 cm: afastamento, nao aproximacao.
