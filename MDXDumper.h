#ifndef MDXDUMPER_H_
#define MDXDUMPER_H_

#include "MDX.h"
#include "tools.h"

class MDXDumperParser: public MDXChannelParser {
	virtual void handleRest(uint8_t duration) { printf("Rest %d (192 / %d)\n", duration, duration == 0 ? 0 : 192 / duration); }
	virtual void handleNote(uint8_t note, uint8_t duration) { printf("Note %d (%s%d) duration %d (192 / %d)\n", note, MDX::noteName(note), MDX::noteOctave(note), duration, duration == 0 ? 0 : 192 / duration); }
	virtual void handleCommand(uint8_t c, ...) { /* printf("Command 0x%02x: %s\n", c, commandName(c)); */ }
	virtual void handleVolumeInc() { printf("VolumeInc\n"); }
	virtual void handleVolumeDec() { printf("VolumeDec\n"); }
	virtual void handleDisableKeyOff() { printf("DisableKeyOff\n"); }
	virtual void handleSyncWait() { printf("SyncWait\n"); }
	virtual void handlePCM8Enable() { printf("PCM8Enable\n"); }
	virtual void handleSetTempo(uint8_t t) { printf("SetTempo %d\n", t); }
	virtual void handleSetVoiceNum(uint8_t t) { printf("SetVoiceNum %d\n", t); }
	virtual void handlePan(uint8_t p) { printf("Pan %d\n", p); }
	virtual void handleSetVolume(uint8_t v) { printf("SetVolume %d\n", v); }
	virtual void handleSoundLength(uint8_t l) { printf("SoundLength %d\n", l); }
	virtual void handleKeyOnDelay(uint8_t d) { printf("KeyOnDelay %d\n", d); }
	virtual void handleSyncSend(uint8_t s) { printf("SyncSend %d\n", s); }
	virtual void handleADPCMNoiseFreq(uint8_t f) { printf("ADPCMNoiseFreq %d\n", f); }
	virtual void handleLFODelaySetting(uint8_t d) { printf("LFODelaySetting %d\n", d); }
	virtual void handleRepeatStart(uint8_t r) { printf("RepeatStart %d\n", r); }
	virtual void handleRepeatEnd(int16_t r) { printf("RepeatEnd %d\n", r); }
	virtual void handleRepeatEscape(int16_t r) { printf("RepeatEscape %d\n", r); }
	virtual void handleDetune(int16_t d) { printf("Detune %d\n", d); }
	virtual void handlePortamento(int16_t t) { printf("Portamento %d\n", t); }
	virtual void handleSetOPMRegister(uint8_t reg, uint8_t val) { printf("SetOPMRegister %d %d;\n", reg, val); }
	virtual void handleDataEnd() { printf("DataEnd\n"); }
	virtual void handleDataEnd(int16_t end) { printf("DataEnd %d\n", end); }
	virtual void handleLFOPitch(uint8_t b, uint16_t period, uint16_t change) { printf("LFOPitch %d %d %d\n", b, period, change); }
	virtual void handleLFOPitchMPON() { printf("LFOPitchMPON\n"); }
	virtual void handleLFOPitchMPOF() { printf("LFOPitchMPOF\n"); }
	virtual void handleLFOVolume(uint8_t b, uint16_t period, uint16_t change) { printf("LFOVolume %d %d %d\n", b, period, change); }
	virtual void handleLFOVolumeMAON() { printf("LFOVolumeMAON\n"); }
	virtual void handleLFOVolumeMAOF() { printf("LFOVolumeMAOF\n"); }
	virtual void handleOPMLFO(uint8_t sync_wave, uint8_t lfrq, uint8_t pmd, uint8_t amd, uint8_t pms_ams) { printf("OPMLFO %d %d %d %d %d\n", sync_wave, lfrq, pmd, amd, pms_ams); }
	virtual void handleOPMLFOMHON() { printf("OPMLFOMHON\n"); }
	virtual void handleOPMLFOMHOF() { printf("OPMLFOMHOF\n"); }
	virtual void handleFadeOut(uint8_t f) { printf("FadeOut %d\n", f); }
	virtual void handlePCM8ExpansionShift() { printf("PCM8ExpansionShift\n"); }
	virtual void handleUndefinedCommand(uint8_t b) { printf("UndefinedCommand %d\n", b); }
	virtual void handleChannelStart(int chan) { printf("ChannelStart %c (%d)\n", MDX::channelName(chan), chan); }
	virtual void handleChannelEnd(int chan) { printf("ChannelEnd %c (%d)\n", MDX::channelName(chan), chan); }
};

class MDXDumper: public MDX {
public:
	MDXDumper(const char *filename) {
		MDXDumperParser p;
		load(filename, p);
	}

	virtual void handleHeader() {
		char *t = iconv_alloc(title, "utf-8", "Shift_JIS");
		printf("Title: \"%s\"\n", t);
		free(t);
		printf("PCM file: \"%s\"\n", pcm_file);
		printf("Voice offset: 0x%04x\n", file_base + Voice_offset);
		printf("MML offsets (%d):\n", mml_offset[0] / 2 - 1);
		for(int i = 0; i < mml_offset[0] / 2 - 1 &&  i < 16; i++) {
			printf("  %c: 0x%04x\n", channelName(i), mml_offset[i]);
		}
	}
	virtual void handleVoice(MDXVoice &v) { v.dump(); }
};

#endif /* MDXDUMPER_H_ */
