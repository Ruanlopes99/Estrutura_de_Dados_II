#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define PENDENTE 0
#define APROVADA 1
#define REJEITADA 2

typedef struct {
    unsigned long id;
    char data[11];
    char hora[9];
    char bandeira[20];
    char categoria[30];
    float valor;
    int status;
    int fraude;
} Transacao;

typedef struct NoFila {
    Transacao t;
    struct NoFila *prox;
} NoFila;

typedef struct {
    NoFila *inicio;
    NoFila *fim;
} Fila;

typedef struct NoPilha {
    Transacao t;
    struct NoPilha *prox;
} NoPilha;

typedef struct NoLista {
    Transacao t;
    struct NoLista *prox;
} NoLista;

typedef struct {
    NoPilha *topo;
    int quantidade;
} Pilha;

typedef struct {
    NoLista *inicio;
} ListaRisco;

void csv_para_binario(const char *csv, const char *bin);
int carregar(const char *bin, Transacao **v);
int cmp_data_hora(const void *a, const void *b);
int cmp_valor(const void *a, const void *b);
void selection_sort_data(Transacao *v, int n);
void enfileirar(Fila *f, Transacao t);
void empilhar(Pilha *p, Transacao t);
Transacao desempilhar(Pilha *p);
void inserirRisco(ListaRisco *l, Transacao t);
int verificaRisco(Transacao t, float limite);
void gerarRelatorio(
    Transacao *v,
    int n,
    Fila fila,
    Pilha pilha,
    ListaRisco risco,
    double tempo_qsort,
    double tempo_selection,
    int aprovadas,
    int rejeitadas
);
void imprimirFila(Fila f);
void imprimirPilha(Pilha p);
void imprimirLista(ListaRisco l);
void limparValor(char *str);
void liberarFila(Fila *f);
void liberarPilha(Pilha *p);
void liberarLista(ListaRisco *l);

int main(){
    Transacao *vetor = NULL;
    Transacao *vetor_selection = NULL;

    int n;
    int aprovadas = 0;
    int rejeitadas = 0;

    float LIMITE_RISCO = 100.0;

    Fila fila = {NULL, NULL};

    Pilha pilha;
    pilha.topo = NULL;
    pilha.quantidade = 0;

    ListaRisco risco;
    risco.inicio = NULL;

    printf("Convertendo CSV para binario...\n");
    csv_para_binario("CreditCardData.csv", "transacoes.dat");

    printf("Carregando arquivo binario...\n");
    n = carregar("transacoes.dat", &vetor);

    if(n == 0){
        printf("Nenhum registro encontrado.\n");
        return 1;
    }

    vetor_selection = malloc(n * sizeof(Transacao));

    if(vetor_selection == NULL){
        printf("Erro de memoria.\n");
        free(vetor);
        return 1;
    }
    memcpy(vetor_selection, vetor, n * sizeof(Transacao));

    printf("Ordenando com qsort...\n");

    clock_t inicio_qsort = clock();
    qsort(vetor, n, sizeof(Transacao), cmp_data_hora);
    clock_t fim_qsort = clock();

    double tempo_qsort = (double)(fim_qsort - inicio_qsort) / CLOCKS_PER_SEC;

    printf("Ordenando com selection sort...\n");

    int n_selection = n;
    if(n_selection > 10000)    {
        n_selection = 10000;
    }
    clock_t inicio_selection = clock();
    selection_sort_data(vetor_selection, n_selection);
    clock_t fim_selection = clock();

    double tempo_selection = (double)(fim_selection - inicio_selection) / CLOCKS_PER_SEC;

    printf("Distribuindo transacoes nas estruturas...\n");

    for(int i = 0; i < n; i++){
        Transacao t = vetor[i];

        if(verificaRisco(t, LIMITE_RISCO)){
            inserirRisco(&risco, t);
        }

        if(t.fraude == 0){
            t.status = APROVADA;
            vetor[i].status = APROVADA;
            enfileirar(&fila, t);
            aprovadas++;
        }
        else{
            t.status = REJEITADA;
            vetor[i].status = REJEITADA;
            empilhar(&pilha, t);
            rejeitadas++;
        }
    }

    printf("\nExecutando teste de undo...\n");

    if(pilha.topo != NULL){
        Transacao recuperada = desempilhar(&pilha);
        printf("Transacao recuperada da pilha: ID %lu | Valor: R$ %.2f\n", recuperada.id, recuperada.valor);
    }
    else{
        printf("Pilha vazia. Nenhuma transacao para desfazer.\n");
    }

    gerarRelatorio(
        vetor,
        n,
        fila,
        pilha,
        risco,
        tempo_qsort,
        tempo_selection,
        aprovadas,
        rejeitadas
    );

    imprimirFila(fila);
    imprimirPilha(pilha);
    imprimirLista(risco);
    liberarFila(&fila);
    liberarPilha(&pilha);
    liberarLista(&risco);

    free(vetor);
    free(vetor_selection);

    printf("\nFim do programa.\n");
    return 0;
}

