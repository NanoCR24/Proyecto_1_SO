programa: programa.c

	gcc -o programa programa.c -Wall


cpu: cpu.c

	gcc -o cpu cpu.c -Wall


ram: ram.c

	gcc -o ram ram.c -Wall


disk: disk.c

	gcc -o disk disk.c -Wall


install: programa cpu ram disk

	cp programa cpu ram disk /usr/local/bin


.PHONY: clean


clean:

	rm -f programa cpu ram disk