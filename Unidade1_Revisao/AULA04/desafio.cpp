#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

// Estrutura do Paciente
typedef struct Paciente {
  char nome[50];
  struct Paciente *prox;
} Paciente;

// Estrutura da Fila Simples
typedef struct Fila {
  Paciente *inicio;
  Paciente *fim;
} Fila;

// Função para criar/inicializar uma fila vazia
Fila* criar_fila() {
  Fila *f = (Fila*) malloc(sizeof(Fila));
  f->inicio = NULL;
  f->fim = NULL;
  return f;
}

// Inserção padrão no final da fila (Garante Ordem de Chegada O(1))
void enfileirar(Fila *f, char nome[]) {
  Paciente *novo = (Paciente*) malloc(sizeof(Paciente));
  strcpy(novo->nome, nome);
  novo->prox = NULL;

  if (f->inicio == NULL) {
    f->inicio = novo;
    f->fim = novo;
  } else {
    f->fim->prox = novo;
    f->fim = novo;
  }
}

// Remoção no início da fila (O(1))
char* desenfileirar(Fila *f) {
	
  if (f->inicio == NULL) return NULL;
  Paciente *aux = f->inicio;
  static char nome_atendido[50];
  strcpy(nome_atendido, aux->nome);
  f->inicio = f->inicio->prox;
  if (f->inicio == NULL) {
    f->fim = NULL;
  }
  free(aux);
  return nome_atendido;
}

// Função para exibir uma fila específica
void imprimir_fila(Fila *f, char nome_fila[]) {
  Paciente *atual = f->inicio;
  printf("Fila %s: ", nome_fila);
  
  if (atual == NULL) {
    printf("[Vazia]\n");
    return;
  }

  while (atual != NULL) {
    printf("[%s] -> ", atual->nome);
    atual = atual->prox;
  }
  printf("NULL\n");
}

int main() {
	setlocale(LC_ALL, "Portuguese");
	
  // Criação das 3 filas independentes de triagem
  Fila *fila_vermelha = criar_fila();
  Fila *fila_amarela = criar_fila();
  Fila *fila_rotina = criar_fila();

  int opcao;

  char nome[50];
  do {
    printf("\n=== SISTEMA DE TRIAGEM HOSPITALAR ===\n");
    printf("1 - Triagem: Emergência (VERMELHA)\n");
    printf("2 - Triagem: Urgência (AMARELA)\n");
    printf("3 - Triagem: Rotina / Efetivo (VERDE/AZUL)\n");
    printf("4 - Chamar Próximo Paciente\n");
    printf("5 - Visualizar Painel de Filas\n");
    printf("0 - Encerrar Expediente\n");
    printf("Escolha uma opção: ");
    scanf("%d", &opcao);
    getchar(); // Limpa o buffer

    switch (opcao) {
      case 1:
        printf("Nome do Paciente (VERMELHA): ");
        fgets(nome, 50, stdin); nome[strcspn(nome, "\n")] = 0;
        enfileirar(fila_vermelha, nome);
        printf("[OK] Paciente gravado na Emergência Imediata.\n");
        break;

      case 2:
        printf("Nome do Paciente (AMARELA): ");
        fgets(nome, 50, stdin); nome[strcspn(nome, "\n")] = 0;
        enfileirar(fila_amarela, nome);
        printf("[OK] Paciente gravado na Urgência.\n");
        break;

      case 3:
        printf("Nome do Paciente (ROTINA): ");
        fgets(nome, 50, stdin); nome[strcspn(nome, "\n")] = 0;
        enfileirar(fila_rotina, nome);
        printf("[OK] Paciente gravado na Rotina por ordem de chegada.\n");
        break;
        
      case 4:
        printf("\n>> CHAMADA DE ATENDIMENTO <<\n");
        // Regra de Negócio: Verifica rigorosamente a hierarquia das filas
        if (fila_vermelha->inicio != NULL) {
          printf("ATENÇÃO: Emergência Vermelha! Chamando: %s\n", desenfileirar(fila_vermelha));
        } else if (fila_amarela->inicio != NULL) {
          printf("Urgência Amarela. Chamando: %s\n", desenfileirar(fila_amarela));
        } else if (fila_rotina->inicio != NULL) {
          printf("Atendimento de Rotina. Chamando: %s\n", desenfileirar(fila_rotina));
        } else {
          printf("Nenhum paciente aguardando em nenhuma das filas.\n");
        }
        break;
        
      case 5:
        printf("\n=== PAINEL DE ESPERA ENFERMARIA ===\n");
        imprimir_fila(fila_vermelha, "VERMELHA (Imediato)");
        imprimir_fila(fila_amarela, "AMARELA (Urgência)");
        imprimir_fila(fila_rotina, "ROTINA (Ordem Chegada)");
        printf("===================================\n");
        break;
        
      case 0:
        printf("Encerrando o sistema...\n");
        break;
      default:
        printf("Opção inválida!\n");
    }
  } while (opcao != 0);
  return 0;
}
