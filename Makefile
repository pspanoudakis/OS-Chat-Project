CC = gcc

# Compile options
# comment-out FLAG to re-enable the disabled warning (see README)
FLAG = -Wno-discarded-qualifiers
LDLIBS = -lcrypto

# Objective files
UTIL_OBJ = build/utils.o
CHAN_OBJ = build/channel.o
CHAN_PAR_OBJ = build/channel_parent.o
P1_OBJ = build/parent1.o
P2_OBJ = build/parent2.o
ENC1_OBJ = build/encrypter1.o
ENC2_OBJ = build/encrypter2.o
ENCR_OBJ = build/encrypt.o
DECR_OBJ = build/decrypt.o
WRIT_OBJ = build/writer.o
READ_OBJ = build/reader.o

OBJS =  $(CHAN_PAR_OBJ) $(CHAN_OBJ) $(P1_OBJ) $(P2_OBJ) $(WRIT_OBJ) $(READ_OBJ) $(ENC1_OBJ) $(ENC2_OBJ) $(ENCR_OBJ) $(DECR_OBJ) $(UTIL_OBJ)

# Executables
CHAN = channel
CHAN_PAR = channel_parent
P1 = parent1
P2 = parent2
ENC1 = encrypter1
ENC2 = encrypter2
ENCR = encrypt
DECR = decrypt
WRIT = writer
READ = reader

EXECS = $(P1) $(P2) $(WRIT) $(READ) $(CHAN_PAR) $(CHAN) $(ENC1) $(ENC2) $(ENCR) $(DECR)

all: $(EXECS)

$(P1): $(P1_OBJ)
	$(CC) -o $(P1) $(P1_OBJ)

$(P2): $(P2_OBJ)
	$(CC) -o $(P2) $(P2_OBJ)

$(WRIT): $(WRIT_OBJ) $(UTIL_OBJ)
	$(CC) -o $(WRIT) $(WRIT_OBJ) $(UTIL_OBJ) $(LDLIBS)

$(READ): $(READ_OBJ)
	$(CC) -o $(READ) $(READ_OBJ) $(UTIL_OBJ) $(LDLIBS)

$(ENC1): $(ENC1_OBJ)
	$(CC) -o $(ENC1) $(ENC1_OBJ)

$(ENC2): $(ENC2_OBJ)
	$(CC) -o $(ENC2) $(ENC2_OBJ)

$(ENCR): $(ENCR_OBJ)
	$(CC) -o $(ENCR) $(ENCR_OBJ) $(UTIL_OBJ) $(LDLIBS)

$(DECR): $(DECR_OBJ)
	$(CC) -o $(DECR) $(DECR_OBJ) $(UTIL_OBJ) $(LDLIBS)

$(CHAN_PAR): $(CHAN_PAR_OBJ)
	$(CC) -o $(CHAN_PAR) $(CHAN_PAR_OBJ)

$(CHAN): $(CHAN_OBJ)
	$(CC) -o $(CHAN) $(CHAN_OBJ) $(UTIL_OBJ) $(LDLIBS)

$(UTIL_OBJ):
	$(CC) -c -o $(UTIL_OBJ) src/utils.c

$(CHAN_OBJ):
	$(CC) -c -o $(CHAN_OBJ) src/channel.c

$(CHAN_PAR_OBJ):
	$(CC) $(FLAG) -c -o $(CHAN_PAR_OBJ) src/channel_parent.c

$(P1_OBJ):
	$(CC) -c -o $(P1_OBJ) src/parent1.c

$(P2_OBJ):
	$(CC) -c -o $(P2_OBJ) src/parent2.c

$(ENC1_OBJ):
	$(CC) -c -o $(ENC1_OBJ) src/encrypter1.c

$(ENC2_OBJ):
	$(CC) -c -o $(ENC2_OBJ) src/encrypter2.c

$(ENCR_OBJ):
	$(CC) -c -o $(ENCR_OBJ) src/encrypt.c

$(DECR_OBJ):
	$(CC) -c -o $(DECR_OBJ) src/decrypt.c

$(WRIT_OBJ):
	$(CC) -c -o $(WRIT_OBJ) src/writer.c

$(READ_OBJ):
	$(CC) -c -o $(READ_OBJ) src/reader.c

clean:
	rm $(EXECS) $(OBJS)
