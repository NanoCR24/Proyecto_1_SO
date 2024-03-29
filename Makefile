all: padre cpu ram disco

padre: padre.c
    gcc -o padre padre.c -Wall

cpu: cpu.c
    gcc -o cpu cpu.c -Wall

ram: ram.c
    gcc -o ram ram.c -Wall

disco: disco.c
    gcc -o disco disco.c -Wall

install:
    cp padre cpu ram disco /usr/local/bin

.PHONY: clean

clean:
    rm -f padre cpu ram disco
