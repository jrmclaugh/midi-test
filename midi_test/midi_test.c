/*
 * midi-test.c
 *
 *  Created on: Jul 20, 2015
 *      Author: james
 */

//#define MIDI_INPUT	1
//#define MIDI_OUTPUT	1
//#define MIDI_IN_OUT	1
//#define MIDI_IN_OUT_THREADED	1
#define MIDI_IN_OUT_LIB	1

// From: https://ccrma.stanford.edu/~craig/articles/linuxmidi/

#ifdef MIDI_OUTPUT
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May  9 18:57:48 PDT 2009
// Last Modified: Sat May  9 19:48:31 PDT 2009
// Filename:      alsarawmidiout.c
// Syntax:        C; ALSA 1.0
// $Smake:        gcc -o %b %f -lasound
//
// Description:	  Send a MIDI note to a synthesizer using the ALSA rawmidi
//                interface.  Reverse engineered from amidi.c (found in
//                ALSA utils 1.0.19 program set).
//
// First double-check that you have the ALSA system installed on your computer
// by running this command-line command:
//    cat /proc/asound/version
// Which should return something like:
//   Advanced Linux Sound Architecture Driver Version 1.0.17.
// This example program should work if the version number (1.0.17 in this
// case) is "1".
//
// Online documentation notes:
//
// http://www.alsa-project.org/alsa-doc/alsa-lib/rawmidi.html
//
// Using SND_RAWMIDI_NONBLOCK flag for snd_rawmidi_open() or
// snd_rawmidi_open_lconf() instruct device driver to return the -EBUSY
// error when device is already occupied with another application. This
// flag also changes behaviour of snd_rawmidi_write() and snd_rawmidi_read()
// returning -EAGAIN when no more bytes can be processed.
//
// Using SND_RAWMIDI_APPEND flag (output only) instruct device driver to
// append contents of written buffer - passed by snd_rawmidi_write() -
// atomically to output ring buffer in the kernel space. This flag also
// means that the device is not opened exclusively, so more applications can
// share given rawmidi device. Note that applications must send the whole
// MIDI message including the running status, because another writting
// application might break the MIDI message in the output buffer.
//
// Using SND_RAWMIDI_SYNC flag (output only) assures that the contents of
// output buffer specified using snd_rawmidi_write() is always drained before
// the function exits. This behaviour is the same as snd_rawmidi_write()
// followed immediately by snd_rawmidi_drain().
//
// http://www.alsa-project.org/alsa-doc/alsa-lib/group___raw_midi.html
//
// int snd_rawmidi_open(snd_rawmidi_t** input, snd_rawmidi_t output,
//                                             const char* name, int mode)
//    intput   == returned input handle (NULL if not wanted)
//    output   == returned output handle (NULL if not wanted)
//    name     == ASCII identifier of the rawmidi handle, such as "hw:1,0,0"
//    mode     == open mode (see mode descriptions above):
//                SND_RAWMIDI_NONBLOCK, SND_RAWMIDI_APPEND, SND_RAWMIDI_SYNC
//
// int snd_rawmidi_close(snd_rawmidi_t* rawmidi)
//    Close a deviced opended by snd_rawmidi_open().  Returns an negative
//    error code if there was an error closing the device.
//
// int snd_rawmidi_write(snd_rawmidi_t* output, char* data, int datasize)
//    output   == midi output pointer setup with snd_rawmidi_open().
//    data     == array of bytes to send.
//    datasize == number of bytes in the data array to write to MIDI output.
//
// const char* snd_strerror(int errornum)
//    errornum == error number returned by an ALSA snd__* function.
//    Returns a string explaining the error number.
//

#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <unistd.h>             /* for sleep() function */