void csv_para_binario(const char *csv, const char *bin){ 
    FILE *f_csv = fopen(csv, "r");
    FILE *f_bin = fopen(bin, "wb");

    if (f_csv == NULL || f_bin == NULL) {
        printf("Erro ao abrir arquivos.\n");
        exit(1);
    }
    char linha[512];
    if(fgets(linha, sizeof(linha), f_csv) == NULL){
        printf("Arquivo CSV vazio.\n");
        fclose(f_csv);
        fclose(f_bin);
        return;
    }
    while (fgets(linha, sizeof(linha), f_csv)) {
            char *tokens[20];
            int idx = 0;
            char *p = strtok(linha, ";");

            while (p != NULL && idx < 20) {
                p[strcspn(p, "\r\n")] = '\0';
                tokens[idx++] = p;
                p = strtok(NULL, ";");
            }
            if (idx < 16)
                continue;

            Transacao t;

            char id_limpo[30];
            int j = 0;

            for(int i = 0; tokens[0][i] != '\0'; i++) {
                if(isdigit(tokens[0][i])) {
                    id_limpo[j++] = tokens[0][i];
                }
            }

            id_limpo[j] = '\0';
            t.id = strtoul(id_limpo, NULL, 10);
            if(j == 0)
                continue;
            strncpy(t.data, tokens[1], 10);
            t.data[10] = '\0';
            strncpy(t.hora, tokens[3], 8);
            t.hora[8] = '\0';
            strncpy(t.bandeira, tokens[4], 19);
            t.bandeira[19] = '\0';
            strncpy(t.categoria, tokens[8], 29);
            t.categoria[29] = '\0';

            char valor[64];

            strncpy(valor, tokens[6], sizeof(valor)-1);
            valor[sizeof(valor)-1] = '\0';

            limparValor(valor);

            t.valor = atof(valor);
            if(t.valor <= 0)
                continue;
            t.fraude = atoi(tokens[15]);
            t.status = PENDENTE;

            fwrite(&t, sizeof(Transacao), 1, f_bin);
    }
    fclose(f_csv);
    fclose(f_bin);
}

int carregar(const char *bin, Transacao **v){
    FILE *f = fopen(bin, "rb");
    if(f == NULL)
        return 0;
    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    int n = tamanho / sizeof(Transacao);
    rewind(f);
    *v = malloc(n * sizeof(Transacao));
    if(*v == NULL){
        fclose(f);
        printf("Erro de memoria.\n");
        exit(1);
    }
    fread(*v, sizeof(Transacao), n, f);
    fclose(f);
    return n;
}

int cmp_valor(const void *a, const void *b){
    Transacao *t1 = (Transacao *)a;
    Transacao *t2 = (Transacao *)b;

    if(t1->valor < t2->valor)
        return 1;
    if(t1->valor > t2->valor)
        return -1;
    return 0;
}
int cmp_data_hora(const void *a, const void *b){
    Transacao *t1 = (Transacao *)a;
    Transacao *t2 = (Transacao *)b;

    int d1, y1;
    int d2, y2;
    int h1;
    int h2;

    sscanf(t1->data, "%d-%*[^-]-%d", &d1, &y1);
    sscanf(t2->data, "%d-%*[^-]-%d", &d2, &y2);
    sscanf(t1->hora, "%d", &h1);
    sscanf(t2->hora, "%d", &h2);

    if(y1 != y2)
        return y1 - y2;
    if(d1 != d2)
        return d1 - d2;
    return h1 - h2;
}

void selection_sort_data(Transacao *v, int n){
    int i, j, menor;
    for(i = 0; i < n - 1; i++){
        menor = i;
        for(j = i + 1; j < n; j++)
        {
            if(cmp_data_hora(&v[j], &v[menor]) < 0){
                menor = j;
            }
        }
        if(menor != i){
            Transacao temp = v[i];
            v[i] = v[menor];
            v[menor] = temp;
        }
    }
}

