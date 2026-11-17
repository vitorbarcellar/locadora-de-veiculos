
/* ============================================================================
 * PROJETO FINAL: SISTEMA DE GESTAO DE LOCADORA DE VEICULOS (ARQUIVO UNICO)
 * ============================================================================
 * Integrantes:
 *   - Membro 1 (Vitor): Modulo de Veiculos & Persistencia em Arquivos
 *   - Membro 2 (Marco): Modulo de Clientes & Menu Principal
 *   - Membro 3 (Giovana): Modulo de Locacoes, Devolucoes & Relatorios
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

/* ============================================================================
 * CONSTANTES E DEFINICOES GLOBAIS
 * ============================================================================ */
#define MAX_VEICULOS 100
#define MAX_CLIENTES 100
#define MAX_LOCACOES 200

#define STATUS_DISPONIVEL 1
#define STATUS_ALUGADO    0

#define ARQUIVO_VEICULOS "veiculos.bin"
#define ARQUIVO_CLIENTES "clientes.bin"
#define ARQUIVO_LOCACOES "locacoes.bin"

/* ============================================================================
 * ESTRUTURAS DE DADOS (STRUCTS)
 * ============================================================================ */

/* Membro 1 (Vitor): Struct Veiculo */
typedef struct {
    char placa[20];
    char modelo[50];
    char marca[50];
    int ano;
    double valorDiaria;
    int status; /* 1 = Disponivel (STATUS_DISPONIVEL), 0 = Alugado (STATUS_ALUGADO) */
} Veiculo;

/* Membro 2 (Marco): Struct Cliente */
typedef struct {
    char nome[100];
    char cnh[20];
    char telefone[20];
} Cliente;

/* Membro 3 (Giovana): Struct Locacao */
typedef struct {
    char placa[20];
    char cnh[20];
    int dias;
    double valorTotal;
    int ativa; /* 1 = Ativa (Alugado), 0 = Devolvida */
} Locacao;

/* ============================================================================
 * PROTOTIPOS DAS FUNCOES
 * ============================================================================ */

/* Funcoes Auxiliares de Entrada e Formatacao */
void limparBuffer(void);
void lerTexto(const char *msg, char *dest, int tam);
int lerInteiro(const char *msg);
double lerDouble(const char *msg);
void imprimirReais(double valor);
int placasIguais(const char *a, const char *b);

/* Modulo de Veiculos - Membro 1 (Vitor) */
void cadastrarVeiculo(Veiculo frota[], int *totalVeiculos);
void listarVeiculos(const Veiculo frota[], int totalVeiculos);
void buscarVeiculoMenu(const Veiculo frota[], int totalVeiculos);
void salvarVeiculos(const Veiculo frota[], int totalVeiculos);
void carregarVeiculos(Veiculo frota[], int *totalVeiculos);

/* Modulo de Clientes - Membro 2 (Marco) */
void cadastrarCliente(Cliente clientes[], int *totalClientes);
void listarClientes(const Cliente clientes[], int totalClientes);
int buscarClientePorCNH(const Cliente clientes[], int totalClientes, const char *cnh);
void buscarClienteMenu(const Cliente clientes[], int totalClientes);
void salvarClientes(const Cliente clientes[], int totalClientes);
void carregarClientes(Cliente clientes[], int *totalClientes);

/* Modulo de Locacoes & Relatorios - Membro 3 (Giovana) */
Veiculo *loc_encontrarVeiculo(Veiculo frota[], int qtdVeic, const char *placa);
Cliente *loc_encontrarCliente(Cliente clientes[], int qtdCli, const char *cnh);
double calcularValorTotal(int dias, double valorDiaria);
void atualizarStatusVeiculo(Veiculo *v, int novoStatus);
int registrarLocacao(Locacao locs[], int *qtdLoc, Veiculo frota[], int qtdVeic, Cliente clientes[], int qtdCli);
int registrarDevolucao(Locacao locs[], int qtdLoc, Veiculo frota[], int qtdVeic);
void relatorioVeiculosDisponiveis(const Veiculo frota[], int qtdVeic);
void relatorioLocacoesAtivas(const Locacao locs[], int qtdLoc);
void relatorioHistorico(const Locacao locs[], int qtdLoc);
void salvarLocacoes(const Locacao locs[], int qtdLoc);
int carregarLocacoes(Locacao locs[], int max);
void menuLocacoes(Locacao locs[], int *qtdLoc, Veiculo frota[], int qtdVeic, Cliente clientes[], int qtdCli);

