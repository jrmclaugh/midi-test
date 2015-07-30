/*
 * midi_synth.c
 *
 *  Created on: Jul 28, 2015
 *      Author: james
 */

#include <unistd.h>
#include <pthread.h>
#include <midi_proc.h>
#include <stdio.h>
#include <stdlib.h>

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

void SynthInit()
{
	notes = noteQueueInit();
	// start thread
	pthread_create(&synthThread, NULL, synthfunction, NULL);
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

void SynthNoteStop(note *key)
{
	key->vel = 0;
	noteQueueAdd(notes,key);
}

static void synthUpdateOutput(note newNote)
{
	// actual synth work
	// TODO: need to figure out timing and calculations/tables
	// TODO: not working, yet (not receiving/printing, move to synthUpdateOutput, pass note)
	printf("Synth Byte: %d %d\n", (unsigned char)newNote.key, newNote.vel);
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
		synthUpdateOutput(newNote);
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
