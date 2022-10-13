#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <memory>

#include <sidplay/sidplay2.h>

void usage(const char* argv0, FILE* out)
{
	fprintf(out, "Usage: %s INPUT\n", argv0);
	fprintf(out, "Converts the first 244s of a given SID tune into a synsedizer song.\n", argv0);
	fprintf(out, "Can be put in a pipeline like:\n\tsidsedizer/sidsedizer foo.sid | ./synsedizer | aplay -\n", argv0);
}

const double kPalCounterFromFreq = 18.0 * (1 << 24) / 17734475;
const double kNtscCounterFromFreq = 14.0 * (1 << 24) / 14318182;
const int kSampleRate = 44100;
const double kCyclesPerSample = 985248 / kSampleRate;

enum {
    // Base for each voice.
    V1 = 0,
    V2 = 7,
    V3 = 14,

    // Frequency, x=  f * (
    FREQ_LO = 0,
    FREQ_HI = 1,
    // Control: low bit is gate on/off, high nibble is waveform, other low bits
    // are ring/sync that we don't support anyway. and test? whatever that is.
    CR = 4,
};

struct asidemu: public sidemu
{
	uint8_t regs[256];
	c64env* env;
	FILE* outfile;
	unsigned long lastWrite;
	unsigned long writes;

	asidemu(FILE* outfile,
			sidbuilder* builder, c64env* env, sid2_model_t model):
		sidemu(builder),
		env(env),
		outfile(outfile),
		lastWrite(0),
		writes(0)
	{
		reset(0);
		fprintf(outfile, "r 44100\n");
	}

	void reset(uint8_t vol) override
	{
		// FIXME What's the actual initial state of a SID?
		memset(regs, 0, sizeof(regs));
	}

	uint8_t read(uint8_t addr) override
	{
		// We don't record reads...
		return regs[addr];
	}

    static bool relevantReg(uint8_t addr)
    {
        uint8_t vreg = addr % 7;
        return addr < 21 && (vreg == FREQ_LO  || vreg == FREQ_HI || vreg == CR);
    }

	void write(uint8_t addr, uint8_t data) override
	{
        const uint8_t delta = regs[addr] ^ data;
        regs[addr] = data;

        // If addr is gate or addr is pitch and corresponding channel's gate is
        // on, output note start/stop.
		if (relevantReg(addr))
		{
            uint8_t voice = addr / 7;
            uint8_t vreg = addr % 7;
            if (vreg == CR && (delta & 1)) {
                // gate change: write note on/off
                if ((data & 1) && (data & 0x70)) {
                    note_on(voice);
                } else {
                    note_off(voice);
                }
            } else if ((vreg == FREQ_LO || vreg == FREQ_HI)
                        && (regs[voice * 7 + CR] & 1)
                        // Ignore noise-only tones (0x80)
                        && (regs[voice * 7 + CR] & 0x70)) {
                // frequency change: write note off and on
                note_off(voice);
                note_on(voice);
            }
        }

        writes++;
    }

    uint16_t get_incr(uint8_t voice)
    {
        return
            uint16_t(regs[voice * 7 + FREQ_HI]) << 8 |
            regs[voice * 7 + FREQ_LO];
    }

    void note_on(uint8_t voice)
    {
        emit_sleep();
        double frequency = get_incr(voice) / kPalCounterFromFreq;
        int wavelength = lrint(kSampleRate / frequency);
        fprintf(outfile, "%c %d\n", 'a' + voice, wavelength);
    }

    void note_off(uint8_t voice)
    {
        emit_sleep();
        fprintf(outfile, "%c\n", 'A' + voice);
    }

    void emit_sleep()
    {
		unsigned long now = getTime();
        unsigned samples = (now - lastWrite) / kCyclesPerSample;
        if (samples) fprintf(outfile, "s %u\n", samples);
        lastWrite = now;
    }

	event_clock_t getTime() const
	{
		return env->context().getTime(env->context().phase());
	}

	const char* credits() override { return "SID to synsedizer converter"; }
	const char* error() override { return NULL; }

	int_least32_t output(uint8_t v) override
	{
		return 1;
	}

	void voice(uint_least8_t, uint_least8_t, bool) override {}
	void gain(int_least8_t) override {}
};

struct asidbuilder: public sidbuilder
{
	FILE* outfile;
	bool locked;
    std::unique_ptr<sidemu> emu;

	asidbuilder(FILE* outfile):
		sidbuilder("SID to synsedizer converter"),
		outfile(outfile),
		locked(false)
	{}

	sidemu* lock(c64env* env, sid2_model_t model) override
	{
		if (!locked && !emu)
		{
			emu = std::make_unique<asidemu>(outfile, this, env, model);
			locked = emu != nullptr;
			return emu.get();
		}
		return nullptr;
	}
	void unlock(sidemu* device) override
	{
		assert(locked && emu.get() == device);
		locked = false;
		emu.reset();
	}

	const char* credits() override { return "SID to synsedizer converter"; }
	const char* error() const override { return NULL; }
};

int main(int argc, const char* argv[])
{
	sidplay2 player;
	//player.debug(true, stderr);

	if (argc != 2)
	{
		usage(argv[0], stderr);
		return 1;
	}

	const char* infile = argv[1];

	sid2_config_t cfg = player.config();
	cfg.clockDefault = SID2_CLOCK_PAL;
	cfg.clockForced = false;
	cfg.clockSpeed = SID2_CLOCK_PAL;
	cfg.frequency = 44100;
	cfg.precision = SID2_DEFAULT_PRECISION;
	cfg.emulateStereo = false;
	cfg.sidDefault = SID2_MOS6581;
	cfg.sidModel = SID2_MOS6581;
	cfg.optimisation = SID2_DEFAULT_OPTIMISATION;
	cfg.sidSamples = true;
	cfg.playback = sid2_mono;

	asidbuilder builder(stdout);
	cfg.sidEmulation = &builder;
	int i = player.config(cfg);
	if (i < 0)
	{
		fprintf(stderr, "config status: %d\n", i);
		abort();
	}

	SidTune tune(infile);
	tune.selectSong(0);
	if (player.load(&tune) < 0)
	{
		fprintf(stderr, "load failed\n");
		abort();
	}
	SidTuneInfo subInfo;
	tune.getInfo(subInfo);

	sid2_info_t info = player.info();
    SidTuneInfo tuneInfo = *player.info().tuneInfo;
	/*if (player.fastForward(0) < 0)
	{
		fprintf(stderr, "fastForward to beginning failed\n");
		abort();
	}*/

	char dummy[44100];
    // TODO Find a better way to do this.....
	int seconds = 244; // The length of Spellbound.sid
	unsigned long samples = 0;
	player.play(dummy, sizeof(dummy));
	while (player.state() != sid2_stopped)
	{
		int res = player.play(dummy, sizeof(dummy));
		assert(res == sizeof(dummy));
		samples += res;
		//printf("%lu: State now %d (play returned %d)\n", (unsigned long)player.time(), player.state(), x);
		//printf("Error: %s\n", player.error());
		if ((float)player.time() / player.timebase() > seconds)
		{
			break;
		}
	}
}
