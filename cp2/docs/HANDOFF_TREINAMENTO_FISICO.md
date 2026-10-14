# Handoff — coleta real, treinamento e ESP32 físico

Este documento permite continuar o projeto sem o histórico do chat. Preparado
em 13/10/2026 para a próxima sessão, prevista para 14/10/2026. O usuário pediu
que a etapa virtual fosse concluída primeiro e que a gravação e o treinamento
com dados físicos fossem feitos depois.

## Objetivo da próxima sessão

Completar e documentar o ciclo **sensor físico → coleta CSV → Edge Impulse →
treino → teste reservado → biblioteca int8 → ESP32 → motores**. A avaliação
exige medições obtidas pelo grupo. O modelo atual é uma demonstração sintética
e não conclui essa exigência.

## Onde trabalhar

- Repositório: <https://github.com/pebarone/cp2-carrinho-tinyml>.
- Workspace original no Windows: `C:\Users\augus\Desktop\CP5_MAKER`.
- Edge Impulse sintético, privado: <https://studio.edgeimpulse.com/studio/1139087>.
- Wokwi: <https://wokwi.com/projects/477821599079365633>.
- Commit de referência antes deste handoff: `27dc221`.
- Vídeo solicitado para 14/10/2026; demonstração presencial em 21/10/2026.

Leia `README.md`, `CONTRIBUTING.md`, possíveis `AGENTS.md` e estes guias:
[hardware](hardware.md), [coleta](coleta.md), [treinamento](treinamento.md) e
[entrega](entrega.md). Verifique `git status` antes de editar e preserve mudanças
do usuário. Confira o diretório atual; o caminho original pode mudar em um clone.

O usuário prefere organização por responsabilidade, ação autônoma e perguntas
somente para informações realmente ausentes. Na sessão anterior autorizou
delegação para GPT-6 Luna em esforço xhigh. Se essa capacidade estiver disponível
e for útil, separe tarefas independentes; um único agente deve operar cada
sessão do navegador e cada compilação. Não execute builds concorrentes no mesmo
diretório de saída. Nunca salve senhas ou tokens no Git.

## O que existe e foi verificado

| Item | Estado ao encerrar a etapa virtual |
| --- | --- |
| Hardware escolhido | ESP32 DevKit/WROOM, HC-SR04, L298N, dois motores DC TT, quatro pilhas |
| Firmware CP2 | Modular em `cp2/firmware/carrinho/`, coleta e inferência com `run_classifier` |
| Dados | 30 sessões sintéticas, 600 janelas de dez leituras; 480 treino / 120 teste |
| Modelo | Edge Impulse, Raw Data, escala DSP 0,01, rede densa 20/10, três classes, EON int8 |
| Configuração | 10 Hz, janela e avanço de 1.000 ms; 100 épocas, learning rate 0,005, CPU |
| Métricas sintéticas | 100% em 96 janelas de validação automática e 120 de teste reservado |
| Compilação | Arduino CLI 1.4.1, core ESP32 3.3.11; inferência: 304.915 bytes flash, 24.560 bytes RAM estática |
| Testes locais | Três testes Python e 19 assertions C++ de política passaram; builds CP1, coleta e inferência passaram |
| Teste virtual | `livre` avança, `aproximando` reduz PWM, `bloqueado` para; baixa confiança, emergência e `S` também param |
| Placa física | Nenhuma gravação, coleta ou validação física foi realizada nesta etapa |

As evidências estão em `cp2/evidence/edge-impulse/` e `cp2/evidence/wokwi/`.
A biblioteca sintética está em `cp2/model/edge-impulse-arduino-v1.zip`, com
identificação e hash em `cp2/model/export.json`. Binários, biblioteca expandida,
`model_config.h` e `.venv` são locais e ignorados pelo Git.

`cp1/` é uma referência por limiar reconstruída nesta sessão, não histórico
recuperado do CP1 original. `knowledge_base/` contém uma atividade anterior de
LDR; seus dados e modelo não pertencem ao carrinho. Preserve essas referências.

## 1. Confirmar montagem e conexão

Peça somente o que estiver faltando: placa conectada, porta serial, montagem
atual e disponibilidade do grupo para mover o obstáculo. Não assuma que a conta
Edge Impulse continua conectada: inspecione a sessão no navegador integrado.
Se precisar de login, o usuário deve concluir a autenticação; não peça senha.

Confira o mapa em `config.h`: TRIG 23, ECHO 22, ENA 25, IN1 26, IN2 27,
ENB 33, IN3 32 e IN4 21. O ECHO físico deve passar pelo divisor de tensão
documentado; a ligação direta no diagrama Wokwi é somente virtual. Remova os
jumpers ENA/ENB do L298N para controlar PWM. Confira a alimentação de 5 V da
lógica, o jumper 5V_EN e o terra comum conforme [hardware](hardware.md).
Quatro pilhas e a queda do L298N não garantem alimentação adequada do ESP32;
use a solução regulada documentada. Comece com as rodas suspensas.