// function declarations:
void errormessage(const char *format, ...);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int status;
   int mode = SND_RAWMIDI_SYNC;
   snd_rawmidi_t* midiout = NULL;
   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }
   if ((status = snd_rawmidi_open(NULL, &midiout, portname, mode)) < 0) {
      errormessage("Problem opening MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   //char noteon[3]  = {0x90, 60, 100};
   //char noteoff[3] = {0x90, 60, 0};
   //char noteon[3]  = {0x90, 60, 11};
   char noteon[3]  = {0xB0, 0x00, 0x7F};
   char noteoff[3] = {0xB0, 0x00, 0x00};

   if ((status = snd_rawmidi_write(midiout, noteon, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   sleep(5);  // pause the program for one second to allow note to sound.

   if ((status = snd_rawmidi_write(midiout, noteoff, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   snd_rawmidi_close(midiout);
   midiout = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
   return 0;          // so might be a good idea to erase it after closing.
}

///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// error -- Print an error message.
//

void errormessage(const char *format, ...) {
   va_list ap;
   va_start(ap, format);
   vfprintf(stderr, format, ap);
   va_end(ap);
   putc('\n', stderr);
}
#endif

#ifdef MIDI_INPUT
//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat May  9 22:03:40 PDT 2009
// Last Modified: Sat May  9 22:03:46 PDT 2009
// Filename:      alsarawmidiin.c
// Syntax:        C; ALSA 1.0
// $Smake:        gcc -o %b %f -lasound
//
// Description:	  Receive MIDI data from a synthesizer using the ALSA rawmidi
//                interface.  Reversed engineered from amidi.c (found in ALSA
//                utils 1.0.19 program set).
//
// First double-check that you have the ALSA system installed on your computer
// by running this command-line command:
//    cat /proc/asound/version
// Which should return something like:
//   Advanced Linux Sound Architecture Driver Version 1.0.17.
// This example program should work if the version number (1.0.17 in this
// case) is "1".
//
// Online documentation notes:
//
// http://www.alsa-project.org/alsa-doc/alsa-lib/rawmidi.html
//
// Using SND_RAWMIDI_NONBLOCK flag for snd_rawmidi_open() or
// snd_rawmidi_open_lconf() instruct device driver to return the -EBUSY
// error when device is already occupied with another application. This
// flag also changes behaviour of snd_rawmidi_write() and snd_rawmidi_read()
// returning -EAGAIN when no more bytes can be processed.
//
// http://www.alsa-project.org/alsa-doc/alsa-lib/group___raw_midi.html
//
// int snd_rawmidi_open(snd_rawmidi_t** input, snd_rawmidi_t output,
//                                             const char* name, int mode)
//    intput   == returned input handle (NULL if not wanted)
//    output   == returned output handle (NULL if not wanted)
//    name     == ASCII identifier of the rawmidi handle, such as "hw:1,0,0"
//    mode     == open mode (see mode descriptions above):
//                SND_RAWMIDI_NONBLOCK, SND_RAWMIDI_APPEND, SND_RAWMIDI_SYNC
//
// int snd_rawmidi_close(snd_rawmidi_t* rawmidi)
//    Close a deviced opended by snd_rawmidi_open().  Returns an negative
//    error code if there was an error closing the device.
//
// int snd_rawmidi_write(snd_rawmidi_t* output, char* data, int datasize)
//    output   == midi output pointer setup with snd_rawmidi_open().
//    data     == array of bytes to send.
//    datasize == number of bytes in the data array to write to MIDI output.
//
// const char* snd_strerror(int errornum)
//    errornum == error number returned by an ALSA snd__* function.
//    Returns a string explaining the error number.
//

#include <alsa/asoundlib.h>     /* Interface to the ALSA system */

// function declarations:
void errormessage(const char *format, ...);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
   int status;
   int mode = SND_RAWMIDI_SYNC;
   snd_rawmidi_t* midiin = NULL;
   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }
   if ((status = snd_rawmidi_open(&midiin, NULL, portname, mode)) < 0) {
      errormessage("Problem opening MIDI input: %s", snd_strerror(status));
      exit(1);
   }

   int maxcount = 1000;   // Exit after this many bytes have been received.
   int count = 0;         // Current count of bytes received.
   char buffer[1];        // Storage for input buffer received
   while (count < maxcount) {
      if ((status = snd_rawmidi_read(midiin, buffer, 1)) < 0) {
         errormessage("Problem reading MIDI input: %s", snd_strerror(status));
      }
      count++;
      if ((unsigned char)buffer[0] >= 0x80) {   // command byte: print in hex
         printf("0x%x ", (unsigned char)buffer[0]);
      } else {
         printf("%d ", (unsigned char)buffer[0]);
      }
      fflush(stdout);
      if (count % 20 == 0) {  // print a newline to avoid line-wrapping
         printf("\n");
      }
   }

   snd_rawmidi_close(midiin);
   midiin  = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
   return 0;          // so might be a good idea to erase it after closing.
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

#endif

#ifdef MIDI_IN_OUT

#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <pthread.h>

#define RED_FULL	15
#define RED_LOW		13
#define AMBER_FULL	63
#define	AMBER_LOW	29
#define YELLOW_FULL	62
#define GREEN_FULL	60
#define GREEN_LOW	28

#define NOTE_ON		GREEN_FULL
#define NOTE_OFF	0

// function declarations:
void errormessage(const char *format, ...);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   int status;
   int mode = SND_RAWMIDI_SYNC;
   snd_rawmidi_t* midiin = NULL;
   snd_rawmidi_t* midiout = NULL;
   int byteNum = 0;
   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }

   if ((status = snd_rawmidi_open(&midiin, &midiout, portname, mode)) < 0) {
      errormessage("Problem opening MIDI input: %s", snd_strerror(status));
      exit(1);
   }

   int maxcount = 1000;   // Exit after this many bytes have been received.
   int count = 0;         // Current count of bytes received.
   char buffer[1];        // Storage for input buffer received
   char outBuffer[3] = {0x90, 0x00, 15};
   char resetPad[3] = {0xB0, 0x00, 0x00};

   if ((status = snd_rawmidi_write(midiout, resetPad, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   while (count < maxcount) {
      if ((status = snd_rawmidi_read(midiin, buffer, 1)) < 0) {
         errormessage("Problem reading MIDI input: %s", snd_strerror(status));
      }
      count++;
      // determine part of message
      if(byteNum == 0)
      {
    	  // note
    	  outBuffer[1] = buffer[0];
    	  byteNum = 1;
      }
      else
      {
    	  if(buffer[0] == 127)
    		  outBuffer[2] = NOTE_ON;
    	  else
    		  outBuffer[2] = NOTE_OFF;
    	  byteNum = 0;
      }
      printf("Byte: %d \n", (unsigned char)buffer[0]);
      // after velocity, write
      if(byteNum == 0)
      {
    	  if ((status = snd_rawmidi_write(midiout, outBuffer, 3)) < 0) {
    		  errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
    		  exit(1);
    	  }
    	  printf("Buf: %d %d %d \n", outBuffer[0], outBuffer[1], outBuffer[2]);
      }
      fflush(stdout);
   }

   snd_rawmidi_close(midiout);
   snd_rawmidi_close(midiin);
   midiout  = midiin = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
   return 0;          // so might be a good idea to erase it after closing.
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

#endif

#ifdef MIDI_IN_OUT_THREADED
#include <alsa/asoundlib.h>     /* Interface to the ALSA system */
#include <pthread.h>

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

// function declarations:
void errormessage(const char *format, ...);
void *midifunction(void *arg);

///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   int status;
   int mode = SND_RAWMIDI_SYNC;
   struct midinodes midi;
   pthread_t midithread;
   int byteNum = 0;
   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }

   midi.midiin = NULL;
   midi.midiout = NULL;

   if ((status = snd_rawmidi_open(&(midi.midiin), &(midi.midiout), portname, mode)) < 0) {
      errormessage("Problem opening MIDI input: %s", snd_strerror(status));
      exit(1);
   }

   int maxcount = 1000;   // Exit after this many bytes have been received.
   int count = 0;         // Current count of bytes received.
   char buffer[1];        // Storage for input buffer received
   char outBuffer[3] = {0x90, 0x00, 15};
   char resetPad[3] = {0xB0, 0x00, 0x00};

   if ((status = snd_rawmidi_write(midi.midiout, resetPad, 3)) < 0) {
      errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
      exit(1);
   }

   // type "man pthread_create" for more information about this function:
   status = pthread_create(&midithread, NULL, midifunction, &midi);
   if (status == -1) {
      errormessage("Unable to create MIDI input thread.");
      exit(1);
   }

   while(1)
   {

   }

   snd_rawmidi_close(midi.midiout);
   snd_rawmidi_close(midi.midiin);
   midi.midiout  = midi.midiin = NULL;    // snd_rawmidi_close() does not clear invalid pointer,
   return 0;          // so might be a good idea to erase it after closing.
}

///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// midiinfunction -- Thread function which waits around until a MIDI
//      input byte arrives and then it prints the byte to the terminal.
//      This thread function does not end gracefully when the program
//      stops.

void *midifunction(void *arg) {
   // this is the parameter passed via last argument of pthread_create():
	struct midinodes *midi = (struct midinodes*)arg;
   //snd_rawmidi_t* midiin = (snd_rawmidi_t*)arg.midiin;
   //snd_rawmidi_t* midiout = (snd_rawmidi_t*)arg.midiout;
   char buffer[1];
   int  count = 0;
   int status;
   int byteNum = 0;
   char outBuffer[3] = {0x90, 0x00, 15};

   while (1) {
         if ((status = snd_rawmidi_read(midi->midiin, buffer, 1)) < 0) {
            errormessage("Problem reading MIDI input: %s", snd_strerror(status));
         }
         count++;
         // determine part of message
         if(byteNum == 0)
         {
       	  // note
       	  outBuffer[1] = buffer[0];
       	  byteNum = 1;
         }
         else
         {
       	  if(buffer[0] == 127)
       		  outBuffer[2] = NOTE_ON;
       	  else
       		  outBuffer[2] = NOTE_OFF;
       	  byteNum = 0;
         }
         printf("Byte: %d \n", (unsigned char)buffer[0]);
         // after velocity, write
         if(byteNum == 0)
         {
       	  if ((status = snd_rawmidi_write(midi->midiout, outBuffer, 3)) < 0) {
       		  errormessage("Problem writing to MIDI output: %s", snd_strerror(status));
       		  exit(1);
       	  }
       	  printf("Buf: %d %d %d \n", outBuffer[0], outBuffer[1], outBuffer[2]);
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
#endif

#ifdef MIDI_IN_OUT_LIB

#include <midi_proc.h>
#include <string.h>
#include <midi_synth.h>

// function declarations:


///////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

   const char* portname = "hw:1,0,0";  // see alsarawportlist.c example program
   if ((argc > 1) && (strncmp("hw:", argv[1], 3) == 0)) {
      portname = argv[1];
   }

   MidiInit((void*)portname);
   SynthInit();

   MidiStartProc((void*)NULL);

   while(1)
   {

   }

   MidiStopProc((void*)NULL);

   return 0;          // so might be a good idea to erase it after closing.
}


#endif
