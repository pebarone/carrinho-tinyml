# Campanha real v1 — 14/10/2026

> Revisão posterior de controle: [navegação autônoma](../../docs/NAVEGACAO_AUTONOMA.md).
> O firmware passa a iniciar após 5 segundos e executar ré/giro para bloqueio.
> Os ensaios abaixo de avanço/lentidão/parada descrevem a versão anterior.
> `autonomous-v2.json`, quando presente, registra o novo teste serial, sem novas
> sessões de treinamento e sem confirmar deslocamento físico por si só.

O ciclo **sensor fisico -> CSV -> Edge Impulse -> treino -> teste -> biblioteca
Arduino -> ESP32 -> inferencia -> acao dos motores** foi executado. O movimento
fisico foi confirmado pelo usuario apos ensaio com rodas suspensas; a serial
registrou as classificacoes, acoes e paradas. Nao houve gravacao de video.

## Projetos separados

| Projeto | Finalidade | Estado |
| --- | --- | --- |
| [cp2-carrinho-real-v1, 1140184](https://studio.edgeimpulse.com/public/1140184/live) | Dados reais HC-SR04 do grupo | Público desde 21/10/2026; treinado, testado e exportado |
| [cp2-carrinho-ultrassom, 1139087](https://studio.edgeimpulse.com/studio/1139087) | Referencia sintetica anterior | Preservado com seus dados, biblioteca e metricas |
| tinyml-ldr-pedrobarone, 1120098 | Atividade anterior de LDR | Preservado, fora do carrinho |

Nao foram misturados dados sinteticos ao lote real nem apagados os projetos
anteriores. As metricas sinteticas nao representam este modelo real.

## Sensor e dataset

ESP32 DevKitC V4 / ESP32-D0WD-V3 rev. 3.1, HC-SR04 (TRIG 23, ECHO 22),
serial COM4 a 115200 baud. Amostragem a 10 Hz, centimetros crus. Os seis
resistores de 330 ohms informados pelo usuario formam 990/990 ohms, com GPIO22
na emenda central. As capturas ocorreram com motores desarmados e sem
alimentacao de Vs durante a coleta inicial.

Quinze sessoes aceitas, cinco por classe, com 464 medicoes reais (462 validas).
O preparador gerou 44 janelas nao sobrepostas de 10 leituras: 440 pontos.
Leituras invalidas, trechos interrompidos e sobras permanecem nos CSVs brutos;
nao foram interpolados. CSVs e JSONs tem origem e SHA-256 verificaveis.

| Classe | Cenario | Training | Testing |
| --- | --- | ---: | ---: |
| livre | Caixa distante e parada | 11 | 3 |
| bloqueado | Caixa proxima e parada | 12 | 3 |
| aproximando | Movimento continuo de longe para perto | 12 | 3 |
| Total | 15 sessoes | 35 | 9 |

Doze sessoes inteiras ficaram no treino e tres no teste, sem intersecao,
com split deterministico (seed 42). Os arquivos estao em
`cp2/data/real/raw/` e `cp2/data/real/edge-impulse-v1/`; manifest.json registra
a sessao de origem de cada janela. O diario completo esta em
`cp2/data/real/diario-coleta.md`; estatisticas por sessao em session-summary.json.

O usuario pediu capturas continuas guiadas: foram dados avisos e 8–10 segundos
para reposicionar entre sessoes, sem confirmacao individual em cada uma.
A mesma caixa e ambiente limitam a independencia entre condicoes.

### Conferencias e revisoes preservadas

- Tres capturas exploratorias em calibration/: medianas 14,560, 44,196 e
  68,377 cm para posicoes nominais 20, 40 e 80 cm. O usuario esclareceu que
  20 e 40 cm foram estimados, sem regua. Nao permitem afirmar erro absoluto
  nem aplicar ganho/offset. Todas tiveram 81 leituras validas a cada 100 ms.
- A captura inicialmente chamada livre-s01 foi reclassificada para
  bloqueado-s00 depois que o usuario confirmou caixa proxima. CSV/hash
  preservados; justificativa no JSON e diario. Nao foi duplicada no lote.
- aproximando-s00 e s02 ficaram em review/: variacao sem aproximacao continua
  e afastamento, respectivamente. A exclusao ocorreu antes de treinar e se
  baseou na incompatibilidade do movimento, nao no desempenho do modelo.
- bloqueado-s01 ficou em aproximadamente 3,9 cm, abaixo da faixa inicialmente
  sugerida de 10–20 cm, mas coerente com obstaculo proximo.

## Treinamento executado no Edge Impulse

CSV Wizard: virgula, cabecalho, timestamp em milissegundos, uma leitura por
linha, apenas distance_cm como entrada, rotulo pelo nome do arquivo, sem
novo fatiamento. Training e Test foram enviados separadamente para preservar
o split por sessao. O site confirmou 35 exemplos de treino e 9 de teste.

Impulse: janela/stride de 1000 ms, 10 Hz, Raw Data com escala 0,01, sem
normalizacao adicional; rede densa 20/10 e tres saidas. Treino: 300 epocas,
learning rate 0,005, CPU. A biblioteca aplica a escala; o firmware nao deve
multiplicar os centimetros novamente antes de run_classifier.

| Particao | Variante | Acertos | Acuracia | Loss |
| --- | --- | --- | --- | --- |
| Validacao interna | int8 | 6/7 | 85,71% | 0,329280 |
| Validacao interna | float32 | 6/7 | 85,71% | 0,321268 |
| Teste reservado | int8 | 8/9 | 88,89% | 0,214955 |
| Teste reservado | float32 | 8/9 | 88,89% | 0,187927 |

Matriz de teste int8, linhas reais e colunas previstas na ordem aproximando,
bloqueado, livre: [[2,0,1],[0,3,0],[0,0,3]]. Uma aproximacao foi confundida
com livre. F1 ponderado aproximadamente 0,886. Valores exatos em metrics.json,
download oficial do site incluindo validacao e testes. validation-metrics.json
foi baixado antes dos testes. Nao houve ajuste depois de olhar o teste reservado.

A validacao interna separa janelas e pode compartilhar sessoes. O teste
reservado separa sessoes, mas tem somente nove janelas, do mesmo ambiente.
Nao estabelece generalizacao nem seguranca de navegacao. Melhorias futuras
precisam de novos objetos, ambientes e teste independente.

## Exportacao, compilacao e ESP32

Biblioteca oficial Arduino EON int8, versao 1.0.1, preservada em
`cp2/model/edge-impulse-arduino-real-v1.zip`. Header
`cp2-carrinho-real-v1_inferencing.h`. ID, contrato e SHA-256 em
`cp2/model/export-real-v1.json`. O ZIP sintetico anterior permanece preservado.

Arduino CLI 1.4.1, core ESP32 3.3.11. Perfil real compilado com 956.431 bytes
de flash e 49.548 bytes de RAM estatica, gravado na COM4 com hashes verificados.
O firmware verifica ID 1140184, 10 leituras, 10 Hz, tres classes e int8.

`build.ps1 -Mode inference -Model real` gera build/tinyml-real/ (padrao).
`-Model synthetic` preserva a reproducao em build/tinyml/; a simulacao solicita
esse perfil explicitamente. A primeira observacao desarmada registrou 120
leituras validas e 11 inferencias livre, confianca 0,92969, action=0.
Tempos dsp/nn inteiros de 0 ms significam abaixo da resolucao reportada,
nao duracao nula. Log e hash do firmware em on-device.json e
on-device-disarmed-serial.txt.

## Acao real dos motores

Firmware inicia desarmado: A arma e S para/desarma. Leitura invalida ou
obstaculo a 20 cm ou menos interrompe movimento na proxima leitura; qualquer
leitura critica impede movimento naquela janela. Acima desse limite, com
confianca >= 0,80, livre -> PWM 160; aproximando -> PWM 110; bloqueado,
classe desconhecida ou baixa confianca -> STOP. OUT3/OUT4 usa sentido
compensado conforme correcao solicitada pelo usuario.

Ensaios curtos com parada confirmada ao fim de cada fase:

- Livre: modelo livre, confianca 0,92969, action=2 (avanco).
- Primeira aproximacao: alvo chegou abaixo de 20 cm cedo; atuou a barreira
  direta, sem completar uma janela segura de inferencia.
- Fase solicitada como obstaculo: classificacoes aproximando com confiancas
  de 0,49–0,54, action=0. Isso verifica parada por baixa confianca, nao uma
  classificacao fisica bloqueado pelo modelo.
- Aproximacao acima de 20 cm: livre/action=2 -> aproximando/confianca 0,97266/
  action=1 -> baixa confianca/action=0. Portanto, houve mudanca de PWM pela
  classificacao TinyML, alem dos intertravamentos de distancia.

O usuario confirmou: **os dois motores giraram, reduziram e pararam**.
Essa e uma observacao visual informada pelo usuario, nao uma observacao
independente do agente. Registros completos em motor-inference-events.json,
motor-slow-events.json e motor-response.json. Estado final: S confirmado,
desarmado. Nao houve ensaio de navegacao autonoma no chao.

Na primeira fase de motores houve 18 leituras invalidas em 50; na repeticao
acima de 20 cm, 49/49 foram validas. A causa nao foi isolada. Interferencia,
alimentacao e geometria precisam de investigacao se a falha se repetir;
nao foi escondida nem tratada como medida valida. Os intertravamentos
mantiveram a parada em leituras invalidas.

## Evidencias e verificacoes

- 01-projeto-real.png: projeto privado criado.
- 02-csv-wizard.png: configuracao de importacao concluida.
- 03-dataset-real.png: split 35/9 preservado.
- 04-impulse.png e 05-features.png: configuracao e features.
- 06-training-settings.png: arquitetura e parametros.
- 07-validation-int8.png: validacao interna.
- 08-test-float32.png e 09-test-int8.png: testes e matrizes.
- metrics.json, dataset.json e session-summary.json: metricas e dados.
- preparation.json: registro historico da preparacao, superado pelos registros
  posteriores; nao representa o estado final.

Quatro testes Python aprovados, incluindo contrato dos dois ZIPs, e assertions
C++ de politica aprovadas. Build real aprovado; resultado do build sintetico
registrado na documentacao de verificacao. Binarios com configuracao Wi-Fi
local nao sao versionados. Ainda faltam o video e a postagem da entrega.
