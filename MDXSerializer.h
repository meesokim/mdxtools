#ifndef MDXSERIALIZER_H_
#define MDXSERIALIZER_H_

#include "MDX.h"

class MDXSerializer;
class MDXSerialParser: public MDXChannelParser {
friend class MDXSerializer;
public:
	uint8_t *data;
	int dataLen, dataPos;
	bool ended;
	MDXSerializer *mdx;
	int ticks, note;
	int loopIterations;
	int repeatStack[5];
	int repeatStackPos;
	int curVoice;
public:
	MDXSerialParser(): data(0), dataLen(0), dataPos(0), ended(false), ticks(0), loopIterations(0), repeatStackPos(0), curVoice(0) {}
private:
	virtual void handleDataEnd() {
		ended = true;
	}
	virtual void handleDataEnd(int16_t ofs) {
		if(loopIterations-- > 0) dataPos -= ofs + 3;
		else ended = true;
	}
	virtual void handleRepeatStart(uint8_t iterations) {
		repeatStack[repeatStackPos++] = iterations - 1;
		if(repeatStackPos >= 5) repeatStackPos = 4;
	}
	virtual void handleRepeatEnd(int16_t ofs) {
		if(repeatStack[repeatStackPos-1]-- > 0) {
			dataPos += ofs;
			if(dataPos < 0) dataPos = 0;
		} else repeatStackPos--;
		if(repeatStackPos < 0) repeatStackPos = 0;
	}
	virtual void handleSetTempo(uint8_t tempo);
	virtual void handleNote(uint8_t n, uint8_t duration);
	virtual void handleRest(uint8_t duration);
	virtual void handleSetVoiceNum(uint8_t voice);
	virtual void handleVolumeInc();
	virtual void handleVolumeDec();
	virtual void handleSetVolume(uint8_t v);
public:
	void feed() {
		eat(data[dataPos++]);
		if(dataPos >= dataLen) ended = true;
	}

	void nextRest() {
		ticks = 0;
		while(ticks <= 0 && !ended) feed();
	}
};

class MDXSerializer: public MDX {
friend class MDXSerialParser;
	MDXSerialParser parsers[16];
	int loopCount;
public:
	int tempo;
	MDXSerializer() {}
	MDXSerializer(const char *filename): loopCount(0), tempo(200) {
		load(filename);
	}
	void load(const char *filename) {
		s.open(filename);
		readHeader();
		readVoices();
		for(int i = 0; i < num_channels; i++) {
			parsers[i].mdx = this;
			parsers[i].channel = i;
			parsers[i].loopIterations = loopCount;
			parsers[i].dataLen = (i < num_channels - 1 ? mml_offset[i+1] : Voice_offset) - mml_offset[i];
			if(parsers[i].dataLen > 0) {
				parsers[i].data = new uint8_t[parsers[i].dataLen];
				s.seek(file_base + mml_offset[i]);
				s.read(parsers[i].data, parsers[i].dataLen);
			}
		}
		bool done = false;
		while(!done) {
			done = true;
			for(int i = 0; i < num_channels; i++) {
				if(parsers[i].ticks <= 0 && !parsers[i].ended)
					parsers[i].nextRest();
			}
			// At this point, parsers should have remaining ticks, unless they're ended
			int minTicks = 0x7fffffff;
			for(int i = 0; i < num_channels; i++) {
				if(parsers[i].ticks > 0) { // don't check if ended
					if(parsers[i].ticks < minTicks) minTicks = parsers[i].ticks;
				}
			}
			bool tickEnded = false;
			for(int i = 0; i < num_channels; i++) {
				if(parsers[i].ticks > 0) {
					parsers[i].ticks -= minTicks;
					if(parsers[i].ticks <= 0) {
						tickEnded = true;
						handleNoteEnd(i);
					}
				}
			}
			if(tickEnded) handleRest(minTicks);
			// Check if we're done
			done = true;
			for(int i = 0; i < num_channels; i++) {
				if(!parsers[i].ended || parsers[i].ticks > 0) done = false;
			}
		}
	}
private:
	virtual void handleRest(int r) {}
	virtual void handleNote(int chan, int note) {}
	virtual void handleNoteEnd(int chan) {}
	virtual void handleSetVoiceNum(int chan, uint8_t t) {}
	virtual void handleVolumeInc(int chan) {}
	virtual void handleVolumeDec(int chan) {}
	virtual void handleSetVolume(int chan, uint8_t v) {}
};

void MDXSerialParser::handleSetTempo(uint8_t tempo) {
	mdx->tempo = tempo;
}

void MDXSerialParser::handleNote(uint8_t n, uint8_t duration) {
	ticks = duration;
	note = n;
	mdx->handleNote(channel, n);
}

void MDXSerialParser::handleRest(uint8_t duration) {
	ticks = duration;
	note = -1;
}

void MDXSerialParser::handleSetVoiceNum(uint8_t t) {
	curVoice = t;
	mdx->handleSetVoiceNum(channel, t);
}

void MDXSerialParser::handleVolumeInc() { mdx->handleVolumeInc(channel); }
void MDXSerialParser::handleVolumeDec() { mdx->handleVolumeDec(channel); }
void MDXSerialParser::handleSetVolume(uint8_t v) { mdx->handleSetVolume(channel, v); }

#endif /* MDXSERIALIZER_H_ */
