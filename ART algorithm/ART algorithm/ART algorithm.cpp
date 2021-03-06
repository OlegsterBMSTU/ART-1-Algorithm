// ART algorithm.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>
#include <cstdio>
#include <assert.h>
#include <ctime>
#include <stdio.h>
#include <cstdlib>

FILE *recomendationFile, *vectorsFile;

/*Listing 1.1*/
/* Data structures ART1 for personalization*/
#define MAX_ITEMS				(11)
#define MAX_CUSTOMERS			(10)
#define TOTAL_PROTOTYPE_VECTORS	(5)

const float beta = 1.0;
const float vigilance = 0.9;	/* внимательность от 0 до 1 */

int numPrototypeVectors = 0;	// Quantity vector's prototype
int prototypeVector[TOTAL_PROTOTYPE_VECTORS][MAX_ITEMS];

/*Vector of sum*/
int sumVector[TOTAL_PROTOTYPE_VECTORS][MAX_ITEMS];

/*Quantity members in clusters */
int members[TOTAL_PROTOTYPE_VECTORS];

/*Number of clusters when customer is had*/
int membership[MAX_CUSTOMERS];

/* Strings name elemnts of vectors */

 char itemName[12][12] = {
	"Hammer", "Paper", "Snickers", "Screwdriver",
	"Pen", "Kit-Kat", "Wrench", "Pencil",
	"Heath-Bar", "TapeMeasur", "Binder" }; 

