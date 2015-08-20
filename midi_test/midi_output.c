/*
 * midi_output.c
 *
 *  Created on: Aug 3, 2015
 *      Author: james
 *
 *      TODO: write so it accepts pointer to buffer (or contains its own)
 *      		continuously updates output based on buffer
 *      		(this thread responsible for continuing output?  or calling thread for continuing buffer update?)
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
//#include <poll.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <midi_def.h>
#include <math.h>
//#include <midi_proc.h>
#include <midi_output.h>
#include <midi_synth.h>

snd_pcm_t *playback_handle;
//static short outbuf[OUT_BUF_SIZE];
static short *outbuf;

int nfds;
int err;
//struct pollfd *pfds;

static pthread_t outputThread;
static void *outputfunction (void *arg);

/*void add_sine_wave(int16_t* buffer, int buffer_length, float frequency, float sampling_ratio, float amplitude)
{
	int i;

    for (i = 0; i < buffer_length; i++)
    {
        float theta = ((float)i / sampling_ratio) * M_PI;
        // make sure to correct for overflows and underflows
        buffer[i] += (int16_t)(sin(theta * frequency) * 32767.0f * amplitude);
    }
}*/

void OutputInit(void *arg, short *buffer)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;
	char *portname = arg;
	unsigned int rate = 44100;

	outbuf = buffer;

	if ((err = snd_pcm_open (&playback_handle, portname, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		/*fprintf (stderr, "cannot open audio device %s (%s)\n",
				portname,
				snd_strerror (err));*/
		printf ("cannot open audio device %s (%s)\n",
						portname,
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		/*fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				snd_strerror (err));*/
		printf ("cannot allocate hardware parameter structure (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		/*fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				snd_strerror (err));*/
		printf ("cannot initialize hardware parameter structure (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		/*fprintf (stderr, "cannot set access type (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set access type (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
		/*fprintf (stderr, "cannot set sample format (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set sample format (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
		/*fprintf (stderr, "cannot set sample rate (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set sample rate (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 2)) < 0) {
		/*fprintf (stderr, "cannot set channel count (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set channel count (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		/*fprintf (stderr, "cannot set parameters (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set parameters (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	/* tell ALSA to wake us up whenever 4096 or more frames
		   of playback data can be delivered. Also, tell
		   ALSA that we'll start the device ourselves.
	 */

	if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
		/*fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
				snd_strerror (err));*/
		printf ("cannot allocate software parameters structure (%s)\n",
						snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
		/*fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
				snd_strerror (err));*/
		printf ("cannot initialize software parameters structure (%s)\n",
						snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, 4096)) < 0) {
		/*fprintf (stderr, "cannot set minimum available count (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set minimum available count (%s)\n",
						snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
		/*fprintf (stderr, "cannot set start mode (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set start mode (%s)\n",
						snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
		/*fprintf (stderr, "cannot set software parameters (%s)\n",
				snd_strerror (err));*/
		printf ("cannot set software parameters (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	/* the interface will interrupt the kernel every 4096 frames, and ALSA
		   will wake up this program very soon after that.
	 */

	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		/*fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				snd_strerror (err));*/
		printf ("cannot prepare audio interface for use (%s)\n",
						snd_strerror (err));
		exit (1);
	}

	pthread_create(&outputThread, NULL, outputfunction, NULL);
}

int
playback_callback (snd_pcm_sframes_t nframes)
{
	int err;

	//printf ("playback callback called with %u frames\n", nframes);

	/* ... fill buf with data ... */

	if ((err = snd_pcm_writei (playback_handle, outbuf, nframes)) < 0) {
		//fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
		printf ("write failed (%s)\n", snd_strerror (err));
	}

	return err;
}

static void *outputfunction (void *arg)
{
	snd_pcm_sframes_t frames_to_deliver;

	printf("output start\n");

	//add_sine_wave(outbuf, OUT_BUF_SIZE, 440.0f, 44100.0f, 0.5f);
	//add_sine_wave(outbuf, OUT_BUF_SIZE, 392.0f, 44100.0f, 0.5f);
	//add_sine_wave(outbuf, OUT_BUF_SIZE, 349.23f, 44100.0f, 0.5f);

	while (1) {

		/* wait till the interface is ready for data, or 1 second
			   has elapsed.
		 */

		if ((err = snd_pcm_wait (playback_handle, 900)) < 0) {
			//fprintf (stderr, "poll failed (%s)\n", strerror (errno));
			printf ("poll failed (%s)\n", strerror (errno));
			break;
		}

		/* find out how much space is available for playback data */

		if ((frames_to_deliver = snd_pcm_avail_update (playback_handle)) < 0) {
			if (frames_to_deliver == -EPIPE) {
				//fprintf (stderr, "an xrun occured\n");
				printf ("an xrun occured\n");
				break;
			} else {
				//fprintf (stderr, "unknown ALSA avail update return value (%d)\n",
				//		(int)frames_to_deliver);
				printf ("unknown ALSA avail update return value (%d)\n",
										(int)frames_to_deliver);
				break;
			}
		}

		//frames_to_deliver = frames_to_deliver > 4096 ? 4096 : frames_to_deliver;
		frames_to_deliver = frames_to_deliver > OUT_BUF_SIZE ? OUT_BUF_SIZE : frames_to_deliver;

		/* deliver the data */

		if (playback_callback (frames_to_deliver) != frames_to_deliver) {
			//fprintf (stderr, "playback callback failed\n");
			printf ("playback callback failed\n");
			break;
		}
	}

	snd_pcm_close (playback_handle);
	exit (0);
}
