SRC:= read_cert.c base64encode.c seuBase64.c
#base64decode.c #seuBase64.c
OBJ:=$(SRC:%.c=%.o)

LDFLAGS= -lssl -lcrypto
all: $(OBJ)
	$(CC) $^ -o read_cert $(LDFLAGS)

clean:
	$(RM) *.o
