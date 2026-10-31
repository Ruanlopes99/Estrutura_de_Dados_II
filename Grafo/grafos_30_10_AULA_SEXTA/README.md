# Grafos, BFS e DFS — Simulação de contingência na rede BSB–CWB

**Disciplina:** Estrutura de Dados II — Ciência da Computação
**Estudo de caso:** pane relacionada ao GPS e impactos em voos no Aeroporto de Brasília (G1, 22 set. 2026)

> **Aviso:** esta é uma simulação computacional didática. Os aeroportos são reais, mas as conexões e os bloqueios são **fictícios**. O resultado não representa rotas aéreas e não pode orientar um voo real.

## 1. Problema

Quando um recurso de navegação fica indisponível, a operação precisa ser replanejada e algumas ligações deixam de estar disponíveis. Este trabalho modela uma rede hipotética de conexões entre aeroportos como um grafo, com cada vértice identificado pelo seu código IATA e estuda o que acontece com a conectividade quando arestas são removidas.

## 2. Pergunta de pesquisa e objetivo

**Pergunta:** quando uma ou mais conexões da rede simulada são bloqueadas, ainda existe caminho entre a origem e o destino? Quantas arestas a mais o desvio exige, e como BFS e DFS se comportam diante disso?

**Objetivo:** implementar BFS e DFS em Python, simular bloqueios sem alterar a rede original e comparar os caminhos encontrados pelos dois algoritmos.

## 3. Estrutura do grafo

- **Tipo:** não dirigido e não ponderado (cada ligação vale nos dois sentidos e todas têm o mesmo "custo").
- **Tamanho:** 11 vértices e 15 arestas.
- **Representação:** lista de adjacência (`dict` de listas), gerada a partir de uma lista de arestas pela função `construir_grafo`. Isso garante que toda aresta apareça nas listas dos dois extremos.

![alt text](<Captura de tela 2026-10-30 205505-1.png>)

| Sigla | Aeroporto (cidade) | Vizinhos |
|---|---|---|
| BSB | Internacional de Brasília (Brasília) | GYN, CNF, CGB, SSA |
| GYN | Santa Genoveva (Goiânia) | BSB, VCP, POA |
| CNF | Internacional de Confins (Belo Horizonte) | BSB, VCP, GIG, GRU |
| VCP | Internacional de Viracopos (Campinas) | GYN, CNF, GRU |
| CGB | Marechal Rondon (Cuiabá) | BSB, POA |
| POA | Internacional Salgado Filho (Porto Alegre) | GYN, CGB, FLN |
| GRU | Internacional de Guarulhos (São Paulo) | CNF, VCP, GIG, CWB |
| GIG | Internacional do Galeão (Rio de Janeiro) | CNF, GRU |
| FLN | Internacional Hercílio Luz (Florianópolis) | POA, CWB |
| CWB | Internacional Afonso Pena (Curitiba) | GRU, FLN |
| SSA | Internacional de Salvador (Salvador) | BSB |

**Justificativa das conexões:**
- **Dois "corredores" até CWB:** um pelo Sudeste (via GRU) e outro pelo Sul (POA → FLN). Assim dá para testar se a rede continua conexa depois de vários bloqueios.
- **Ciclos** (por exemplo, BSB–GYN–VCP–CNF–BSB). Eles mostram por que o controle de `visitados` é obrigatório.
- **SSA** tem uma única ligação, ou seja, a aresta BSB–SSA é uma **ponte**. Ela serve para demonstrar desconexão com um único bloqueio.

## 4. Como executar

Requisito: Python 3.8 ou superior, sem bibliotecas externas.

```bash
python solucao_grafos.py            # modo interativo
python solucao_grafos.py --testes   # bateria de testes automáticos
```

No modo interativo, o usuário informa a origem e o destino pela sigla (`bsb`, `CWB`) ou pelo nome da cidade, com ou sem acento (`brasilia`). Aeroportos inexistentes (como `MAO`) são rejeitados com a lista de opções válidas. Em seguida, o usuário pode bloquear quantas conexões quiser, no formato `SIGLA A - SIGLA B` (ex.: `CNF - GRU`). O programa recusa conexões que não existem.

## 5. Organização do código

| Função | Papel |
|---|---|
| `construir_grafo` | Gera a lista de adjacência simétrica a partir das arestas |
| `bfs` | Busca em largura com `deque` (fila FIFO). Marca o vértice como visitado ao enfileirar |
| `dfs` | Busca em profundidade com pilha (LIFO). Usa `reversed()` para explorar o 1º vizinho primeiro |
| `bloquear_conexoes` | Devolve uma **cópia** da rede sem as arestas indicadas, removidas nos dois sentidos |
| `reordenar_vizinhos` | Cópia com outra ordem de vizinhos (Experimento D) |
| `AEROPORTOS` | Dicionário sigla → (cidade, nome do aeroporto), usado na exibição e na validação |
| `resolver_nome` / `normalizar` | Validação da entrada: aceita sigla ou cidade |
| `formatar` | Exibe o caminho e a quantidade de arestas, ou a mensagem de "sem caminho" |

