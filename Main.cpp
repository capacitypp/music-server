#include <iostream>

#include <AL/alut.h>

using namespace std;

int main(int argc, char** argv)
{
	ALuint buffer;
	ALuint source;
	alutInit(&argc, argv);
	buffer = alutCreateBufferHelloWorld();
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
	alSourcePlay(source);
	alutSleep(3);
	alutExit();

	return 0;
}

