#include <string.h>
#include <stdio.h>
#include <stdlib.h>


/*##########
	Diretorio Raiz
	Ate 2TB de espaco: 
  ##########*/

struct directory_entry{
	unsigned char dir; // 1 byte
	char name[20]; // 20 byte
	unsigned int size_bytes; // 4 byte
	unsigned int start; // 4 byte
};
// Total = 32

struct root_table_directory{
	unsigned int free_blocks_list;
	struct directory_entry list_entry[127]; //32 * 141
	unsigned int not_used[7];

}root;

struct table_directory{
	struct directory_entry list_entry[16];
};

/* ####
	Setor de arquivo.
	Tam 512 bytes
   #### */	
struct sector_data{
	unsigned char data[508];
	unsigned int next_sector;	
};

int main( int argc, char *argv[])
{
	if ( argc == 1 )
	{
		printf("Commands:\n-init\n-create\n-read\n-del\n-pwd\n-mkdir\n-ls\n-cd\n");
		return 0;
	}
	else
	{
		if ( strcmp( argv[1], "-init") == 0 )
		{
			printf("sizeof:%d\n", sizeof(struct table_directory) );
			FILE *fp;
			fp = fopen ( "memory", "w" );
			int i = 0;
			int tamMax = atoi( argv[2] ) * 2 * 1000 - 8; // Cada bloco ocupa 512, então para gerar 1 kb, deve ser gerado
			
			printf("%d\n", tamMax);
			struct sector_data setores[tamMax];
			while ( i+1 < tamMax )
			{
				setores[i].next_sector = &setores[i+1];
				i++;
			}
			setores[i+1].next_sector = 0;

			fwrite( &root, sizeof( root ), 1, fp);
			fwrite( setores, 512, tamMax, fp );
			fclose( fp );
		}
		else if( strcmp( argv[1], "-create") == 0)
		{
			printf("TODO2\n");
		}
		else if( strcmp( argv[1], "-read" ) == 0)
		{
			printf("TODO\n");
		}
		else if( strcmp( argv[1], "-del" ) == 0)
		{
			printf("TODO\n");
		} 
		else if( strcmp( argv[1], "-pwd" ) == 0)
		{
			printf("TODO\n");
		}
		else if( strcmp( argv[1], "-mkdir" ) == 0)
		{
			printf("TODO\n");
		}
		else if( strcmp( argv[1], "-ls" ) == 0)
		{
			printf("TODO\n");
		}
		else if( strcmp( argv[1], "-cd" ) == 0)
		{
			printf("TODO\n");
		}
		else
		{
			printf("Commando Invalido\n");
		}
	}
	return 0;
}