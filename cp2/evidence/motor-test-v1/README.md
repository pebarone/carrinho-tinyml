# Teste isolado dos motores — 14/10/2026

Sketch: `cp2/firmware/teste_motores/teste_motores.ino`.
Placa: ESP32 DevKitC V4, ESP32-D0WD-V3 rev. 3.1, COM4.
Arduino CLI 1.4.1, core ESP32 3.3.11.

Compilacao e upload concluidos, com integridade da gravacao verificada.
Uso: 272.264 bytes de flash e 22.124 bytes de RAM estatica.
O teste substitui temporariamente o TinyML na placa, preservado no repositorio.

A serial confirmou inicializacao parada, comando `S` e rejeicao de comando
invalido. Apos o usuario confirmar que Vs estava desconectado, foi enviado
`A` e observada a sequencia frente (1 s), pausa (300 ms), tras (1 s), pausa
(300 ms), repetida por aproximadamente 7,8 s. As mensagens respeitaram a
ordem e os tempos esperados, com tolerancia de observacao serial de 120 ms.
O teste terminou com `S` confirmado e motores desarmados.

**Nao houve verificacao de movimento fisico ou medicao dos GPIOs.**
O usuario informou USB na ESP32 e 5V da logica da L298N, com Vs/12V sem
alimentacao. As mensagens verificam a execucao do programa, nao a resposta
eletrica da ponte, a rotacao ou o angulo dos motores. O sensor nao participa.

Os registros e hash do sketch estao em `verification.json`. Para movimento
fisico, conectar positivo das pilhas somente a Vs, GND comum e rodas
suspensas, mantendo 5V_EN, ENA e ENB sem jumpers. O positivo das pilhas
fica separado da linha de 5V alimentada pelo USB.
