#include <iostream>

#include "WavePlayer.hpp"

using namespace std;

string WavePlayer::toString(unsigned size)
{
	string str;
	for (unsigned i = 0; i < size; i++)
		str += data[i];
	return str;
}

void WavePlayer::read(void* ptr, unsigned size)
{
	copy(data.begin(), data.begin() + size, (unsigned char*)ptr);
	dispose(size);
}

void WavePlayer::dispose(unsigned size)
{
	idx += size;
	data.erase(data.begin(), data.begin() + size);
}

void WavePlayer::queueBuffer(unsigned size, ALuint buffer)
{
	ALenum format;
	if (channels == 1)
		format = (bitswidth == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	else
		format = (bitswidth == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	alBufferData(buffer, format, bufferData, size, samplerate);
}

bool WavePlayer::processRIFFChunk(void)
{
	switch (idx) {
	case 0:
	case 8:
		if (data.size() < 4)
			return false;
		dispose(4);
		break;
	case 4:
		if (data.size() < sizeof(dataSize))
			return false;
		read(&dataSize, sizeof(dataSize));
		cout << "data size : " << dataSize << "[B] (" << dataSize / 1024 << "[KB])" <<  endl;
		break;
	};
	if (idx == 12) {
		idx = 0;
		mode = FMT_CHUNK;
	}
	return true;
}

bool WavePlayer::processFMTChunk(void)
{
	switch (idx) {
	case 0:
	case 4:
		if (data.size() < 4)
			return false;
		dispose(4);
		break;
	case 8:
		if (data.size() < sizeof(format))
			return false;
		read(&format, sizeof(format));
		cout << "format : " << format << endl;
		break;
	case 10:
		if (data.size() < sizeof(channels))
			return false;
		read(&channels, sizeof(channels));
		cout << "channels : " << channels << endl;
		break;
	case 12:
		if (data.size() < sizeof(samplerate))
			return false;
		read(&samplerate, sizeof(samplerate));
		cout << "samplerate : " << samplerate << endl;
		break;
	case 16:
		if (data.size() < sizeof(bytepersec))
			return false;
		read(&bytepersec, sizeof(bytepersec));
		cout << "bytepersec : " << bytepersec << endl;
		break;
	case 20:
		if (data.size() < sizeof(blockalign))
			return false;
		read(&blockalign, sizeof(blockalign));
		cout << "blockalign : " << blockalign << endl;
		break;
	case 22:
		if (data.size() < sizeof(bitswidth))
			return false;
		read(&bitswidth, sizeof(bitswidth));
		cout << "bitswidth : " << bitswidth << endl;
		/*
		break;
	case 24:
		if (data.size() < sizeof(extended_size))
			return false;
		read(&extended_size, sizeof(extended_size));
		cout << "extended_size : " << extended_size << endl;
		break;
	case 26:
		if (data.size() < extended_size)
			return false;
		dispose(extended_size);
		*/
		idx = 0;
		mode = DATA_CHUNK;
		break;
	};
	return true;
}

bool WavePlayer::processDATAChunk(void)
{
	switch (idx) {
	case 0:
		if (data.size() < 4)
			return false;
		if (toString(4) == "LIST") {
			idx = 0;
			mode = LIST_CHUNK;
			break;
		}
		dispose(4);
		break;
	case 4:
		if (data.size() < sizeof(dataChunkSize))
			return false;
		read(&dataChunkSize, sizeof(dataChunkSize));
		cout << "data chunk size : " << dataChunkSize << "[B] (" << dataChunkSize / 1024 << "[KB])" <<  endl;
		idx = 0;
		mode = DATA;
		queuedSize = 0;
		break;
	};
	return true;
}

bool WavePlayer::processDATA(void)
{
	if (data.size() < bufferSize && data.size() + queuedSize != dataChunkSize)
		return false;
	unsigned size = bufferSize;
	if (data.size() < size)
		size = data.size();

	bool flag = false;
	unsigned idx;
	for (unsigned i = 0; i < sizeof(buffers) / sizeof(buffers[0]); i++) {
		if (!buffers[i]) {
			flag = true;
			idx = i;
			break;
		}
	}
	if (flag) {
		cout << "alGenBuffers" << endl;
		cout << "buffer size : " << data.size() << "[B] (" << data.size() / 1024 << "[KB])" << endl;
		alGenBuffers(1, &buffers[idx]);
		copy(data.begin(), data.begin() + size, bufferData);
		dispose(size);
		queueBuffer(size, buffers[idx]);
		alSourceQueueBuffers(source, 1, &buffers[idx]);
		if (!idx)
			alSourcePlay(source);
		if (!idx)
			cout << "alSourcePlay" << endl;
	} else {
		int processed;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		if (!processed)
			return false;
		cout << "buffer size : " << data.size() << "[B] (" << data.size() / 1024 << "[KB])" << endl;
		copy(data.begin(), data.begin() + size, bufferData);
		dispose(size);
		ALuint buffer;
		alSourceUnqueueBuffers(source, 1, &buffer);
		queueBuffer(size, buffer);
		alSourceQueueBuffers(source, 1, &buffer);
	}

	return true;
}

bool WavePlayer::processLISTChunk(void)
{
	switch (idx) {
	case 0:
		if (data.size() < 4)
			return false;
		dispose(4);
		break;
	case 4:
		if (data.size() < sizeof(listChunkSize))
			return false;
		read(&listChunkSize, sizeof(listChunkSize));
		cout << "list chunk size : " << listChunkSize << endl;
		break;
	default:
		if (data.size() < listChunkSize)
			return false;
		dispose(listChunkSize);
		idx = 0;
		mode = DATA_CHUNK;
		break;
	};
	return true;
}

WavePlayer::WavePlayer()
	: mode(RIFF_CHUNK), idx(0), bufferSize(1024 * 1024)
{
	bufferData = new unsigned char[bufferSize];
	alGenSources(1, &source);
	for (unsigned i = 0; i < sizeof(buffers) / sizeof(buffers[0]); i++)
		buffers[i] = 0;
}

WavePlayer::~WavePlayer()
{
	delete[] bufferData;
	alDeleteSources(1, &source);
	for (unsigned i = 0; i < sizeof(buffers) / sizeof(buffers[0]); i++)
		if (buffers[i])
			alDeleteBuffers(1, &buffers[i]);
}

void WavePlayer::addData(unsigned char* data, unsigned size)
{
	for (unsigned i = 0; i < size; i++)
		this->data.push_back(data[i]);
}

bool WavePlayer::process(void)
{
	switch (mode) {
	case RIFF_CHUNK:
		return processRIFFChunk();
	case FMT_CHUNK:
		return processFMTChunk();
	case DATA_CHUNK:
		return processDATAChunk();
	case DATA:
		return processDATA();
	case LIST_CHUNK:
		return processLISTChunk();
	};
	return true;
}

bool WavePlayer::isProcessed(void) const
{
	return data.size() ? false : true;
}

void WavePlayer::pause(void)
{
	alSourcePause(source);
}

void WavePlayer::resume(void)
{
	alSourcePlay(source);
}

