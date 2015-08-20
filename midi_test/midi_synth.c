/*
 * midi_synth.c
 *
 *  Created on: Jul 28, 2015
 *      Author: james
 */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <midi_def.h>
#include <midi_proc.h>
#include <midi_synth.h>
//#include <midi_output.h>
#include <midi_launchpad_notes.h>

#define QUEUESIZE 10

typedef struct {
	note notes[QUEUESIZE];
	long head, tail;
	int full, empty;
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
} noteQueue;

static noteQueue *noteQueueInit (void);
static void noteQueueAdd (noteQueue *q, note *in);
static void noteQueueDelete (noteQueue *q);
static void noteQueueDel (noteQueue *q, note *out);

static noteQueue *notes;
static pthread_t synthThread;
static void *synthfunction(void *arg);
static short *outputBuffer;

void SynthInit(short *buffer)
{
	notes = noteQueueInit();
	outputBuffer = buffer;
	// start thread
	pthread_create(&synthThread, NULL, synthfunction, NULL);

	//add_sine_wave(outputBuffer, OUT_BUF_SIZE, 440.0f, 44100.0f, 0.5f);
	//add_sine_wave(outputBuffer, OUT_BUF_SIZE, 392.0f, 44100.0f, 0.5f);
	//add_sine_wave(outputBuffer, OUT_BUF_SIZE, 349.23f, 44100.0f, 0.5f);
}

void SynthNoteStart(note key)
{
	// act as "producer" (called from other thread)
	// lock queue
	pthread_mutex_lock (notes->mut);
	// queue note (wait if full)
	noteQueueAdd(notes, &key);
	// unlock queue
	pthread_mutex_unlock (notes->mut);
	pthread_cond_signal(notes->notEmpty);

}

void SynthNoteStop(note key)
{
	key.vel = 0;
	noteQueueAdd(notes, &key);
}

void add_sine_wave(int16_t* buffer, int buffer_length, float frequency, float sampling_ratio, float amplitude)
{
	int i;

    for (i = 0; i < buffer_length; i++)
    {
        float theta = ((float)i / sampling_ratio) * M_PI;
        // make sure to correct for overflows and underflows
        buffer[i] += (int16_t)(sin(theta * frequency) * 32767.0f * amplitude);
    }
}

static void synthUpdateOutput(int16_t *buf, note newNote)
{
	// actual synth work
	// TODO: need to figure out timing and calculations/tables
	printf("Synth Byte: %d %d\n", (unsigned char)newNote.key, newNote.vel);

	// process new note
	if(newNote.vel > 0)
	{
		// 1. add note to list of notes
		// 2. add note at velocity to buffer
	}
	else
	{
		// 1. clear buffer
		// memset(buf, 0, sizeof(*buf));
		// 2. remove note from list of notes
		// 3. reassemble buffer based on still-playing notes
	}
	// add sine
	//add_sine_wave(buf, OUT_BUF_SIZE, 440.0f, 44100.0f, 0.5f);
	memset(buf, 0, OUT_BUF_SIZE);
	if(newNote.vel > 0)
	{
		add_sine_wave(buf, OUT_BUF_SIZE, noteArray[newNote.key - NOTE_BASE], 44100.0f, 0.5f);
	}
}

static void *synthfunction(void *arg)
{
	note newNote;

	printf("synth thread start\n");

	while(1)
	{
		// lock queue
		pthread_mutex_lock (notes->mut);
		// dequeue note (wait if empty)
		while (notes->empty) {
			printf ("note queue EMPTY.\n");
			pthread_cond_wait (notes->notEmpty, notes->mut);
		}
		noteQueueDel (notes, &newNote);
		// unlock queue
		pthread_mutex_unlock (notes->mut);
		// play note (update note(s?) playing?)
		synthUpdateOutput(outputBuffer, newNote);
		// wait for not full condition
		pthread_cond_signal (notes->notFull);
	}

	noteQueueDelete (notes);

	return NULL;
}

static noteQueue *noteQueueInit (void)
{
	noteQueue *q;

	q = (noteQueue *)malloc (sizeof (noteQueue));
	if (q == NULL) return (NULL);

	q->empty = 1;
	q->full = 0;
	q->head = 0;
	q->tail = 0;
	q->mut = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init (q->mut, NULL);
	q->notFull = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notFull, NULL);
	q->notEmpty = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init (q->notEmpty, NULL);

	return (q);
}

static void noteQueueDelete (noteQueue *q)
{
	pthread_mutex_destroy (q->mut);
	free (q->mut);
	pthread_cond_destroy (q->notFull);
	free (q->notFull);
	pthread_cond_destroy (q->notEmpty);
	free (q->notEmpty);
	free (q);
}

static void noteQueueAdd (noteQueue *q, note *in)
{
	q->notes[q->tail] = *in;
	q->tail++;
	if (q->tail == QUEUESIZE)
		q->tail = 0;
	if (q->tail == q->head)
		q->full = 1;
	q->empty = 0;

	return;
}

static void noteQueueDel (noteQueue *q, note *out)
{
	*out = q->notes[q->head];

	q->head++;
	if (q->head == QUEUESIZE)
		q->head = 0;
	if (q->head == q->tail)
		q->empty = 1;
	q->full = 0;

	return;
}
