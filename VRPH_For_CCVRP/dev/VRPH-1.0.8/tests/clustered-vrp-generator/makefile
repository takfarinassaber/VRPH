ROOT       =  .

OUTPUT_BIN =  $(ROOT)/rndmap

CFLAGS     =  -O3 -march=native -Wall -lm

CC         =  gcc

RM         =  rm



$(OUTPUT_BIN)  :  $(ROOT)/main.c
	$(CC) $< $(CFLAGS) -o $@


clean:
	$(RM) $(OUTPUT_BIN)
