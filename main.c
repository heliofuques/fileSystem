#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// DEFINES e STRUCTS

#define BLOCK_SIZE 512
#define ROOT_ENTRIES 127
#define NAME_SIZE 20

#define ROOT 1
#define PATH 0

#define DIR 1
#define ARQ 0

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
struct directory_entry 		   	entryContext;
struct table_directory 		   	tableContext;
FILE 							*fileSystem;
FILE 								*toSave;



char** returnPathList(char path[128])
{
	char **completePath;
	int dir_count = 0;
	completePath = malloc(1000);
 	do 
	{
		completePath[dir_count] = strtok_r( path, "/", &path );
		dir_count++;
	} while( completePath[dir_count - 1 ] != NULL );


	return completePath;
}

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
struct directory_entry *getDirectoryEntry( char path[128] )
{
	int i, dir_count = 0, root_or_not=0;
	char **completePath, name[20];

	fileSystem = fopen ( "memory", "r+" );

	completePath = returnPathList( path );
	while ( completePath[dir_count] != NULL )
		dir_count++;

	strncpy( name, completePath[0],  NAME_SIZE );

	if ( dir_count == 1 )
	{
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if( strcmp(root.list_entry[i].name, name) == 0 )
			{
				fclose( fileSystem );
				return &root.list_entry[i];
			}
		}
	}
	else
	{	
		dir_count = 0;
		while ( completePath[dir_count] != NULL )
		{
			if ( root_or_not == 0 )
			{
				for ( i = 0; i < ROOT_ENTRIES; i++)
				{
					if ( strcmp(root.list_entry[i].name, completePath[dir_count]) == 0 && root.list_entry[i].dir == DIR )
					{
						entryContext = root.list_entry[i];
						fseek( fileSystem, BLOCK_SIZE * entryContext.start, SEEK_SET );
						fread( &tableContext, BLOCK_SIZE, 1, fileSystem );
						root_or_not = 1;
						dir_count++;
						break;
					}
				}
			}
			else
			{
				for( i = 0; i < 16; i++ )
				{	
					if( strcmp(tableContext.list_entry[i].name, completePath[dir_count]) == 0 && tableContext.list_entry[i].dir == DIR)
					{
						entryContext = tableContext.list_entry[i];
						fseek( fileSystem, BLOCK_SIZE * entryContext.start, SEEK_SET );
						fread( &tableContext, BLOCK_SIZE, 1, fileSystem );
						dir_count++;
						break;
					}
				}
			}
			dir_count++;
		}
		strncpy( name, completePath[dir_count -1 ],  NAME_SIZE );
		for ( i = 0; i < 16; i++)
		{
			if( strcmp(tableContext.list_entry[i].name, name) == 0 )
			{
				fclose( fileSystem );
				return &tableContext.list_entry[i];
			}
		}

	}	
	return NULL;
}