Na raiz do projeto, use PowerShell:

```powershell
git status --short
arduino-cli board list
arduino-cli core list
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
powershell -ExecutionPolicy Bypass -File .\cp2\tools\build.ps1 -Mode collection
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM5 --input-dir build/collection cp2/firmware/carrinho
```

**COM5 é exemplo:** substitua pela porta confirmada. O sketch de coleta mantém
os motores desligados e transmite a 115200 baud. Verifique distâncias conhecidas,
intervalos de aproximadamente 100 ms, orientação do sensor e leituras inválidas.
Feche o monitor serial antes da captura: dois programas não devem disputar a COM.

## 2. Coletar sessões reais com rótulos consistentes

Mantenha as classes `livre`, `aproximando` e `bloqueado`, a entrada `distance_cm`
em centímetros e as janelas de dez leituras, salvo mudança justificada e
documentada em firmware, processamento e testes.

- `livre`: obstáculo distante e distância estável, com variação entre sessões.
- `bloqueado`: obstáculo próximo e estável. A faixa de 9–23 cm usada na
  demonstração é um ponto de partida, a ser conferido na montagem física.
- `aproximando`: distância diminuindo durante toda a captura. Uma distância
  estática intermediária não demonstra aproximação temporal.

Registre a definição operacional das classes, materiais, distâncias, operador,
orientação, ambiente e sessão em `cp2/data/real/diario-coleta.md`. Varie condições
entre sessões; não invente medições nem copie os sintéticos como `origin: real`.

```powershell
.\.venv\Scripts\python.exe cp2/tools/dataset.py capture --port COM5 --label livre --seconds 3 --output cp2/data/real/raw/livre-s00.csv
.\.venv\Scripts\python.exe cp2/tools/dataset.py capture --port COM5 --label bloqueado --seconds 3 --output cp2/data/real/raw/bloqueado-s00.csv
.\.venv\Scripts\python.exe cp2/tools/dataset.py capture --port COM5 --label aproximando --seconds 3 --output cp2/data/real/raw/aproximando-s00.csv
```

Execute uma captura por vez, com o cenário preparado e novo identificador
(`s01`, `s02`, ...). O script espera **dois segundos** após abrir a COM e só
então inicia o tempo pedido. Coordene o início do movimento com essa espera.
Para `aproximando`, use uma trajetória que caiba na captura curta; por exemplo,
aproximar de aproximadamente 120 para 60 cm em três segundos, se a montagem
permitir. Não retorne o obstáculo ao início nem o mantenha parado dentro do
mesmo arquivo rotulado como aproximação. Ajuste duração e percurso em conjunto.

O preparador exige pelo menos cinco sessões válidas por classe. Comece com
dez por classe e amplie se o teste mostrar problemas; procure quantidade
semelhante de janelas por classe. Sessões de três segundos geram cerca de três
janelas, dependendo de perdas. O número efetivo deve vir dos CSVs e do manifesto.
Uma sessão longa fatiada não substitui sessões independentes de teste.

Cada coleta cria CSV e JSON com origem, rótulo, quantidade e SHA-256. Preserve
ambos. Se uma sessão contiver movimentos de outra classe ou falhas de montagem,
documente a exclusão e recapture; não ajuste os rótulos para melhorar a métrica.

## 3. Preparar e importar somente a campanha física

```powershell
.\.venv\Scripts\python.exe cp2/tools/dataset.py prepare cp2/data/real/raw cp2/data/real/edge-impulse-v1
.\.venv\Scripts\python.exe -m unittest discover -s cp2/tests -v
```

O destino deve ser novo. **Não use `--allow-simulated` nesta campanha.** Confira
o manifesto, `origin: real`, quantidade por classe, descarte de leituras
inválidas/intervalos irregulares e ausência de sessões compartilhadas entre
`training/` e `testing/`. A separação é determinística, aproximadamente 80/20,
por sessão inteira.

No Edge Impulse, prefira criar um projeto separado para os dados físicos,
preservando o projeto sintético 1139087. Registre o novo ID e URL. Não apague
o projeto/dataset anterior para liberar espaço nem misture sintéticos aos reais
sem uma decisão explícita e uma avaliação física separada.

Configure o CSV Wizard: cabeçalho, vírgula, série temporal com uma leitura por
linha, `timestamp` em milissegundos, somente `distance_cm` como entrada, sem
coluna de rótulo e sem novo fatiamento das janelas. Importe `training/` em
Training e `testing/` em Test, com rótulo pelo prefixo do nome do arquivo.
Não escolha divisão automática no upload: isso perderia o split por sessão.

## 4. Treinar, avaliar e salvar evidências físicas

Reproduza inicialmente o Impulse existente: 1.000 ms / avanço 1.000 ms / 10 Hz,
Raw Data com escala 0,01 e Classification denso 20/10, três saídas, 100 épocas
e learning rate 0,005. A biblioteca aplica a escala; o ESP32 envia centímetros
crus, sem normalizar de novo. Ajuste parâmetros com base no treino/validação.

