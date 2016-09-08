#include <iostream>

#include <AL/alut.h>

using namespace std;

int main(int argc, char** argv)
{
	ALuint buffer;
	ALuint source;
	alutInit(&argc, argv);
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
	alutExit();

	return 0;
}

