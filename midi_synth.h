/*
 * midi_synth.h
 *
 *  Created on: Jul 28, 2015
 *      Author: james
 */

#ifndef MIDI_SYNTH_H_
#define MIDI_SYNTH_H_
#include <stdlib.h>

#define OUT_BUF_SIZE	44100

void SynthInit(short *buffer);
void SynthNoteStart(note key);
void SynthNoteStop(note key);
void add_sine_wave(int16_t* buffer, int buffer_length, float frequency, float sampling_ratio, float amplitude);

#endif /* MIDI_SYNTH_H_ */
