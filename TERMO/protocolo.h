#ifndef PROTOCOLO
#define PROTOCOLO

#include <sys/types.h>
#include <sys/socket.h>

#define MSG_SIZE 128

typedef enum {
	MSG_START,
	MSG_GUESS,
	MSG_FEEDBACK,
	MSG_WIN,
	MSG_ERROR,
	MSG_EXIT
} MessageType;

typedef struct {
	int type;
	int guess[5];
	int feedback[5];
	int attempts;
	int win_status;
	char message[MSG_SIZE];
} GameMessage;

static ssize_t recv_all(int fd, void *buf, size_t len){
	size_t total = 0;
	while (total < len){
			ssize_t n = recv(fd, (char *)buf + total, len - total, 0);
			if (n <= 0) return n;
			total += n;
	}
	return total;
}

static ssize_t send_all(int fd, const void *buf, size_t len){
	size_t total = 0;
	while (total < len){
			ssize_t n = send(fd, (const char *)buf + total, len - total, 0);
			if (n <= 0) return -1;
			total += n;
	}
	return total;
}

#endif
