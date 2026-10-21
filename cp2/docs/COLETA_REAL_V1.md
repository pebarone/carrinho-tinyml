# Campanha fisica v1

Projeto real criado: [cp2-carrinho-real-v1, 1140184](https://studio.edgeimpulse.com/public/1140184/live). O projeto sintetico 1139087 continua preservado. Estado atual, prints e pendencias: [evidencias](../evidence/real-training-v1/README.md).

## Comportamento vigente

O perfil de inferência arma sozinho 5 segundos após ligar ou RESET. `S` para
e cancela a partida automática; `A` rearma. Não espere o envio de `A` para
que os motores possam se mover. O modo collection mantém os motores desligados.

A política atual, PWM, ré/giro e proteções estão no
[guia de navegação](NAVEGACAO_AUTONOMA.md). O modelo real padrão é float32 v2;
o treinamento permanece o lote real de 35/9 janelas, sem dados novos.
A distância de 20 cm é uma proteção inicial, sem frenagem calibrada no chão.
OUT3/OUT4 tem compensação de sentido no código: não inverta também os fios.

## Antes de coletar

Use USB na ESP32, com sensor em 5V e terra comum. Desconecte a alimentacao
Vs/12V dos motores. Retire ENA, ENB e 5V_EN; a logica da ponte pode receber
5V do USB. Nao alimente motores pelo pino 5V da ESP32.
O usuario montou seis resistores de 330 ohms, com GPIO22 na emenda central
(990 ohms entre ECHO e GPIO22; 990 ohms entre GPIO22 e GND).

Grave o modo collection do sketch carrinho. Ele transmite
timestamp,distance_cm,valid a cada 100 ms; W/S do sketch manual nao se aplicam.
Nesse modo A nao arma e nenhum dado coletado aciona os motores.

Prefira alvos planos a distancias conhecidas. Se nao houver regua ou espaco, registre explicitamente que as distancias sao estimadas e use a captura apenas para verificar estabilidade, sem afirmar erro absoluto. Registre os dados
em uma pasta de calibracao, fora de data/real/raw: calibracao nao e uma
classe de treino. Investigue leituras muito diferentes do alvo e perdas de
ECHO antes da campanha. Feche o Monitor Serial antes de abrir a porta no PC.

## Sessoes rotuladas

Comece com dez capturas independentes de tres segundos por classe:

- livre: distancia estavel acima de 40 cm; varie distancias e objetos.
- bloqueado: distancia estavel entre 10 e 20 cm.
- aproximando: distancia diminuindo continuamente, por exemplo 100 -> 30 cm
  em tres segundos. Varie velocidades e percursos nas demais sessoes.

O trecho parado ou o retorno do objeto ao inicio nao pertence ao arquivo de
aproximacao. O lote inicial não incluiu posições estáticas intermediárias nem afastamento.
Na próxima campanha, inclua posições estáticas de 20–60 cm, trajetórias de
aproximação/afastamento e distâncias maiores, com objetos e ambientes variados. Se o teste reservado mostrar lacunas, amplie a campanha antes de
aprovar o comportamento. Nao trate trinta capturas como garantia de qualidade.

Use dataset.py capture para preservar CSV + JSON, origin real e hash.
Ele espera dois segundos apos abrir a COM antes de iniciar a captura.
Registre operador, material, distancia, ambiente e anomalias no diario.
Use nomes novos para cada coleta, sem sobrescrever sessoes.

Prepare com dataset.py prepare, reservando sessoes inteiras para testing
(aproximadamente 20%). Nao misture dados sinteticos ao lote real. Use projeto
Edge Impulse separado, mantendo janela de 1 s, 10 Hz e apenas distance_cm.
Consulte coleta.md e treinamento.md para os comandos e configuracao inicial.

O modelo real 1140184 ja foi treinado, exportado, compilado e gravado. A primeira verificacao na ESP32 registrou 11 inferencias e 120 leituras validas, desarmada. Em seguida, um teste armado registrou mudanca de livre/PWM 160 para aproximando/PWM 110 e parada; o usuario confirmou ambos os motores reagindo. O teste terminou desarmado. Esse registro é histórico, anterior à navegação atual. Para repetir, use rodas suspensas e pilhas apenas em Vs/12V, com GND comum; A arma e S desarma.
