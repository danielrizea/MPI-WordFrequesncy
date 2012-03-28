#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"
#include "utils.h"
/*This is one of the reduce programs

*/


char* strtolower(char str[]) { int i; for (i = 0; str[i] != '\0'; ++i) { 
			if (str[i] >= 'A' && str[i] <= 'Z') { 
				str[i] = str[i] + 'a' - 'A'; } 
			}
			return str;}

void process_chunck(char *buffer,Hashtable hashtable){

	char delimiters[]=" ,!.?/*():;\t\n{}<>#=&$%^+\"";
	char *word;

	word = strtok(buffer,delimiters);

	while(word!=NULL){

	put_element(hashtable,strtolower(word));
	word = strtok(NULL,delimiters);

	}


}

int main(int argc, char *argv[]){

	int  numtasks, rank, rc,tag=1;
	MPI_Comm parentcomm;
	MPI_Status stat;
	int a=1;
	char *buffer;
	int buffer_size;


	MPI_Datatype MPI_ENTRY_WORD, oldtypes[2]; 
	int          blockcounts[2];
	int i,j;
	/* MPI_Aint type used to be consistent with syntax of */
	/* MPI_Type_extent routine */
	MPI_Aint    offsets[2], extent;


	/******** creare hashtable **********/
	Hashtable hashtable;
	hashtable = create_hashtable(500);
	
	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	//commit structura mpi 
	
	offsets[0] = 0;
	oldtypes[0] = MPI_INT;
	blockcounts[0] = ENTRIES + 1;

	/* Setup description of the 2 MPI_INT fields n, type */
	/* Need to first figure offset by getting size of MPI_FLOAT */
	MPI_Type_extent(MPI_INT, &extent);
	offsets[1] = (ENTRIES+1) * extent;
	oldtypes[1] = MPI_CHAR;
	blockcounts[1] = ENTRIES * 50;

	/* Now define structured type and commit it */
	MPI_Type_struct(2, blockcounts, offsets, oldtypes, &MPI_ENTRY_WORD);
	MPI_Type_commit(&MPI_ENTRY_WORD);

	MPI_Comm_get_parent( &parentcomm );

	//printf ("Reducer: Number of tasks= %d My rank= %d\n", numtasks,rank);
	MPI_Send(&a,1,MPI_INT,0,tag,parentcomm);

	MPI_Recv(&buffer_size, 1, MPI_INT, MPI_ANY_SOURCE, tag, parentcomm, &stat);
	//alocam spatiu si pentru terminatorul de sir de caractere
	buffer = (char*)malloc((buffer_size+1)*sizeof(char));
			
	MPI_Recv(&buffer[0],buffer_size,MPI_CHAR,MPI_ANY_SOURCE,tag,parentcomm,&stat);
	//se dauaga terminatorul
	buffer[buffer_size]=0;		
	//printf("Reducer %d: string :|%s|\n",rank,buffer);
	//printf("Reduceri %d:Am primit de la parintele %d valoarea %d \n",rank,stat.MPI_SOURCE,buffer_size);
	

   /*******  do some work *******/

	/*********** process work *************/
	process_chunck(buffer,hashtable);

	entry.numar_intrari = 0;
	
	for(i=0;i<hashtable->size;i++){
			
		for(j=0;j<hashtable->buckets[i].current;j++){
			
			if(entry.numar_intrari == ENTRIES){
			
				MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);
				entry.numar_intrari=0;
				
							
			}
			else
			{
				entry.aparitii[entry.numar_intrari] = hashtable->buckets[i].entries[j].number;
				//if(strcmp(hashtable->buckets[i].entries[j].value,"said")==0)
					//printf("Reducer %d: has said %d \n",rank,hashtable->buckets[i].entries[j].number);				
				strcpy(entry.cuvinte[entry.numar_intrari],hashtable->buckets[i].entries[j].value);
				entry.numar_intrari++;				
				
			}
			
		}
		
	}

		if(entry.numar_intrari > 0){
			MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);
			entry.numar_intrari=0;
		}

	/************ Terminare trimitere hashtable *************/
	//printf("Reducer %d: Send exit to mapper\n",rank);
	entry.aparitii[0] = -1;
	
	MPI_Send(&entry,1,MPI_ENTRY_WORD,0,tag,parentcomm);
	
	MPI_Finalize();
	free(hashtable);
	return 0;
}
