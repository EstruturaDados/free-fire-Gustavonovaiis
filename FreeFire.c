#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMP 10 // capacidade da mochila 10/10 como no exemplo

// Estrutura de um componente da torre
typedef struct {
    char nome[30];
    char tipo[20];
    int prioridade; // 1 (mais urgente) a 10 (menos urgente)
} Componente;

// Remove '\n' ao final de strings lidas com fgets
static void trimNewline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n > 0 && s[n - 1] == '\n') s[n - 1] = '\0';
}

// Leitura segura de string com prompt usando fgets
static void lerString(const char *prompt, char *buffer, size_t tam) {
    for (;;) {
        printf("%s", prompt);
        if (fgets(buffer, (int)tam, stdin) == NULL) {
            // Entrada encerrada
            buffer[0] = '\0';
            return;
        }
        trimNewline(buffer);
        // Evita aceitar vazio
        if (buffer[0] != '\0') break;
        printf("Entrada vazia. Tente novamente.\n");
    }
}

// Lê inteiro em intervalo [min, max] usando fgets + strtol
static int lerIntIntervalo(const char *prompt, int min, int max) {
    char linha[64];
    for (;;) {
        long val;
        char *endp;
        printf("%s", prompt);
        if (!fgets(linha, sizeof(linha), stdin)) {
            // EOF: tenta novamente
            clearerr(stdin);
            continue;
        }
        trimNewline(linha);
        if (linha[0] == '\0') {
            printf("Entrada vazia. Tente novamente.\n");
            continue;
        }
        val = strtol(linha, &endp, 10);
        if (*endp != '\0') {
            printf("Valor invalido. Digite um numero inteiro.\n");
            continue;
        }
        if (val < min || val > max) {
            printf("Fora do intervalo [%d, %d].\n", min, max);
            continue;
        }
        return (int)val;
    }
}

// Exibe vetor de componentes formatado
static void mostrarComponentes(Componente v[], int n) {
    printf("\n--- Componentes ---\n");
    for (int i = 0; i < n; i++) {
        printf("%2d) Nome: %-28s | Tipo: %-18s | Prioridade: %d\n",
               i + 1, v[i].nome, v[i].tipo, v[i].prioridade);
    }
    printf("-------------------\n");
}

// Bubble Sort por nome (string). Conta apenas comparacoes de nomes.
static void bubbleSortNome(Componente v[], int n, unsigned long long *comparacoes) {
    if (comparacoes) *comparacoes = 0ULL;
    for (int i = 0; i < n - 1; i++) {
        int trocou = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (comparacoes) (*comparacoes)++;
            if (strcmp(v[j].nome, v[j + 1].nome) > 0) {
                Componente tmp = v[j];
                v[j] = v[j + 1];
                v[j + 1] = tmp;
                trocou = 1;
            }
        }
        if (!trocou) break; // Otimizacao: ja ordenado
    }
}

// Insertion Sort por tipo (string). Conta comparacoes de tipos.
static void insertionSortTipo(Componente v[], int n, unsigned long long *comparacoes) {
    if (comparacoes) *comparacoes = 0ULL;
    for (int i = 1; i < n; i++) {
        Componente chave = v[i];
        int j = i - 1;
        // Conta apenas comparacoes entre strings de tipo
        while (j >= 0) {
            if (comparacoes) (*comparacoes)++;
            if (strcmp(v[j].tipo, chave.tipo) > 0) {
                v[j + 1] = v[j];
                j--;
            } else {
                break;
            }
        }
        v[j + 1] = chave;
    }
}

// Selection Sort por prioridade (int). Conta comparacoes de prioridade.
static void selectionSortPrioridade(Componente v[], int n, unsigned long long *comparacoes) {
    if (comparacoes) *comparacoes = 0ULL;
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            if (comparacoes) (*comparacoes)++;
            if (v[j].prioridade < v[minIdx].prioridade) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            Componente tmp = v[i];
            v[i] = v[minIdx];
            v[minIdx] = tmp;
        }
    }
}

