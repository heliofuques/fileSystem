#! /bin/bash
size=10000

./simul.fs -format $size
./simul.fs -mkdir /fotos
./simul.fs -ls /

./simul.fs -mkdir /fotos/treta
./simul.fs -mkdir /fotos/aff
./simul.fs -mkdir /fotos/nudes
./simul.fs -ls /fotos/

./simul.fs -rmdir /fotos/treta

./simul.fs -rmdir /fotos/aff

./simul.fs -rmdir /fotos/

./simul.fs -ls /
./simul.fs -ls /fotos/

./simul.fs -rmdir /fotos/

./simul.fs -ls /fotos/


./simul.fs -create ./arqTeste/exemplo /fotos/nudes/ex
./simul.fs -read ./arquivosLidos/exemplo2 /fotos/nudes/ex

./simul.fs -create ./arqTeste/foto_teste.jpg /fotos/foto.jpg
./simul.fs -read ./arquivosLidos/foto_lida.jpg /fotos/foto.jpg

./simul.fs -create ./arqTeste/foto_teste2.jpg /fotos/eumesmo.jpg
./simul.fs -read ./arquivosLidos/eumesmo.jpg /fotos/eumesmo.jpg

./simul.fs -ls /fotos
./simul.fs -ls /fotos/nudes
./simul.fs -del /fotos/eumesmo.jpg
./simul.fs -ls /fotos