#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// DEFINES e STRUCTS

#define BLOCK_SIZE 512
#define ROOT_ENTRIES 127

#define ROOT 1
#define PATH 0

#define ARQ 0
#define DIR 1

struct directory_entry{
	unsigned int dir; 
	char name[20]; 
	unsigned int size_bytes;
	unsigned int start;
};
struct root_table_directory{
	unsigned int free_blocks_list;
	struct directory_entry list_entry[127]; 
	unsigned int not_used[7];

};
struct table_directory{
	struct directory_entry list_entry[16];
};
struct sector_data{
	unsigned char data[508];
	unsigned int next_sector;	
};


// GLOBALS
struct root_table_directory 		   root;

FILE 							*fileSystem;
FILE 								*toSave;


int openRoot( void )
{
	fileSystem = fopen ( "memory", "r" );
	if (fileSystem == NULL)
	{
		perror("ERROR:");
		return -1;
	}
	if (fread(&root, sizeof(root), 1, fileSystem ) < 0 )
	{
		perror("ERROR:");
		return -1;
	}
	fclose(fileSystem);
	return 0;
} 
struct directory_entry *getDirectoryEntry( char name[20] )
{
	int i;
	// TODO para quando não estiver no diretorio root
	if ( openRoot( ) < 0 )
		return NULL;
	for ( i = 0; i < ROOT_ENTRIES; i++)
	{
		if( strcmp(root.list_entry[i].name, name) == 0 )
		{
			return &root.list_entry[i];
		}
	}
	return NULL;
}
int createDirectoryEntry( int root_or_not, int file_dir , int size, char name[20])
{
	int i,rt;
	fileSystem = fopen ( "memory", "r" );
	fread(&root, sizeof(root), 1, fileSystem );
	fseek(fileSystem, 0L, SEEK_SET);
	if ( root_or_not == 1 )
	{
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if (root.list_entry[i].start == 0)
			{	
				printf("entrei\n");
				struct sector_data setor;

				/* Lẽ o setor livre e atualiza para o próximo da lista */
				fread( &setor, BLOCK_SIZE * root.free_blocks_list, 1, fileSystem );


				printf("%d\n", root.free_blocks_list );
				/* Atualiza e retorna o bloco livre adquirido */
				root.list_entry[i].start = root.free_blocks_list;
				root.list_entry[i].size_bytes = size;
				strncpy( root.list_entry[i].name, name, 20);
				root.list_entry[i].dir = file_dir;
				/* TODO para quando precisar de mais de um bloco */
				fclose(fileSystem);
				rt = root.list_entry[i].start;
				root.free_blocks_list = setor.next_sector;
				printf("saindo %d\n", rt);
				return rt;
			}
		}
	}
	else
	{
		printf("TODO\n");
	}
   
}


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
			fileSystem = fopen ( "memory", "w" );
			int i = 0;
			int tamMax = (atoi( argv[2] ) * 2) - 8; // Cada bloco ocupa 512, então para gerar 1 kb, deve ser gerado
			printf("%d\n",tamMax );
			struct sector_data setores[tamMax];

			memset( setores, 0, sizeof(setores));
			while ( i+1 < tamMax )
			{	
				setores[i].next_sector = i+9;
				i++;
			}
			setores[i+1].next_sector = 0;
			root.free_blocks_list = 9;
			fwrite( &root, sizeof( root ), 1, fileSystem);
			fwrite( setores, 512, tamMax, fileSystem );
			fclose( fileSystem );
		}
		else if( strcmp( argv[1], "-create") == 0)
		{
			char *buffer;
			int size,rt, i, bloco_livre;

			// VERIFICAR NÚMEROS DE ARGUMENTOS 

			toSave = fopen(argv[2], "r");
			fseek(toSave, 0L, SEEK_END);
			size = ftell(toSave);
			fseek(toSave, 0L, SEEK_SET);
			// CHAMAR FUNÇÂO Q ACHA o BLOCO LIVRE PARA AQUELE DIRETÒRIO
			bloco_livre = createDirectoryEntry( ROOT, ARQ ,size , argv[3]);


			buffer = (char*) malloc( size );
			rt = fread( buffer, size,  1, toSave);

			fileSystem = fopen ( "memory", "r+'" );
			if ( fseek( fileSystem, bloco_livre * 512, SEEK_SET ) < 0)
			{
				perror("ERROR:");
				return -1;
			}

			// TODO, para quando for mais de um bloco
			if ( fwrite( buffer, size, 1, fileSystem) < 0 )
			{
				perror("ERROR:");
				return -1;
			}
			free( buffer );
			fclose(toSave);
			fclose(fileSystem);
		}
		else if( strcmp( argv[1], "-read" ) == 0)
		{
			struct directory_entry dir;
			char buffer[512];
			int size,rt;

			if ( openRoot() < 0 )
				return -1;
			dir = *getDirectoryEntry( argv[3] ); // possível bug
			toSave = fopen(argv[2], "w");

			

			fileSystem = fopen ( "memory", "r" );
			fseek ( fileSystem, dir.start * 512, SEEK_SET );
			fread( buffer, dir.size_bytes, 1, fileSystem);
			
			printf("buffer:%s\n", buffer );

			write(buffer, dir.size_bytes, 1, toSave);
 

			fclose( fileSystem );
			fclose( toSave );

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