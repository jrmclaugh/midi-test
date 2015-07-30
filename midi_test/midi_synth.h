/*
 * midi_synth.h
 *
 *  Created on: Jul 28, 2015
 *      Author: james
 */

#ifndef MIDI_SYNTH_H_
#define MIDI_SYNTH_H_

void SynthInit();
void SynthNoteStart(note key);
void SynthNoteStop(note key);

#endif /* MIDI_SYNTH_H_ */
