#! /bin/bash
size=10

./simul.fs -format $size
./simul.fs -create ./arqTeste/exemplo /ex

./simul.fs -ls /
./simul.fs -del /ex

./simul.fs -ls /	