.PHONY: all

all: task1 backup

task1: task1.c utils/files.c utils/files.h
	gcc -c -no-pie -o utils/files.o utils/files.c
	gcc -no-pie -o bin/task1 task1.c utils/files.o
	rm utils/files.o

backup: backup.c utils/files.c utils/files.h
	gcc -c -no-pie -o utils/files.o utils/files.c
	gcc -no-pie -o bin/backup backup.c utils/files.o
	rm utils/files.o

