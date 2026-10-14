# Montagem física ilustrada

Abra [a montagem no Wokwi](https://wokwi.com/projects/477883916754720769)
ou [o guia HTML interativo](../../docs/montagem.html).

Este diagrama inclui HC-SR04, cinco resistores de 330 Ω, ESP32, os terminais
da L298N, dois motores e as duas alimentações. `supply` representa uma fonte
nominal com terminais passivos; a ligação PLUS do powerbank representa o cabo
USB/linha de 5 V, não uma conexão das pilhas à ESP32. Siga o mapa físico no HTML.

## Componentes personalizados

- `l298n.chip.*`: terminais da ponte e aproximação digital de enable/direção.
  Não calcula alimentação, corrente, perda de tensão, frenagem ou proteção.
- `motor.chip.*`: indicador de direção/atividade digital, atualizado a 10 Hz
  para evitar redesenhar a cada pulso PWM. Não mede velocidade, torque ou inércia.
- `supply.chip.*`: terminais visuais de fonte; não gera uma tensão simulada.

Os pinos e arquivos JSON foram conferidos. A execução com o divisor resistivo
produziu leituras inválidas no navegador Wokwi. A compilação dos chips customizados
também registrou falha de acesso ao serviço de compilação; seu comportamento
dinâmico não foi validado. Portanto, este projeto serve para visualizar a
montagem e não é uma evidência de inferência ou funcionamento elétrico.
O suporte analógico de resistores é limitado, conforme a
[documentação Wokwi](https://docs.wokwi.com/parts/wokwi-resistor).

O [projeto original de teste](https://wokwi.com/projects/477821599079365633)
mantém o diagrama digital já utilizado nas evidências TinyML. Seu ECHO direto
é exclusivo do simulador; na montagem real use o divisor de tensão.

## Alimentação sem fio e armação

O desenho usa powerbank USB para a ESP32 e quatro pilhas para os motores.
Para alimentar tudo com as quatro pilhas, falta um conversor regulado de 5 V
adequado à faixa de tensão delas. Os positivos ficam separados e os GNDs são comuns.

O firmware físico inicia desarmado e hoje aceita `A`/`S` apenas pela serial.
Trocar o PC por powerbank reinicia a placa. Antes de operar sem PC, é necessário
adicionar uma forma de armação/parada no carrinho ou pela rede; a consulta HTTP
atual é somente leitura. Esse guia não altera o firmware gravado.
