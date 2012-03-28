#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "utils.h"
/*This is the main master program

*/


void swap(Entry *x,Entry *y)
{
   Entry temp;
   temp = *x;
   *x = *y;
   *y = temp;
}

int choose_pivot(int i,int j )
{
   return((i+j) /2);
}

void quicksort(Entry list[],int m,int n)
{
   int key,i,j,k;
   if( m < n)
   {
      k = choose_pivot(m,n);
      swap(&list[m],&list[k]);
      key = list[m].number;
      i = m+1;
      j = n;
      while(i <= j)
      {
         while((i <= n) && (list[i].number >= key))
                i++;
         while((j >= m) && (list[j].number < key))
                j--;
         if( i < j)
                swap(&list[i],&list[j]);
      }
	  // swap two elements
      swap(&list[m],&list[j]);
	  // recursively sort the lesser list
      quicksort(list,m,j-1);
      quicksort(list,j+1,n);
   }
}

void printlist(Entry list[],int n,FILE *out)
{
   int i;
   for(i=0;i<n;i++)
      fprintf(out,"[%s %d]\n",list[i].value,list[i].number);
}


int main(int argc, char *argv[]){
	int  numtasks, rank, rc,tag=1;

	FILE *configFile,*fileOut;

	char *command; 
	char **argv2; 
	command = "./mapper_program"; 
		
	char configFileName[] = "config.in";
	int maxMappers;
	int maxReducers;	
	MPI_Status stat;
	MPI_Comm intercomm;
	Hashtable hashtable;
	int i,j;
	
	char fileInName[100];
	char fileOutName[100];
	MPI_Datatype MPI_ENTRY_WORD, oldtypes[2]; 
	int          blockcounts[2];

	/* MPI_Aint type used to be consistent with syntax of */
	/* MPI_Type_extent routine */
	MPI_Aint    offsets[2], extent;

	
	hashtable = create_hashtable(1000);
	
	configFile = fopen(configFileName,"r");
	if(configFile == NULL)
		printf("Error opening %s file for configurations\n",configFileName);
	
	//citire fisier de configuratii
	fscanf(configFile,"%d\n",&maxMappers);
	fscanf(configFile,"%d\n",&maxReducers);
	

	fscanf(configFile,"%s\n",fileInName);
	fscanf(configFile,"%s\n",fileOutName);

	fileOut = fopen(fileOutName,"w");
	printf("Master: mappers %d reducers %d\n",maxMappers,maxReducers);
	

	
	
	
	char buff[20];
	sprintf(buff,"%d",maxReducers);
		
	argv2=(char **)malloc(3 * sizeof(char *)); 
	argv2[0] = (char*)malloc(100*sizeof(char));
	strcpy(argv2[0],fileInName);	
	argv2[1] = buff; 
	

	argv2[2] = NULL; 
	
	
	printf("Master: Fisierul de citit %s | %s ------------------\n",argv2[1],argv2[0]);

	int *errcodes = (int*)malloc(maxMappers * sizeof(int));
	
	
	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	//commit structura mpi 
	
	offsets[0] = 0;
	oldtypes[0] = MPI_INT;
	blockcounts[0] = ENTRIES + 1;

	/* Setup description of the 2 MPI_INT fields n, type */
	/* Need to first figure offset by getting size of MPI_FLOAT */
	MPI_Type_extent(MPI_INT, &extent);
	offsets[1] = (ENTRIES + 1) * extent;
	oldtypes[1] = MPI_CHAR;
	blockcounts[1] = ENTRIES * 50;

	/* Now define structured type and commit it */
	MPI_Type_struct(2, blockcounts, offsets, oldtypes, &MPI_ENTRY_WORD);
	MPI_Type_commit(&MPI_ENTRY_WORD);


	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
		
	//proces principal ce va face spawn proceselor map		
	MPI_Comm_spawn(command,argv2,maxMappers,MPI_INFO_NULL,0,MPI_COMM_WORLD,&intercomm,errcodes);
		
	

	
	//for(i=0;i<maxMappers;i++)
	//	MPI_Send(&i,1,MPI_INT,i,tag,intercomm);

	
	
	
   /*******  do some work *******/

	int ok=0;

	while(ok<maxMappers){

		MPI_Recv(&entry,1,MPI_ENTRY_WORD,MPI_ANY_SOURCE,tag,intercomm,&stat);
		
		if(entry.aparitii[0] == -1){
			ok++;
			printf("Master:Finalize from %d\n",stat.MPI_SOURCE);
		}
		else
			for(i=0;i<entry.numar_intrari;i++)
				merge_elements(hashtable,entry.cuvinte[i],entry.aparitii[i]);

	}

	

	
	MPI_Finalize();
	//print_hashtable(hashtable,fileOut);
	
	
   Entry *list;
	int listSize = 1000;
	int listPoz = 0;
	list = (Entry*) malloc(sizeof(Entry)*listSize);
	
   printf("Master: start to make list\n");
	for(i=0;i<hashtable->size;i++){

		if(hashtable->buckets[i].size > 0){
			for(j=0;j<hashtable->buckets[i].current;j++){
				if(listPoz == listSize){
					list = (Entry*)realloc(list,sizeof(Entry)*(listSize+1000));
					listSize += 1000;				
				}
				list[listPoz].value = hashtable->buckets[i].entries[j].value;
				list[listPoz].number = 	hashtable->buckets[i].entries[j].number;
				listPoz++;			
					
			}
			
		}
				
	}
	printf ("Master: %d am terminat\n",rank);

   // sort the list using quicksort
   quicksort(list,0,listPoz-1);

   // print the result
   //printf("The list after sorting using quicksort algorithm:\n");
   printlist(list,listPoz,fileOut);




	free_hashtable(hashtable);
	fclose(fileOut);
	return 0;
}