int createDirectoryEntryRoot( char name[20], int i )
{
	// Pegar um bloco livre
	struct sector_data setor;

	root.list_entry[i].size_bytes = 512;

	root.list_entry[i].start = root.free_blocks_list;
	root.list_entry[i].dir = DIR;
	//root.list_entry[i].name = name;
	strncpy ( root.list_entry[i].name, name, 20);

	fseek( fileSystem, BLOCK_SIZE * root.free_blocks_list, SEEK_SET );
	fread( &setor, BLOCK_SIZE , 1, fileSystem );
	root.free_blocks_list = setor.next_sector;

	if ( fseek(fileSystem, 0L, SEEK_SET) < 0 )
		perror("ERROR:");
	
	if ( fwrite(&root, sizeof(root), 1, fileSystem) < 0)
		perror("ERROR:");

	if ( fseek(fileSystem, BLOCK_SIZE * root.list_entry[i].start, SEEK_SET) < 0 )
		perror("ERROR:");

	if ( fwrite(&tableContext, BLOCK_SIZE, 1, fileSystem) < 0 )
		perror("ERROR:");
	
	entryContext = root.list_entry[i];



	return 0;
	// salvar por cima um table directory
}
int createDirectoryEntry( char name[20], int dir_n )
{
	// Pegar um bloco livre
	struct sector_data setor;
	struct directory_entry BeforeContext = entryContext;


	int i;
	// ESCREVER SOBRE O NOVO CONTEXT
	entryContext.size_bytes = 512;
	entryContext.start = root.free_blocks_list;
	entryContext.dir = DIR;
	strncpy(entryContext.name, name, 20);
	// FIM DA ESCRITA ///

	// ATUALIZAr o BLOCO LIVRE DO ROOT //
	fseek(fileSystem, BLOCK_SIZE * root.free_blocks_list, SEEK_SET);
	fread( &setor, BLOCK_SIZE , 1, fileSystem );

	root.free_blocks_list = setor.next_sector;
	fseek(fileSystem, 0L, SEEK_SET);
	fwrite(&root, sizeof(root), 1, fileSystem);
	// FIM //

	// ATUALIZAR O CONTEXT ANTERIOR COM AO NOVO DIR

	tableContext.list_entry[dir_n] = entryContext;
	fseek(fileSystem, BLOCK_SIZE * BeforeContext.start, SEEK_SET);
	fwrite(&tableContext, BLOCK_SIZE, 1, fileSystem);
	// FIM //

	// SALVAR O NOVO CONTEXT
	memset ( &tableContext, 0 , sizeof( tableContext ) ); 
	fseek(fileSystem, BLOCK_SIZE * entryContext.start, SEEK_SET);
	fwrite(&tableContext, BLOCK_SIZE, 1, fileSystem);
	// FIM //
	// salvar por cima um table directory
}


int mkdirCommand( char path[128] )
{
	char **completePath, name[20];
	int i,dir_count = 0;


	memset ( &tableContext, 0 , sizeof( tableContext ) ); 
	openRoot();
	fileSystem = fopen ( "memory", "r+" );

	completePath = returnPathList( path );
	strcpy( name, completePath[0]);

	for ( i = 0; i < ROOT_ENTRIES; i++)
	{
		if( strcmp(root.list_entry[i].name, name) == 0 )
		{
			entryContext = root.list_entry[i];

			fseek(fileSystem, BLOCK_SIZE * entryContext.start , SEEK_SET);
			fread(&tableContext, BLOCK_SIZE  , 1, fileSystem);
			dir_count++;
		}
	}
	if ( dir_count == 0 )
	{
		// cria novo dir
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if ( root.list_entry[i].size_bytes == 0)
			{
				createDirectoryEntryRoot( name, i  );
				dir_count++;
				break;
			}
		}
		//printf("Sem espaço no diretorio root para criar: %s\n",name );
	}
	while ( completePath[dir_count] != NULL )
	{
		int flag = 0;
		strcpy( name, completePath[dir_count]);
		for( i = 0; i < 16; i++ )
		{
			if( strcmp(tableContext.list_entry[i].name, name) == 0 )
			{
				//entra no context e continua o trabalho
				entryContext = tableContext.list_entry[i];
				fseek(fileSystem, entryContext.start * BLOCK_SIZE, SEEK_SET);
				fread( &tableContext,  BLOCK_SIZE, 1, fileSystem );
				flag = 1;
			}
		}

		if ( flag == 0 )
		{
			int dir_count2;
			for ( dir_count2 = 0; dir_count2 < 16; dir_count2 ++)
			{
				if( tableContext.list_entry[dir_count2].size_bytes == 0 )
				{
					createDirectoryEntry( name, dir_count2 );
					break;
				}
				
			}
		}
		// verifica se a pasta está no context

		// se estiver, entra, caso contrário cria uma nova e entra no context
		dir_count++;
	}
	fclose(fileSystem);
}
void updateNextFreeBlock ( int freeSector )
{
	openRoot( );

	fileSystem = fopen ( "memory", "r+" );
	root.free_blocks_list = freeSector;	
	fwrite(&root, sizeof(root), 1, fileSystem);
	fclose(fileSystem);
}

