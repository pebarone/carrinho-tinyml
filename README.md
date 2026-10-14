# Carrinho TinyML — CP1 → CP2

Projeto acadêmico de carrinho com ESP32 WROOM, HC-SR04, ponte H L298N e dois motores TT. O repositório contém uma referência CP1 baseada em limiares e a versão CP2 com classificação TinyML. `knowledge_base/` preserva uma atividade anterior de LDR, separada deste carrinho.

Projeto e demonstração: [repositório GitHub](https://github.com/pebarone/cp2-carrinho-tinyml) · [simulação Wokwi](https://wokwi.com/projects/477821599079365633) · [Edge Impulse, projeto 1139087](https://studio.edgeimpulse.com/studio/1139087).

## CP1: referência por limiar

O sketch em `cp1/firmware/carrinho_threshold/` usa a distância diretamente: acima de 40 cm avança; acima de 20 cm até 40 cm reduz a velocidade; em 20 cm ou menos para; abaixo de 8 cm aplica a parada de emergência. Esse firmware foi reconstruído neste workspace como referência comparativa. Não é um arquivo histórico recuperado do CP1 original. Consulte [cp1/README.md](cp1/README.md).

## CP2: classificação TinyML

O sketch em `cp2/firmware/carrinho/` coleta distância a 10 Hz e, no modo de inferência, classifica janelas de 10 leituras como `livre`, `aproximando` ou `bloqueado`. A inferência e o controle estão divididos entre `ultrasonic.h`, `inference.h`, `motors.h`, `policy.h` e `config.h`. A política exige armação serial (`A`), confiança mínima de 0,80 e parada diante de leitura inválida ou distância abaixo de 8 cm; `S` desarma e para.

O modelo foi treinado no projeto privado [CP2 Carrinho Ultrassom, ID 1139087](https://studio.edgeimpulse.com/studio/1139087). Na validação automática do Edge Impulse, int8 acertou 96/96 janelas (30 `aproximando`, 36 `bloqueado`, 30 `livre`), com accuracy, F1 ponderado e AUC de 1,00 e loss `0,0002451005595369078`. No teste reservado, separado por sessão, int8 e float32 acertaram 120/120 janelas (40 por classe); as métricas completas estão em [metrics.json](cp2/evidence/edge-impulse/metrics.json) e as capturas em [cp2/evidence/edge-impulse](cp2/evidence/README.md).

Esses resultados vêm das 600 janelas sintéticas simples do repositório (6.000 medições; 480 para treino e 120 para teste). A validação de 96 janelas é uma divisão automática de 20% das janelas de treino do Edge Impulse, sem separação por sessão; por isso, não mede generalização independente entre sessões. O teste reservado de 120 janelas, separado por sessão, é a avaliação mais independente disponível neste conjunto, mas continua sintético. Para sustentar o resultado acadêmico, ainda é necessário coletar dados físicos do grupo, repetir o teste separado por sessão e validar o carrinho real.

A biblioteca Arduino oficial está versionada em `cp2/model/edge-impulse-arduino-v1.zip`. O firmware TinyML compilou com Arduino CLI 1.4.1 e core ESP32 3.3.11: 304.915 bytes de flash e 24.560 bytes de RAM estática.

Na execução manual do projeto Wokwi, a 100 cm o modelo previu `livre` (confiança 0,99609, ação 2, PWM 160); a 15 cm previu `bloqueado` (0,99609, ação 0, motores parados). Na transição de 80,7 cm para 40,3 cm, previu `aproximando` (0,99609, ação 1, PWM 110). Uma leitura estática ambígua a 40,3 cm teve maior score 0,53516, abaixo do limite de 0,80, e acionou a parada segura (ação 0). A 5 cm, a parada de emergência manteve os sinais desligados mesmo após a política receber ação de movimento. O comando `S` também desarmou o controle (registro `# disarmed`) e encerrou a simulação em estado seguro. As inferências observadas levaram 2–3 ms. A coleta simulada a 100 cm reportou cerca de 100,8 cm a cada 100 ms. Esses cenários foram executados manualmente no navegador Wokwi; os resultados não aprovam os cenários experimentais pelo CLI nem validam o carrinho físico. Consulte [as evidências Wokwi](cp2/evidence/README.md). A validação física e a coleta de dados reais do grupo continuam pendentes.

As avaliações Edge Impulse e as demonstrações Wokwi usam dados ou sensores simulados. Para concluir a entrega acadêmica, colete medições reais, refaça o treinamento/teste com sessões físicas separadas e valide o carrinho montado.

Em 14/10/2026, o firmware com TinyML e Wi-Fi opcional foi gravado na ESP32
D0WD-V3 da COM4. Gravação, inicialização e desarme serial foram verificados;
o sensor não forneceu leituras válidas e o hotspot estava desligado. Ainda
faltam montagem/teste dos periféricos e treino com dados reais. O registro
está em [physical-setup-v1](cp2/evidence/physical-setup-v1/README.md).

## Início rápido

No PowerShell, crie e ative o ambiente Python e instale as dependências usadas pelas ferramentas de dados:

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install -r requirements.txt
```

Com o Arduino CLI e o core ESP32 instalados, compile a coleta ou a inferência a partir da raiz do repositório:

```powershell
powershell -ExecutionPolicy Bypass -File .\cp2\tools\build.ps1 -Mode collection
powershell -ExecutionPolicy Bypass -File .\cp2\tools\build.ps1 -Mode inference
```

Para gravar o firmware de inferência em um ESP32 conectado, substitua `COM5` pela porta serial da sua placa e execute o comando após compilar:

```powershell
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM5 --input-dir build/tinyml cp2/firmware/carrinho
```

O ambiente `.venv` é usado pelas ferramentas Python; a compilação e o upload dependem do Arduino CLI e do core ESP32.

Para configurar o hotspot, copie `cp2/firmware/carrinho/wifi_config.example.h`
para `wifi_config.h` e preencha SSID/senha localmente; esse arquivo é ignorado
pelo Git. Use uma rede de 2,4 GHz. O firmware informa no serial o IP da consulta
HTTP de estado quando conectado e funciona normalmente sem Wi-Fi. A consulta
não aciona motores nem substitui a coleta CSV e o treinamento no Edge Impulse.

## Organização

| Pasta | Responsabilidade |
| --- | --- |
| `cp1/` | Firmware e testes da referência reconstruída por limiar. |
| `cp2/firmware/` | Sketch modular, pinos, política de segurança e integração do modelo. |
| `cp2/tools/` | Captura e preparação de dados; compilação e cenários Wokwi. |
| `cp2/tests/` | Testes existentes para o parser e preparação do conjunto. |
| `cp2/data/` | Sessões, partições e manifestos de origem. |
| `cp2/model/` | ZIP versionado da biblioteca oficial de inferência e instruções para recompilar. |
| `cp2/evidence/` | Capturas, métricas, compilação e demonstração. |
| `cp2/docs/` | Procedimentos de hardware, coleta, treinamento e entrega. |
| `knowledge_base/` | Atividade anterior de LDR, Wokwi e Edge Impulse, preservada à parte. |

## Guias

- [Guia visual HTML — cada fio e ordem de montagem](cp2/docs/montagem.html)
- [Montagem ilustrada no Wokwi](https://wokwi.com/projects/477883916754720769)
- [Handoff para coleta e treinamento físicos — próxima sessão](cp2/docs/HANDOFF_TREINAMENTO_FISICO.md)
- [Mapa de ligação da placa física](cp2/docs/LIGACAO_PLACA_FISICA.md)
- [Hardware e ligação segura](cp2/docs/hardware.md)
- [Coleta e preparação de dados](cp2/docs/coleta.md)
- [Treinamento e integração do modelo](cp2/docs/treinamento.md)
- [Evidências e entrega](cp2/docs/entrega.md)
- [Guia para contribuições](CONTRIBUTING.md)

As instruções de armazenamento estão em [cp2/data](cp2/data/README.md), [cp2/model](cp2/model/README.md) e [cp2/evidence](cp2/evidence/README.md).

## Datas informadas

- Envio do vídeo: **14/10/2026**.
- Demonstração presencial: **21/10/2026**.
