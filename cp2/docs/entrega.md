# CP2 — Carrinho com TinyML

**[Repositório](https://github.com/pebarone/cp2-carrinho-tinyml)** · **[Edge Impulse público — projeto 1140184](https://studio.edgeimpulse.com/public/1140184/live)** · **Seção da entrega: `cp2/`**

O CP2 acrescenta ao carrinho a classificação de distâncias com um modelo
Edge Impulse executado na ESP32. A saída altera os comandos dos motores.

## 1. Sensor, coleta e CSVs

O grupo utilizou **HC-SR04**, ESP32 DevKitC V4 e captura serial a 115200 baud.
O sensor foi amostrado a **10 Hz**, com motores desligados durante a coleta;
uma caixa foi posicionada perto, longe ou movida em direção ao sensor.
Cada linha bruta contém `timestamp` (ms), `distance_cm` e `valid`.

Foram aceitas **15 sessões reais**, cinco por classe: **464 medições, 462
válidas**. Após descartar leituras inválidas e sobras, sem interpolar, foram
preparadas **44 janelas de 10 leituras (440 pontos)**.

| Classe | Situação coletada | Janelas de treino | Janelas de teste |
| --- | --- | ---: | ---: |
| `livre` | Caixa distante e parada | 11 | 3 |
| `aproximando` | Caixa se aproximando continuamente | 12 | 3 |
| `bloqueado` | Caixa próxima e parada | 12 | 3 |
| **Total** | **15 sessões** | **35** | **9** |

Os [CSVs brutos e metadados](../data/real/raw/), [CSVs importados](../data/real/edge-impulse-v1/)
e [manifesto de origem](../data/real/edge-impulse-v1/manifest.json) estão
versionados. O [diário de coleta](../data/real/diario-coleta.md) registra
reposicionamentos, reclassificações e exclusões anteriores ao treinamento.
Tentativas inadequadas e conferências exploratórias foram preservadas fora
do treino. As distâncias de referência estimadas não foram tratadas como
calibração precisa. Não foram misturados dados sintéticos ao lote real.

## 2. Preparação e treinamento no Edge Impulse

O projeto real é separado da referência sintética anterior. Em **21/10/2026**,
foi tornado público: qualquer pessoa pode visualizar e clonar; edição continua
restrita aos colaboradores. O Edge Impulse o disponibiliza sob BSD de 3 cláusulas.

1. Separação por **sessão inteira**, seed 42: 12 sessões para treino e 3 para teste. Uma sessão nunca aparece nas duas partições.
2. Importação das pastas `training/` e `testing/` separadamente pelo CSV Wizard: vírgula, cabeçalho, timestamp em ms, somente `distance_cm` como entrada e rótulo pelo nome do arquivo.
3. Impulse: **1.000 ms de janela e avanço**, **10 Hz**, processamento **Raw Data**, escala **0,01**. A biblioteca aplica a escala; o código fornece centímetros.
4. Classificador: rede densa com **20 e 10 neurônios**, três saídas, **300 épocas**, learning rate **0,005**, treinamento em CPU.
5. Validação interna de 20% das 35 janelas: 28 para ajuste e 7 para validação. O teste reservado de 9 janelas foi avaliado separadamente.

### Resultados

| Avaliação | Acertos | Acurácia | Loss float32 |
| --- | ---: | ---: | ---: |
| Validação interna | 6/7 | 85,71% | 0,321268 |
| Teste reservado | 8/9 | 88,89% | 0,187927 |

Int8 obteve os mesmos totais de acerto. Matriz de confusão do **teste reservado**,
com linhas reais e colunas previstas:

| Real / Previsto | aproximando | bloqueado | livre |
| --- | ---: | ---: | ---: |
| aproximando | 2 | 0 | 1 |
| bloqueado | 0 | 3 | 0 |
| livre | 0 | 0 | 3 |

Uma janela de aproximação foi confundida com livre; F1 ponderado do teste ≈ **0,886**.
A validação interna não é separada por sessão. O teste reservado é, mas tem
apenas nove janelas, da mesma caixa/ambiente: não demonstra generalização.

**Evidências:** [importação e dataset](../evidence/real-training-v1/03-dataset-real.png),
[Impulse](../evidence/real-training-v1/04-impulse.png),
[configuração do treino](../evidence/real-training-v1/06-training-settings.png),
[validação](../evidence/real-training-v1/07-validation-int8.png),
[teste float32](../evidence/real-training-v1/08-test-float32.png) e
[métricas oficiais completas](../evidence/real-training-v1/metrics.json).

## 3. Biblioteca, ESP32 e ação no carrinho

O modelo foi exportado como **biblioteca Arduino oficial EON**. O perfil atual
usa **float32 v2 / biblioteca 1.0.2**, do mesmo treinamento, para evitar a
saturação de entrada da exportação int8 anterior acima de aproximadamente
61,9 cm. [ZIP atual](../model/edge-impulse-arduino-real-v1-float32.zip),
[versão e SHA-256](../model/export-real-v1-float32.json) e exports anteriores
estão preservados em `cp2/model/`.

| Etapa no código | Arquivo |
| --- | --- |
| Ler HC-SR04 e formar janela de 10 distâncias | [carrinho.ino](../firmware/carrinho/carrinho.ino), [ultrasonic.h](../firmware/carrinho/ultrasonic.h) |
| Executar `run_classifier` localmente e obter classe/confiança | [inference.h](../firmware/carrinho/inference.h) |
| Usar a previsão para decidir ação e PWM | [policy.h](../firmware/carrinho/policy.h), [navigation.h](../firmware/carrinho/navigation.h) |
| Aplicar sinais e PWM à L298N | [motors.h](../firmware/carrinho/motors.h) |

Após 5 segundos da partida, o carrinho arma automaticamente. Acima de 40 cm,
com confiança ≥ 0,80, `livre` avança (PWM 255), `aproximando` reduz (220) e
`bloqueado` para. Uma classe conhecida com baixa confiança permite avanço
cauteloso (180). Entre 40 e 20 cm, a distância limita o PWM de 220 a 180.
Em até 20 cm, a proteção direta inicia parada, ré curta e giro à direita;
essa manobra é acionada pela distância, não apresentada como decisão do modelo.
A inferência tem efeito real sobre avanço e velocidade, além dessa proteção.

Leitura inválida para. `S` desarma; `A` rearma. O sensor só observa à frente;
a ré e o giro são temporizados, sem garantia de ângulo ou saída de cantos.
Pinos, alimentação e reprodução: [hardware](hardware.md),
[navegação e instalação](NAVEGACAO_AUTONOMA.md) e [comandos de build](../../README.md#compilar-e-gravar).

## 4. O que foi verificado

| Verificação | Evidência / alcance |
| --- | --- |
| Sensor → inferência real na ESP32 (int8) | [Log na placa](../evidence/real-training-v1/on-device.json): 120 leituras válidas e 11 inferências |
| Inferência → motores (int8) | [Ensaio e confirmação visual do grupo](../evidence/real-training-v1/motor-response.json): os dois motores avançaram, reduziram e pararam |
| Partida e manobra autônoma (versão int8) | [Teste serial](../evidence/real-training-v1/autonomous-v2.json); [aproximação, ré e giro](../evidence/real-training-v1/navigation-distance-v3.json) |
| Revisão atual float32 | [Verificação desktop e compilações](../evidence/navigation-float32-v4/README.md): 8/9 no teste reservado; sem novo treinamento |
| Acesso público ao projeto | [Captura de publicação](../evidence/real-training-v1/10-projeto-publico.png) |

Verificação em 21/10: build real aprovado (953.667 bytes de flash), cinco
testes Python aprovados e assertions C++ aprovadas. O grupo de regressões
TensorFlow foi ignorado neste ambiente por falta de NumPy/TensorFlow; a
execução anterior está registrada na revisão de 18/10. Nenhum upload foi feito
nesta atualização de documentação.

**Limites atuais:** a revisão float32 ainda requer gravação e validação física;
os testes de bancada anteriores não comprovam navegação no chão dessa versão.
O grupo também observou queda das pilhas de 5,5 V para 3,8 V sob carga,
impedindo partida consistente; é uma limitação de alimentação a resolver para
a demonstração. O vídeo será acrescentado pelo grupo.

## 5. Evolução e entrega

`cp1/` mantém a referência por limiar reconstruída; `cp2/` acrescenta dataset,
modelo e inferência. A etapa sintética/Wokwi e a atividade de LDR foram
preservadas e identificadas como históricas. Os commits separam coleta/treino,
integração/testes e documentação, sem reescrever o histórico do CP1.

| Requisito da avaliação | Onde conferir |
| --- | --- |
| Dataset e coleta — 10 pontos | Seção 1, CSVs e diário |
| Edge Impulse — 15 pontos | Projeto público, seção 2, prints e métricas |
| Integração ESP32 — 10 pontos | Seções 3–4, código e biblioteca |
| Organização/evolução — 5 pontos | Pastas CP1/CP2 e histórico Git |
| Funcionamento — 60 pontos | Evidências de bancada; demonstração física e vídeo devem complementar |

Para o **Teams**: enviar o link do repositório, indicar `cp2/` e este documento,
acrescentar o vídeo e os integrantes abaixo. Datas informadas na atividade:
vídeo em **14/10** e apresentação presencial em **21/10**.

| Integrante | RM |
| --- | --- |
| João Pedro de Albuquerque Oliveira | 551579 |
| Pedro Augusto Carneiro Barone Bomfim | 99781 |
| Matheus Augusto Santos Rego | 551466 |

**Vídeo da demonstração:** pendente — o grupo adicionará o link aqui.
