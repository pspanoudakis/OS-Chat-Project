CC = gcc

# Compile options
# comment-out FLAG to re-enable the disabled warning (see README)
FLAG = -Wno-discarded-qualifiers
LDLIBS = -lcrypto

# Source and Executable files
UTIL_OBJ = utils.o
CHAN_OBJ = channel.o
CHAN = channel
CHAN_PAR_OBJ = channel_parent.o
CHAN_PAR_SRC = channel_parent.c
CHAN_PAR = channel_parent
P1 = parent1
P1_OBJ = parent1.o
P2 = parent2
P2_OBJ = parent2.o
ENC1 = encrypter1
ENC1_OBJ = encrypter1.o
ENC2 = encrypter2
ENC2_OBJ = encrypter2.o
ENCR = encrypt
ENCR_OBJ = encrypt.o
DECR = decrypt
DECR_OBJ = decrypt.o
WRIT = writer
WRIT_OBJ = writer.o
READ = reader
READ_OBJ = reader.o

EXECS = $(P1) $(P2) $(WRIT) $(READ) $(CHAN_PAR) $(CHAN) $(ENC1) $(ENC2) $(ENCR) $(DECR)
OBJS =  $(CHAN_PAR_OBJ) $(CHAN_OBJ) $(P1_OBJ) $(P2_OBJ) $(WRIT_OBJ) $(READ_OBJ) $(ENC1_OBJ) $(ENC2_OBJ) $(ENCR_OBJ) $(DECR_OBJ) $(UTIL_OBJ)

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

$(CHAN_PAR_OBJ):
	$(CC) $(FLAG) -c -o $(CHAN_PAR_OBJ) $(CHAN_PAR_SRC)

$(CHAN): $(CHAN_OBJ)
	$(CC) -o $(CHAN) $(CHAN_OBJ) $(UTIL_OBJ) $(LDLIBS)

clean:
	rm $(EXECS) $(OBJS)
