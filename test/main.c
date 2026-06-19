// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#endif

struct app_config {
	char *host;
	char *service;
};

int run_server(const char *service);
int parse_argv(struct app_config *conf, int argc, char ** argv);
int connect_host(char *host, char *service);
void print_delta(const struct timespec *start, const struct timespec *stop);
int try_read(int sfd);

int run_server(const char *service)
{
    struct addrinfo hints;
    struct addrinfo *result;
    int sfd;
    int cfd;
    int s;
    char msg[] = "hello";

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       /* IPv4 only, matching old nc -4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, service, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    sfd = socket(result->ai_family,
                 result->ai_socktype,
                 result->ai_protocol);
    if (sfd < 0) {
        freeaddrinfo(result);
        return -2;
    }

    if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
        perror("bind");
        freeaddrinfo(result);
        return -3;
    }

    if (listen(sfd, 1) < 0) {
        perror("listen");
        freeaddrinfo(result);
        return -4;
    }

    fprintf(stderr, "listening on port %s\n", service);

    cfd = accept(sfd, NULL, NULL);
    if (cfd < 0) {
        perror("accept");
        return -5;
    }

#ifdef _WIN32
    send(cfd, msg, (int)strlen(msg), 0);
    closesocket(cfd);
    closesocket(sfd);
#else
    write(cfd, msg, strlen(msg));
    close(cfd);
    close(sfd);
#endif

    freeaddrinfo(result);

    return 0;
}

int main(int argc, char **argv) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
#endif

	if (argc == 3 && strcmp(argv[1], "--listen") == 0) {
		return run_server(argv[2]);
	}

	struct app_config conf;
	int ret = 0;
	int sfd = 0;
	struct timespec start_all, start, stop;

	if (0 != (ret = parse_argv(&conf, argc, argv))) {
		fprintf(stderr, "error: parsing arguments: %d\n", ret);
		fprintf(stderr, "usage: %s HOST PORT\n", argv[0]);
		return ret;
	}

	fprintf(stderr, "happy-eyeballing %s:%s ... \n", conf.host, conf.service);

	timespec_get(&start_all, TIME_UTC);
	timespec_get(&start, TIME_UTC);
	if((sfd = connect_host(conf.host, conf.service)) < 0)
	{
		fprintf(stderr, "error: connecting: %d\n", sfd);
		return sfd;
	}
	timespec_get(&stop, TIME_UTC);

	print_delta(&start, &stop);

	fprintf(stderr, "reading ...\n");

	timespec_get(&start, TIME_UTC);
	if ((ret = try_read(sfd)) < 0) {
		fprintf(stderr, "error: reading: %d\n", ret);
		return ret;
	}
	timespec_get(&stop, TIME_UTC);

	print_delta(&start, &stop);
	print_delta(&start_all, &stop);

	return ret;

#ifdef _WIN32
    WSACleanup();
#endif
}

int parse_argv(struct app_config *conf, int argc, char ** argv) {
	if (argc < 3) {
		return -1;
	}

	conf->host = strdup(argv[1]);
	conf->service = strdup(argv[2]);

	return 0;
}

void print_delta(const struct timespec *start, const struct timespec *stop) {
    long long startms =
        (long long)start->tv_sec * 1000 +
        start->tv_nsec / 1000000;

    long long stopms =
        (long long)stop->tv_sec * 1000 +
        stop->tv_nsec / 1000000;

    fprintf(stderr, "%lldms\n", stopms - startms);
}

int try_read(int sfd) {
	char buf[10];
	int s;

#ifdef _WIN32
	while((s = recv(sfd, buf, sizeof(buf), 0)) < 0) {
#else
	while((s = read(sfd, buf, sizeof(buf))) < 0) {
#endif
		if (EAGAIN != errno) {
			perror("error: reading: ");
			return -4;
		}
	}

	fprintf(stderr, "read: ");
	printf("%.*s\n", s, buf);

	return 0;
}