// Busca binaria por nome. Retorna indice ou -1. Conta comparacoes de nomes.
static int buscaBinariaPorNome(Componente v[], int n, const char *chave, unsigned long long *comparacoes) {
    int ini = 0, fim = n - 1;
    if (comparacoes) *comparacoes = 0ULL;
    while (ini <= fim) {
        int meio = ini + (fim - ini) / 2;
        int cmp = strcmp(v[meio].nome, chave);
        if (comparacoes) (*comparacoes)++;
        if (cmp == 0) return meio;
        if (cmp < 0) ini = meio + 1;
        else fim = meio - 1;
    }
    return -1;
}

// Mede tempo de execucao (em segundos) de um algoritmo de ordenacao
static double medirTempo(void (*alg)(Componente*, int, unsigned long long*),
                         Componente *v, int n, unsigned long long *comparacoes) {
    clock_t ini = clock();
    alg(v, n, comparacoes);
    clock_t fim = clock();
    return (double)(fim - ini) / CLOCKS_PER_SEC;
}

// Mantém o cabeçalho e o menu exatamente no padrão do print
static void mostrarMenu(int qtd, int capacidade, int ordenadoPorNome) {
    printf("\n===========================================================\n");
    printf("    GN PLANO DE FUGA - CODIGO DA ILHA (NIVEL MESTRE)  \n");
    printf("=============================================================\n");
    printf("Itens na Mochila: %d/%d\n", qtd, capacidade);
    printf("Status da Ordenacao por Nome: %s\n\n", ordenadoPorNome ? "ORDENADO" : "NAO ORDENADO");
    printf("1. Adicionar Componente\n");
    printf("2. Descartar Componente\n");
    printf("3. Listar Componentes (Inventario)\n");
    printf("4. Organizar Mochila (Ordenar Componentes)\n");
    printf("5. Busca Binaria por Componente-Chave (por nome)\n");
    printf("0. ATIVAR TORRE DE FUGA (Sair)\n");
    printf("==============================================================\n");
}

