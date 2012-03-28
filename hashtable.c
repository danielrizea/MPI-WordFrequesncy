#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "hashtable.h"

int hash_function(char* value,Hashtable hashtable){

	int key=0;
	int i=0;
	while(value[i] != 0){
		key += i*value[i];
		
		i++;
	}
	key = key % hashtable->size;
	
	return key;
}

Hashtable create_hashtable(int size){

	int i;
	Hashtable hashtable = (Hashtable)malloc(sizeof(Hash));
	
	hashtable->size=size;
	
	hashtable->buckets = (Bucket*)malloc(size*sizeof(Bucket));
	
	for(i=0;i<size;i++){
	
		hashtable->buckets[i].current = 0;
		hashtable->buckets[i].size = 0;
	}
	
	return hashtable;
}




int add_element(Hashtable hashtable,char *element){

	int key = hash_function(element,hashtable);

	int current = hashtable->buckets[key].current;
	
	if(current == hashtable->buckets[key].size){
		//printf("Alloca space\n");
		hashtable->buckets[key].entries = (Entry*)realloc(hashtable->buckets[key].entries, (BUCKET_GROWTH + hashtable->buckets[key].size)*sizeof(Entry)); 
		hashtable->buckets[key].size = hashtable->buckets[key].size + BUCKET_GROWTH ;
	}

	hashtable->buckets[key].entries[current].value = strdup(element); 
	hashtable->buckets[key].entries[current].number = 1;
	
	hashtable->buckets[key].current++;
	
	return 0;
}


int add_element_val(Hashtable hashtable,char *element,int value){

	int key = hash_function(element,hashtable);

	int current = hashtable->buckets[key].current;
	
	if(current == hashtable->buckets[key].size){
		//printf("Alloca space\n");
		hashtable->buckets[key].entries = (Entry*)realloc(hashtable->buckets[key].entries, (BUCKET_GROWTH + hashtable->buckets[key].size)*sizeof(Entry)); 
		hashtable->buckets[key].size = hashtable->buckets[key].size + BUCKET_GROWTH ;
	}

	hashtable->buckets[key].entries[current].value = strdup(element); 
	hashtable->buckets[key].entries[current].number = value;
	
	hashtable->buckets[key].current++;
	
	return 0;
}

int put_element(Hashtable hashtable,char *element){

	int i;
	int key = hash_function(element,hashtable);
		
	if(hashtable->buckets[key].size==0){
		add_element(hashtable, element);
		//printf("Se adauaga cuvantul %s\n",element);
		return 0;
	}
	
	for(i=0;i<hashtable->buckets[key].current;i++){
		
		if(strcmp(hashtable->buckets[key].entries[i].value,element) == 0){
			hashtable->buckets[key].entries[i].number++;
			
			//printf("Cuvantul %s exista cu aparitiile %d\n",element,hashtable->buckets[key].entries[i].number);
			return 0;
		}
	}

	add_element(hashtable,element);
	return 0;
	
}
int merge_elements(Hashtable hashtable,char * element,int value){
	
	int i;
	int key = hash_function(element,hashtable);
		
	if(hashtable->buckets[key].size==0){
		add_element_val(hashtable, element,value);
		//printf("Se adauaga cuvantul %s\n",element);
		return 0;
	}
	
	for(i=0;i<hashtable->buckets[key].current;i++){
		
		if(strcmp(hashtable->buckets[key].entries[i].value,element) == 0){
			hashtable->buckets[key].entries[i].number+=value;
			
			//printf("Cuvantul %s exista cu aparitiile %d\n",element,hashtable->buckets[key].entries[i].number);
			return 0;
		}
	}

	add_element_val(hashtable,element,value);
	return 0;

}

void free_hashtable(Hashtable hashtable){

	int i;

	for(i=0;i<hashtable->size;i++)
		if(hashtable->buckets[i].size != 0)
			free(hashtable->buckets[i].entries);
	free(hashtable->buckets);
	free(hashtable);


}

void print_hashtable(Hashtable hashtable,FILE *std){
	
	int i,j;
	printf("HashMap content\n");
	for(i=0;i<hashtable->size;i++){

		if(hashtable->buckets[i].size > 0){
			for(j=0;j<hashtable->buckets[i].current;j++)
				fprintf(std,"[%s %d]\n",hashtable->buckets[i].entries[j].value,hashtable->buckets[i].entries[j].number);	
		}
				
	}

}


