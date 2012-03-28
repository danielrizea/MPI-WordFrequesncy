#include "mpi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hashtable.h"
#include "utils.h"
/*
	This is one of the mapper programs
*/


//fuctie ce afla dimensiunea fisierulu iinitial
#define ENTRIES 100
int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}

int main(int argc, char *argv[]){

	int  numtasks, rank, rc,tag=1,i,j;
	int maxReducers;
	FILE *file;
	char *command; 
	command = "./reduce_program"; 
	MPI_File mpi_file;
	MPI_Status status,stat;
  	int bufsize,filesize,chuncksize;
  	char *buf;
	int offset;
	int chunck;
	int aux;
	MPI_Comm intercomm_reducers,parentcomm;
	
	Hashtable hashtable;
	
	hashtable = create_hashtable(500);
		
	
	

	
	MPI_Datatype MPI_ENTRY_WORD, oldtypes[2]; 
	int          blockcounts[2];

	/* MPI_Aint type used to be consistent with syntax of */
	/* MPI_Type_extent routine */
	MPI_Aint    offsets[2], extent;


	
	//aflare dimensiune fisier
	file = fopen(argv[1],"r");
	filesize = fsize(file);
	//printf("Mapper: Fisierul are dimensiunea %d\n",filesize);	
	fclose(file);
	maxReducers = atoi(argv[2]);
	int *errcodes = (int*)malloc(maxReducers * sizeof(int));

	//printf("Mapper: Fisierul de intrare este %s ---- %s|\n",argv[1],argv[2]);

	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_get_parent( &parentcomm );

	
	//commit structura mpi 
	
	offsets[0] = 0;
	oldtypes[0] = MPI_INT;
	blockcounts[0] = ENTRIES + 1;

	/*Commit structura definita pentru comunicatia informatiilor intre reduceri si mapperi si mapperi si master
	*/
	
	MPI_Type_extent(MPI_INT, &extent);
	offsets[1] = (ENTRIES + 1) * extent;
	oldtypes[1] = MPI_CHAR;
	blockcounts[1] = ENTRIES * 50;

	/* Now define structured type and commit it */
	MPI_Type_struct(2, blockcounts, offsets, oldtypes, &MPI_ENTRY_WORD);
	MPI_Type_commit(&MPI_ENTRY_WORD);

	//printf("Redy for spawn");
		
	//proces principal ce va face spawn proceselor map		
	MPI_Comm_spawn(command,MPI_ARGV_NULL,maxReducers,MPI_INFO_NULL,0,MPI_COMM_SELF,&intercomm_reducers,errcodes);
	for(i=0;i<maxReducers;i++)
		if(errcodes[i] == MPI_SUCCESS)
			printf("Mapper %d: spwan reducer %d\n",rank,i);
		else
			printf("Mapper %d:error spwn reducer %d\n",rank,i);
		
 	bufsize = filesize/numtasks;

	if(rank == numtasks-1){
		bufsize = filesize - rank*bufsize;
	}	
	chuncksize = bufsize/maxReducers;
	

	 buf = (char*)malloc(sizeof(char)*bufsize);
  
 	 MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &mpi_file);
 	 MPI_File_read_at(mpi_file, rank*bufsize, buf, bufsize, MPI_CHAR, &status);
 	 MPI_File_close(&mpi_file);

		//printf("Mapper %d: {%s}\n",rank,buf);
	   /*******  do some work *******/
	chunck = chuncksize;
	offset=0;
	//printf("Mapper %d: asteapta procesele reduce\n",rank);
	/********** asteapta ca toti reducerii sa fie activi**************/	
	for(i=0;i<maxReducers;i++){
		MPI_Recv(&aux,1,MPI_INT,MPI_ANY_SOURCE,tag,intercomm_reducers,&stat);
	}

	/********** raspandire chunk-uri catre procesele reduce *************/	
	for(i=0;i<maxReducers;i++){
	
		if(i == maxReducers-1)
		{
			chunck=bufsize-i*chunck;
		}

		//printf("Mapper %d: prepare to send data to reducer %d\n",rank,i);
		MPI_Send(&chunck, 1, MPI_INT, i, tag, intercomm_reducers);
		MPI_Send(&buf[offset], chunck, MPI_CHAR, i, tag, intercomm_reducers);
		
		offset += chunck;
	}
	 	

	int ok=0;
	while(ok<maxReducers){

		MPI_Recv(&entry,1,MPI_ENTRY_WORD,MPI_ANY_SOURCE,tag,intercomm_reducers,&stat);
		
		
		if(entry.aparitii[0] == -1)
			ok++;
		else
			for(i=0;i<entry.numar_intrari;i++)
				merge_elements(hashtable,entry.cuvinte[i],entry.aparitii[i]);
		
	}
	printf("Mapper %d: Am toate informatiile de la procesele reduce\n",rank);


	entry.numar_intrari=0;
	/******* Trimite informatiile la procesul principal  **************/
	for(i=0;i<hashtable->size;i++){
			
		for(j=0;j<hashtable->buckets[i].current;j++){
			
			if(entry.numar_intrari == ENTRIES){
			
				
				MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);
				//printf("Mapper %d: trimite hash date\n",rank);
				entry.numar_intrari=0;
							
			}
			else
			{
				//printf("Mapper %d: Cuv %s\n",rank,hashtable->buckets[i].entries[j].value);
				entry.aparitii[entry.numar_intrari] = hashtable->buckets[i].entries[j].number;
				strcpy(entry.cuvinte[entry.numar_intrari],hashtable->buckets[i].entries[j].value);
				entry.numar_intrari++;				
				
			}
			
		}
		
	}

		if(entry.numar_intrari > 0){
			MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);
			//printf("Mapper %d: trimite hash date\n",rank);
			entry.numar_intrari=0;
		}

	/************ Terminare trimitere hashtable *************/
	entry.aparitii[0] = -1;
	
	//printf("Mapper %d: send finish to main process\n ",rank);
	MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);


	//printf("Mapper %d: %d %s",rank,bufsize,buf);

	printf ("Mapper:  %d am terminat\n", rank);


	//print_hashtable(hashtable,stdout);
	MPI_Finalize();
	free_hashtable(hashtable);

	return 0;
}



