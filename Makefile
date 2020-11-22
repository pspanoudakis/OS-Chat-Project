CC = gcc

# Compile options
CFLAGS = -Wno-discarded-qualifiers			# comment-out this line to re-enable the flags (see README)
LDLIBS = -lcrypto

# Source and Executable files
UTIL_SRC = utils.c

CHAN_SRC = channel.c
CHAN = channel
CHAN_PAR_SRC = channel_parent.c
CHAN_PAR = channel_parent
P1 = parent1
P1_SRC = parent1.c
P2 = parent2
P2_SRC = parent2.c
ENC1 = encrypter1
ENC1_SRC = encrypter1.c
ENC2 = encrypter2
ENC2_SRC = encrypter2.c
ENCR = encrypt
ENCR_SRC = encrypt.c
DECR = decrypt
DECR_SRC = decrypt.c
WRIT = writer
WRIT_SRC = writer.c
READ = reader
READ_SRC = reader.c

all: $(CHAN_PAR) $(CHAN) $(P1) $(P2) $(WRIT) $(READ) $(ENC1) $(ENC2) $(ENCR) $(DECR)

$(P1):
	$(CC) -o $(P1) $(P1_SRC)

$(P2):
	$(CC) -o $(P2) $(P2_SRC)

$(WRIT):
	$(CC) -o $(WRIT) $(WRIT_SRC) $(UTIL_SRC) $(LDLIBS)

$(READ):
	$(CC) -o $(READ) $(READ_SRC) $(UTIL_SRC) $(LDLIBS)

$(ENC1):
	$(CC) -o $(ENC1) $(ENC1_SRC)

$(ENC2):
	$(CC) -o $(ENC2) $(ENC2_SRC)

$(ENCR):
	$(CC) -o $(ENCR) $(ENCR_SRC) $(UTIL_SRC) $(LDLIBS)

$(DECR):
	$(CC) -o $(DECR) $(DECR_SRC) $(UTIL_SRC) $(LDLIBS)

$(CHAN_PAR):
	$(CC) $(CFLAGS) -o $(CHAN_PAR) $(CHAN_PAR_SRC)

$(CHAN):
	$(CC) -o $(CHAN) $(CHAN_SRC) $(UTIL_SRC) $(LDLIBS)

clean:
	rm $(CHAN_PAR) $(CHAN) $(P1) $(P2) $(WRIT) $(READ) $(ENC1) $(ENC2) $(ENCR) $(DECR)