void enfileirar(Fila *f, Transacao t){
    NoFila *novo = malloc(sizeof(NoFila));
    if(novo == NULL){
        printf("Erro de memoria.\n");
        exit(1);
    }
    novo->t = t;
    novo->prox = NULL;
    if(f->fim != NULL){
        f->fim->prox = novo;
    }else{
        f->inicio = novo;
    }
    f->fim = novo;
}

void empilhar(Pilha *p, Transacao t){
    NoPilha *novo = malloc(sizeof(NoPilha));
    if(novo == NULL){
        printf("Erro de memoria.\n");
        exit(1);
    }

    novo->t = t;
    novo->prox = p->topo;
    p->topo = novo;
    p->quantidade++;
}

Transacao desempilhar(Pilha *p){
    if(p->topo == NULL){
        printf("Pilha vazia.\n");
        exit(1);
    }

    NoPilha *temp = p->topo;
    Transacao t = temp->t;

    p->topo = temp->prox;
    free(temp);
    p->quantidade--;
    return t;
}

int verificaRisco(Transacao t, float limite){
    int h;
    sscanf(t.hora, "%d", &h);
    return (t.valor > limite && (h >= 23 || h < 5));
}

void inserirRisco(ListaRisco *l, Transacao t){
    NoLista *novo = malloc(sizeof(NoLista));
    if(novo == NULL){
        printf("Erro de memoria.\n");
        exit(1);
    }
    novo->t = t;
    novo->prox = l->inicio;
    l->inicio = novo;
}

void imprimirFila(Fila f){
    NoFila *aux = f.inicio;
    printf("\n==================================================\n");
    printf("FILA - TRANSACOES APROVADAS\n");
    printf("==================================================\n");

    if(aux == NULL){
        printf("Fila vazia.\n");
        return;
    }

    while(aux != NULL){
        printf("ID: %lu | Data: %s | Hora: %s | Valor: R$ %.2f | Bandeira: %s | Categoria: %s\n", aux->t.id, aux->t.data, aux->t.hora, aux->t.valor, aux->t.bandeira, aux->t.categoria);
        aux = aux->prox;
    }
}

void imprimirPilha(Pilha p)
{
    NoPilha *aux = p.topo;
    printf("\n==================================================\n");
    printf("PILHA - TRANSACOES REJEITADAS\n");
    printf("==================================================\n");

    if(aux == NULL){
        printf("Pilha vazia.\n");
        return;
    }

    while(aux != NULL){
        printf("ID: %lu | Data: %s | Hora: %s | Valor: R$ %.2f | Bandeira: %s | Categoria: %s\n", aux->t.id, aux->t.data, aux->t.hora, aux->t.valor, aux->t.bandeira, aux->t.categoria);
        aux = aux->prox;
    }
}

void imprimirLista(ListaRisco l)
{
    NoLista *aux = l.inicio;
    printf("\n==================================================\n");
    printf("LISTA DE TRANSACOES SUSPEITAS\n");
    printf("==================================================\n");

    if(aux == NULL){
        printf("Lista vazia.\n");
        return;
    }

    while(aux != NULL){
        printf("ID: %lu | Data: %s | Hora: %s | Valor: R$ %.2f | Bandeira: %s | Categoria: %s\n", aux->t.id, aux->t.data, aux->t.hora, aux->t.valor, aux->t.bandeira, aux->t.categoria);
        aux = aux->prox;
    }
}

void liberarFila(Fila *f)
{
    NoFila *aux = f->inicio;

    while(aux != NULL){
        NoFila *temp = aux;
        aux = aux->prox;
        free(temp);
    }
    f->inicio = NULL;
    f->fim = NULL;
}

void liberarPilha(Pilha *p)
{
    NoPilha *aux = p->topo;

    while(aux != NULL){
        NoPilha *temp = aux;
        aux = aux->prox;
        free(temp);
    }
    p->topo = NULL;
    p->quantidade = 0;
}

void liberarLista(ListaRisco *l)
{
    NoLista *aux = l->inicio;

    while(aux != NULL){
        NoLista *temp = aux;
        aux = aux->prox;
        free(temp);
    }
    l->inicio = NULL;
}

