# CP1: referência por limiar

Este firmware foi reconstruído neste workspace como referência para comparar o controle simples por distância com o classificador TinyML do CP2. Não é apresentado como firmware histórico recuperado do CP1 original.

Com HC-SR04 e L298N, o ESP32 aplica estas regras: acima de 40 cm, avança; acima de 20 cm e até 40 cm, reduz a velocidade; em 20 cm ou menos, para. Abaixo de 8 cm e em leituras inválidas, a parada também é imediata. O carrinho inicia desarmado: envie `A` para armar e `S` para parar e desarmar. A saída serial a 115200 baud é `timestamp,distance_cm,valid,action`.

Compile para ESP32 DevKit V1 com Arduino CLI, a partir da raiz do repositório:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 cp1/firmware/carrinho_threshold
```

O firmware do CP2 troca as regras fixas por janelas de 10 leituras, inferência e a política de segurança documentada em [`../cp2/docs/treinamento.md`](../cp2/docs/treinamento.md).
