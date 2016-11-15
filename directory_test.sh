#! /bin/bash
size=10000

./simul.fs -format $size
./simul.fs -mkdir /fotos
./simul.fs -ls /
./simul.fs -create ./arqTeste/exemplo /fotos/ex

./simul.fs -ls /fotos/
./simul.fs -read ./arquivosLidos/exemplo2 /fotos/ex


./simul.fs -mkdir /fotos/treta
./simul.fs -mkdir /fotos/aff
./simul.fs -mkdir /fotos/nudes
./simul.fs -ls /fotos/


./simul.fs -create ./arqTeste/foto_teste2.jpg /eumesmo.jpg
./simul.fs -read ./arquivosLidos/eumesmo.jpg /eumesmo.jpg

./simul.fs -ls /