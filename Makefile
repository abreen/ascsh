LDFLAGS = -lascd -lfann -lgmp -lreadline
CLEAN = ascsh *.o

ascsh: libascd.h
	gcc -L/home/abreen/lib -o ascsh libascd.h main.c $(LDFLAGS)

clean:
	$(RM) $(wildcard $(CLEAN))
