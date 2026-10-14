# Ligação da placa física

Mapa correspondente ao firmware `cp2/firmware/carrinho/`. Os números abaixo
são **GPIOs escritos na placa**, não a posição do pino no conector.

## Sensor HC-SR04

| Pino do sensor | Ligação |
| --- | --- |
| VCC | Linha regulada de 5 V |
| GND | GND comum |
| TRIG | GPIO 23 da ESP32 |
| ECHO | GPIO 22 através do divisor abaixo |

```text
HC-SR04 ECHO ── resistor 1 kΩ ──┬── GPIO 22
                              │
                         resistor 2 kΩ
                              │
                             GND
```

Os resistores não têm polaridade. Não conecte ECHO de 5 V diretamente ao GPIO.
Se não tiver os resistores, monte o restante e deixe ECHO desconectado até
instalar o divisor ou um conversor de nível apropriado.

## Ponte H L298N

| Pino/terminal do módulo | Ligação |
| --- | --- |
| ENA | GPIO 25 da ESP32; retirar jumper ENA |
| IN1 | GPIO 26 |
| IN2 | GPIO 27 |
| IN3 | GPIO 32 |
| IN4 | GPIO 21 |
| ENB | GPIO 33 da ESP32; retirar jumper ENB |
| OUT1 e OUT2 | Os dois fios do motor A |
| OUT3 e OUT4 | Os dois fios do motor B |
| Vs / terminal marcado 12V | Positivo do suporte de quatro pilhas |
| GND | Negativo das pilhas e GND comum |
| 5V da lógica | Linha regulada de 5 V, com jumper 5V_EN retirado |

O nome `12V` identifica o terminal de alimentação dos motores; não significa
que é necessário aplicar 12 V neste carrinho. Use as quatro pilhas informadas,
conferindo a especificação dos motores. O L298N perde tensão na ponte; o PWM
e a capacidade de partir com carga ainda precisam ser ajustados na montagem.

## Alimentação durante o teste no PC

1. ESP32 alimentada pelo cabo USB do PC.
2. Pilhas alimentam somente o estágio dos motores pelo terminal Vs da L298N.
3. Sensor e lógica da L298N recebem 5 V regulados. A linha de 5 V USB da placa
   pode ser usada se disponibilizada no seu conector: confira o pino marcado
   `5V`/`V5` ou a documentação do `VIN` antes de utilizá-lo. Não use `3V3`
   para a lógica da L298N e não aplique as quatro pilhas ao GPIO ou à linha de 5 V.
4. Una GND da ESP32, sensor, ponte, fonte de 5 V e negativo das pilhas.
5. Retire também o jumper de regulador `5V_EN` da L298N para usar os 5 V externos.
   Não confunda esse jumper com ENA/ENB e confirme as inscrições do módulo.

Faça as ligações com USB e pilhas desconectados. Depois confira a montagem,
conecte USB, teste o sensor e só então alimente os motores com rodas suspensas.
Para uso sem PC, forneça 5 V regulados por fonte/conversor apropriado; as pilhas
não substituem diretamente a alimentação USB. Consulte os detalhes e fontes
em [hardware.md](hardware.md).

## Funcionamento do firmware gravado

O modo de inferência inicia **desarmado**. Pelo monitor serial da COM da placa,
a 115200 baud, envie `A` para armar e `S` para parar/desarmar. Leitura inválida,
distância abaixo de 8 cm ou confiança abaixo de 0,80 impedem movimento.

| Classe com confiança suficiente | Ação |
| --- | --- |
| `livre` | Avançar, PWM 160 |
| `aproximando` | Reduzir, PWM 110 |
| `bloqueado` | Parar |

Se um motor girar ao contrário, desarme, retire alimentação e inverta os dois
fios desse motor. Nunca ligue um motor diretamente a um GPIO ou ao pino 5V da ESP32.
Somente após conferir sentido e leitura do sensor faça o teste no chão.

O modelo disponível é o Edge Impulse int8 treinado com dados sintéticos. O
firmware opera no hardware físico, mas o treinamento com medições reais do
grupo continua pendente; siga o [handoff](HANDOFF_TREINAMENTO_FISICO.md).

## Hotspot e consulta pelo navegador

O firmware pode conectar-se ao hotspot do celular, configurado para **2,4 GHz**.
Copie `wifi_config.example.h` para `wifi_config.h` e preencha SSID/senha somente
no arquivo local, ignorado pelo Git. A configuração fornecida pelo usuário foi
aplicada localmente; ela não faz parte do código publicado.

A conexão acontece em segundo plano e tenta reconectar a cada 30 segundos.
O sensor e a inferência continuam funcionando com o hotspot desligado. Quando
conectar, o serial mostrará `# wifi,connected,http://<IP>`.
Abra esse endereço, ou `http://<IP>/state`, no navegador de um aparelho com
acesso à mesma rede. A resposta JSON mostra distância, validade do sensor,
classe, confiança, armação e ação atual: 0 = parar, 1 = reduzir, 2 = avançar.
Atualize a página para obter nova leitura. O acesso depende também de o hotspot
permitir comunicação com os dispositivos conectados.

A consulta é somente leitura; não arma motores pela rede. Continue usando `A`
e `S` pelo serial. Os pedidos HTTP rodam em uma tarefa separada para não bloquear
o ciclo de leitura/parada. Não há envio automático de dados ao Edge Impulse:
os CSVs são coletados pela ferramenta e importados no site para treinar/exportar
o modelo, conforme o fluxo da tarefa.
