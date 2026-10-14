# Projeto de captura Wokwi

`diagram.json` conecta o módulo `wokwi-photoresistor-sensor` ao ESP32 DevKit-C V4: AO em VP/GPIO36, VCC em 3V3 e GND em GND.1. A configuração usa `rl10=15` kΩ e `gamma=0.6`.

O módulo Wokwi reduz AO quando a iluminância aumenta. Por isso, o firmware usa `feature = 1 - ADC_médio/4095`, com janela de 10 amostras, uma leitura a cada 5 ms e limiar 0,45. Assim, label 0 é escuro e label 1 é claro.

Compile com `powershell -ExecutionPolicy Bypass -File build.ps1`. O firmware e o ELF são gravados em `.wokwi-firmware/`, conforme `wokwi.toml`; os demais arquivos intermediários não são mantidos no projeto. Com o Wokwi CLI instalado e `WOKWI_CLI_TOKEN` configurado fora dos arquivos do projeto, execute a partir desta pasta:

```powershell
wokwi-cli . --timeout 15000 --scenario scenario.yaml --serial-log-file serial.log
```

O cenário mantém 1 lux por 5 s e depois 1000 lux por 5 s. Os logs de inicialização do ESP32 são ignorados pelo parser. Após a simulação:

```powershell
python ../processar_captura.py serial.log
```

O parser só substitui os CSVs sintéticos por dados derivados do log se todas as janelas estiverem consistentes e houver exemplos das duas classes. Os valores resultantes são dados da simulação Wokwi, não medições de placa física.
