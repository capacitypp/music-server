#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <AL/alut.h>

#define PORT 23456

using namespace std;

void task(int sock)
{
	cout << "connected." << endl;
	char buf[256];
	strcpy(buf, "server.");
	write(sock, buf, strlen(buf) + 1);

	close(sock);
}

void server(void)
{
	int sock;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cerr << "socket() failed." << endl;
		exit(1);
	}

	struct sockaddr_in saddr;
	memset((char*)&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(PORT);

	if (bind(sock, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
		cerr << "bind() failed." << endl;
		exit(1);
	}

	if (listen(sock, 1) < 0) {
		cerr << "listen() failed." << endl;
		exit(1);
	}

	struct sockaddr_in caddr;
	int sockClient;
	socklen_t len = sizeof(caddr);
	while (1) {
		if ((sockClient = accept(sock, (struct sockaddr*)&caddr, &len)) < 0) {
			cerr << "accept() failed." << endl;
			exit(1);
		}
		task(sockClient);
	}

	close(sock);
}

int main(int argc, char** argv)
{
	alutInit(&argc, argv);
/*
	ALuint buffer;
	ALuint source;
	buffer = alutCreateBufferFromFile("data/sample.wav");
	if (buffer == AL_NONE) {
		cerr << "alutCreateBufferFromFile() failed." << endl;
		alutExit();
		return 1;
	}
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcePlay(source);
	alutSleep(10);
*/
	alutExit();
	server();

	return 0;
}

