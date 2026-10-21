#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "protocolo.h"
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int formato_valido(char* s){
	// verifica se palavra tem formato valido e converte para maiusculas
	int i;
	for (i=0; s[i] != '\0'; i++){
		if (i >= 5)
			return 0;
		if (s[i] >= 'a' && s[i] <= 'z')
			s[i] = s[i] - 'a' + 'A';
		if (s[i] < 'A' || s[i] > 'Z')
			return 0;
	}
	return i == 5;
}

int palpite_valida(int guess[5]){
	for (int i =0; i < 5; i++){
		if (guess[i] >= 'a' && guess[i] <= 'z') 
			guess[i] -= 'a' - 'A'; // capitalize
		if (guess[i] < 'A' || guess[i] > 'Z')
			return 0; // caracter check
	}
	return 1;
}

 int verificar_corretas(const char *palavra, const int guess[5], int feedback[5]){
        int restantes[26] = {0};   // letras da palavra nao usadas em acertos exatos
        int exatas = 0;

        for (int i = 0; i < 5; i++){
                if (guess[i] == palavra[i]){
                        feedback[i] = 2;
                        exatas++;
                } else {
                        feedback[i] = 0;
                        restantes[palavra[i] - 'A']++;
                }
        }
        for (int i = 0; i < 5; i++){
                if (feedback[i] != 2 && restantes[guess[i] - 'A'] > 0){
                        feedback[i] = 1;
                        restantes[guess[i] - 'A']--;
                }
        }
        return exatas;
  }


int main(int argc, char *argv[]){

	if (argc!=4){
		printf("[ERRO] Esperados 3 argumentos: protocolo ([v4, v6]), porta (int), palavra (char[5]).\n");
		return 1;
	}
	char *protocolo = argv[1];
	char *porta_raw = argv[2];
	int porta = strtol(porta_raw, NULL, 10);
	char *palavra = argv[3];
	if (!formato_valido(palavra)){
      printf("[ERRO] A palavra deve ter 5 letras de A a Z.\n");
      return 1;
	}


	int sockid;
	struct sockaddr_storage addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t addrlen;

	if (strcmp(protocolo, "v4") == 0){
		struct sockaddr_in *a4 = (struct sockaddr_in *)&addr;
		a4->sin_family = AF_INET;
		a4->sin_port = htons(porta);
		a4->sin_addr.s_addr = INADDR_ANY;
		addrlen = sizeof(struct sockaddr_in);}
	else if (strcmp(protocolo, "v6") == 0){
		struct sockaddr_in6 *a6 = (struct sockaddr_in6 *)&addr;
		a6->sin6_family = AF_INET6;
		a6->sin6_port = htons(porta);
		a6->sin6_addr = in6addr_any;
		addrlen = sizeof(struct sockaddr_in6);}
	else{
		printf("Opção %s inválida, selecione protocolo v4 ou v6.\n", protocolo);
		return 1;}
	
	// Setup server
	sockid = socket(addr.ss_family, SOCK_STREAM, 0);		
	if (sockid < 0) {
		perror("socket");
		return 1;
	}		
	int rb 	= bind(sockid, (struct sockaddr *)&addr, addrlen);
	if (rb < 0){
		perror("bind");			
		close(sockid);
		return 1;
	}
	int rl = listen(sockid, 10);
	if(rl < 0){
		perror("listen");
		close(sockid);
		return 1;
	}
	printf("Servidor iniciado em modo IP%s na porta %d\n", protocolo, porta);

	int client = accept(sockid, NULL, NULL);
	if(client < 0){
		perror("accept");
		close(sockid);
		return 1;
	}

	
	printf("Cliente conectado\n");
	int tentativas = 0;

	GameMessage msg, resp;
	memset(&msg, 0, sizeof(msg));
	msg.type = MSG_START;
	send_all(client, &msg, sizeof(msg));


	while (recv_all(client, &msg, sizeof(msg))>0){
		if (msg.type == MSG_EXIT) break;

		memset(&resp, 0, sizeof(resp));
		if (msg.type != MSG_GUESS || !palpite_valida(msg.guess)){
			resp.type = MSG_ERROR;
			resp.win_status = -1;
			resp.attempts = tentativas;
			send_all(client, &resp, sizeof(resp));
			continue;
		}

		tentativas++;
		memcpy(resp.guess, msg.guess, sizeof(resp.guess));
		int exatas = verificar_corretas(palavra, msg.guess, resp.feedback);
		resp.attempts = tentativas;

		if (exatas == 5){
			resp.type=MSG_WIN;
			resp.win_status =1;
			send_all(client,  &resp, sizeof(resp));
			break;
		}
		resp.type = MSG_FEEDBACK;
		send_all(client, &resp, sizeof(resp));
	}
	close(client);
	close(sockid);
	printf("Cliente desconectado\n");
	return 0;
}

