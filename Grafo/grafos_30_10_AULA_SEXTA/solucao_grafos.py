"""
Solução — Grafos, BFS e DFS (Estrutura de Dados II)

Pergunta de pesquisa:
    Quando uma ou mais conexões da rede simulada são bloqueadas, ainda existe
    caminho entre a origem e o destino? Quantas arestas a mais o desvio exige,
    e como BFS e DFS se comportam diante disso?

AVISO: simulação didática. As conexões abaixo são FICTÍCIAS e não representam
rotas aéreas reais nem servem para orientar navegação.

Uso:
    python solucao_grafos.py            -> modo interativo
    python solucao_grafos.py --testes   -> executa a bateria de testes
"""

import sys
import unicodedata
from collections import deque

# ---------------------------------------------------------------------------
# 1. Modelagem: grafo NÃO dirigido, não ponderado, construído a partir de uma
#    lista de arestas. Assim cada ligação aparece automaticamente nos dois
#    sentidos e a lista de adjacência nunca fica inconsistente.
# ---------------------------------------------------------------------------
ARESTAS = [
    ("BSB", "GYN"),          # Brasília–Goiânia
    ("BSB", "CNF"),   # Brasília–Confins (BH)
    ("BSB", "CGB"),           # Brasília–Cuiabá
    ("BSB", "SSA"),           # única ligação de Salvador (vértice "folha")
    ("GYN", "VCP"),        # Goiânia–Viracopos (Campinas)
    ("GYN", "POA"),
    ("CNF", "VCP"),
    ("CNF", "GIG"),
    ("CNF", "GRU"),
    ("VCP", "GRU"),
    ("CGB", "POA"),
    ("POA", "FLN"),     # corredor alternativo pelo Sul
    ("GRU", "GIG"),
    ("GRU", "CWB"),        # principal acesso a Curitiba (via Guarulhos)
    ("FLN", "CWB"),         # acesso alternativo (via Florianópolis)
]


def construir_grafo(arestas):
    """Cria a lista de adjacência (dict de listas) de um grafo não dirigido."""
    grafo = {}
    for a, b in arestas:
        grafo.setdefault(a, []).append(b)
        grafo.setdefault(b, []).append(a)
    return grafo


GRAFO = construir_grafo(ARESTAS)

# Vértices são códigos IATA; este dicionário dá o nome para exibição.
AEROPORTOS = {
    "BSB": ("Brasília", "Aeroporto Internacional de Brasília"),
    "GYN": ("Goiânia", "Aeroporto Santa Genoveva"),
    "CNF": ("Belo Horizonte", "Aeroporto Internacional de Confins"),
    "CGB": ("Cuiabá", "Aeroporto Marechal Rondon"),
    "SSA": ("Salvador", "Aeroporto Internacional de Salvador"),
    "VCP": ("Campinas", "Aeroporto Internacional de Viracopos"),
    "POA": ("Porto Alegre", "Aeroporto Internacional Salgado Filho"),
    "GRU": ("São Paulo", "Aeroporto Internacional de Guarulhos"),
    "GIG": ("Rio de Janeiro", "Aeroporto Internacional do Galeão"),
    "FLN": ("Florianópolis", "Aeroporto Internacional Hercílio Luz"),
    "CWB": ("Curitiba", "Aeroporto Internacional Afonso Pena"),
}


# ---------------------------------------------------------------------------
# 2. Algoritmos de busca
# ---------------------------------------------------------------------------
def bfs(rede, origem, destino):
    """Busca em largura com fila FIFO. Retorna o caminho com MENOS ARESTAS
    (grafo não ponderado) ou None se não houver caminho."""
    fila = deque([(origem, [origem])])
    visitados = {origem}
    while fila:
        atual, caminho = fila.popleft()          # FIFO: sai o mais antigo
        if atual == destino:
            return caminho
        for vizinho in rede.get(atual, []):
            if vizinho not in visitados:
                visitados.add(vizinho)           # marca ao enfileirar
                fila.append((vizinho, caminho + [vizinho]))
    return None


def dfs(rede, origem, destino):
    """Busca em profundidade com pilha LIFO. Retorna UM caminho (não
    necessariamente o mais curto) ou None se não houver caminho."""
    pilha = [(origem, [origem])]
    visitados = set()
    while pilha:
        atual, caminho = pilha.pop()             # LIFO: sai o mais recente
        if atual in visitados:
            continue
        visitados.add(atual)
        if atual == destino:
            return caminho
        # reversed() faz o 1º vizinho da lista ser explorado primeiro
        for vizinho in reversed(rede.get(atual, [])):
            if vizinho not in visitados:
                pilha.append((vizinho, caminho + [vizinho]))
    return None