Gere features, treine e execute Model Testing nas variantes float32 e int8.
O teste reservado deve continuar fora do ajuste de parâmetros. Se for usado
para orientar novas mudanças, faça nova coleta independente para o teste final.
Não prometa reproduzir os 100% sintéticos.

Registre em `cp2/evidence/physical-v1/`: projeto e configuração, dataset usado,
contagens por classe/partição, parâmetros, accuracy, loss, F1, matriz de confusão,
testes de inferência, JSON oficial e prints. A validação automática do Edge
Impulse pode compartilhar sessões com o treino: declare essa limitação e
priorize o teste reservado por sessão. Inspecione especialmente falsos `livre`
em situações bloqueadas. Registre também casos ambíguos e leituras inválidas.

## 5. Exportar o modelo físico e atualizar a integração

Exporte **Arduino library, EON Compiler, int8**. Preserve o ZIP sintético v1;
salve o novo pacote com nome próprio, por exemplo
`cp2/model/edge-impulse-arduino-physical-v1.zip`, e registre SHA-256, ID, origem
real, cabeçalho exportado e vínculo com a campanha física.

**Atenção ao código atual:** `build.ps1` aponta para o ZIP sintético v1 e só
descompacta se a pasta da biblioteca não existir. Trocar apenas um ZIP pode
deixar a compilação usando a biblioteca antiga. Use uma pasta de expansão
própria para o modelo físico e atualize de forma consistente:

- `cp2/tools/build.ps1`: ZIP, pasta, biblioteca e cabeçalho esperados;
- `model_config.example.h` e o `model_config.h` local: inclusão correta;
- `inference.h`: ID esperado do novo projeto e compatibilidade da entrada;
- metadados do modelo e documentação: origem, versão, hash e resultados.

Mantenha as verificações de janela, frequência, três classes e int8. Atualize
o ID esperado com o ID real; não remova as assertions para aceitar um modelo
incompatível. O firmware escolhe ações pelos nomes das classes, não pela ordem.
Se alterar nomes ou significado das classes, atualize política e testes.

```powershell
powershell -ExecutionPolicy Bypass -File .\cp2\tests\test_policy.ps1
powershell -ExecutionPolicy Bypass -File .\cp2\tools\build.ps1 -Mode inference
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM5 --input-dir build/tinyml cp2/firmware/carrinho
```

Confira no build o novo modelo vinculado e registre uso de flash/RAM e hash do
binário. Não grave um binário antigo encontrado em `build/tinyml/` sem verificar
que veio da compilação concluída com o modelo físico.

## 6. Testar a placa e concluir a entrega

Primeiro com rodas suspensas, acompanhe leituras, scores, classe e ação no serial.
O firmware inicia desarmado: `A` arma e `S` para. Verifique `livre` → avanço
(PWM 160), `aproximando` → redução (PWM 110), `bloqueado` → parada, confiança
abaixo de 0,80 → parada, leitura inválida → parada e distância abaixo de 8 cm
→ emergência. Esses são valores atuais, não calibração física comprovada.
Confira o sentido dos dois motores e ajuste PWM se necessário para vencer o
atrito do carrinho, documentando mudanças. Depois teste no chão em área livre.

Grave um vídeo curto mostrando entrada do sensor, inferência na própria ESP32
e mudança perceptível dos motores causada pela classe. Evidencie a redução de
velocidade e a parada classificada antes da emergência: somente um corte por
distância não demonstra a integração TinyML. Guarde logs, resultado esperado
versus observado e condições de alimentação em `cp2/evidence/physical-v1/`.

Atualize README, guias de coleta/treinamento/entrega e metadados sem transformar
as métricas sintéticas históricas em resultados físicos. Faça commits separados
de coleta, treino/exportação e integração/testes; envie ao GitHub. Inclua no
Teams o link do repositório, indicação de `cp2/`, vídeo e nomes/RMs já registrados
em [entrega](entrega.md). Não envie a postagem no Teams sem solicitação do usuário.

## Critério de conclusão do agente

- CSVs reais do grupo, metadados, diário e contagens conferidos;
- split físico por sessão, treinamento e teste int8 documentados;
- biblioteca física exportada, identificada e efetivamente usada no build;
- firmware gravado e inferência física alterando o comportamento dos motores;
- vídeo/logs e documentação atualizados, commits enviados, árvore Git limpa.

Se faltar placa, alimentação, acesso à conta ou participação na coleta, avance
com o trabalho independente e registre precisamente o bloqueio. Não declare
execução física ou resultado de treino sem tê-los observado.

## Prompt para iniciar a próxima sessão

> Leia `cp2/docs/HANDOFF_TREINAMENTO_FISICO.md` e continue a etapa física do
> carrinho ESP32 + HC-SR04 + L298N deste repositório. Preserve os resultados
> sintéticos existentes, colete dados reais com o grupo, treine e teste no Edge
> Impulse, integre a biblioteca física, grave a ESP32 e valide as ações dos
> motores. Documente evidências, faça commits por etapa e envie ao GitHub.
