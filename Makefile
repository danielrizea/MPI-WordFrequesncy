build:
	mpicc -g -Wall master.c hashtable.c -o master
	mpicc -g -Wall mapper_program.c hashtable.c -o mapper_program
	mpicc -g -Wall reduce_program.c hashtable.c -o reduce_program
clean:
	rm master mapper_program reduce_program