# ---------------------------------------------------------------------------
# 3. Simulação de contingência
# ---------------------------------------------------------------------------
def bloquear_conexoes(rede, bloqueios):
    """Retorna uma CÓPIA da rede sem as arestas informadas (nos dois
    sentidos). A rede original nunca é alterada."""
    copia = {no: vizinhos.copy() for no, vizinhos in rede.items()}
    for a, b in bloqueios:
        if b in copia.get(a, []):
            copia[a].remove(b)
        if a in copia.get(b, []):
            copia[b].remove(a)
    return copia


def reordenar_vizinhos(rede, vertice, nova_ordem):
    """Cópia da rede com a ordem dos vizinhos de um vértice alterada
    (mesmo grafo, só muda a ordem de exploração)."""
    copia = {no: vizinhos.copy() for no, vizinhos in rede.items()}
    copia[vertice] = list(nova_ordem)
    return copia


# ---------------------------------------------------------------------------
# 4. Utilidades de entrada e saída
# ---------------------------------------------------------------------------
def normalizar(texto):
    """Remove acentos, espaços extras e diferença de maiúsculas,
    para aceitar 'brasilia', 'BRASÍLIA', ' Brasília ' etc."""
    sem_acento = unicodedata.normalize("NFKD", texto.strip())
    sem_acento = "".join(c for c in sem_acento if not unicodedata.combining(c))
    return " ".join(sem_acento.lower().split())


def resolver_nome(rede, digitado):
    """Converte a sigla IATA ou o nome da cidade digitado no código do
    vértice (ex.: 'bsb' ou 'brasilia' -> 'BSB'), ou None se não existir."""
    alvo = normalizar(digitado)
    for vertice in rede:
        cidade = AEROPORTOS.get(vertice, (vertice,))[0]
        if alvo in (normalizar(vertice), normalizar(cidade)):
            return vertice
    return None


def formatar(caminho):
    if caminho is None:
        return "nenhum caminho encontrado (destino inalcançável)"
    arestas = len(caminho) - 1
    return f"{' -> '.join(caminho)}  [{arestas} aresta{'s' if arestas != 1 else ''}]"


def comparar(rede, origem, destino, titulo):
    print(f"\n=== {titulo} ===")
    c_bfs = bfs(rede, origem, destino)
    c_dfs = dfs(rede, origem, destino)
    print(f"BFS: {formatar(c_bfs)}")
    print(f"DFS: {formatar(c_dfs)}")
    if c_bfs and c_dfs and len(c_dfs) > len(c_bfs):
        print(f"-> DFS usou {len(c_dfs) - len(c_bfs)} aresta(s) a mais que o BFS.")
    return c_bfs, c_dfs


# ---------------------------------------------------------------------------
# 5. Modo interativo
# ---------------------------------------------------------------------------
def pedir_vertice(rede, rotulo):
    while True:
        digitado = input(f"{rotulo}: ")
        nome = resolver_nome(rede, digitado)
        if nome:
            return nome
        print(f"  '{digitado}' não existe na rede. Opções: {', '.join(sorted(rede))}")


def pedir_bloqueios(rede):
    bloqueios = []
    print("\nBloqueie conexões (formato: SIGLA A - SIGLA B, ex.: CNF - GRU). Enter vazio para encerrar.")
    while True:
        linha = input("Bloquear: ").strip()
        if not linha:
            return bloqueios
        partes = linha.split("-")
        if len(partes) != 2:
            print("  Use o formato 'SIGLA A - SIGLA B'.")
            continue
        a, b = resolver_nome(rede, partes[0]), resolver_nome(rede, partes[1])
        if not a or not b:
            print("  Cidade inexistente na rede.")
        elif b not in rede[a]:
            print(f"  Não existe conexão direta entre {a} e {b}.")
        else:
            bloqueios.append((a, b))
            print(f"  Conexão {a}–{b} será bloqueada na simulação.")


def modo_interativo():
    print("Rede simulada (fictícia) — aeroportos disponíveis (sigla ou cidade):")
    for vertice in sorted(GRAFO):
        cidade, nome = AEROPORTOS[vertice]
        print(f"  {vertice} – {cidade} ({nome}): {', '.join(GRAFO[vertice])}")
    origem = pedir_vertice(GRAFO, "\nOrigem")
    destino = pedir_vertice(GRAFO, "Destino")
    comparar(GRAFO, origem, destino, "Cenário normal")
    bloqueios = pedir_bloqueios(GRAFO)
    if bloqueios:
        rotulo = ", ".join(f"{a}–{b}" for a, b in bloqueios)
        comparar(bloquear_conexoes(GRAFO, bloqueios), origem, destino,
                 f"Cenário com bloqueio: {rotulo}")
    print("\n(A rede original permanece intacta.)")


