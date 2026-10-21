# Treinamento e integração do modelo

## Modelo real v1 — 14/10/2026

Projeto [cp2-carrinho-real-v1, 1140184](https://studio.edgeimpulse.com/public/1140184/live), separado do sintético. Dataset: 15 sessões reais, 464 medições, 44 janelas (35 treino / 9 teste), com sessões inteiras reservadas. Treino no Edge Impulse: Raw Data, escala 0,01, 10 Hz, janela/stride de 1 s, rede densa 20/10, 300 épocas, learning rate 0,005, CPU. Validação interna int8: 6/7 (85,71%); teste reservado int8 e float32: 8/9 (88,89%). Uma aproximação foi confundida com livre.

Veja [métricas, prints e limitações](../evidence/real-training-v1/README.md), [diário](../data/real/diario-coleta.md) e [exportação versionada](../model/export-real-v1.json). São poucos dados do mesmo ambiente; a validação interna não separa sessões, e o teste reservado é pequeno. A exportação real int8 original é `edge-impulse-arduino-real-v1.zip`. Desde 18/10, o perfil padrão usa a exportação oficial float32 v2 `edge-impulse-arduino-real-v1-float32.zip`, sem novo treinamento; consulte [o manifesto atual](../model/export-real-v1-float32.json). Os uploads e PWM descritos a seguir são históricos. O perfil real compilou (956.431 bytes flash, 49.548 bytes RAM), foi gravado e executou 11 inferências na ESP32 com os motores desarmados. Depois, o teste armado registrou livre/PWM 160 e aproximando/PWM 110, seguido de parada; o usuário confirmou movimento, redução e parada dos dois motores. Logs nas evidências da campanha. O ensaio terminou desarmado.

Compile o perfil real com `cp2/tools/build.ps1 -Mode inference -Model real` (padrão), saída `build/tinyml-real/`. Para reproduzir a referência sintética, use `-Model synthetic`, saída `build/tinyml/`. O firmware verifica ID, frequência, janela, classes, float32 v2 no perfil real e int8 no sintético. A inferência arma automaticamente em 5 segundos; S cancela a partida/desarma e A rearma; não são os comandos W/S do sketch manual. A parada direta a 20 cm é proteção adicional e não substitui demonstrar a ação por classificação.

## Referência sintética preservada

O projeto privado [CP2 Carrinho Ultrassom, ID 1139087](https://studio.edgeimpulse.com/studio/1139087) já tem um impulso e um modelo treinado. A entrada corresponde a uma janela de 1 s, avanço de 1 s e frequência de 10 Hz: 10 leituras da única variável `distance_cm`.

## Configuração treinada

- DSP interno com fator de escala `0.01`; o firmware envia centímetros crus e **não** deve aplicar outra escala antes de `run_classifier`.
- DSP produz 10 features para o classificador.
- Rede densa: camadas com 20 e 10 neurônios, seguidas pela saída de três classes (`aproximando`, `bloqueado`, `livre`).
- Treino por 100 épocas, taxa de aprendizado `0.005`, em CPU.

O firmware verifica uma feature bruta por quadro, janela de 10 amostras, intervalo de 100 ms e três classes em `inference.h`.

## Resultados registrados

Na validação do Edge Impulse, int8 e float32 classificaram corretamente as 96 janelas. A matriz int8 tem suporte de 30 `aproximando`, 36 `bloqueado` e 30 `livre`.

| Partição e variante | Accuracy | F1 ponderado | AUC | Loss | Amostras |
| --- | ---: | ---: | ---: | ---: | ---: |
| Validação int8 | 100% | 1,00 | 1,00 | 0,0002451005595369078 | 96 |
| Validação float32 | 100% | 1,00 | 1,00 | 0,0006456008896617337 | 96 |
| Teste reservado int8 | 100% | 1,00 | 1,00 | 0,0007202527067551028 | 120 |
| Teste reservado float32 | 100% | 1,00 | 1,00 | 0,0009657444567473066 | 120 |

Esses 96 exemplos são 20% das janelas da partição de treino, escolhidos automaticamente pelo Edge Impulse sem agrupamento por sessão. Portanto, a validação pode conter janelas da mesma sessão no treino e na validação e não deve ser tratada como teste independente.

Há também um conjunto reservado de 120 janelas em `cp2/data/synthetic/edge-impulse/testing/`, separado por sessão antes do envio. Int8 e float32 acertaram 120/120, com 40 exemplos por classe. As capturas estão em [03-test-float32.jpg](../evidence/edge-impulse/03-test-float32.jpg) e [04-test-int8.jpg](../evidence/edge-impulse/04-test-int8.jpg); [metrics.json](../evidence/edge-impulse/metrics.json) contém os resultados completos de validação e teste. Esse conjunto continua sintético, apesar da separação independente por sessão.

As capturas [01-impulse.jpg](../evidence/edge-impulse/01-impulse.jpg), [02-training.jpg](../evidence/edge-impulse/02-training.jpg), [03-test-float32.jpg](../evidence/edge-impulse/03-test-float32.jpg), [04-test-int8.jpg](../evidence/edge-impulse/04-test-int8.jpg) e [metrics.json](../evidence/edge-impulse/metrics.json) guardam os resultados atuais. Para sustentar uma conclusão acadêmica, colete dados reais do carrinho e repita a avaliação com sessões inteiras reservadas para teste.

## Exportação e compilação

O ZIP oficial exportado está versionado em `cp2/model/edge-impulse-arduino-v1.zip`. Foi gerado em 13/10/2026 para o projeto 1139087, com EON int8 1.0.1; SHA-256: `A75AFC6DD4720ECC02080FFA7A0DDE3AFE033B357C1542D9E52B8EEB7AE71C0E`. O cabeçalho exportado é `cp2-carrinho-ultrassom_inferencing.h`.

Compile o modo de inferência a partir da raiz do repositório:

```powershell
powershell -ExecutionPolicy Bypass -File .\cp2\tools\build.ps1 -Mode inference -Model synthetic
```

O script descompacta o ZIP em `cp2/model/library/`, cria `model_config.h` a partir do exemplo se necessário e passa a biblioteca ao Arduino CLI. O conteúdo expandido e `model_config.h` são locais e ignorados pelo repositório. Os binários ficam em `build/tinyml/`. A compilação foi concluída com Arduino CLI 1.4.1 e core ESP32 3.3.11: 304.915 bytes de flash e 24.560 bytes de RAM estática. O arquivo `build/tinyml/carrinho.ino.merged.bin` foi usado para executar o firmware no navegador Wokwi.

No navegador Wokwi, o firmware compilado foi verificado manualmente em `livre` a 100 cm e `bloqueado` a 15 cm, e na transição 80,7 → 40,3 cm para `aproximando`. Uma leitura estática ambígua a 40,3 cm ficou abaixo do limiar de confiança 0,80 e resultou em ação 0; a emergência a 5 cm manteve os sinais desligados mesmo após uma nova ação de movimento. As capturas e logs estão em [evidence/wokwi](../evidence/README.md). Esses resultados são do navegador, não dos cenários automatizados CLI. Na montagem real, teste com as rodas suspensas; envie `A` para armar e `S` para desarmar. A validação física e os dados reais do grupo permanecem pendentes.
