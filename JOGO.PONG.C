/*
 * ============================================================
 * Autor  : Venicius
 * Turma  : [PROGRAMAÇAO C]
 * Curso  : Faculdade CDL
 * Disciplina: Topicos e Praticas em Desenvolvimento de Sistemas
 * Trabalho  : TRAB1 - Jogo Rebata-A-Bola
 * ============================================================
 *
 * Descricao:
 *   O usuario controla uma barra rebatedora na linha 21 da tela.
 *   A bolinha se move em diagonal e rebate nas paredes e na barra.
 *   Tecla A -> move a barra para a esquerda
 *   Tecla L -> move a barra para a direita
 *   Tecla Q -> sai do jogo
 *
 * Compilacao:
 *   Linux/Mac : gcc -o jogo VeniciusTRAB1.c
 *   Windows   : gcc -o jogo VeniciusTRAB1.c (MinGW)
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
/* No Windows, usa Sleep(ms) e _kbhit()/_getch() */
#define DELAY_MS 80
#define CLEAR_SCREEN() system("cls")
void esperar_ms(int ms) { Sleep(ms); }
int tecla_disponivel() { return _kbhit(); }
char ler_tecla() { return _getch(); }
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
/* No Linux/Mac, configura terminal para leitura nao-bloqueante */
#define DELAY_MS 80000 /* microsegundos para usleep */
#define CLEAR_SCREEN() system("clear")

void esperar_ms(int us) { usleep(us); }

/* Habilita modo raw no terminal (sem esperar ENTER) */
void habilitar_modo_raw()
{
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &t);
    fcntl(0, F_SETFL, O_NONBLOCK);
}

/* Restaura terminal ao sair */
void restaurar_terminal()
{
    struct termios t;
    tcgetattr(0, &t);
    t.c_lflag |= (ICANON | ECHO);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &t);
    int flags = fcntl(0, F_GETFL);
    fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
}

int tecla_disponivel()
{
    char c;
    return read(0, &c, 1) == 1 ? (int)c : 0;
}

char ler_tecla()
{
    /* Nao usada separadamente no Linux; logica embutida em tecla_disponivel */
    return 0;
}
#endif

/* ============================================================
 * Constantes do jogo
 * ============================================================ */
#define LARGURA_TELA 40 /* Numero de colunas do campo */
#define ALTURA_TELA 22  /* Numero de linhas do campo (1..21 + borda inferior) */
#define LINHA_BARRA 21  /* Linha fixa da barra rebatedora (1-indexado) */
#define TAM_BARRA 5     /* Tamanho da barra em caracteres */

/* ============================================================
 * Estrutura do estado do jogo
 * ============================================================ */
typedef struct
{
    /* Posicao da bolinha */
    int bola_x;
    int bola_y;
    /* Direcao da bolinha: +1 ou -1 */
    int dir_x;
    int dir_y;
    /* Posicao do centro da barra */
    int barra_x;
    /* Pontuacao e vidas */
    int pontos;
    int vidas;
    /* Flag de fim de jogo */
    int game_over;
} EstadoJogo;

/* ============================================================
 * Funcao: inicializar_jogo
 * Configura o estado inicial do jogo
 * ============================================================ */
void inicializar_jogo(EstadoJogo *jogo)
{
    jogo->bola_x = LARGURA_TELA / 2; /* Bolinha comeca no centro */
    jogo->bola_y = 10;
    jogo->dir_x = 1; /* Movimento diagonal inicial */
    jogo->dir_y = 1;
    jogo->barra_x = LARGURA_TELA / 2; /* Barra centralizada */
    jogo->pontos = 0;
    jogo->vidas = 3;
    jogo->game_over = 0;
}

/* ============================================================
 * Funcao: desenhar_tela
 * Renderiza o campo, bola e barra na tela
 * ============================================================ */
void desenhar_tela(const EstadoJogo *jogo)
{
    /* Buffer de tela para evitar flickering */
    char tela[ALTURA_TELA + 1][LARGURA_TELA + 2];
    int linha, col;

    /* Preenche campo com espacos */
    for (linha = 0; linha <= ALTURA_TELA; linha++)
    {
        for (col = 0; col < LARGURA_TELA + 2; col++)
        {
            tela[linha][col] = ' ';
        }
    }

    /* Desenha bordas laterais e topo */
    for (linha = 0; linha <= ALTURA_TELA; linha++)
    {
        tela[linha][0] = '|';                /* Borda esquerda */
        tela[linha][LARGURA_TELA + 1] = '|'; /* Borda direita  */
    }
    /* Borda superior */
    for (col = 0; col <= LARGURA_TELA + 1; col++)
    {
        tela[0][col] = '-';
    }

    /* Posiciona a bolinha (linha 1-indexada -> indice 0-indexado) */
    if (jogo->bola_y >= 1 && jogo->bola_y <= LINHA_BARRA)
    {
        tela[jogo->bola_y][jogo->bola_x] = 'O';
    }

    /* Desenha a barra rebatedora na linha 21 */
    int inicio_barra = jogo->barra_x - TAM_BARRA / 2;
    for (col = 0; col < TAM_BARRA; col++)
    {
        int pos = inicio_barra + col;
        if (pos >= 1 && pos <= LARGURA_TELA)
        {
            tela[LINHA_BARRA][pos] = '=';
        }
    }

    /* Limpa tela e imprime buffer */
    CLEAR_SCREEN();

    /* Cabecalho com pontuacao e vidas */
    printf("  REBATA-A-BOLA  |  Pontos: %4d  |  Vidas: %d\n", jogo->pontos, jogo->vidas);

    /* Imprime o campo linha a linha */
    for (linha = 0; linha <= ALTURA_TELA; linha++)
    {
        for (col = 0; col <= LARGURA_TELA + 1; col++)
        {
            putchar(tela[linha][col]);
        }
        putchar('\n');
    }

    /* Instrucoes na parte inferior */
    printf("  [A] Esquerda  [L] Direita  [Q] Sair\n");
}