static void pausar(void) {
    printf("Pressione ENTER para continuar...");
    fflush(stdout);
    int c;
    // limpa até o próximo '\n'
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Adiciona um componente (1 por vez)
static void adicionarComponente(Componente comps[], int *qtd) {
    if (*qtd >= MAX_COMP) {
        printf("Mochila cheia! Remova algo antes de adicionar.\n");
        return;
    }
    printf("\n-- Adicionar Componente --\n");
    lerString("Nome (ate 29 chars): ", comps[*qtd].nome, sizeof(comps[*qtd].nome));
    lerString("Tipo (controle/suporte/propulsao, ate 19 chars): ", comps[*qtd].tipo, sizeof(comps[*qtd].tipo));
    comps[*qtd].prioridade = lerIntIntervalo("Prioridade (1 a 10): ", 1, 10);
    (*qtd)++;
    printf("Componente adicionado!\n");
}

// Remove por indice
static void descartarComponente(Componente comps[], int *qtd) {
    if (*qtd == 0) {
        printf("Mochila vazia. Nada a descartar.\n");
        return;
    }
    mostrarComponentes(comps, *qtd);
    int idx = lerIntIntervalo("Informe o numero do item para descartar: ", 1, *qtd) - 1;
    for (int i = idx; i < *qtd - 1; i++) {
        comps[i] = comps[i + 1];
    }
    (*qtd)--;
    printf("Item descartado.\n");
}

// Submenu para escolher o algoritmo de ordenacao
static void submenuOrdenar(Componente comps[], int qtd, int *ordenadoPorNome) {
    if (qtd <= 1) {
        printf("Poucos itens. Nao ha necessidade de ordenar.\n");
        return;
    }
    printf("\n-- Organizar Mochila (Ordenar Componentes) --\n");
    printf("1) Bubble Sort por NOME\n");
    printf("2) Insertion Sort por TIPO\n");
    printf("3) Selection Sort por PRIORIDADE\n");
    printf("0) Voltar\n");
    int opc = lerIntIntervalo("Escolha: ", 0, 3);

    if (opc == 1) {
        unsigned long long compsCount = 0;
        double tempo = medirTempo(bubbleSortNome, comps, qtd, &compsCount);
        *ordenadoPorNome = 1;
        printf("\nOrdenado por NOME (Bubble Sort).\nComparacoes: %llu | Tempo: %.6fs\n", compsCount, tempo);
        mostrarComponentes(comps, qtd);
    } else if (opc == 2) {
        unsigned long long compsCount = 0;
        double tempo = medirTempo(insertionSortTipo, comps, qtd, &compsCount);
        *ordenadoPorNome = 0;
        printf("\nOrdenado por TIPO (Insertion Sort).\nComparacoes: %llu | Tempo: %.6fs\n", compsCount, tempo);
        mostrarComponentes(comps, qtd);
    } else if (opc == 3) {
        unsigned long long compsCount = 0;
        double tempo = medirTempo(selectionSortPrioridade, comps, qtd, &compsCount);
        *ordenadoPorNome = 0;
        printf("\nOrdenado por PRIORIDADE (Selection Sort).\nComparacoes: %llu | Tempo: %.6fs\n", compsCount, tempo);
        mostrarComponentes(comps, qtd);
    }
}

int main(void) {
    Componente comps[MAX_COMP];
    int qtd = 0;                 // mochila inicia vazia
    int ordenadoPorNome = 0;     // status exibido no cabecalho
    int executando = 1;

    while (executando) {
        mostrarMenu(qtd, MAX_COMP, ordenadoPorNome);
        int opc = lerIntIntervalo("Escolha uma opcao: ", 0, 5);

        if (opc == 1) {
            adicionarComponente(comps, &qtd);
            ordenadoPorNome = 0; // perdeu a ordenacao por nome
            pausar();
        } else if (opc == 2) {
            descartarComponente(comps, &qtd);
            ordenadoPorNome = 0;
            pausar();
        } else if (opc == 3) {
            if (qtd == 0) printf("Mochila vazia.\n");
            else mostrarComponentes(comps, qtd);
            pausar();
        } else if (opc == 4) {
            submenuOrdenar(comps, qtd, &ordenadoPorNome);
            pausar();
        } else if (opc == 5) {
            if (!ordenadoPorNome) {
                printf("Status: NAO ORDENADO por nome.\nUse a opcao 4 -> Bubble Sort por NOME antes de buscar.\n");
            } else if (qtd == 0) {
                printf("Mochila vazia. Nada para buscar.\n");
            } else {
                char chave[30];
                lerString("Digite o NOME do componente-chave: ", chave, sizeof(chave));
                unsigned long long compsCount = 0;
                clock_t t0 = clock();
                int idx = buscaBinariaPorNome(comps, qtd, chave, &compsCount);
                clock_t t1 = clock();
                double tempo = (double)(t1 - t0) / CLOCKS_PER_SEC;

                if (idx >= 0) {
                    printf("\nComponente encontrado na posicao %d.\n", idx + 1);
                    printf("Confirmacao: Nome: %s | Tipo: %s | Prioridade: %d\n",
                           comps[idx].nome, comps[idx].tipo, comps[idx].prioridade);
                } else {
                    printf("\nComponente nao encontrado.\n");
                }
                printf("Comparacoes (busca): %llu | Tempo: %.6fs\n", compsCount, tempo);
            }
            pausar();
        } else if (opc == 0) {
            printf("\nATIVANDO TORRE DE FUGA... Boa sorte!\n");
            executando = 0;
        }
    }
    return 0;
}

