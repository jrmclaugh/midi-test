/*
 * midi_proc.c
 *
 *  Created on: Jul 24, 2015
 *      Author: james
 */

#include <stddef.h>
#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <pthread.h>
#include <midi_def.h>
#include <midi_synth.h>
#include <midi_proc.h>

// button light colors
#define RED_FULL	15
#define RED_LOW		13
#define AMBER_FULL	63
#define	AMBER_LOW	29
#define YELLOW_FULL	62
#define GREEN_FULL	60
#define GREEN_LOW	28

#define NOTE_ON		GREEN_FULL
#define NOTE_OFF	0

struct midinodes {
	   snd_rawmidi_t* midiin;
	   snd_rawmidi_t* midiout;
};

static struct midinodes midi;
static pthread_t midithread;

static void *midifunction(void *arg);
static void errormessage(const char *format, ...);

int MidiInit(void *arg)
{
	int status;
	int mode = SND_RAWMIDI_SYNC;
	char *portname = arg;
	midi.midiin = NULL;
	midi.midiout = NULL;
	char resetPad[3] = {0xB0, 0x00, 0x00};

	// open in/out
	if ((status = snd_rawmidi_open(&(midi.midiin), &(midi.midiout), portname, mode)) < 0) {
		errormessage("Problem opening MIDI input: %s", snd_strerror(status));
		exit(1);
	}

	// reset launchpad (replace with generic reset)
	if ((status = snd_rawmidi_write(midi.midiout, resetPad, 3)) < 0) {
		errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
		exit(1);
	}

	return NULL;
}

int MidiStartProc(void *arg)
{
	int status;

	// type "man pthread_create" for more information about this function:
	status = pthread_create(&midithread, NULL, midifunction, &midi);
	if (status == -1) {
		errormessage("Unable to create MIDI input thread.");
		exit(1);
	}
	return NULL;
}

int MidiStopProc(void *arg)
{
	snd_rawmidi_close(midi.midiout);
	snd_rawmidi_close(midi.midiin);
	midi.midiout  = midi.midiin = NULL;    // snd_rawmidi_close() does not clear invalid pointer,

	return NULL;
}

void *midifunction(void *arg)
{
	// this is the parameter passed via last argument of pthread_create():
	struct midinodes *midi = (struct midinodes*)arg;
	char buffer[1];
	int  count = 0;
	int status;
	int byteNum = 0;
	char outBuffer[3] = {0x90, 0x00, 15};
	note currentNote;

	printf("midi thread start\n");

	while (1) {
		if ((status = snd_rawmidi_read(midi->midiin, buffer, 1)) < 0) {
			errormessage("Problem reading MIDI input: %s", snd_strerror(status));
		}
		count++;
		// determine part of message
		if(byteNum == 0)
		{
			// note
			outBuffer[1] = currentNote.key = buffer[0];
			byteNum = 1;
		}
		else
		{
			if(buffer[0] > 0)
				outBuffer[2] = currentNote.vel = NOTE_ON;
			else
				outBuffer[2] = currentNote.vel = NOTE_OFF;
			byteNum = 0;
		}
		// after velocity, write
		if(byteNum == 0)
		{
			if ((status = snd_rawmidi_write(midi->midiout, outBuffer, 3)) < 0) {
				errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
				exit(1);
			}
			printf("Note: %d %d %d \n", outBuffer[0], currentNote.key, currentNote.vel);
			SynthNoteStart(currentNote);
		}
		fflush(stdout);
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// error -- print error message
//

void errormessage(const char *format, ...) {
   va_list ap;
   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);
   putc('\n', stderr);
}
