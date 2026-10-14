# Primeira gravação física — 14/10/2026

O firmware TinyML com conexão Wi-Fi opcional foi compilado e gravado na ESP32
D0WD-V3, revisão 3.1, conectada à COM4. O gravador confirmou a integridade dos
dados escritos e reiniciou a placa. Identificadores individuais e credenciais
não são registrados nestas evidências.

`flash.json` identifica firmware, modelo, versões e resultado da gravação.
O binário contém a configuração Wi-Fi local e permanece fora do Git.

`boot-check.json` registra uma verificação de aproximadamente 34 segundos:
inicialização TinyML, resposta a `S`, 338 leituras, nenhuma leitura válida,
nenhuma falha detectada e tentativa de reconexão Wi-Fi em segundo plano.
Não foi enviado `A`; a placa ficou desarmada. O hotspot estava indisponível,
conforme informado pelo usuário.

Isso confirma gravação e execução do firmware físico, não o funcionamento do
sensor, movimento dos motores, conexão ao hotspot ou inferência com entrada
física válida. O modelo continua sendo o sintético do projeto 1139087.
Conecte conforme [o mapa de ligação](../../docs/LIGACAO_PLACA_FISICA.md), teste
sensor/motores e prossiga com a coleta e treinamento descritos no handoff.