int createFileEntry( int size, char path[128] )
{
	int i,rt;
	int dir_count = 0;
	char name[20], **completePath;

	fileSystem = fopen ( "memory", "r+" );
	fread(&root, sizeof(root), 1, fileSystem );
	fseek(fileSystem, 0L, SEEK_SET);

	completePath = returnPathList( path);
	while ( completePath[dir_count] != NULL )
		dir_count++;

	
	strncpy( name, completePath[0],  NAME_SIZE );


	if ( dir_count == 1 )
	{
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if (root.list_entry[i].start == 0)
			{	
				struct sector_data setor;

				/* Lẽ o setor livre e atualiza para o próximo da lista */
				fseek(fileSystem, BLOCK_SIZE*root.free_blocks_list, SEEK_SET);
				fread( &setor, BLOCK_SIZE, 1, fileSystem );

				/* Atualiza e retorna o bloco livre adquirido */
				root.list_entry[i].start = root.free_blocks_list;
				root.list_entry[i].size_bytes = size;
				strncpy( root.list_entry[i].name, name, 20);
				root.list_entry[i].dir = ARQ;
				/* TODO para quando precisar de mais de um bloco */
				root.free_blocks_list = setor.next_sector;
				rt = root.list_entry[i].start;

				fseek(fileSystem, 0L, SEEK_SET);
				fwrite(&root, sizeof(root), 1, fileSystem);
				fclose(fileSystem);

				return rt;
			}
		}
	}
	else
	{
		dir_count = 0;

		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if( strcmp(root.list_entry[i].name, name) == 0 )
			{
				entryContext = root.list_entry[i];

				fseek(fileSystem, BLOCK_SIZE * entryContext.start , SEEK_SET);
				fread(&tableContext, BLOCK_SIZE  , 1, fileSystem);
				dir_count++;
			}
		}
		if ( dir_count == 0 )
		{
			// cria novo dir
			for ( i = 0; i < ROOT_ENTRIES; i++)
			{
				if ( root.list_entry[i].size_bytes == 0)
				{
					createDirectoryEntryRoot( name, i  );
					dir_count++;
					break;
				}
			}
			//printf("Sem espaço no diretorio root para criar: %s\n",name );
		}
		while ( completePath[dir_count+1] != NULL )
		{
			int flag = 0;
			strcpy( name, completePath[dir_count]);
			for( i = 0; i < 16; i++ )
			{
				if( strcmp(tableContext.list_entry[i].name, name) == 0 )
				{
					//entra no context e continua o trabalho
					entryContext = tableContext.list_entry[i];
					fseek(fileSystem, entryContext.start * BLOCK_SIZE, SEEK_SET);
					fread( &tableContext,  BLOCK_SIZE, 1, fileSystem );
					flag = 1;
				}
			}
			dir_count++;
		}
		strcpy( name, completePath[dir_count]);
		int dir_count2;
		for ( dir_count2 = 0; dir_count2 < 16; dir_count2 ++)
		{
			if( tableContext.list_entry[dir_count2].size_bytes == 0 )
			{
				struct sector_data setor;

				/* Lẽ o setor livre e atualiza para o próximo da lista */
				fseek(fileSystem, BLOCK_SIZE*root.free_blocks_list, SEEK_SET);
				fread( &setor, BLOCK_SIZE, 1, fileSystem );

				/* Atualiza e retorna o bloco livre adquirido */
				tableContext.list_entry[dir_count2].start = root.free_blocks_list;
				tableContext.list_entry[dir_count2].size_bytes = size;
				strncpy( tableContext.list_entry[dir_count2].name, name, 20);
				tableContext.list_entry[dir_count2].dir = ARQ;
				/* TODO para quando precisar de mais de um bloco */
				root.free_blocks_list = setor.next_sector;
				rt = tableContext.list_entry[dir_count2].start;

				fseek(fileSystem, entryContext.start * BLOCK_SIZE, SEEK_SET);
				fwrite(&tableContext, sizeof(tableContext), 1, fileSystem);
				fclose(fileSystem);

				return rt;
			}
			
		}
		

		printf("TODO\n");
	}
   
}
int contextEnter( char **completePath )
{
	int	dir_count = 0, root_or_not = 0, i;
	while ( completePath[dir_count] != NULL )
	{
		if ( root_or_not == 0 )
		{
			for ( i = 0; i < ROOT_ENTRIES; i++)
			{
				if ( strcmp(root.list_entry[i].name, completePath[dir_count]) == 0 && root.list_entry[i].dir == DIR )
				{
					entryContext = root.list_entry[i];
					fseek( fileSystem, BLOCK_SIZE * entryContext.start, SEEK_SET );
					fread( &tableContext, BLOCK_SIZE, 1, fileSystem );
					root_or_not = 1;
					dir_count++;
					printf("carregando o %d\n", BLOCK_SIZE * entryContext.start );
					break;
				}
			}
		}
		else
		{
			for( i = 0; i < 16; i++ )
			{	
				if( strcmp(tableContext.list_entry[i].name, completePath[dir_count]) == 0 && tableContext.list_entry[i].dir == DIR)
				{
					entryContext = tableContext.list_entry[i];
					fseek( fileSystem, BLOCK_SIZE * entryContext.start, SEEK_SET );
					fread( &tableContext, BLOCK_SIZE, 1, fileSystem );
					dir_count++;
					break;
				}
			}
		}
	}
}
void listCommand( char path[128] )
{
	/* VERIFICAR O CAMINHO E TALS */
	int i, dir_count = 0, root_or_not = 0;
	char **completePath;

	openRoot( );
	fileSystem = fopen ( "memory", "r+" );

	completePath = returnPathList( path );
	while ( completePath[dir_count] != NULL )
	{
		dir_count++;
	}
	// Entrar no último context
	if ( dir_count == 0 )
	{
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if ( root.list_entry[i].size_bytes > 0 )
			{
				if ( root.list_entry[i].dir == DIR )
				{
					printf("d ");
					printf("%s\n", root.list_entry[i].name);
				}
				else
				{
					printf("f ");
					printf("%s\t%d\n", root.list_entry[i].name, root.list_entry[i].size_bytes);
				}
			}
		}
	} 
	else
	{
		contextEnter( completePath );
		for ( i = 0; i < 16; i++)
		{
			if ( tableContext.list_entry[i].size_bytes > 0 )
			{
				if ( tableContext.list_entry[i].dir == DIR )
				{
					printf("d ");
					printf("%s\n", tableContext.list_entry[i].name);
				}
				else
				{
					printf("f ");
					printf("%s\t%d\n", tableContext.list_entry[i].name, tableContext.list_entry[i].size_bytes);
				}
			}
		}
	}


	fclose(fileSystem);
}