void gerarRelatorio(
    Transacao *v,
    int n,
    Fila fila,
    Pilha pilha,
    ListaRisco risco,
    double tempo_qsort,
    double tempo_selection,
    int aprovadas,
    int rejeitadas
){
    FILE *relatorio = fopen("relatorio_consolidado.txt", "w");

    if(relatorio == NULL){
        printf("Erro ao gerar relatorio.\n");
        return;
    }

    float totalVisa = 0;
    float totalMaster = 0;
    int qtdFila = 0;
    int qtdRisco = 0;

    Transacao maior = v[0];

    for(int i = 0; i < n; i++){
        if(strstr(v[i].bandeira, "Visa") != NULL){
            totalVisa += v[i].valor;
        }

        if(strstr(v[i].bandeira, "Master") != NULL){
            totalMaster += v[i].valor;
        }

        if(v[i].valor > maior.valor){
            maior = v[i];
        }
    }

    NoFila *auxFila = fila.inicio;

    while(auxFila != NULL){
        qtdFila++;
        auxFila = auxFila->prox;
    }

    NoLista *auxLista = risco.inicio;

    while(auxLista != NULL){
        qtdRisco++;
        auxLista = auxLista->prox;
    }

    fprintf(relatorio, "==================================================\n");
    fprintf(relatorio, "RELATORIO DE PERFORMANCE E AUDITORIA DE TRANSACOES\n");
    fprintf(relatorio, "==================================================\n");

    fprintf(relatorio, "\n1. DESEMPENHO DA ORDENACAO\n");
    fprintf(relatorio, "  - qsort 100000 registros: %.6lf segundos\n", tempo_qsort);
    fprintf(relatorio, "  - selection sort 10000 registros: %.6lf segundos\n", tempo_selection);

    if(tempo_qsort < tempo_selection){
        fprintf(relatorio, "  - Algoritmo mais rapido: qsort\n");
    }
    else{
        fprintf(relatorio, "  - Algoritmo mais rapido: selection sort\n");
    }

    fprintf(relatorio, "\n2. METRICAS FINANCEIRAS\n");
    fprintf(relatorio, "  - Total Visa: R$ %.2f\n", totalVisa);
    fprintf(relatorio, "  - Total Mastercard: R$ %.2f\n", totalMaster);
    fprintf(relatorio, "  - Maior transacao:\n");
    fprintf(relatorio, "      ID: %lu | Data: %s | Hora: %s | Valor: R$ %.2f | Categoria: %s\n", maior.id, maior.data, maior.hora, maior.valor, maior.categoria);

    fprintf(relatorio, "\n3. STATUS DAS ESTRUTURAS\n");
    fprintf(relatorio, "  - Fila FIFO: %d transacoes\n", qtdFila);
    fprintf(relatorio, "  - Pilha Rejeicoes: %d transacoes\n", pilha.quantidade);
    fprintf(relatorio, "  - Lista de Suspeitas: %d transacoes\n", qtdRisco);

    fprintf(relatorio, "\n4. STATUS DAS TRANSACOES\n");
    fprintf(relatorio, "  - Aprovadas: %d\n", aprovadas);
    fprintf(relatorio, "  - Rejeitadas: %d\n", rejeitadas);

    if(n > 0){
        fprintf(relatorio, "  - Percentual aprovadas: %.2f%%\n", (aprovadas * 100.0) / n);
        fprintf(relatorio, "  - Percentual rejeitadas: %.2f%%\n", (rejeitadas * 100.0) / n);
    }

    fprintf(relatorio, "\n==================================================\n");
    fprintf(relatorio, "LISTA DE TRANSACOES SUSPEITAS\n");
    fprintf(relatorio, "==================================================\n");
    auxLista = risco.inicio;

    while(auxLista != NULL){
        fprintf(relatorio,
                "[ID: %lu] - Data: %s %s - Valor: R$ %.2f - Categoria: %s - Bandeira: %s\n", auxLista->t.id, auxLista->t.data, auxLista->t.hora, auxLista->t.valor, auxLista->t.categoria, auxLista->t.bandeira);
        auxLista = auxLista->prox;
    }
    fclose(relatorio);
    printf("\nRelatorio gerado com sucesso!\n");
}

void limparValor(char *str)
{
    char temp[64];
    int j = 0;

    for(int i = 0; str[i] != '\0'; i++){
        if(isdigit((unsigned char)str[i]) || str[i] == '.' || str[i] == ','){
            temp[j++] = str[i];
        }
    }

    temp[j] = '\0';

    for(int i = 0; temp[i] != '\0'; i++){
        if(temp[i] == ','){
            temp[i] = '.';
        }
    }
    strcpy(str, temp);
}