# Modelo do CP2

## Modelo real float32 (perfil padrão desde 18/10/2026)

O perfil `-Model real` usa `edge-impulse-arduino-real-v1-float32.zip`, biblioteca
oficial EON 1.0.2 / deployment v2 do projeto 1140184. Hash e contrato em
[export-real-v1-float32.json](export-real-v1-float32.json). A extração fica em
`library/real-v1-float32/` e o binário em `build/tinyml-real/`.

A referência oficial `cp2-carrinho-real-v1.float32.tflite` permite testar no
computador as mesmas entradas e pesos float32. O DSP recebe centímetros e
aplica escala 0,01 uma vez. A variante evita o recorte de entrada do int8
anterior acima de 61,9 cm. Não houve novo treinamento: os resultados 8/9 do
lote real continuam pequenos e não demonstram generalização. Esta revisão
precisa de nova gravação e teste físico; os uploads abaixo são históricos.

No Arduino IDE, substitua a biblioteca real 1.0.1 pela nova 1.0.2. O firmware
verifica projeto, deployment v2, float32, frequência e tamanho da janela.

## Modelo real v1 int8 (histórico)

[Projeto 1140184](https://studio.edgeimpulse.com/public/1140184/live), dados físicos do HC-SR04. Arquivo [edge-impulse-arduino-real-v1.zip](edge-impulse-arduino-real-v1.zip); identificação, contrato e hash em [export-real-v1.json](export-real-v1.json). Header: `cp2-carrinho-real-v1_inferencing.h`; EON int8, biblioteca 1.0.1. Treino/teste e limitações em [real-training-v1](../evidence/real-training-v1/README.md).

O build anterior extraía em `library/real-v1/` e compilava em `build/tinyml-real/`. `model_config.real.h` inclui a biblioteca real e `inference.h` verifica ID 1140184 e contrato de entrada. O modo de coleta mantém motores parados. O perfil real foi compilado, gravado e testado: inferência e mudança de ação registradas; movimento dos motores confirmado pelo usuário. Estado final desarmado.

## Modelo sintético preservado

O modelo foi treinado no projeto privado [CP2 Carrinho Ultrassom, ID 1139087](https://studio.edgeimpulse.com/studio/1139087). A configuração, métricas e capturas disponíveis estão descritas em [treinamento](../docs/treinamento.md) e arquivadas em `cp2/evidence/edge-impulse/`.

O impulso espera uma janela de 10 valores de `distance_cm` a 10 Hz. O DSP interno aplica escala `0.01` e produz 10 features; o firmware deve enviar distâncias em centímetros sem pré-escala adicional. O classificador usa camadas densas de 20 e 10 neurônios e três saídas.

O treino e as avaliações estão concluídos no conjunto sintético disponível. A validação automática registrou 100% em 96 janelas e o teste reservado, separado por sessão, registrou 100% em 120 janelas nas variantes int8 e float32. Os detalhes estão em `cp2/evidence/edge-impulse/metrics.json`. A validação automática não é separada por sessão, e nenhum desses dados substitui coleta real para a entrega acadêmica.

O ZIP oficial exportado está versionado em [edge-impulse-arduino-v1.zip](edge-impulse-arduino-v1.zip), gerado em 13/10/2026 para o projeto 1139087 com EON int8 1.0.1. SHA-256: `A75AFC6DD4720ECC02080FFA7A0DDE3AFE033B357C1542D9E52B8EEB7AE71C0E`. O cabeçalho é `cp2-carrinho-ultrassom_inferencing.h`. `cp2/tools/build.ps1 -Mode inference -Model synthetic` extrai o ZIP para `library/`, cria `model_config.h` a partir do exemplo e compila com essa biblioteca. A cópia expandida e `model_config.h` são ignorados; o ZIP versionado é a fonte reproduzível.

O modo de inferência compilou com Arduino CLI 1.4.1 e core ESP32 3.3.11: 304.915 bytes de flash e 24.560 bytes de RAM estática. O binário merged em `build/tinyml/carrinho.ino.merged.bin` foi usado nos cenários manuais do navegador Wokwi: `livre` a 100 cm, `bloqueado` a 15 cm, `aproximando` na transição de 80,7 para 40,3 cm, parada segura para baixa confiança a 40,3 cm, parada de emergência a 5 cm inclusive após uma nova ação de movimento, e desarme com `S`. Consulte [as evidências](../evidence/README.md). Os cenários CLI continuam experimentais; ainda falta validar o modelo e a política na montagem física com dados reais.
