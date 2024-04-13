all: programa cpu ram disco

programa: programa.c
    gcc -o programa programa.c -Wall

cpu: cpu.c
    gcc -o cpu cpu.c -Wall

ram: ram.c
    gcc -o ram ram.c -Wall

disco: disco.c
    gcc -o disco disco.c -Wall

install:
    cp programa cpu ram disco /usr/local/bin

.PHONY: clean

clean:
    rm -f programa cpu ram disco