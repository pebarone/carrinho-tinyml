# Contribuições

Leia primeiro o [README](README.md) e a documentação de `cp2/` relacionada à mudança. Preserve a separação entre a atividade anterior de LDR em `knowledge_base/` e o carrinho TinyML em `cp2/`.

## Dados e resultados

- Registre a origem de cada coleta (`real`, `synthetic` ou `wokwi`) e mantenha o arquivo JSON de metadados criado pela ferramenta junto ao CSV. Não rotule dados simulados como medições físicas.
- Não sobrescreva sessões existentes. Dê a cada coleta um nome novo e gere cada conjunto preparado em uma pasta de destino ainda inexistente.
- Mantenha sessões inteiras em apenas uma partição de treino ou teste. Não misture janelas da mesma sessão entre as partições.
- Só publique métricas depois de obtê-las no treinamento/teste correspondente e associe-as à origem e à versão do conjunto usado.
- Diferencie a validação automática de 96 janelas do Edge Impulse, que não é agrupada por sessão, do teste reservado de 120 janelas. Os resultados atuais de ambas as partições são sintéticos; não os descreva como validação física.
- Não adicione tokens, credenciais ou configurações privadas. `cp2/firmware/carrinho/model_config.h` e `cp2/model/library/` são saídas locais ignoradas; documente os passos para recriá-las.

## Firmware e documentação

Siga os pinos e parâmetros de `cp2/firmware/carrinho/config.h`. Mudanças de janela, frequência ou rótulos precisam continuar compatíveis com o impulso Edge Impulse e com as verificações de compilação do firmware. Documente alterações de ligação, coleta, configuração ou comportamento observável.

Instale a única dependência Python listada e execute os testes existentes do parser quando alterar a preparação de dados:

```powershell
python -m pip install -r requirements.txt
python -m unittest discover -s cp2/tests
```

Para mudanças de firmware, registre placa, versão do core ESP32, modo de compilação e resultado observado. Não descreva uma compilação ou teste físico como concluído sem executá-lo e guardar a evidência correspondente.
