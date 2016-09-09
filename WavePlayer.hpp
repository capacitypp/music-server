#ifndef ___Class_WavePlayer
#define ___Class_WavePlayer

#include <iostream>
#include <deque>

#include <AL/alut.h>

class WavePlayer {
	enum MODE {RIFF_CHUNK, FMT_CHUNK, DATA_CHUNK, DATA, LIST_CHUNK};
	MODE mode;
	unsigned idx;
	std::deque<unsigned char> data;
	int32_t dataSize;
	int16_t format;
	uint16_t channels;
	uint32_t samplerate;
	uint32_t bytepersec;
	uint16_t blockalign;
	uint16_t bitswidth;
	uint16_t extended_size;
	int32_t dataChunkSize;
	int32_t listChunkSize;

	unsigned bufferSize;
	unsigned char* bufferData;
	unsigned queuedSize;

	ALuint source;
	ALuint buffers[2];

private:
	std::string toString(unsigned size);
	void read(void* ptr, unsigned size);
	void dispose(unsigned size);
	void queueBuffer(unsigned size, ALuint buffer);
	bool processRIFFChunk(void);
	bool processFMTChunk(void);
	bool processDATAChunk(void);
	bool processDATA(void);
	bool processLISTChunk(void);

public:
	WavePlayer();
	~WavePlayer();
	void addData(unsigned char* data, unsigned size);
	bool process(void);
	bool isProcessed(void) const;
	void pause(void);
	void resume(void);
};

#endif

