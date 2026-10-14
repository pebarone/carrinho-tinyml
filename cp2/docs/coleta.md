# Coleta e preparação de dados

## Protocolo serial

`cp2/firmware/carrinho/config.h` define `COLLECTION_MODE` como `1` por padrão. Nesse modo, os motores ficam desligados e o ESP32 envia a 115200 baud o cabeçalho `timestamp,distance_cm,valid`, seguido de uma linha a cada 100 ms. `timestamp` é o tempo em milissegundos desde a inicialização; `valid` vale `1` para uma distância de 2 a 400 cm e `0` para uma leitura inválida.

## Capturar uma sessão física

Instale as dependências (`python -m pip install -r requirements.txt`), grave o firmware de coleta e conecte o ESP32. Faça uma captura separada por classe. O exemplo abaixo captura 30 segundos em uma porta COM:

```powershell
python cp2/tools/dataset.py capture --port COM5 --label livre --seconds 30 --output cp2/data/real/raw/livre-s00.csv
```

Troque `COM5`, o rótulo e o identificador de sessão conforme necessário. Os rótulos aceitos são `livre`, `aproximando` e `bloqueado`. Para `aproximando`, mova o obstáculo em direção ao sensor de forma contínua; para `bloqueado`, mantenha um obstáculo próximo e estável; para `livre`, mantenha o campo sem obstáculos. Registre distância aproximada, arranjo, sessão e qualquer anomalia no diário da coleta. Repita em ao menos cinco sessões independentes por rótulo.

A ferramenta cria ao lado do CSV um JSON com rótulo, origem, contagem de medições e hash SHA-256. Ela não sobrescreve uma sessão. Selecione um novo nome para cada tentativa e mantenha o CSV e seu JSON juntos.

## Preparar janelas para Edge Impulse

```powershell
python cp2/tools/dataset.py prepare cp2/data/real/raw cp2/data/real/edge-impulse
```

O destino precisa ainda não existir. O parser verifica os metadados e o hash, descarta trechos interrompidos por leituras inválidas ou intervalos fora de 80–120 ms e agrupa leituras válidas em janelas contíguas, sem sobreposição, de 10 pontos. Cada CSV de janela contém `timestamp,distance_cm`; `distance_cm` é a única entrada do classificador. A ferramenta exige pelo menos cinco sessões por classe e separa sessões inteiras: cerca de 80% para `training/` e 20% para `testing/`. Janelas da mesma sessão nunca atravessam as duas partições.

Importe `training/` e `testing/` nas áreas correspondentes do Edge Impulse, mantendo o nome de classe e a origem associados. O `testing/` gerado pelo script é a partição reservada, com sessões inteiras fora do treino. Depois de alterar arquivos, gere uma nova pasta de destino para preservar o conjunto anterior.

O Edge Impulse também reserva automaticamente 20% das janelas importadas em `training/` para validação. Essa validação não é separada por sessão: janelas da mesma sessão podem aparecer tanto no treino usado para ajustar o modelo quanto na validação. Trate-a como acompanhamento do treino, não como avaliação independente. Para a comparação mais forte disponível, use o teste reservado `testing/`, separado por sessão. Registre a origem do conjunto; no estado atual, os dois conjuntos presentes são sintéticos.

## Conjunto sintético já presente

`cp2/data/synthetic/raw/` contém 30 sessões artificiais. `cp2/data/synthetic/edge-impulse/` contém 600 janelas artificiais, com 10 medições por janela: 480 em `training/` e 120 em `testing/`. A separação é por sessão. O manifesto marca todas as janelas como `origin: synthetic`.

Esse material serve para exercitar parser, formato e fluxo de importação. Não descreve medições do HC-SR04 do grupo e não deve fundamentar uma alegação de desempenho real. O conjunto reservado de 120 janelas está separado por sessão e fornece um teste independente do treino dentro desse material sintético. A preparação bloqueia dados simulados por padrão. Para testar o fluxo local deliberadamente, use uma pasta nova e a opção `--allow-simulated`; mantenha a origem sintética explícita. Para a entrega acadêmica, ainda é necessário coletar dados físicos e repetir a avaliação mantendo sessões inteiras no teste.
