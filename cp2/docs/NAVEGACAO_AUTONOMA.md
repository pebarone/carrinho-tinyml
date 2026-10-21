# Navegação autônoma com o modelo real

O firmware anterior iniciava desarmado e exigia `A` na serial. Sua política
avançava/reduzia/parava, mas não executava curvas. Isso explica ficar parado
ao desligar do PC, mesmo quando a alimentação estava correta.

A versão atual usa a biblioteca oficial Arduino EON **float32 v2 (1.0.2)** do
projeto Edge Impulse **1140184**, exportada em 18/10/2026. O treinamento e o
split de 35/9 janelas reais foram preservados. Float32 remove a saturação da
entrada int8 acima de aproximadamente 61,9 cm; não amplia os dados de treino.

## Comportamento

- Ao ligar ou pressionar RESET, espera 5 segundos e arma automaticamente.
- Antes de avançar, exige uma janela nova de 10 leituras válidas a 10 Hz.
- Acima de 40 cm, com confiança >= 0,80: `livre` avança com PWM 255;
  `aproximando` avança com PWM 220; `bloqueado` para.
- De 40 cm até acima de 20 cm, uma previsão conhecida com score válido
  limita o avanço por distância: PWM 220 a 180, atualizado a cada 100 ms.
- Com confiança abaixo de 0,80 e classe conhecida, avança cautelosamente
  com PWM 180, sempre sujeito à parada por distância. Isso evita ficar
  permanentemente parado em distâncias estáticas de 41–54 cm.
- Em 20 cm ou menos, independentemente da previsão: para, pausa 150 ms,
  ré 300 ms, pausa 150 ms, giro à direita 450 ms e pausa 150 ms.
- Depois da manobra, exige nova janela antes de avançar. Se o alvo ainda
  estiver em até 20 cm, repete a manobra.
- Leitura inválida, amostragem atrasada, erro de inferência, score inválido
  ou classe desconhecida param o avanço. Leitura inválida cancela a manobra.
- Ré/giro da manobra podem ocorrer perto do alvo; o limite de 20 cm impede
  avanço. O sensor dianteiro não verifica obstáculos atrás.
- `S` para, desarma e cancela a partida automática; `A` rearma e exige nova
  janela. RESET reinicia a espera de 5 segundos. O modo collection nunca arma.

Na serial, `action=0` significa parado, `1` lento, `2` avanço, `3` ré e `4`
giro à direita. Linhas `# drive` mostram ação, PWM e distância;
linhas `# avoidance` mostram as etapas da manobra. A política
básica de `policy.h` continua retornando STOP para bloqueio; `navigation.h`
implementa a redução por distância e a sequência temporizada acionada pelo
limite de parada. O modelo sozinho não inicia mais ré por indicar `bloqueado`
com o alvo distante. A ré e o giro têm duração fixa, mesmo se o alvo se afastar
durante a sequência; leitura inválida ou `S` cancelam a manobra.

O HC-SR04 só mede à frente: a ré é curta e não verifica obstáculos atrás;
o giro tenta uma direção e mede novamente. Sem encoders, 450 ms não garantem
um ângulo específico. É uma navegação reativa, sem mapa e sem garantia de
sair de cantos. A janela de aproximadamente 1 segundo limita a reação do
modelo; o limite direto de distância atua entre inferências.

## Compilar e gravar

Sketch: `cp2/firmware/carrinho/carrinho.ino`. O padrão no Arduino IDE agora
é inferência; instale o ZIP `cp2/model/edge-impulse-arduino-real-v1-float32.zip` pelo
menu de inclusão de biblioteca ZIP. Substitua a biblioteca real 1.0.1 pela
1.0.2; a compilação recusa o modelo int8 antigo no perfil real. Selecione ESP32 Dev Module e a porta da
placa. A coleta continua disponível explicitamente pelo script de build.

```powershell
powershell -ExecutionPolicy Bypass -File cp2/tools/build.ps1 -Mode inference -Model real
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM4 --input-dir build/tinyml-real cp2/firmware/carrinho
```

