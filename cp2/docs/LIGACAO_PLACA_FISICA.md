# Ligação da placa física

Mapa correspondente ao firmware `cp2/firmware/carrinho/`. Os números abaixo
são **GPIOs escritos na placa**, não a posição do pino no conector.

Veja também [o guia HTML interativo](montagem.html) e
[a montagem ilustrada no Wokwi](https://wokwi.com/projects/477883916754720769).

## Sensor HC-SR04

| Pino do sensor | Ligação |
| --- | --- |
| VCC | Pino 5V da ESP32 alimentada pelo USB |
| GND | GND comum |
| TRIG | GPIO 23 da ESP32 |
| ECHO | GPIO 22 através do divisor abaixo |

```text
HC-SR04 ECHO ── 330 Ω ── 330 Ω ──┬── GPIO 22
                               │
                             330 Ω
                               │
                             330 Ω
                               │
                             330 Ω
                               │
                              GND
```

Use cinco resistores de 330 Ω: dois em série entre ECHO e o ponto ligado ao
GPIO 22; três em série entre esse mesmo ponto e GND. Os resistores não têm
polaridade. O divisor tem 660 Ω acima e 990 Ω abaixo: 5 V × 990 / (660 + 990)
= 3,0 V nominais no GPIO. Isole as emendas para evitar contato entre terminais.
Não conecte ECHO de 5 V diretamente ao GPIO; alimentá-lo pelo pino 5V da ESP32
não torna o sinal ECHO compatível com os GPIOs de 3,3 V.

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
| 5V da lógica | Pino 5V da ESP32 alimentada pelo USB, com jumper 5V_EN retirado |

O nome `12V` identifica o terminal de alimentação dos motores; não significa
que é necessário aplicar 12 V neste carrinho. Use as quatro pilhas informadas,
conferindo a especificação dos motores. O L298N perde tensão na ponte; o PWM
e a capacidade de partir com carga ainda precisam ser ajustados na montagem.

## Alimentação durante o teste no PC

1. ESP32 alimentada pelo cabo USB do PC.
2. Pilhas alimentam somente o estágio dos motores pelo terminal Vs da L298N.
3. O pino `5V` da ESP32 alimentada pelo USB fornece a linha de alimentação para
   VCC do sensor e o terminal `5V` da lógica da L298N. Essa montagem dispensa
   uma fonte de 5 V separada durante o teste no PC. Se outra placa usar apenas
   a inscrição `VIN`, confira seu esquema antes de tratá-lo como saída USB.
   Não use `3V3`
   para a lógica da L298N e não aplique as quatro pilhas ao GPIO ou à linha de 5 V.
4. Una GND da ESP32, sensor, ponte, fonte de 5 V e negativo das pilhas.
5. Retire também o jumper de regulador `5V_EN` da L298N para usar os 5 V externos.
   Não confunda esse jumper com ENA/ENB e confirme as inscrições do módulo.

## Ordem de montagem

1. Desconecte USB e retire as pilhas do suporte. Remova os jumpers `ENA`, `ENB`
   e `5V_EN` da L298N; identifique cada um pela inscrição do módulo.
2. Ligue GND da ESP32 ao GND do sensor, ao GND da L298N e ao negativo do suporte.
3. Distribua o pino 5V da ESP32 para VCC do sensor e para o terminal 5V da L298N.
4. Ligue TRIG ao GPIO 23. Monte o divisor de cinco resistores de 330 Ω conforme
   o desenho e ligue seu ponto central ao GPIO 22.
5. Ligue os seis sinais da ponte: ENA → 25, IN1 → 26, IN2 → 27,
   IN3 → 32, IN4 → 21 e ENB → 33.
6. Ligue um motor entre OUT1 e OUT2, e o outro entre OUT3 e OUT4.
   Ligue o positivo do suporte ao terminal Vs/12V; mantenha o suporte sem pilhas.
7. Confira as conexões, conecte somente o USB e abra o monitor serial em
   115200 baud. Com os motores desarmados, confira as distâncias do sensor.
8. Suspenda as rodas, coloque as pilhas e envie `A` pelo serial para testar.
   Envie `S` para parar. Se precisar inverter os fios de um motor, desligue
   USB e pilhas antes de mexer na montagem.

Para uso sem PC, forneça 5 V regulados por fonte/conversor apropriado; as pilhas
não substituem diretamente a alimentação USB. Um powerbank no conector USB
mantém a mesma distribuição de 5 V. Consulte os detalhes e fontes
em [hardware.md](hardware.md).

## Funcionamento do firmware gravado

O perfil de inferência atual usa o modelo real float32 1140184 e arma
**automaticamente após 5 segundos**. Pelo serial a 115200 baud, `S` para,
desarma e cancela a partida; `A` rearma. Trocar para powerbank ou pressionar
RESET reinicia a contagem. Não é necessário PC ou Wi-Fi para navegar.

O modo collection mantém os motores desligados. Para conferir o sensor sem
movimento no perfil de inferência, envie `S` antes de energizar Vs. A política
vigente, PWM e sequência de ré/giro estão em
[NAVEGACAO_AUTONOMA.md](NAVEGACAO_AUTONOMA.md). A saída OUT3/OUT4 já tem
compensação de sentido no código; não inverta também seus fios.

A exportação float32 atual requer nova gravação. Os ensaios de bancada com
int8 são históricos e não validam o deslocamento no chão desta revisão.

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
classe, confiança, armação e ação atual: 0 = parar, 1 = reduzir, 2 = avançar,
3 = ré e 4 = giro à direita.
Atualize a página para obter nova leitura. O acesso depende também de o hotspot
permitir comunicação com os dispositivos conectados.

A consulta é somente leitura; não arma motores pela rede. Continue usando `A`
e `S` pelo serial. Os pedidos HTTP rodam em uma tarefa separada para não bloquear
o ciclo de leitura/parada. Não há envio automático de dados ao Edge Impulse:
os CSVs são coletados pela ferramenta e importados no site para treinar/exportar
o modelo, conforme o fluxo da tarefa.
