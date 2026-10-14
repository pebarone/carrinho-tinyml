# Simulação do carrinho TinyML

Para visualizar a montagem física completa, incluindo resistores, ponte H,
motores, pilhas e powerbank, abra [a montagem ilustrada](assembly/README.md)
e [o guia HTML](../docs/montagem.html). Ela está em um projeto Wokwi separado:
o divisor não foi validado no motor de simulação analógica, e o comportamento
dinâmico dos chips customizados também não foi validado. O projeto original
abaixo mantém o diagrama digital usado nos testes anteriores.

**Estado atual:** coleta e compilação TinyML foram verificadas. Em execução manual no navegador Wokwi, o firmware classificou 100 cm como `livre` (0,99609; ação 2; PWM 160), 15 cm como `bloqueado` (0,99609; ação 0), e a transição de 80,7 para 40,3 cm como `aproximando` (0,99609; ação 1; PWM 110). Uma leitura estática ambígua a 40,3 cm ficou abaixo da confiança mínima 0,80 e acionou a parada segura. A emergência a 5 cm manteve os sinais dos motores desligados mesmo após uma leitura de movimento. As inferências levaram 2–3 ms. Os LEDs representam ENA/ENB simulados; nada disso valida o carrinho físico ou aprova os cenários CLI.

O diagrama usa o ESP32 DevKit-C V4 e o HC-SR04 com TRIG no GPIO 23 e ECHO no GPIO 22. A distância inicial é 100 cm. Os nomes de pino numéricos seguem o [modelo do DevKit-C V4 no Wokwi](https://github.com/wokwi/wokwi-boards/blob/main/boards/esp32-devkit-c-v4/board.json). O analisador lógico registra TRIG, ECHO e os seis sinais que iriam para o L298N: ENA/GPIO 25, IN1/GPIO 26, IN2/GPIO 27, ENB/GPIO 33, IN3/GPIO 32 e IN4/GPIO 21.

O Wokwi não simula o carro mecânico nem o módulo L298N. Os dois LEDs identificados como `Motor A PWM proxy` e `Motor B PWM proxy` estão ligados aos sinais ENA/ENB e mostram visualmente o PWM de habilitação. Eles são apenas indicadores dos sinais de controle; não representam velocidade, torque ou movimento das rodas.

O [HC-SR04 do Wokwi](https://docs.wokwi.com/parts/wokwi-hc-sr04) pode ser ajustado no controle de distância do sensor enquanto a simulação interativa está rodando. O cenário `approach` tenta automatizar essa mudança, mas o HC-SR04 ainda não aparece na [lista oficial de controles dos cenários](https://docs.wokwi.com/wokwi-ci/automation-scenarios). Se o CLI rejeitar `set-control`, abra a simulação no VS Code, arme com `A` e mova o controle do sensor gradualmente de 120 cm para 15 cm. Use `S` para desarmar.

## Compilar

Execute os comandos a partir de `cp2/wokwi`:

```powershell
powershell -ExecutionPolicy Bypass -File ..\tools\build.ps1 -Mode collection
powershell -ExecutionPolicy Bypass -File ..\tools\build.ps1 -Mode inference
```

O modo `inference` usa a biblioteca Arduino versionada em `cp2/model/edge-impulse-arduino-v1.zip`, extraída localmente em `cp2/model/library/`; o script também cria `model_config.h` a partir do exemplo se o arquivo local ainda não existir. A biblioteca não é instalada no diretório global do Arduino. Os binários ficam em `build/collection/` e `build/tinyml/`. O `wokwi.toml` aponta para `build/tinyml`; `wokwi.collection.toml` é a configuração alternativa para coleta no simulador do VS Code.

## Execução TinyML no navegador

O [projeto Wokwi público](https://wokwi.com/projects/477821599079365633) mantém a coleta como firmware padrão. Para testar TinyML, compile `inference` e, no projeto aberto no navegador, pressione **F1 → Upload Firmware** e selecione `build/tinyml/carrinho.ino.merged.bin`. Abra o monitor serial a 115200 baud, envie `A` e ajuste a distância do HC-SR04.

As capturas `cp2/evidence/wokwi/02-free.jpg` e `free-serial.txt` registram o teste manual a **100 cm** (`livre`, confiança 0,99609, ação 2, PWM 160). `03-blocked.jpg` e `blocked-serial.txt` registram **15 cm** (`bloqueado`, confiança 0,99609, ação 0). `06-approach.jpg` e `approach-transition-serial.txt` registram a transição **80,7 → 40,3 cm** (`aproximando`, 0,99609, ação 1, PWM 110). Para uma leitura estática a 40,3 cm, `05-low-confidence.jpg` e `approach-serial.txt` registram scores ambíguos (máximo 0,53516) e ação 0 por confiança menor que 0,80. `04-emergency.jpg` e `emergency-after-motion-serial.txt` confirmam que a 5 cm os sinais continuam desligados mesmo após surgir uma ação de movimento. Os testes foram feitos manualmente no navegador. Os cenários CLI permanecem experimentais e não verificados; em particular, o cenário fixo `free` começa em 120 cm, diferente do teste manual a 100 cm.

`emergency-serial.txt` registra também a execução do cenário de emergência. `07-disarmed.jpg` e `disarmed-serial.txt` registram que o comando `S` emitiu `# disarmed` e deixou a simulação parada em estado seguro. O resultado de aproximação acima é uma transição ajustada manualmente no navegador, não uma aprovação da automação `approach` do CLI.

## Executar cenarios

Instale o Wokwi CLI pelo [guia oficial](https://docs.wokwi.com/wokwi-ci/cli-installation) e configure um token de CI em `WOKWI_CLI_TOKEN`. A licença da extensão Wokwi do VS Code não é um token do CLI.

```powershell
powershell -ExecutionPolicy Bypass -File ..\tools\simulate.ps1 -Mode collection -Scenario collection
powershell -ExecutionPolicy Bypass -File ..\tools\simulate.ps1 -Mode inference -Scenario blocked
powershell -ExecutionPolicy Bypass -File ..\tools\simulate.ps1 -Mode inference -Scenario free
powershell -ExecutionPolicy Bypass -File ..\tools\simulate.ps1 -Mode inference -Scenario emergency
powershell -ExecutionPolicy Bypass -File ..\tools\simulate.ps1 -Mode inference -Scenario approach
```

Os cenários fixos iniciam o HC-SR04 a 100 cm (coleta), 15 cm (bloqueado), 120 cm (livre) ou 5 cm (emergência). `A` arma a inferência e `S` para os motores. O cenário de aproximação altera a distância ao longo da execução se a versão local do Wokwi aceitar o controle `distance`.

O script salva o log serial e a captura VCD em `build/wokwi/`. Um VCD contém os sinais digitais registrados pelo [analisador lógico Wokwi](https://docs.wokwi.com/guides/logic-analyzer) e pode ser aberto no PulseView ou GTKWave. As execuções CLI não foram validadas como resultados; a aproximação automatizada depende de suporte do Wokwi ao controle de distância.

## Limites da simulação e fiação real

O diagrama liga ECHO diretamente ao ESP32 para reproduzir o sinal digital no simulador. Em uma montagem física, o ECHO do HC-SR04 pode sair em 5 V; use divisor resistivo ou conversor de nível para limitar esse sinal a 3,3 V no ESP32. A montagem real também precisa do L298N, motores, fonte apropriada e terra comum.