## 6. Resultados (origem BSB, destino CWB)

| Cenário | BFS | Arestas | DFS | Arestas |
|---|---|:-:|---|:-:|
| **A** Rede completa | BSB → CNF → GRU → CWB | 3 | BSB → GYN → VCP → CNF → GIG → GRU → CWB | 6 |
| **B** Bloqueio CNF–GRU | BSB → GYN → VCP → GRU → CWB | 4 | BSB → GYN → VCP → CNF → GIG → GRU → CWB | 6 |
| **C1** Bloqueios CNF–GRU e GRU–CWB | BSB → GYN → POA → FLN → CWB | 4 | (mesmo caminho) | 4 |
| **C2** + bloqueio FLN–CWB | sem caminho | — | sem caminho | — |
| **Extra** Ponte BSB–SSA (destino SSA) | sem caminho | — | sem caminho | — |
| **D** Vizinhos de BSB reordenados (CGB primeiro) | BSB → CNF → GRU → CWB | 3 | BSB → CGB → POA → GYN → VCP → CNF → GIG → GRU → CWB | 8 |

Todos os 14 testes automáticos passaram. As saídas completas estão em `evidencias_testes.txt` (testes) e `evidencia_interativo.txt` (sessão interativa com entrada inválida e bloqueio inválido).

### Interpretação

- **A e B:** o BFS sempre devolve o caminho com o menor número de arestas. Com o bloqueio CNF–GRU, o desvio custa **+1 aresta** (de 3 para 4). O DFS segue o primeiro ramo até o fim e devolve caminhos mais longos, embora válidos.
- **C1:** mesmo com o corredor do Sudeste cortado, CWB continua alcançável pelo corredor Sul (POA e FLN). A rede ainda é **conexa** entre os dois vértices.
- **C2:** com as três arestas bloqueadas, CWB fica sem nenhuma ligação. O grafo se torna **desconexo**, e os dois algoritmos devolvem `None` depois de esgotar a fila ou a pilha.
- **Extra:** remover uma única **ponte** (BSB–SSA) já isola um vértice. Isso mostra que a robustez depende de haver caminhos alternativos.
- **D:** o grafo é o mesmo, mas basta mudar a ordem dos vizinhos para o DFS devolver um caminho de 8 arestas. O BFS mantém o mínimo de 3, porque explora por níveis, e o número mínimo de arestas não depende da ordem.

## 7. Comparação BFS × DFS

| Aspecto | BFS | DFS |
|---|---|---|
| Estrutura | Fila (FIFO) | Pilha (LIFO) ou recursão |
| Estratégia | Explora por níveis (distância em arestas) | Aprofunda um ramo antes de voltar |
| Menor caminho? | **Sim**, em número de arestas, num grafo não ponderado | Não garante |
| Sensível à ordem dos vizinhos? | Pode mudar *qual* caminho mínimo sai, mas não o seu tamanho | Sim, pode mudar o caminho e o tamanho |
| Complexidade | O(V + E) | O(V + E) |
| Uso típico | Menor número de conexões, alcance por níveis | Testar se há caminho, explorar componentes |

## 8. Limitações

- **"Menor" aqui significa menos arestas**, e não menor tempo, distância ou custo. Para isso seria preciso um grafo **ponderado** e outro algoritmo, como Dijkstra ou A\* (Gomes, 2022, cap. 7).
- As conexões são fictícias e não consideram capacidade, horários, restrições operacionais, meteorologia nem regras de tráfego aéreo.
- O grafo é não dirigido. Uma rede real pode ter ligações num único sentido (grafo dirigido).
- O caminho é guardado como lista copiada a cada passo (`caminho + [vizinho]`). Isso é simples, mas gasta mais memória que guardar só o "pai" de cada vértice, o que faria diferença em redes grandes.
- O bloqueio é binário (a aresta existe ou não). Não há degradação parcial nem probabilidade de falha.

## 9. Referências

GOMES, Paulo César Rodacki. **Grafos: conceitos fundamentais, algoritmos e aplicações**. Blumenau: Editora IFC, 2022. ISBN 978-65-88089-12-5.

G1 DISTRITO FEDERAL. **Pane em GPS afeta voos da Azul no Aeroporto de Brasília; passageiros reclamam de atrasos**. 22 set. 2026. Disponível em: https://g1.globo.com/df/distrito-federal/noticia/2026/09/22/pane-em-gps-afeta-voos-da-azul-no-aeroporto-de-brasilia-passageiros-reclamam-de-atrasos.ghtml.
