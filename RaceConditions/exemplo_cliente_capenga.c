/*
 * exemplo_cliente_capenga.c
 *
 * Cliente do exemplo mínimo: manda 10 PINGs pela rede capenga e
 * mostra quais respostas chegaram. Sem retransmissão de propósito —
 * é só para você observar a rede ruim em ação antes de resolver o
 * problema de verdade no TP.
 *
 * Uso: ./exemplo_cliente_capenga
 */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "rede_capenga.h"

#define PORT 9090
#define BUF_SIZE 128

int main(void) {
    /* Ajuste esses valores para deixar a rede mais ou menos capenga */
    capenga_config(0.3 /* perda */, 0.1 /* corrupcao */,
                    0.1 /* duplicacao */, 100 /* atraso max em ms */);

    int sockfd = capenga_socket();
    if (sockfd < 0) {
        perror("capenga_socket");
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 300000; /* 300 ms de timeout de leitura */
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    int perdidos = 0;

    for (int i = 1; i <= 10; i++) {
        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg), "PING %d", i);

        capenga_sendto(sockfd, msg, strlen(msg), 0,
                        (struct sockaddr *)&addr, sizeof(addr));
        printf("[EXEMPLO] Enviado:  %s\n", msg);

        char resp[BUF_SIZE] = {0};
        ssize_t n = capenga_recvfrom(sockfd, resp, sizeof(resp) - 1, 0,
                                      NULL, NULL);
        if (n > 0) {
            resp[n] = '\0';
            printf("[EXEMPLO] Resposta: %s\n\n", resp);
        } else {
            printf("[EXEMPLO] Sem resposta (a rede capenga engoliu o pacote).\n\n");
            perdidos++;
        }

        usleep(200000);
    }

    printf("[EXEMPLO] Total de PINGs sem resposta: %d de 10\n", perdidos);

    close(sockfd);
    return 0;
}
