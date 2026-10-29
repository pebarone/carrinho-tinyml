# Projeto mecânico e montagem

## Chassi

Plataforma em **acrílico transparente**, com:

- dois motorredutores DC TT (caixa amarela) com rodas de borracha;
- roda boba (rodízio) no lado oposto aos motores;
- furos oblongos que permitem reposicionar os componentes.

![Carrinho visto de cima](../media/fotos/carrinho-final-topo.jpg)

## Adaptações realizadas

| Adaptação | Por quê |
| --- | --- |
| **HC-SR04 na borda frontal**, sobre a roda boba, voltado para frente | Campo de visão livre; a frente do carrinho é o lado do sensor |
| **Suporte de 4 pilhas AA no centro**, à frente da ponte H | Centraliza o peso entre o eixo dos motores e a roda boba |
| **Ponte H L298N no centro do chassi**, entre os motores | Fios curtos até OUT1–OUT4 dos motores |
| **ESP32 sobre o chassi, junto à protoboard** | Acesso livre ao conector USB para gravação |
| **Protoboard transversal na traseira**, sobre o eixo dos motores | Concentra GND comum, linha de 5 V e o divisor resistivo do ECHO sem solda |
| Sentido do motor B compensado no firmware | Evita desmontar a fiação dos motores (ver [motors.h](../cp2/firmware/carrinho/motors.h)) |

![Vista diagonal: protoboard traseira, ponte H, pilhas e sensor](../media/fotos/carrinho-final-diagonal.jpg)

O mapa completo das ligações está em [03-hardware-eletronica.md](03-hardware-eletronica.md).

## Carenagem

Ainda não há carenagem nesta versão: os componentes estão expostos sobre o
chassi, o que facilitou as trocas de pilhas, regravação da ESP32 e ajustes
de fiação durante os testes. A carenagem está registrada no
[backlog](01-requisitos-planejamento.md#backlog).
