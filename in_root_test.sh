#! /bin/bash
size=10000

./simul.fs -format $size


./simul.fs -create ./arqTeste/exemplo /ex
./simul.fs -read ./arquivosLidos/exemplo2 /ex

./simul.fs -create ./arqTeste/foto_teste.jpg /foto.jpg
./simul.fs -read ./arquivosLidos/foto_lida.jpg /foto.jpg

./simul.fs -create ./arqTeste/foto_teste2.jpg /eumesmo.jpg
./simul.fs -read ./arquivosLidos/eumesmo.jpg /eumesmo.jpg

./simul.fs -ls /
