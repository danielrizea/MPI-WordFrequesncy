#include<stdio.h>
#include<string.h>
#include<stdlib.h>


#ifndef HASHTABLE_H_
#define HASHTABLE_H_

// here goes all the code


/*Functie hash table (hashmap) implementare proprie
*
*Am implementat doar operatiile de adauga si merge (merge pentru refacerea tabelului hash prin comunicatiile MPI)
*Altceva nu am considerat ca am avut nevoie explicit (resize tabela, stergere element)
*/

#define BUCKET_GROWTH 10

typedef struct{

	char *value;
	int number;

} Entry;

typedef struct{

	int size;
	int current;
	Entry* entries;

} Bucket;

typedef struct {

	int size;
	Bucket *buckets;

} Hash;

typedef Hash *Hashtable;

int hash_function(char* value,Hashtable hashtable);

Hashtable create_hashtable(int size);

int add_element(Hashtable hashtable,char* value);

int put_element(Hashtable hashtable,char* value);

void print_hashtable(Hashtable hashtable,FILE *std);

void free_hashtable(Hashtable hastable);

int merge_elements(Hashtable hashtable,char *element, int value);

#endif