# ---------------------------------------------------------------------------
# 6. Bateria de testes (evidências)
# ---------------------------------------------------------------------------
def executar_testes():
    O, D = "BSB", "CWB"
    falhas = 0

    def verificar(condicao, descricao):
        nonlocal falhas
        print(f"  [{'OK' if condicao else 'FALHOU'}] {descricao}")
        if not condicao:
            falhas += 1

    def caminho_valido(rede, caminho):
        return all(b in rede[a] for a, b in zip(caminho, caminho[1:]))

    print(f"Rede: {len(GRAFO)} vértices, {len(ARESTAS)} arestas (não dirigida)")
    verificar(all(a in GRAFO[b] for a in GRAFO for b in GRAFO[a]),
              "lista de adjacência simétrica (toda aresta nos dois sentidos)")

    # Experimento A — rede completa
    b, d = comparar(GRAFO, O, D, "A) Rede completa")
    verificar(len(b) - 1 == 3, "BFS encontra o mínimo de 3 arestas")
    verificar(caminho_valido(GRAFO, b) and caminho_valido(GRAFO, d), "caminhos usam apenas arestas existentes")

    # Experimento B — um bloqueio
    r1 = bloquear_conexoes(GRAFO, [("CNF", "GRU")])
    b, d = comparar(r1, O, D, "B) Bloqueio CNF–GRU")
    verificar(len(b) - 1 == 4, "desvio exige 4 arestas (+1)")
    verificar("GRU" in GRAFO["CNF"], "rede original NÃO foi alterada")

    # Experimento C — dois bloqueios: ainda conexo
    r2 = bloquear_conexoes(GRAFO, [("CNF", "GRU"), ("GRU", "CWB")])
    b, d = comparar(r2, O, D, "C1) Bloqueios CNF–GRU e GRU–CWB")
    verificar(b is not None and "FLN" in b, "ainda há caminho, pelo corredor Sul (POA e FLN)")

    # Experimento C — três bloqueios: Curitiba fica isolada
    r3 = bloquear_conexoes(GRAFO, [("CNF", "GRU"), ("GRU", "CWB"),
                                   ("FLN", "CWB")])
    b, d = comparar(r3, O, D, "C2) Bloqueios CNF–GRU, GRU–CWB e FLN–CWB")
    verificar(b is None and d is None, "sem caminho: CWB ficou sem arestas (grafo desconexo)")

    # Ponte: Palmas só tem uma ligação
    r4 = bloquear_conexoes(GRAFO, [("BSB", "SSA")])
    b, d = comparar(r4, O, "SSA", "Extra) Bloqueio da ponte BSB–SSA")
    verificar(b is None, "remover uma ponte desconecta o vértice folha (SSA)")

    # Experimento D — ordem dos vizinhos
    rD = reordenar_vizinhos(GRAFO, "BSB", ["CGB", "GYN", "CNF", "SSA"])
    b0, d0 = bfs(GRAFO, O, D), dfs(GRAFO, O, D)
    b1, d1 = comparar(rD, O, D, "D) Mesma rede, vizinhos de BSB reordenados (CGB primeiro)")
    verificar(d0 != d1, "DFS muda de caminho só pela ordem dos vizinhos")
    verificar(len(b0) == len(b1), "BFS mantém o mesmo número mínimo de arestas")

    # Casos-limite
    print("\n=== Casos-limite ===")
    verificar(bfs(GRAFO, O, O) == [O] and dfs(GRAFO, O, O) == [O], "origem = destino retorna [origem] (0 arestas)")
    verificar(resolver_nome(GRAFO, "  brasilia ") == "BSB" and resolver_nome(GRAFO, "bsb") == "BSB", "entrada por sigla ou cidade, sem acento/maiúsculas, é aceita")
    verificar(resolver_nome(GRAFO, "Manaus") is None, "aeroporto inexistente é rejeitado")
    verificar(bfs(GRAFO, O, "Manaus") is None, "busca para vértice ausente retorna None sem erro")

    print(f"\nResultado: {'TODOS OS TESTES PASSARAM' if falhas == 0 else f'{falhas} FALHA(S)'}")
    return falhas


if __name__ == "__main__":
    if "--testes" in sys.argv:
        sys.exit(1 if executar_testes() else 0)
    modo_interativo()
