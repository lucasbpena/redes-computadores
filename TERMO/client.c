#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "protocolo.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	
	if (argc!=3){
		printf("[ERRO] Esperados 2 argumentos: endereco ip, porta (int).\n");
		return 1;
	}
	char *ip = argv[1];

	char *porta_raw = argv[2];
	int porta = strtol(porta_raw, NULL, 10);

	  struct sockaddr_storage addr;
  memset(&addr, 0, sizeof(addr));
  struct sockaddr_in  *a4 = (struct sockaddr_in *)&addr;
  struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)&addr;
  socklen_t addrlen;

  if (inet_pton(AF_INET, ip, &a4->sin_addr) == 1){
        a4->sin_family = AF_INET;
        a4->sin_port = htons(porta);
        addrlen = sizeof(*a4);
  } else if (inet_pton(AF_INET6, ip, &a6->sin6_addr) == 1){
        a6->sin6_family = AF_INET6;
        a6->sin6_port = htons(porta);
        addrlen = sizeof(*a6);
  } else {
        printf("[ERRO] Endereço IP inválido.\n");
        return 1;
  }

  int sockid = socket(addr.ss_family, SOCK_STREAM, 0);
  if (sockid < 0 || connect(sockid, (struct sockaddr *)&addr, addrlen) < 0){
        perror("connect");
        return 1;
  }

  GameMessage msg;
  if (recv_all(sockid, &msg, sizeof(msg)) <= 0 || msg.type != MSG_START){
        close(sockid);
        return 1;
  }

  char linha[64];
  while (1){
        printf("Insira seu palpite:\n> ");
        fflush(stdout);
        if (!fgets(linha, sizeof(linha), stdin)){      // EOF: avisa o servidor e sai
                memset(&msg, 0, sizeof(msg));
                msg.type = MSG_EXIT;
                send_all(sockid, &msg, sizeof(msg));
                break;
        }
        linha[strcspn(linha, "\n")] = '\0';
        size_t n = strlen(linha);

        memset(&msg, 0, sizeof(msg));
        msg.type = MSG_GUESS;
        // tamanho != 5 -> envia vetor inválido para o servidor responder MSG_ERROR
        for (int i = 0; i < 5; i++){
                char c = (n == 5) ? linha[i] : 0;
                if (c >= 'a' && c <= 'z')
                        c = c - 'a' + 'A'; // converte para maiuscula
                msg.guess[i] = c;
        }

        if (send_all(sockid, &msg, sizeof(msg)) < 0) break;
        int guess[5];
        memcpy(guess, msg.guess, sizeof(guess));
        if (recv_all(sockid, &msg, sizeof(msg)) <= 0) break;

        if (msg.type == MSG_ERROR){
                printf("ERRO: Insira uma sequência de 5 caracteres de A a Z!\n");
        } else if (msg.type == MSG_FEEDBACK){
                printf("Dica:");
                for (int i = 0; i < 5; i++){
                        if (msg.feedback[i] == 2)      printf(" %c", guess[i]);
                        else if (msg.feedback[i] == 1) printf(" *");
                        else                           printf(" _");
                }
                printf("\nTentativas realizadas: %d\n", msg.attempts);
        } else if (msg.type == MSG_WIN){
                printf("Parabéns! Você venceu!\n");
                break;
        }
  }
  close(sockid);
  return 0;
}