O funcionamento não depende de PC, Monitor Serial ou Wi-Fi. Precisa, porém,
de alimentação adequada para **ESP32/sensor/lógica e para Vs dos motores**.
Com 5V_EN removido, alimentar somente o borne 12V/Vs não alimenta a lógica.
Veja [hardware.md](hardware.md). Preserve a compensação de sentido do motor
OUT3/OUT4 existente no código; não inverta novamente esses fios.

Os ensaios anteriores de avanço/lentidão/parada continuam documentados como
históricos em `cp2/evidence/real-training-v1/`. Eles não validam esta nova
manobra nem navegação no chão. O novo ensaio deve distinguir os comandos
vistos pela serial do deslocamento observado pelo usuário.

## Revisão de 18/10/2026

O perfil real passou a usar float32 v2 e o avanço cauteloso de baixa confiança.
Os testes desktop incluem igualdade dos pesos do ZIP com o TFLite oficial,
entradas intermediárias/distantes e contrato dos exports; as assertions C++
incluem as janelas reais de 45/50/55 cm e os intertravamentos de navegação.
Registros e limites em [navigation-float32-v4](../evidence/navigation-float32-v4/README.md).
A nova versão ainda requer gravação e ensaio físico. As seções de 14–15/10
abaixo são históricas e pertencem ao firmware int8 anterior.

## Histórico de verificação — 14/10/2026

Compilação e gravação na COM4 concluídas (core ESP32 3.3.11): 957.135 bytes
de flash, 49.564 bytes de RAM estática. As asserções C++ de política e manobra
(incluindo cancelamento e estouro de millis) e os quatro testes Python passaram.

Sem enviar `A`, a serial registrou `auto_armed`, três sequências completas de
ré/parada/giro/parada após `bloqueado` (0,82422–0,86719) e depois avanço após
`livre` (0,92969). Foram 130 leituras de teste, todas válidas; nenhuma foi
incluída no dataset. `S` foi confirmado ao final, deixando a placa desarmada.
RESET ou desligar/religar inicia novamente a contagem automática de 5 segundos.
Registro: `cp2/evidence/real-training-v1/autonomous-v2.json`.
O teste comprova execução e comandos na placa; deslocamento, ângulo e desvio
no chão ainda precisam ser observados fisicamente.

## Histórico de ajuste e teste de aproximação — 15/10/2026

Foram corrigidos dois problemas: os PWM lento e de cruzeiro estavam ambos
em 255, e a ré dependia de uma classificação `bloqueado` com alta confiança,
mesmo quando a distância já exigia parar. A redução agora é atualizada por
leitura na faixa de 40–20 cm; a manobra começa pelo limite direto de 20 cm.

Firmware real compilado e gravado na ESP32-D0WD-V3 pela COM4, com Arduino CLI
1.4.1 e core ESP32 3.3.11: 957.679 bytes de flash e 49.572 bytes de RAM estática.
As asserções C++ da política/manobra e os quatro testes Python passaram.
O modo de coleta também compilou: 926.151 bytes de flash e 47.316 bytes de
RAM estática; não foi gravado sobre o firmware de inferência.

O ensaio serial registrou 408 leituras válidas de 3,91 a 82,92 cm. Durante a
aproximação, PWM 202 a 31,13 cm, 190 a 25,33 cm e 180 a 20,03 cm; em 19,69 cm
houve parada seguida de ré/pausa/giro/parada. Foram 16 manobras completas
enquanto o alvo permaneceu próximo. Ao afastá-lo, voltou a `livre` (0,92969)
e avanço com PWM 255. O usuário confirmou redução das rodas, ré e giro.
`S` foi confirmado ao final; a placa ficou desarmada. `A` rearma e RESET
reinicia a partida automática em 5 segundos.

Registro: `cp2/evidence/real-training-v1/navigation-distance-v3.json`, separado
do dataset. Este ensaio não verifica deslocamento/ângulo no chão nem calibra
o PWM mínimo para diferentes pisos, cargas ou alimentação.
