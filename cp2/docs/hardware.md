# Hardware do carrinho

## Componentes

- ESP32 DevKit com módulo WROOM;
- sensor ultrassônico HC-SR04;
- ponte H L298N;
- dois motores DC TT;
- suporte para quatro pilhas AA;
- cabo USB do PC para alimentar o ESP32; seu pino 5V alimenta sensor e lógica do L298N (powerbank USB para uso sem PC);
- cinco resistores de 330 Ω para reduzir a tensão do pino ECHO.

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
4. Alimente o ESP32 pelo USB do PC e distribua seu pino `5V` para VCC do HC-SR04 e para o terminal `5V` da lógica do L298N, com `5V_EN` removido. Se uma placa tiver somente a inscrição `VIN`, confirme seu esquema antes de usá-lo como saída USB. Não ligue a bateria dos motores ao pino 5V/VIN do ESP32 nem alimente motores por essa linha.
5. Una os GND do ESP32, HC-SR04, L298N, fonte de 5 V e suporte das pilhas. O terra comum é necessário para os sinais terem a mesma referência.
6. Ligue `TRIG` diretamente ao GPIO 23. O ECHO do HC-SR04 pode chegar a 5 V e o ESP32 não é tolerante a essa tensão: conecte **ECHO → dois resistores de 330 Ω em série → nó/GPIO 22** e ligue **três resistores de 330 Ω em série desse nó ao GND**. O divisor de 660 Ω / 990 Ω reduz 5 V para aproximadamente 3,0 V. Os resistores não têm polaridade; isole as emendas. Veja o desenho e a ordem de montagem em [LIGACAO_PLACA_FISICA.md](LIGACAO_PLACA_FISICA.md).

Confira as inscrições e o esquema do módulo L298N específico antes de aplicar energia; módulos visualmente parecidos podem variar. Evite curtos entre os terminais das pilhas e mantenha o carrinho suspenso durante os primeiros testes de direção.

Para usar somente as quatro pilhas, a linha de lógica precisa de um conversor
regulado de 5 V apropriado à tensão das pilhas; com NiMH, pode ser necessário
elevar a tensão. O powerbank é a alternativa documentada para os primeiros
testes. A separação entre alimentação dos motores e lógica, e a queda na ponte,
constam no [datasheet L298 da ST](https://www.st.com/resource/en/datasheet/l298.pdf).
Os limites elétricos do microcontrolador constam no
[datasheet ESP32 da Espressif](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf).

## Parâmetros do firmware

O firmware atual inicia autonomamente após 5 segundos, usa o modelo real float32 para avançar/reduzir e permite avanço cauteloso em baixa confiança e executa ré curta/giro ao medir 20 cm ou menos, independentemente da classe. `S` para e cancela a partida; `A` rearma. A distância de 20 cm bloqueia avanço, mas permite a manobra de escape; leitura inválida cancela qualquer movimento. Consulte [navegação autônoma](NAVEGACAO_AUTONOMA.md) para tempos, ações, instalação no Arduino IDE e limites. Os ensaios Wokwi abaixo são históricos, anteriores à navegação autônoma.

O cenário de coleta no Wokwi verificou o sinal do sensor: com alvo configurado em 100 cm, o firmware reportou leituras válidas próximas de 100,8 cm a cada 100 ms. Na execução manual de inferência, foram verificados `livre` a 100 cm, `bloqueado` a 15 cm, a transição `aproximando` de 80,7 para 40,3 cm, parada segura para uma leitura ambígua a 40,3 cm e emergência a 5 cm mesmo após ação de movimento. Os registros estão em `cp2/evidence/wokwi/`. O diagrama liga o ECHO diretamente ao ESP32 e representa os motores com LEDs de PWM; essa ligação direta é somente para a simulação. Na montagem real, use o divisor descrito acima e o L298N com alimentação apropriada. A simulação não valida a montagem física.