int delFile( char path[128] )
{
	/* VERIFICAR O CAMINHO E TALS */
	int i, dir_count = 0, root_or_not = 0;
	char **completePath, name[20];

	openRoot( );
	fileSystem = fopen ( "memory", "r+" );

	completePath = returnPathList( path );
	while ( completePath[dir_count] != NULL )
	{
		dir_count++;
	}
	// Entrar no último context
	strncpy( name, completePath[0],  NAME_SIZE );
	if ( dir_count == 1 )
	{
		for ( i = 0; i < ROOT_ENTRIES; i++)
		{
			if ( strcmp(root.list_entry[i].name, name) == 0 )
			{
				printf("prox free:%d\n", root.free_blocks_list );
				struct sector_data setor;
				fseek(fileSystem, BLOCK_SIZE*root.list_entry[i].start, SEEK_SET);
				fread( &setor, BLOCK_SIZE, 1, fileSystem );

				printf("primeiro do mundo:%d\n",root.list_entry[i].start  );

				while ( setor.next_sector > 0 )
				{
					printf("entrei\n");
					printf("next:%d\n", setor.next_sector );
					fseek(fileSystem, BLOCK_SIZE* setor.next_sector, SEEK_SET);
					fread( &setor, BLOCK_SIZE, 1, fileSystem );
				}
				printf("%d\n", setor.next_sector );
				setor.next_sector = root.free_blocks_list;


				root.free_blocks_list = root.list_entry[i].start;

				printf("first sector%d\n", root.free_blocks_list );

				fseek(fileSystem, BLOCK_SIZE*root.free_blocks_list, SEEK_SET);
				fwrite( &setor, BLOCK_SIZE, 1, fileSystem );
				/* Lẽ o setor livre e atualiza para o próximo da lista */

				root.list_entry[i].start = 0;
				// Mudar o próximo bloco livre
				// Alterar entry 
				root.list_entry[i].size_bytes = 0;

				// salva saporra
				fseek( fileSystem, 0l, SEEK_SET );
				fwrite( &root, sizeof(root), 1, fileSystem);

			}
		}

		int bloco = root.free_blocks_list;
		int contador= 0;
		while (bloco > 0)
		{
			printf("bloco:%d\n", bloco );
			struct sector_data setor;
			fseek(fileSystem, bloco * BLOCK_SIZE, SEEK_SET);
			fread( &setor, BLOCK_SIZE, 1, fileSystem );

			bloco = setor.next_sector;
			contador++;
		}
		printf("total de blocos livres:%d\n", contador );
	} 
	else
	{
		printf("TODO\n");
		contextEnter( completePath );
		for ( i = 0; i < 16; i++)
		{
			if ( tableContext.list_entry[i].size_bytes > 0 )
			{
				if ( tableContext.list_entry[i].dir == DIR )
				{
					printf("d ");
					printf("%s\n", tableContext.list_entry[i].name);
				}
				else
				{
					printf("f ");
					printf("%s\t%d\n", tableContext.list_entry[i].name, tableContext.list_entry[i].size_bytes);
				}
			}
		}
	}


	fclose(fileSystem);

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
		if ( strcmp( argv[1], "-format") == 0 )
		{
			fileSystem = fopen ( "memory", "w" );
			int i = 0;
			int tamMax = (atoi( argv[2] ) * 2) - 8; // Cada bloco ocupa 512, então para gerar 1 kb, deve ser gerado

			
			// FINAL DA OPERACAO SOBRE O ULTIMO SETOR

			printf("total de blocos:%d\n", tamMax );
			root.free_blocks_list = 9;
			fwrite( &root, sizeof( root ), 1, fileSystem);


			while ( i+1 < tamMax )
			{	
				struct sector_data setores;
				memset( &setores, 0, sizeof(setores));
				setores.next_sector = i+9;
				fwrite( &setores, 512, 1, fileSystem );
				i++;
			}
			// OPERACAO SOBRE O ÚLTIMO SETOR
			struct sector_data last_sector;
			memset(&last_sector, 0, sizeof(last_sector));
			fwrite( &last_sector, 512, 1, fileSystem );
			last_sector.next_sector = 0;

			printf("Sistema de arquivo formatado com: %d bytes\n", atoi( argv[2] ) * 512 * 2  );
			fclose( fileSystem );
		}
		else if( strcmp( argv[1], "-create") == 0)
		{
			char *buffer, toPass[128];
			int size,rt, i, bloco_livre,fd;

			struct stat stbuf;

			// VERIFICAR NÚMEROS DE ARGUMENTOS 
			if (argc < 4 || argc > 4)
			{
				printf("Número de argumentos inválido\n Use -create SRC DEST");
				return -1;
			}
			printf("Arquivo:%s\nDestino no sistema de arquivos:%s\n", argv[2], argv[3] );

			toSave = fopen(argv[2], "r");

			fseek(toSave, 0L, SEEK_END);
			size = ftell(toSave);
			fseek(toSave, 0L, SEEK_SET);

			// CHAMAR FUNÇÂO Q ACHA o BLOCO LIVRE PARA AQUELE DIRETÒRIO
			memset( toPass, '\0', 128 );
			memcpy( toPass, argv[3], strlen(argv[3]) );
			bloco_livre = createFileEntry( size , toPass);


			fileSystem = fopen ( "memory", "r+'" );
			if ( fseek( fileSystem, bloco_livre * 512, SEEK_SET ) < 0)
			{
				perror("ERROR:");
				return -1;
			}

			buffer = (char*) malloc( BLOCK_SIZE );
			struct sector_data setor;
			while ( size > 0 )
			{
				// Leitura para encher o buffer
				if ( size > ( BLOCK_SIZE - sizeof( int ) ) )
				{
					fread( buffer, BLOCK_SIZE - sizeof ( int ), 1, toSave );
					// att o próximo bloco livre
					// Lê o setor da memória
					fread( &setor, BLOCK_SIZE, 1,fileSystem );

					fseek( fileSystem, BLOCK_SIZE * bloco_livre, SEEK_SET );
					// retorna para o ponto do setor
					bloco_livre = setor.next_sector;
					//updateNextFreeBlock ( setor.next_sector );
					if (bloco_livre == 0 )
					{
						printf("ERROR: Sistema de arquivo cheio!\n");
						return -1;
					}
					// salva os dados
					memcpy( setor.data, buffer, BLOCK_SIZE - sizeof( int ) );
					fwrite( &setor , BLOCK_SIZE , 1 , fileSystem );
					size = size - BLOCK_SIZE + sizeof ( int );
				}
				else
				{
					fread( buffer, size , 1, toSave );
					// Lê o setor da memória
					fread( &setor, BLOCK_SIZE, 1, fileSystem );
					// retorna para o ponto do setor
					fseek( fileSystem, BLOCK_SIZE* bloco_livre, SEEK_SET );
					// att o próximo bloco livre
					bloco_livre = setor.next_sector;
					setor.next_sector = 0 ;
					memcpy( setor.data, buffer, size );
					fwrite( &setor, BLOCK_SIZE , 1, fileSystem );
					break;
				}
			}
			updateNextFreeBlock ( bloco_livre );	

			free( buffer );
			fclose(toSave);
		}
		else if( strcmp( argv[1], "-read" ) == 0)
		{
			struct directory_entry dir;
			struct sector_data bloco;
			char buffer[508];
			int size,rt, setor;

			if ( openRoot() < 0 )
				return -1;

			dir = *getDirectoryEntry( argv[3] ); 
			toSave = fopen(argv[2], "w");
			

			fileSystem = fopen ( "memory", "r" );
			size = dir.size_bytes;
			setor = dir.start;
			while ( size > 0 )
			{
				if ( size > ( BLOCK_SIZE - sizeof( int ) ) )
				{
					fseek( fileSystem, setor * BLOCK_SIZE, SEEK_SET );
					fread( &bloco, BLOCK_SIZE , 1,fileSystem );

					fwrite( bloco.data, BLOCK_SIZE - sizeof( int ), 1, toSave );

					setor = bloco.next_sector;
					size = size - BLOCK_SIZE + sizeof ( int );
				}
				else
				{
					fseek ( fileSystem, setor * 512, SEEK_SET );
					fread( buffer, size, 1, fileSystem);			
					fwrite(buffer, size, 1, toSave);
					break;
				}

			}

			printf("Arquivo lido do sistema de arquivos\n");
			fclose( fileSystem );
			fclose( toSave );

		}
		else if( strcmp( argv[1], "-del" ) == 0)
		{
			printf("Deletando arquivo %s\n", argv[2]);
			delFile( argv[2] );
		} 
		else if( strcmp( argv[1], "-mkdir" ) == 0)
		{
			printf("Criando path:%s\n", argv[2] );
			mkdirCommand( argv[2] );		
		}
		else if( strcmp( argv[1], "-ls" ) == 0)
		{
			printf("Exibindo arquivos da pasta %s\n", argv[2]);
			listCommand( argv[2] );
		}
		else
		{
			printf("Commando Invalido\n");
		}
	}
	printf("\n##################################\n");
	return 0;
}