/* ============================================================
 * Funcao: mover_bola
 * Atualiza posicao da bolinha e trata colisoes
 * ============================================================ */
void mover_bola(EstadoJogo *jogo)
{
    int novo_x = jogo->bola_x + jogo->dir_x;
    int novo_y = jogo->bola_y + jogo->dir_y;

    /* --- Colisao com paredes laterais --- */
    if (novo_x <= 0 || novo_x > LARGURA_TELA)
    {
        jogo->dir_x = -jogo->dir_x; /* Inverte direcao horizontal */
        novo_x = jogo->bola_x + jogo->dir_x;
    }

    /* --- Colisao com parede superior --- */
    if (novo_y <= 0)
    {
        jogo->dir_y = -jogo->dir_y; /* Inverte direcao vertical */
        novo_y = jogo->bola_y + jogo->dir_y;
    }

    /* --- Colisao com a barra rebatedora --- */
    if (novo_y == LINHA_BARRA)
    {
        int inicio_barra = jogo->barra_x - TAM_BARRA / 2;
        int fim_barra = jogo->barra_x + TAM_BARRA / 2;

        if (novo_x >= inicio_barra && novo_x <= fim_barra)
        {
            /* Rebateu na barra! Inverte direcao vertical e pontua */
            jogo->dir_y = -jogo->dir_y;
            novo_y = jogo->bola_y + jogo->dir_y;
            jogo->pontos += 10;
        }
    }

    /* --- Bolinha passou da barra (perdeu vida) --- */
    if (novo_y > LINHA_BARRA)
    {
        jogo->vidas--;
        if (jogo->vidas <= 0)
        {
            jogo->game_over = 1; /* Sem vidas -> Game Over */
        }
        else
        {
            /* Reinicia posicao da bolinha */
            jogo->bola_x = LARGURA_TELA / 2;
            jogo->bola_y = 10;
            jogo->dir_x = 1;
            jogo->dir_y = 1;
            return;
        }
    }

    /* Atualiza posicao */
    jogo->bola_x = novo_x;
    jogo->bola_y = novo_y;
}

/* ============================================================
 * Funcao: processar_entrada
 * Le tecla pressionada e move a barra
 * ============================================================ */
void processar_entrada(EstadoJogo *jogo, char tecla)
{
    switch (tecla)
    {
    case 'a':
    case 'A':
        /* Move barra para esquerda, respeitando limite */
        if (jogo->barra_x - TAM_BARRA / 2 > 1)
            jogo->barra_x--;
        break;

    case 'l':
    case 'L':
        /* Move barra para direita, respeitando limite */
        if (jogo->barra_x + TAM_BARRA / 2 < LARGURA_TELA)
            jogo->barra_x++;
        break;

    case 'q':
    case 'Q':
        /* Sair do jogo */
        jogo->game_over = 1;
        break;

    default:
        /* Outras teclas sao ignoradas */
        break;
    }
}

/* ============================================================
 * Funcao principal: main
 * Loop principal do jogo
 * ============================================================ */
int main(void)
{
    EstadoJogo jogo;

#ifndef _WIN32
    habilitar_modo_raw(); /* Configura terminal para leitura instantanea */
#endif

    inicializar_jogo(&jogo);

    /* Loop principal do jogo */
    while (!jogo.game_over)
    {
        /* Desenha estado atual */
        desenhar_tela(&jogo);

        /* Le tecla se houver disponivel (nao bloqueante) */
#ifdef _WIN32
        if (tecla_disponivel())
        {
            char t = ler_tecla();
            processar_entrada(&jogo, t);
        }
#else
        {
            char c;
            int n = read(0, &c, 1);
            if (n == 1)
            {
                processar_entrada(&jogo, c);
            }
        }
#endif

        /* Move a bolinha */
        mover_bola(&jogo);

        /* Aguarda antes do proximo frame */
        esperar_ms(DELAY_MS);
    }

    /* Mensagem de fim de jogo */
    CLEAR_SCREEN();
    printf("\n\n");
    printf("  *** GAME OVER ***\n");
    printf("  Pontuacao final: %d\n\n", jogo.pontos);

#ifndef _WIN32
    restaurar_terminal(); /* Restaura terminal ao modo normal */
#endif

    return 0;
}
