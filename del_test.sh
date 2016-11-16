#! /bin/bash
size=10

./simul.fs -format $size
./simul.fs -create ./arqTeste/exemplo /ex
./simul.fs -create ./arqTeste/text /teste/text
./simul.fs -create ./arqTeste/text2 /teste/treta

./simul.fs -ls /teste/

./simul.fs -del /ex
./simul.fs -del /teste/text
./simul.fs -del /teste/treta

./simul.fs -ls /teste/