# Modelo do CP2

O modelo foi treinado no projeto privado [CP2 Carrinho Ultrassom, ID 1139087](https://studio.edgeimpulse.com/studio/1139087). A configuração, métricas e capturas disponíveis estão descritas em [treinamento](../docs/treinamento.md) e arquivadas em `cp2/evidence/edge-impulse/`.

O impulso espera uma janela de 10 valores de `distance_cm` a 10 Hz. O DSP interno aplica escala `0.01` e produz 10 features; o firmware deve enviar distâncias em centímetros sem pré-escala adicional. O classificador usa camadas densas de 20 e 10 neurônios e três saídas.

O treino e as avaliações estão concluídos no conjunto sintético disponível. A validação automática registrou 100% em 96 janelas e o teste reservado, separado por sessão, registrou 100% em 120 janelas nas variantes int8 e float32. Os detalhes estão em `cp2/evidence/edge-impulse/metrics.json`. A validação automática não é separada por sessão, e nenhum desses dados substitui coleta real para a entrega acadêmica.

O ZIP oficial exportado está versionado em [edge-impulse-arduino-v1.zip](edge-impulse-arduino-v1.zip), gerado em 13/10/2026 para o projeto 1139087 com EON int8 1.0.1. SHA-256: `A75AFC6DD4720ECC02080FFA7A0DDE3AFE033B357C1542D9E52B8EEB7AE71C0E`. O cabeçalho é `cp2-carrinho-ultrassom_inferencing.h`. `cp2/tools/build.ps1 -Mode inference` extrai o ZIP para `library/`, cria `model_config.h` a partir do exemplo e compila com essa biblioteca. A cópia expandida e `model_config.h` são ignorados; o ZIP versionado é a fonte reproduzível.

O modo de inferência compilou com Arduino CLI 1.4.1 e core ESP32 3.3.11: 304.915 bytes de flash e 24.560 bytes de RAM estática. O binário merged em `build/tinyml/carrinho.ino.merged.bin` foi usado nos cenários manuais do navegador Wokwi: `livre` a 100 cm, `bloqueado` a 15 cm, `aproximando` na transição de 80,7 para 40,3 cm, parada segura para baixa confiança a 40,3 cm, parada de emergência a 5 cm inclusive após uma nova ação de movimento, e desarme com `S`. Consulte [as evidências](../evidence/README.md). Os cenários CLI continuam experimentais; ainda falta validar o modelo e a política na montagem física com dados reais.
