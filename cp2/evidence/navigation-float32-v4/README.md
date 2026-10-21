# Revisão de navegação e exportação float32 — 18/10/2026

## Alterações

- Modelo real 1140184: exportação oficial Arduino EON float32, deployment v2,
  biblioteca 1.0.2. ZIP, TFLite oficial e hashes em `cp2/model/`.
- Mesmo treinamento e mesmas partições reais de 35/9 janelas; nenhum dado novo.
- Previsão ambígua de classe conhecida permite avanço cauteloso PWM 180,
  sujeito a leitura válida, armação e distância acima de 20 cm. A leitura
  direta de até 20 cm continua iniciando parada, ré, pausa, giro e parada.
- Score inválido, classe desconhecida, desarme e sensor inválido não permitem
  esse avanço. O score usado no PWM é o da previsão recém-recebida.
- Cenários Wokwi atualizados para os banners, partida automática, sentido
  compensado dos motores e etapas da manobra. `approach` usa 45 cm estáticos.

## Verificação desktop

`python -m unittest discover -s cp2/tests -v`: 8 testes passaram, sem skips.
Inclui parser/preparação, contratos/hashes dos modelos, pesos do Arduino
iguais aos do TFLite oficial e entradas de 20 a 400 cm. Os testes de modelo
requerem NumPy/TensorFlow; as dependências do firmware não mudaram.

`cp2/tests/test_policy.ps1`: assertions C++ CP1/CP2 passaram, incluindo as
janelas estáticas reais de 45/50/55 cm, PWM cauteloso, parada por distância,
desarme, erro de sensor, scores inválidos, sequência de manobra e rollover.

`desktop-model.json` registra a execução do TFLite oficial no computador:
8/9 acertos no teste reservado, mesmos resultados gerais do modelo real
anterior; a aproximação confundida com livre permanece uma limitação.
Não se trata de novo treinamento nem validação de segurança no chão.

## Compilação

Arduino CLI 1.4.1, core ESP32 3.3.11, alvo `esp32:esp32:esp32`.
Os três perfis compilaram:

| Perfil | Flash | RAM estática |
| --- | ---: | ---: |
| Inferência real float32 | 953.667 bytes | 49.420 bytes |
| Coleta | 926.167 bytes | 47.316 bytes |
| Inferência sintética int8 | 957.659 bytes | 49.572 bytes |

Comandos e hashes finais em `verification.json`.

## Limites e próxima verificação

Nenhum upload ou acionamento físico foi feito nesta revisão. O firmware
compilado deve ser gravado e ensaiado com o carrinho antes de aprovar
frenagem, sentido/ângulo do giro, tração e retomada no chão. O sensor só
mede à frente; a ré e o giro têm duração fixa e a falta de eco continua
causando parada. Mais dados reais são necessários para ampliar a avaliação.

Os cinco YAMLs foram analisados quanto à estrutura e aos banners atuais.
A execução Wokwi CLI foi tentada, mas o CLI não está instalado e
`WOKWI_CLI_TOKEN` não está configurado. Isso não é um teste CLI aprovado.
Os registros int8 de 14–15/10 permanecem históricos, sem alteração.
