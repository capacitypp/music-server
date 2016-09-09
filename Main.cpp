#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <AL/alut.h>

#include "WavePlayer.hpp"

#define PORT 23456

using namespace std;

string readLine(int sock)
{
	string line;
	char buf;
	while (1) {
		if (!read(sock, &buf, 1))
			continue;
		if (buf == '\r')
			continue;
		if (buf == '\n')
			break;
		line += buf;
	}
	return line;
}

void play(const string& fpath)
{
	ALuint buffer;
	ALuint source;
	buffer = alutCreateBufferFromFile(fpath.c_str());
	if (buffer == AL_NONE) {
		cerr << "alutCreateBufferFromFile() failed." << endl;
		alutExit();
		exit(1);
	}
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcePlay(source);
	//alutSleep(10);
}

void task(int sock)
{
	cout << "connected." << endl;
	char buf[256];
	strcpy(buf, "server.");
	write(sock, buf, strlen(buf) + 1);

	string line = readLine(sock);
	cout << "[" << line << "]" << endl;
	if (line == "play") {
		string fpath = readLine(sock);
		play(fpath);
	} else
		cerr << "unknown command." << endl;

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
		alutExit();
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
	WavePlayer player;
	ifstream ifs;
	ifs.open("data/sample.wav", ios::binary);
	ifs.seekg(0, ifs.end);
	int size = ifs.tellg();
	ifs.seekg(0, ifs.beg);
	unsigned char buf[256];
	while (size) {
		int readSize = (size > sizeof(buf)) ? sizeof(buf) : size;
		ifs.read((char*)buf, readSize);
		player.addData(buf, readSize);
		size -= readSize;
	}
	return 0;

	server();
	alutExit();

	return 0;
}

