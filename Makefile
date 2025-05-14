make:
	gcc -o checklist main.c
install:
	cp -f checklist /usr/local/bin/todo
uninstall:
	rm -f /usr/local/bin/todo
