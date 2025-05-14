make:
	gcc -o checklist main.c
install:
	cp -f checklist /usr/local/bin/checklist
uninstall:
	rm -f /usr/local/bin/checklist
