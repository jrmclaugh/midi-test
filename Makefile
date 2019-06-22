midi_test : midi_test.c midi_def.h midi_output.c \
            midi_proc.c midi_synth.c midi_launchpad_notes.h \
            midi_output.h midi_proc.h midi_synth.h
	gcc midi_output.c midi_proc.c midi_synth.c midi_test.c \
		-I . -I /usr/include -lpthread -lm -lasound -o midi_test

clean : 
	rm -f midi_test
