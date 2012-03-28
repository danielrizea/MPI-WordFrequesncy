#include <stdio.h>


#ifndef UTILS_H_
#define UTILS_H_


#define ENTRIES 100


	//presupunem ca orice cuvant are sub 50 de caractere;
	//structura folosita pentru transportul datelor intre maperi si reduceri si maperi si masteri


	//se trimit cate ENTRIES informatii despre ENTRIES cuvinte intr-un MPI send deoarece este mai eficient sa face mai putine senduri decat daca am fi facut cate un send pentru fiecare cuvant in parte.Eliminam un overhead important in comunicatie


	typedef struct {
		//numarul de aparitii ale cuvantului
  		int aparitii[ENTRIES];
	
		//cate cuvinte sunt in structura
		int numar_intrari; 		

		//valorile cuvintelor, presupunem cuvinte sub 50 caractere
		char cuvinte[ENTRIES][50];
  	}          Ent;
	Ent    entry;



#endif