int database[MAX_CUSTOMERS][MAX_ITEMS] = {
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0},
	{ 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
	{ 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0},
	{ 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1},
	{ 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
	{ 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
	{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0},
	{ 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0},
	{ 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0}
};

void printDataBase()
{
	int i, j;
	fprintf_s(vectorsFile, "D A T A B A S E\n\n");
	for (i = 0; i < MAX_CUSTOMERS; i++) {
		for (j = 0; j < MAX_ITEMS; j++) {
			fprintf_s(vectorsFile, "%i ", database[i][j]);
		}
		fprintf_s(vectorsFile, "\n");
	}
	fprintf_s(vectorsFile, "***END***\n\n\n");
}
/*Listing 1.3*/
/* Initializations data structurs algorighm */
void initializate(void) 
{
	int i, j;
	/*Cleaning vectors-prorotype*/
	for (i = 0; i < TOTAL_PROTOTYPE_VECTORS; i++) {
		for (j = 0; j < MAX_ITEMS; j++) {
			prototypeVector[i][j] = 0;
			sumVector[i][j] = 0;
		}
		members[i] = 0;
	}
	/*Reset values belong vectors to clusters*/
	for (j = 0; j < MAX_CUSTOMERS; j++) {
		membership[j] = -1;
	}
	printDataBase();
}

/*Listing 1.4*/
/*auxiliary functions for ART1 algorithm*/
int vectorMagnitude(int *vector)
{
	int j, total = 0;
	for (j = 0; j < MAX_ITEMS; j++) {
		if (vector[j] == 1) total++;
	}
	return total;
}
void vectorBitwiseAnd(int *result, int *v, int *w)
{
	int i;
	for (i = 0; i < MAX_ITEMS; i++) {
		result[i] = (v[i] && w[i]);
	}
	return;
}

/* Listing 1.5 */
/* Functions of control vectors-prototype */
int createNewPrororypeVector(int *example)
{
	int i, cluster;
	for (cluster = 0; cluster < TOTAL_PROTOTYPE_VECTORS; cluster++) {
		if (members[cluster] == 0) break;
	}
	if (cluster == TOTAL_PROTOTYPE_VECTORS) assert(0);
	fprintf_s(vectorsFile, "Creating new cluster %i\n", cluster);
	#ifdef DEBUG
		printf("Creating new cluster %i\n", cluster);
	#endif // DEBUG
	numPrototypeVectors++;
	for (i = 0; i < MAX_ITEMS; i++) {
		prototypeVector[cluster][i] = example[i];
	#ifdef DEBUG
			printf("%1d", example[i]);
	#endif // DEBUG
			fprintf_s(vectorsFile, "%1d ", example[i]);
	}
	members[cluster] = 1;
#ifdef DEBUG
	printf("\n");
#endif // DEBUG
	fprintf_s(vectorsFile, "\n");
	
	return cluster;
}
void updatePrototypeVectors(int cluster)
{
	int item, customer, first = 1;
	assert(cluster >= 0);
#ifdef DEBUG
	printf("Recomputing prototype Vector %d (%d)\n", cluster, members[cluster]);
#endif // DEBUG
	//fprintf_s(vectorsFile, "Recomputing prototype Vector %d (%d)\n", cluster, members[cluster]);
	for (item = 0; item < MAX_ITEMS; item++) {
		prototypeVector[cluster][item] = 0;
		sumVector[cluster][item] = 0;
	}
	for (customer = 0; customer < MAX_CUSTOMERS; customer++) {
		if (membership[customer] == cluster) {
			if (first) {
				for (item = 0; item < MAX_ITEMS; item++) {
					prototypeVector[cluster][item] = database[customer][item];
					sumVector[cluster][item] = database[customer][item];
				}
				first = 0;
			}
			else {
				for (item = 0; item < MAX_ITEMS; item++) {
					prototypeVector[cluster][item] = prototypeVector[cluster][item] && database[customer][item];
					sumVector[cluster][item] += database[customer][item];
				}
			}
		}
	}
	return;
}

/*Listing 1.6*/
/*ART-1 Algotihm */
int performART1(void)
{
	int andresult[MAX_ITEMS];
	int pvec, magPE, magP, magE;
	float result, test;
	int count = 50;
	int index, done = 0;

	while (!done) {
		done = 1;
		for (index = 0; index < MAX_CUSTOMERS; index++) {	//Going to all customers
			for (pvec = 0; pvec < TOTAL_PROTOTYPE_VECTORS; pvec++) { //step 3 
				/* Does it have any elements this cluster? */
				if (members[pvec]) {
					vectorBitwiseAnd(andresult, &database[index][0], &prototypeVector[pvec][0]);
					magPE = vectorMagnitude(andresult);
					magP = vectorMagnitude(&prototypeVector[pvec][0]);
					magE = vectorMagnitude(&database[index][0]);
					/* Thisi is the formula 3.2*/
					result = (float)magPE / (beta + (float)magP);	//left part (There is a formula 3.2 on the page 45 in the Programming of Artifical Intelegency book
					test = (float)magE / (beta + (float)MAX_ITEMS);//righr part
					/* The formula 3.2 */
					if (result>test)
					{
						/* Attention Test */
						if (((float)magPE / (float)magE) < vigilance) {
							int old;
							/*We are convinced that this is other cluster */
							if (membership[index] != pvec) {
								/*Move customer to other cluster*/
								old = membership[index];
								membership[index] = pvec;
								if (old > 0) {
									members[old]--;
									if (members[old] == 0) numPrototypeVectors--;
								}
								members[pvec]++;
								/*Count vectors-prototype for each clusters*/
								if ((old > 0) && (old < TOTAL_PROTOTYPE_VECTORS)) {
									updatePrototypeVectors(old);
								}
								updatePrototypeVectors(pvec);
								done = 0;
								break;
							}
							else {
								/* In this cluster */
							}
						} /*Clothing Attention Test*/
					}
				}
			}
			if (membership[index] == -1) {
				/*Situable cluster wasn't found*/
				/*Create a new cluster for this vector-p*/
				membership[index] = createNewPrororypeVector(&database[index][0]);
				done = 0;
			}

		} /*Clothing cycle for each customers*/
		if (!count--) break;
	}
	return 0;
}

/*Listing 1.7 */
/*Recomendation algorithm */

void makeRecomendation(int customer)
{
	int bestItem = -1;
	int val = 0;
	int item;
	
	for (item = 0; item < MAX_ITEMS; item++) {
		if ((database[customer][item] == 0) && (sumVector[membership[customer]][item] > val)) {
			bestItem = item;
			val = sumVector[membership[customer]][item];
		}
	}
	printf("For Customer %d, ", customer);
	fprintf_s(recomendationFile, "For Customer %d, ", customer);
	if (bestItem > 0) {
		printf("The best recomendation is %d (%s)\n", bestItem, itemName[bestItem]);
		fprintf_s(recomendationFile, "The best recomendation is %d (%s)\n", bestItem, itemName[bestItem]);
		printf("Owned by %d out of %d members of this cluster\n", sumVector[membership[customer]][bestItem], members[membership[customer]]);
		fprintf_s(recomendationFile, "Owned by %d out of %d members of this cluster\n", sumVector[membership[customer]][bestItem], members[membership[customer]]);
	}
	else {
		fprintf_s(recomendationFile, "No recomendation\n");
		printf("No recomendation can be made.\n");
	}

	printf("Already owns: ");

	for (item = 0; item < MAX_ITEMS; item++) {
		if (database[customer][item]) printf("%s", itemName[item]);
	}
	printf("\n\n");
	fprintf(recomendationFile, "\n\n");
	
}



int main()
{
    std::cout << "Hello World!\n"; 
	int customer;
	fopen_s(&recomendationFile, "recomendationFile.txt", "w");
	fopen_s(&vectorsFile, "vectorsFile.txt", "w");
	srand(time(NULL));
	initializate();
	performART1();
	for (customer = 0; customer < MAX_CUSTOMERS; customer++) {
		makeRecomendation(customer);
	}
	fclose(recomendationFile);
	fclose(vectorsFile);
	return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