/* Menu Principal - Membro 2 (Marco) */
void exibirMenuPrincipal(void);

/* ============================================================================
 * FUNCOES AUXILIARES DE ENTRADA E FORMATACAO
 * ============================================================================ */

void limparBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void lerTexto(const char *msg, char *dest, int tam) {
    size_t len;
    int c;

    printf("%s", msg);
    fflush(stdout);
    if (fgets(dest, tam, stdin) == NULL) {
        dest[0] = '\0';
        return;
    }
    len = strlen(dest);
    if (len > 0 && dest[len - 1] == '\n') {
        dest[len - 1] = '\0';
        len--;
    } else if (len == (size_t)(tam - 1)) {
        while ((c = getchar()) != '\n' && c != EOF) {
            /* descarta excesso de caracteres */
        }
    }
    while (len > 0 && (dest[len - 1] == '\r' || dest[len - 1] == ' ')) {
        dest[len - 1] = '\0';
        len--;
    }
}

int lerInteiro(const char *msg) {
    char buf[64];
    char *fim;
    long v;

    lerTexto(msg, buf, (int)sizeof(buf));
    if (buf[0] == '\0') {
        return -1;
    }
    errno = 0;
    v = strtol(buf, &fim, 10);
    if (errno != 0 || *fim != '\0' || v < 0 || v > INT_MAX) {
        return -1;
    }
    return (int)v;
}

double lerDouble(const char *msg) {
    char buf[64];
    char *fim;
    double v;

    lerTexto(msg, buf, (int)sizeof(buf));
    if (buf[0] == '\0') {
        return -1.0;
    }
    /* Substitui virgula por ponto caso o usuario digite no padrao BR */
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ',') {
            buf[i] = '.';
        }
    }
    errno = 0;
    v = strtod(buf, &fim);
    if (errno != 0 || *fim != '\0' || v < 0.0) {
        return -1.0;
    }
    return v;
}

void imprimirReais(double valor) {
    long centavos = (long)(valor * 100.0 + 0.5);
    printf("R$ %ld,%02ld", centavos / 100, centavos % 100);
}

