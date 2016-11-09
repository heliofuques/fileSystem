compiler = gcc
exec_name = simul.fs
file_name = main.c

all:
	$(compiler) -o $(exec_name) $(file_name) -g
