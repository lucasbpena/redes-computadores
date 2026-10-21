/*
 * exemplo_servidor_capenga.c
 *
 * Exemplo mínimo de uso da biblioteca rede_capenga: um servidor que
 * responde PONG a cada PING recebido. Não implementa nenhuma
 * confiabilidade — o objetivo é só mostrar a API e deixar visível
 * o problema (mensagens que somem, chegam corrompidas ou duplicadas)
 * que vocês vão resolver no TP de verdade.
 *
 * Uso: ./exemplo_servidor_capenga
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "rede_capenga.h"

#define PORT 9090
#define BUF_SIZE 128

int main(void) {
    setvbuf(stdout, NULL, _IOLBF, 0);

    int sockfd = capenga_socket();
    if (sockfd < 0) {
        perror("capenga_socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("[EXEMPLO] Servidor capenga escutando na porta %d...\n", PORT);

    char buf[BUF_SIZE];
    while (1) {
        struct sockaddr_in cli;
        socklen_t cli_len = sizeof(cli);

        ssize_t n = capenga_recvfrom(sockfd, buf, sizeof(buf) - 1, 0,
                                      (struct sockaddr *)&cli, &cli_len);
        if (n <= 0) {
            continue;
        }
        buf[n] = '\0';
        printf("[EXEMPLO] Recebido: %s\n", buf);

        const char *resp = "PONG";
        capenga_sendto(sockfd, resp, strlen(resp), 0,
                        (struct sockaddr *)&cli, cli_len);
    }

    return 0;
}
