# Guia visual e montagem Wokwi — 14/10/2026

- [HTML standalone](../../docs/montagem.html): SVG de conexões, filtros por
  alimentação/sensor/controle/motores, tabela de terminais, oito passos e checklist.
- [Projeto ilustrado no Wokwi](https://wokwi.com/projects/477883916754720769).
- `wokwi-montagem.png`: captura do projeto ilustrado salvo.
- `guia-completo.png`: captura integral do HTML no navegador.
- `guia-diagrama.png`: detalhe do diagrama interativo.
- Fontes dos componentes/diagrama em `cp2/wokwi/assembly/`.

## Verificações

- JSON: 20 componentes com IDs únicos, 28 conexões com nomes de pino válidos.
- Conferidos os seis GPIOs da ponte e a topologia dos cinco resistores de 330 Ω.
- IDs HTML únicos, links Markdown locais existentes e JavaScript sem erro de sintaxe.
- No navegador, os sete filtros atualizaram o texto e destacaram seus grupos;
  a checklist respondeu ao clique. Não houve overflow horizontal da página no
  viewport desktop observado. O teste de viewport móvel não foi confirmado,
  pois a ferramenta manteve o viewport desktop apesar da configuração solicitada.
- O projeto original Wokwi teve seu diagrama anterior restaurado; a montagem
  ilustrada tem URL própria e não substitui as evidências anteriores de TinyML.
  Após restaurar, a coleta a 100 cm voltou a reportar leituras válidas de
  aproximadamente 100,79–100,88 cm a cada 100 ms; o teste foi encerrado.

## Limites

O divisor resistivo produziu `distance_cm=-1`, `valid=0` na simulação da montagem.
Isso não é um teste elétrico do divisor físico. A compilação dos chips personalizados
registrou falha de acesso ao serviço remoto e seu comportamento dinâmico não foi
validado. Nenhuma medição física nova, teste de motores reais ou nova inferência
TinyML foi executada nesta etapa. O firmware físico não foi alterado.

A alimentação desenhada usa powerbank e pilhas; a alternativa com uma única
bateria requer conversor de 5 V. A armação sem PC continua pendente, porque
o firmware atual inicia desarmado e só recebe `A`/`S` pela serial.
