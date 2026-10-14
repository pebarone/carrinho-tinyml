# Hardware do carrinho

## Componentes

- ESP32 DevKit com módulo WROOM;
- sensor ultrassônico HC-SR04;
- ponte H L298N;
- dois motores DC TT;
- suporte para quatro pilhas AA;
- fonte regulada de 5 V, como um powerbank, para o ESP32, o sensor e a lógica do L298N;
- resistores de 1 kΩ e 2 kΩ para reduzir a tensão do pino ECHO.

## Mapa de pinos

O mapa abaixo corresponde a `cp2/firmware/carrinho/config.h`.

| Sinal | ESP32 |
| --- | --- |
| HC-SR04 TRIG | GPIO 23 |
| HC-SR04 ECHO, após divisor | GPIO 22 |
| L298N ENA (PWM motor A) | GPIO 25 |
| L298N IN1 / IN2 | GPIO 26 / GPIO 27 |
| L298N ENB (PWM motor B) | GPIO 33 |
| L298N IN3 / IN4 | GPIO 32 / GPIO 21 |

Os motores conectam-se às saídas OUT1/OUT2 e OUT3/OUT4 do L298N. Se um lado girar no sentido oposto, inverta os dois fios daquele motor ou ajuste a lógica de direção depois de confirmar a montagem.

## Alimentação e ligações

1. Ligue as quatro pilhas ao terminal de alimentação dos motores (`Vs`/`12V`) e ao GND do L298N. Quatro alcalinas fornecem cerca de 6 V nominais; quatro NiMH fornecem cerca de 4,8 V nominais. A tensão cai com a descarga e o L298N perde parte da tensão na ponte, então o motor pode receber bem menos que a tensão do suporte. Verifique a especificação dos motores e teste a rotação com as rodas suspensas.
2. Remova os jumpers `ENA` e `ENB` do módulo L298N: os GPIOs 25 e 33 passam a controlar esses pinos por PWM.
3. Como `Vs` baixo não garante 5 V estáveis no regulador de algumas placas L298N, retire o jumper `5V_EN` quando alimentar o motor com quatro AA. Aplique 5 V regulados ao terminal de lógica `5V` do módulo, conforme a identificação da sua placa. Não alimente esse terminal ao mesmo tempo pelo regulador integrado.
4. Alimente o ESP32 pela saída USB de 5 V e ligue o VCC do HC-SR04 à linha regulada de 5 V. Use uma saída/linha regulada apropriada também para a lógica do L298N. Não ligue a bateria dos motores ao pino 5V/VIN do ESP32.
5. Una os GND do ESP32, HC-SR04, L298N, fonte de 5 V e suporte das pilhas. O terra comum é necessário para os sinais terem a mesma referência.
6. Ligue `TRIG` diretamente ao GPIO 23. O ECHO do HC-SR04 pode chegar a 5 V e o ESP32 não é tolerante a essa tensão: conecte **ECHO → resistor de 1 kΩ → nó/GPIO 22** e ligue **resistor de 2 kΩ desse nó ao GND**. O divisor reduz 5 V para aproximadamente 3,3 V.

Confira as inscrições e o esquema do módulo L298N específico antes de aplicar energia; módulos visualmente parecidos podem variar. Evite curtos entre os terminais das pilhas e mantenha o carrinho suspenso durante os primeiros testes de direção.

Para usar somente as quatro pilhas, a linha de lógica precisa de um conversor
regulado de 5 V apropriado à tensão das pilhas; com NiMH, pode ser necessário
elevar a tensão. O powerbank é a alternativa documentada para os primeiros
testes. A separação entre alimentação dos motores e lógica, e a queda na ponte,
constam no [datasheet L298 da ST](https://www.st.com/resource/en/datasheet/l298.pdf).
Os limites elétricos do microcontrolador constam no
[datasheet ESP32 da Espressif](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf).

## Parâmetros do firmware

O firmware está dividido em `carrinho.ino` (ciclo principal), `ultrasonic.h` (sensor), `motors.h` (PWM e ponte H), `inference.h` (janela e Edge Impulse), `policy.h` (decisão segura) e `config.h` (pinos e parâmetros). A amostragem ocorre a cada 100 ms (10 Hz), em janelas de 10 leituras. Em inferência, `livre` permite avanço, `aproximando` reduz o PWM e `bloqueado` mantém os motores parados. Leitura inválida ou distância abaixo de 8 cm também para os motores. O firmware inicia desarmado; `A` arma e `S` desarma.

O cenário de coleta no Wokwi verificou o sinal do sensor: com alvo configurado em 100 cm, o firmware reportou leituras válidas próximas de 100,8 cm a cada 100 ms. Na execução manual de inferência, foram verificados `livre` a 100 cm, `bloqueado` a 15 cm, a transição `aproximando` de 80,7 para 40,3 cm, parada segura para uma leitura ambígua a 40,3 cm e emergência a 5 cm mesmo após ação de movimento. Os registros estão em `cp2/evidence/wokwi/`. O diagrama liga o ECHO diretamente ao ESP32 e representa os motores com LEDs de PWM; essa ligação direta é somente para a simulação. Na montagem real, use o divisor descrito acima e o L298N com alimentação apropriada. A simulação não valida a montagem física.
