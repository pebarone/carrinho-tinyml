# Carrinho TinyML — CP1 → CP2

Carrinho com **ESP32 DevKitC V4, HC-SR04, ponte L298N e dois motores TT**.
No CP2, dados reais do sensor treinam um modelo que interfere no movimento.

**[Documento de entrega CP2](cp2/docs/entrega.md)** · **[Edge Impulse público](https://studio.edgeimpulse.com/public/1140184/live)** · [GitHub](https://github.com/pebarone/cp2-carrinho-tinyml)

## O que foi desenvolvido

**Sensor → CSV → Edge Impulse → treinamento → teste → biblioteca Arduino → inferência na ESP32 → motores.**

- **Dados reais:** 15 sessões, 464 medições (462 válidas), 44 janelas de 1 segundo; 35 para treino e 9 para teste, separadas por sessão.
- **Classes:** `livre`, `aproximando` e `bloqueado`.
- **Resultado:** 85,71% na validação interna (6/7) e 88,89% no teste reservado (8/9). Um caso de aproximação foi confundido com livre.
- **Integração:** inferência e reação dos motores confirmadas em bancada na versão int8. O perfil atual usa a biblioteca oficial float32 v2, com o mesmo treinamento; sua validação física ainda está pendente.
- **Vídeo:** será acrescentado pelo grupo.

O firmware inicia após 5 segundos. A classificação controla avanço/velocidade;
o limite direto de 20 cm inicia parada, ré curta e giro. Leitura inválida para;
`S` desarma e `A` rearma. Detalhes em [navegação autônoma](cp2/docs/NAVEGACAO_AUTONOMA.md).

## Onde encontrar

| Material | Local |
| --- | --- |
| Relatório, métricas, matriz e integrantes | [Entrega CP2](cp2/docs/entrega.md) |
| CSVs reais e origem das sessões | [Dados reais](cp2/data/real/) · [diário](cp2/data/real/diario-coleta.md) |
| Treino e teste enviados ao Edge Impulse | [Dataset preparado](cp2/data/real/edge-impulse-v1/) |
| Código da ESP32 | [carrinho.ino](cp2/firmware/carrinho/carrinho.ino) |
| Biblioteca oficial e versões | [Modelos](cp2/model/README.md) |
| Prints, métricas e testes físicos | [Campanha real](cp2/evidence/real-training-v1/README.md) |
| Verificação da revisão float32 | [Revisão atual](cp2/evidence/navigation-float32-v4/README.md) |
| Ligações e alimentação | [Hardware](cp2/docs/hardware.md) · [guia visual](cp2/docs/montagem.html) |

## Compilar e gravar

Requer Arduino CLI, core ESP32 e, para as ferramentas de coleta, Python com
`pip install -r requirements.txt`. No PowerShell, na raiz do repositório:

```powershell
powershell -ExecutionPolicy Bypass -File cp2/tools/build.ps1 -Mode inference -Model real
arduino-cli upload --fqbn esp32:esp32:esp32 --port COM4 --input-dir build/tinyml-real cp2/firmware/carrinho
```

Troque `COM4` pela porta da placa. No Arduino IDE, instale o [ZIP float32](cp2/model/edge-impulse-arduino-real-v1-float32.zip), abra `cp2/firmware/carrinho/carrinho.ino` e selecione ESP32 Dev Module. Para coletar, use `build.ps1 -Mode collection`; esse modo não aciona motores. O firmware funciona sem PC ou Wi-Fi, desde que a alimentação seja adequada.

## Evolução preservada

- [CP1](cp1/README.md): referência por limiar reconstruída para comparação; não é um arquivo recuperado do CP1 original.
- CP2 inicial: [dados/modelo sintéticos e Wokwi](cp2/evidence/README.md), mantidos como etapa de desenvolvimento.
- CP2 real: coleta física, projeto Edge Impulse **1140184**, treinamento, testes, integração e navegação. O histórico Git registra essas etapas sem substituir o CP1.

A atividade de LDR em `knowledge_base/` é independente deste carrinho.