int placasIguais(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static int veiculoDisponivel(const Veiculo *v) {
    return v->status == STATUS_DISPONIVEL;
}

/* ============================================================================
 * MODULO DE VEICULOS - MEMBRO 1 (VITOR)
 * ============================================================================ */

void cadastrarVeiculo(Veiculo frota[], int *totalVeiculos) {
    if (*totalVeiculos >= MAX_VEICULOS) {
        printf("\nERRO: Limite maximo de veiculos cadastrados (%d) atingido!\n", MAX_VEICULOS);
        return;
    }

    Veiculo v;
    printf("\n====== CADASTRO DE VEICULO ======\n");

    lerTexto("Placa do veiculo (ex: ABC1D23): ", v.placa, sizeof(v.placa));
    if (v.placa[0] == '\0') {
        printf("ERRO: Placa nao pode ser vazia.\n");
        return;
    }

    /* Verifica duplicidade de placa */
    if (loc_encontrarVeiculo(frota, *totalVeiculos, v.placa) != NULL) {
        printf("ERRO: Ja existe um veiculo cadastrado com a placa %s.\n", v.placa);
        return;
    }

    lerTexto("Marca (ex: Toyota, Fiat): ", v.marca, sizeof(v.marca));
    lerTexto("Modelo (ex: Corolla, Argo): ", v.modelo, sizeof(v.modelo));

    v.ano = lerInteiro("Ano de fabricacao: ");
    if (v.ano < 1900 || v.ano > 2100) {
        printf("ERRO: Ano invalido.\n");
        return;
    }

    v.valorDiaria = lerDouble("Valor da diaria (R$): ");
    if (v.valorDiaria <= 0.0) {
        printf("ERRO: O valor da diaria deve ser maior que zero.\n");
        return;
    }

    v.status = STATUS_DISPONIVEL;

    frota[*totalVeiculos] = v;
    (*totalVeiculos)++;

    printf("\nVeiculo cadastrado com sucesso! Status inicial: Disponivel.\n");
}

void listarVeiculos(const Veiculo frota[], int totalVeiculos) {
    if (totalVeiculos == 0) {
        printf("\nAinda nao ha veiculos cadastrados na frota.\n");
        return;
    }

    printf("\n====== LISTA COMPLETA DE VEICULOS ======\n");
    printf("%-10s %-14s %-16s %-6s %-13s %s\n",
           "PLACA", "MARCA", "MODELO", "ANO", "DIARIA", "STATUS");
    printf("------------------------------------------------------------------------\n");

    for (int i = 0; i < totalVeiculos; i++) {
        printf("%-10.10s %-14.14s %-16.16s %-6d ",
               frota[i].placa, frota[i].marca, frota[i].modelo, frota[i].ano);
        imprimirReais(frota[i].valorDiaria);
        printf("   %s\n", frota[i].status == STATUS_DISPONIVEL ? "Disponivel" : "Alugado");
    }
}

void buscarVeiculoMenu(const Veiculo frota[], int totalVeiculos) {
    char placa[20];
    printf("\n====== BUSCAR VEICULO ======\n");
    lerTexto("Digite a placa do veiculo: ", placa, sizeof(placa));

    Veiculo *v = loc_encontrarVeiculo((Veiculo *)frota, totalVeiculos, placa);
    if (v == NULL) {
        printf("Veiculo com placa '%s' nao encontrado.\n", placa);
        return;
    }

    printf("\n--- Veiculo Encontrado ---\n");
    printf("Placa:  %s\n", v->placa);
    printf("Marca:  %s\n", v->marca);
    printf("Modelo: %s\n", v->modelo);
    printf("Ano:    %d\n", v->ano);
    printf("Diaria: ");
    imprimirReais(v->valorDiaria);
    printf("\nStatus: %s\n", v->status == STATUS_DISPONIVEL ? "Disponivel" : "Alugado");
}

void salvarVeiculos(const Veiculo frota[], int totalVeiculos) {
    FILE *arquivo = fopen(ARQUIVO_VEICULOS, "wb");
    if (arquivo == NULL) {
        printf("\nERRO: Nao foi possivel abrir '%s' para gravacao!\n", ARQUIVO_VEICULOS);
        return;
    }
    fwrite(&totalVeiculos, sizeof(int), 1, arquivo);
    if (totalVeiculos > 0) {
        fwrite(frota, sizeof(Veiculo), (size_t)totalVeiculos, arquivo);
    }
    fclose(arquivo);
}

void carregarVeiculos(Veiculo frota[], int *totalVeiculos) {
    FILE *arquivo = fopen(ARQUIVO_VEICULOS, "rb");
    if (arquivo == NULL) {
        *totalVeiculos = 0;
        return;
    }
    if (fread(totalVeiculos, sizeof(int), 1, arquivo) != 1) {
        *totalVeiculos = 0;
        fclose(arquivo);
        return;
    }
    if (*totalVeiculos > MAX_VEICULOS) {
        *totalVeiculos = MAX_VEICULOS;
    }
    if (*totalVeiculos > 0) {
        fread(frota, sizeof(Veiculo), (size_t)(*totalVeiculos), arquivo);
    }
    fclose(arquivo);
}

/* ============================================================================
 * MODULO DE CLIENTES - MEMBRO 2 (MARCO)
 * ============================================================================ */

void cadastrarCliente(Cliente clientes[], int *totalClientes) {
    if (*totalClientes >= MAX_CLIENTES) {
        printf("\nERRO: LIMITE MAXIMO DE CLIENTES CADASTRADOS (%d) ATINGIDO!\n", MAX_CLIENTES);
        return;
    }

    Cliente c;
    printf("\n====== CADASTRO DE CLIENTE ======\n");

    lerTexto("Digite o nome completo: ", c.nome, sizeof(c.nome));
    if (c.nome[0] == '\0') {
        printf("ERRO: O nome nao pode ser vazio.\n");
        return;
    }

    lerTexto("Digite o numero da CNH: ", c.cnh, sizeof(c.cnh));
    if (c.cnh[0] == '\0') {
        printf("ERRO: A CNH nao pode ser vazia.\n");
        return;
    }

    /* Verifica duplicidade de CNH */
    if (buscarClientePorCNH(clientes, *totalClientes, c.cnh) != -1) {
        printf("ERRO: Ja existe um cliente cadastrado com a CNH %s.\n", c.cnh);
        return;
    }

    lerTexto("Digite o telefone: ", c.telefone, sizeof(c.telefone));

    clientes[*totalClientes] = c;
    (*totalClientes)++;

    printf("\nCliente cadastrado com sucesso!\n");
}

void listarClientes(const Cliente clientes[], int totalClientes) {
    if (totalClientes == 0) {
        printf("\nAinda nao ha clientes cadastrados.\n");
        return;
    }

    printf("\n====== LISTA DE CLIENTES ======\n");
    for (int i = 0; i < totalClientes; i++) {
        printf("Cliente %d:\n", i + 1);
        printf("---------------------------------\n");
        printf("Nome:     %s\n", clientes[i].nome);
        printf("CNH:      %s\n", clientes[i].cnh);
        printf("Telefone: %s\n", clientes[i].telefone);
        printf("=================================\n");
    }
}

int buscarClientePorCNH(const Cliente clientes[], int totalClientes, const char *cnh) {
    for (int i = 0; i < totalClientes; i++) {
        if (strcmp(clientes[i].cnh, cnh) == 0) {
            return i; /* Retorna o indice do cliente encontrado */
        }
    }
    return -1; /* Retorna -1 se nao encontrado */
}

void buscarClienteMenu(const Cliente clientes[], int totalClientes) {
    char cnh[20];
    printf("\n====== BUSCAR CLIENTE ======\n");
    lerTexto("Digite a CNH do cliente: ", cnh, sizeof(cnh));

    int idx = buscarClientePorCNH(clientes, totalClientes, cnh);
    if (idx == -1) {
        printf("Cliente com CNH '%s' nao encontrado.\n", cnh);
        return;
    }

    printf("\n--- Cliente Encontrado ---\n");
    printf("Nome:     %s\n", clientes[idx].nome);
    printf("CNH:      %s\n", clientes[idx].cnh);
    printf("Telefone: %s\n", clientes[idx].telefone);
}

void salvarClientes(const Cliente clientes[], int totalClientes) {
    FILE *arquivo = fopen(ARQUIVO_CLIENTES, "wb");
    if (arquivo == NULL) {
        printf("\nERRO: NAO FOI POSSIVEL ABRIR O ARQUIVO '%s' PARA GRAVACAO!\n", ARQUIVO_CLIENTES);
        return;
    }

    fwrite(&totalClientes, sizeof(int), 1, arquivo);
    if (totalClientes > 0) {
        fwrite(clientes, sizeof(Cliente), (size_t)totalClientes, arquivo);
    }
    fclose(arquivo);
}

void carregarClientes(Cliente clientes[], int *totalClientes) {
    FILE *arquivo = fopen(ARQUIVO_CLIENTES, "rb");
    if (arquivo == NULL) {
        *totalClientes = 0;
        return;
    }

    if (fread(totalClientes, sizeof(int), 1, arquivo) != 1) {
        *totalClientes = 0;
        fclose(arquivo);
        return;
    }
    if (*totalClientes > MAX_CLIENTES) {
        *totalClientes = MAX_CLIENTES;
    }
    if (*totalClientes > 0) {
        fread(clientes, sizeof(Cliente), (size_t)(*totalClientes), arquivo);
    }
    fclose(arquivo);
}

/* ============================================================================
 * MODULO DE LOCACOES E RELATORIOS - MEMBRO 3 (GIOVANA)
 * ============================================================================ */

Veiculo *loc_encontrarVeiculo(Veiculo frota[], int qtdVeic, const char *placa) {
    for (int i = 0; i < qtdVeic; i++) {
        if (placasIguais(frota[i].placa, placa)) {
            return &frota[i];
        }
    }
    return NULL;
}

Cliente *loc_encontrarCliente(Cliente clientes[], int qtdCli, const char *cnh) {
    for (int i = 0; i < qtdCli; i++) {
        if (strcmp(clientes[i].cnh, cnh) == 0) {
            return &clientes[i];
        }
    }
    return NULL;
}

/* Parametros por VALOR */
double calcularValorTotal(int dias, double valorDiaria) {
    return (double)dias * valorDiaria;
}

/* Parametro por REFERENCIA (ponteiro) */
void atualizarStatusVeiculo(Veiculo *v, int novoStatus) {
    v->status = novoStatus;
}

int registrarLocacao(Locacao locs[], int *qtdLoc,
                     Veiculo frota[], int qtdVeic,
                     Cliente clientes[], int qtdCli) {
    char cnh[20];
    char placa[20];
    Cliente *cli;
    Veiculo *vei;
    Locacao nova;
    int dias;

    printf("\n====== REGISTRAR LOCACAO ======\n");

    if (*qtdLoc >= MAX_LOCACOES) {
        printf("ERRO: Limite de %d locacoes atingido.\n", MAX_LOCACOES);
        return 0;
    }

    /* 1) Valida se CNH pertence a um cliente cadastrado */
    lerTexto("CNH do cliente: ", cnh, (int)sizeof(cnh));
    cli = loc_encontrarCliente(clientes, qtdCli, cnh);
    if (cli == NULL) {
        printf("ERRO: Cliente nao cadastrado com essa CNH.\n");
        return 0;
    }

    /* 2) Valida se o veiculo existe */
    lerTexto("Placa do veiculo: ", placa, (int)sizeof(placa));
    vei = loc_encontrarVeiculo(frota, qtdVeic, placa);
    if (vei == NULL) {
        printf("ERRO: Veiculo nao cadastrado com essa placa.\n");
        return 0;
    }

    /* 3) Valida se o veiculo esta disponivel */
    if (!veiculoDisponivel(vei)) {
        printf("ERRO: O veiculo %s ja esta alugado.\n", vei->placa);
        return 0;
    }

    /* 4) Valida quantidade de dias */
    dias = lerInteiro("Quantidade de dias: ");
    if (dias <= 0) {
        printf("ERRO: A quantidade de dias deve ser maior que zero.\n");
        return 0;
    }

    /* Preenche struct e atualiza registros via ponteiros */
    strncpy(nova.placa, vei->placa, sizeof(nova.placa) - 1);
    nova.placa[sizeof(nova.placa) - 1] = '\0';
    strncpy(nova.cnh, cli->cnh, sizeof(nova.cnh) - 1);
    nova.cnh[sizeof(nova.cnh) - 1] = '\0';
    nova.dias = dias;
    nova.valorTotal = calcularValorTotal(dias, vei->valorDiaria);
    nova.ativa = 1;

    locs[*qtdLoc] = nova;
    (*qtdLoc)++;
    atualizarStatusVeiculo(vei, STATUS_ALUGADO);

    printf("\n--- LOCACAO REGISTRADA COM SUCESSO ---\n");
    printf("Cliente: %s (CNH %s)\n", cli->nome, cli->cnh);
    printf("Veiculo: %s %s (Placa %s)\n", vei->marca, vei->modelo, vei->placa);
    printf("Dias:    %d\n", nova.dias);
    printf("Total estimado: ");
    imprimirReais(nova.valorTotal);
    printf("\n");
    return 1;
}

int registrarDevolucao(Locacao locs[], int qtdLoc,
                       Veiculo frota[], int qtdVeic) {
    char placa[20];
    Veiculo *vei;

    printf("\n====== REGISTRAR DEVOLUCAO ======\n");

    lerTexto("Placa do veiculo: ", placa, (int)sizeof(placa));
    vei = loc_encontrarVeiculo(frota, qtdVeic, placa);
    if (vei == NULL) {
        printf("ERRO: Veiculo nao cadastrado com essa placa.\n");
        return 0;
    }

    for (int i = 0; i < qtdLoc; i++) {
        if (locs[i].ativa == 1 && placasIguais(locs[i].placa, placa)) {
            locs[i].ativa = 0;
            atualizarStatusVeiculo(vei, STATUS_DISPONIVEL);

            printf("\n--- DEVOLUCAO REGISTRADA COM SUCESSO ---\n");
            printf("Veiculo: %s %s (Placa %s)\n", vei->marca, vei->modelo, vei->placa);
            printf("Dias:    %d\n", locs[i].dias);
            printf("Valor final: ");
            imprimirReais(locs[i].valorTotal);
            printf("\n");
            return 1;
        }
    }

    printf("ERRO: Nao ha locacao ativa para a placa %s.\n", vei->placa);
    return 0;
}

void relatorioVeiculosDisponiveis(const Veiculo frota[], int qtdVeic) {
    int achou = 0;

    printf("\n====== VEICULOS DISPONIVEIS ======\n");
    for (int i = 0; i < qtdVeic; i++) {
        if (veiculoDisponivel(&frota[i])) {
            if (!achou) {
                printf("%-10s %-14s %-16s %-5s %s\n",
                       "PLACA", "MARCA", "MODELO", "ANO", "DIARIA");
                printf("---------------------------------------------------------\n");
                achou = 1;
            }
            printf("%-10.10s %-14.14s %-16.16s %-5d ",
                   frota[i].placa, frota[i].marca, frota[i].modelo, frota[i].ano);
            imprimirReais(frota[i].valorDiaria);
            printf("\n");
        }
    }
    if (!achou) {
        printf("Nenhum veiculo disponivel no momento.\n");
    }
}

void relatorioLocacoesAtivas(const Locacao locs[], int qtdLoc) {
    int achou = 0;

    printf("\n====== LOCACOES ATIVAS ======\n");
    for (int i = 0; i < qtdLoc; i++) {
        if (locs[i].ativa == 1) {
            if (!achou) {
                printf("%-10s %-20s %-6s %s\n", "PLACA", "CNH", "DIAS", "TOTAL");
                printf("-------------------------------------------------\n");
                achou = 1;
            }
            printf("%-10.10s %-20.20s %-6d ", locs[i].placa, locs[i].cnh, locs[i].dias);
            imprimirReais(locs[i].valorTotal);
            printf("\n");
        }
    }
    if (!achou) {
        printf("Nenhuma locacao ativa no momento.\n");
    }
}

void relatorioHistorico(const Locacao locs[], int qtdLoc) {
    printf("\n====== HISTORICO DE LOCACOES ======\n");
    if (qtdLoc == 0) {
        printf("Nenhuma locacao registrada ate agora.\n");
        return;
    }
    printf("%-10s %-20s %-6s %-14s %s\n", "PLACA", "CNH", "DIAS", "TOTAL", "SITUACAO");
    printf("---------------------------------------------------------------\n");
    for (int i = 0; i < qtdLoc; i++) {
        printf("%-10.10s %-20.20s %-6d ", locs[i].placa, locs[i].cnh, locs[i].dias);
        imprimirReais(locs[i].valorTotal);
        printf("   %s\n", locs[i].ativa == 1 ? "Ativa" : "Devolvida");
    }
}

void salvarLocacoes(const Locacao locs[], int qtdLoc) {
    FILE *f = fopen(ARQUIVO_LOCACOES, "wb");
    if (f == NULL) {
        printf("\nERRO: Nao foi possivel abrir '%s' para gravacao!\n", ARQUIVO_LOCACOES);
        return;
    }
    if (qtdLoc > 0) {
        fwrite(locs, sizeof(Locacao), (size_t)qtdLoc, f);
    }
    fclose(f);
}

int carregarLocacoes(Locacao locs[], int max) {
    FILE *f = fopen(ARQUIVO_LOCACOES, "rb");
    long tamanho;
    int n = 0;

    if (f == NULL) {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    tamanho = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (tamanho <= 0) {
        fclose(f);
        return 0;
    }

    while (!feof(f) && n < max) {
        if (fread(&locs[n], sizeof(Locacao), 1, f) == 1) {
            n++;
        }
    }
    fclose(f);
    return n;
}

void menuLocacoes(Locacao locs[], int *qtdLoc,
                  Veiculo frota[], int qtdVeic,
                  Cliente clientes[], int qtdCli) {
    int opcao;

    do {
        printf("\n================================\n");
        printf("     LOCACOES E RELATORIOS\n");
        printf("================================\n");
        printf("1. Registrar locacao\n");
        printf("2. Registrar devolucao\n");
        printf("3. Veiculos disponiveis\n");
        printf("4. Locacoes ativas\n");
        printf("5. Historico de locacoes\n");
        printf("0. Voltar ao menu principal\n");
        printf("================================\n");
        opcao = lerInteiro("Escolha uma opcao (de 0 a 5): ");

        switch (opcao) {
            case 1:
                registrarLocacao(locs, qtdLoc, frota, qtdVeic, clientes, qtdCli);
                break;
            case 2:
                registrarDevolucao(locs, *qtdLoc, frota, qtdVeic);
                break;
            case 3:
                relatorioVeiculosDisponiveis(frota, qtdVeic);
                break;
            case 4:
                relatorioLocacoesAtivas(locs, *qtdLoc);
                break;
            case 5:
                relatorioHistorico(locs, *qtdLoc);
                break;
            case 0:
                break;
            default:
                printf("\nOpcao invalida! Digite um numero de 0 a 5.\n");
                break;
        }
    } while (opcao != 0);
}

/* ============================================================================
 * MENU PRINCIPAL E FUNCAO MAIN - MEMBRO 2 (MARCO)
 * ============================================================================ */

void exibirMenuPrincipal(void) {
    printf("\n========================================\n");
    printf("   SISTEMA DE GESTAO DE LOCADORA (C)    \n");
    printf("========================================\n");
    printf("1. Cadastrar Cliente          [Marco]\n");
    printf("2. Listar Clientes            [Marco]\n");
    printf("3. Buscar Cliente (CNH)       [Marco]\n");
    printf("----------------------------------------\n");
    printf("4. Cadastrar Veiculo          [Vitor]\n");
    printf("5. Listar Veiculos            [Vitor]\n");
    printf("6. Buscar Veiculo (Placa)     [Vitor]\n");
    printf("----------------------------------------\n");
    printf("7. Locacoes e Relatorios      [Giovana]\n");
    printf("========================================\n");
    printf("0. Salvar Dados e Sair\n");
    printf("========================================\n");
}

int main(void) {
    Veiculo frota[MAX_VEICULOS];
    Cliente clientes[MAX_CLIENTES];
    Locacao locacoes[MAX_LOCACOES];

    int totalVeiculos = 0;
    int totalClientes = 0;
    int totalLocacoes = 0;
    int opcao;

    /* Carregamento automatico dos dados persistentes ao iniciar */
    carregarVeiculos(frota, &totalVeiculos);
    carregarClientes(clientes, &totalClientes);
    totalLocacoes = carregarLocacoes(locacoes, MAX_LOCACOES);

    printf("Sistema iniciado! Dados carregados: %d veiculo(s), %d cliente(s), %d locacao(oes).\n",
           totalVeiculos, totalClientes, totalLocacoes);

    do {
        exibirMenuPrincipal();
        opcao = lerInteiro("Escolha uma opcao (de 0 a 7): ");

        switch (opcao) {
            case 1:
                cadastrarCliente(clientes, &totalClientes);
                break;
            case 2:
                listarClientes(clientes, totalClientes);
                break;
            case 3:
                buscarClienteMenu(clientes, totalClientes);
                break;
            case 4:
                cadastrarVeiculo(frota, &totalVeiculos);
                break;
            case 5:
                listarVeiculos(frota, totalVeiculos);
                break;
            case 6:
                buscarVeiculoMenu(frota, totalVeiculos);
                break;
            case 7:
                menuLocacoes(locacoes, &totalLocacoes, frota, totalVeiculos, clientes, totalClientes);
                break;
            case 0:
                salvarVeiculos(frota, totalVeiculos);
                salvarClientes(clientes, totalClientes);
                salvarLocacoes(locacoes, totalLocacoes);
                printf("\nDados salvos com sucesso em 'veiculos.bin', 'clientes.bin' e 'locacoes.bin'.\n");
                printf("Encerrando o sistema...\n");
                break;
            default:
                printf("\nOpcao invalida! Digite um numero de 0 a 7.\n");
                break;
        }
    } while (opcao != 0);

    return 0;
}