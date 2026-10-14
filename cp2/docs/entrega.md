# Evidências e entrega

## Já registrado

- O Edge Impulse tem configuração, treino, validação e teste int8/float32 em `cp2/evidence/edge-impulse/`. A validação automática acertou 96/96; o teste reservado por sessão acertou 120/120 (40 por classe) nas duas variantes. Os dados são sintéticos.
- O ZIP Arduino oficial está versionado em `cp2/model/edge-impulse-arduino-v1.zip`. A compilação de inferência terminou com Arduino CLI 1.4.1 e core ESP32 3.3.11: 304.915 bytes de flash e 24.560 bytes de RAM estática.
- A coleta Wokwi registrou alvo de 100 cm e leituras válidas próximas de 100,8 cm, a cada 100 ms.
- Na execução manual do projeto Wokwi, a 100 cm o classificador previu `livre` (0,99609, ação `2`); a 15 cm previu `bloqueado` (0,99609, ação `0`). O tempo de inferência foi de 2–3 ms. A execução desligou os LEDs que representam os sinais dos motores no caso bloqueado e os habilitou no caso livre.
- Na transição de 80,7 cm para 40,3 cm, o modelo previu `aproximando` (0,99609; ação `1`; PWM 110; inferência de 2 ms). Em uma leitura estática a 40,3 cm, a distribuição foi `aproximando` 0,53516, `bloqueado` 0,07812 e `livre` 0,38672; como o maior score ficou abaixo do limiar 0,80, a política aplicou ação `0` (parada segura).
- A distância crítica de 5 cm interrompeu os motores previamente habilitados pela ação `1`. A evidência inclui `wokwi/04-emergency.jpg` e `wokwi/emergency-after-motion-serial.txt`.
- O comando serial `S` desarmou o firmware; `wokwi/disarmed-serial.txt` registra `# disarmed`, e `wokwi/07-disarmed.jpg` mostra os sinais dos motores desabilitados.
- O código-fonte dos firmwares CP1 por limiar e CP2 TinyML está no repositório. O CP1 é uma reconstrução comparativa, não um artefato histórico recuperado.

Todos os resultados Wokwi acima foram verificados manualmente no navegador, usando `build/tinyml/carrinho.ino.merged.bin`; LEDs representam os sinais ENA/ENB na simulação. Os cenários do Wokwi CLI, incluindo aproximação, continuam experimentais e não foram aprovados por esses testes manuais.

## Pendente para a entrega física

- **Dados acadêmicos e montagem:** coletar leituras reais, manter sessões inteiras fora do treino, refazer a avaliação e demonstrar o carrinho físico. Ainda não há validação física registrada.

A validação automática de 96 janelas usa 20% das janelas de treino sem agrupamento por sessão. O teste reservado de 120 janelas é mais independente por manter sessões separadas, mas ambas as partições são sintéticas. Não apresente essas métricas ou as respostas do Wokwi como desempenho do hardware real.

Guarde os registros em `cp2/evidence/`, com contexto, origem dos dados e versões. Não armazene tokens, credenciais ou dados pessoais desnecessários.

## Datas informadas

- Envio do vídeo: **14/10/2026**.
- Demonstração presencial: **21/10/2026**.

No vídeo, indique a origem sintética dos dados de treinamento e identifique como simulação os resultados obtidos no Wokwi. Se a escola exigir o firmware histórico do CP1, confirme qual registro original deve acompanhar a referência reconstruída deste repositório.

## Postagem no Teams

Publique a entrega na seção CP2 do Teams e inclua o link do repositório, o vídeo da demonstração e a identificação dos integrantes.

| Item | Dado para a postagem |
| --- | --- |
| Repositório | https://github.com/pebarone/cp2-carrinho-tinyml |
| Seção | CP2 |
| Vídeo | Anexar ou incluir o link do vídeo; envio até 14/10/2026 |
| Integrante 1 | João Pedro de Albuquerque Oliveira — RM551579 |
| Integrante 2 | Pedro Augusto Carneiro Barone Bomfim — RM99781 |
| Integrante 3 | Matheus Augusto Santos Rego — RM551466 |
