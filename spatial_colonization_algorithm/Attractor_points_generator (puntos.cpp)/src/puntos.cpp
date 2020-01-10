/*
 ============================================================================
 Name        : puntos.cpp
 Author      : Curro Jim�nez & A. Ruiz
 Version     : 2.1 -- Generaci�n de puntos de atracci�n para SCA.cpp
               Optimizado para recintos con forma el�ptica o poligonal
 Copyright   : UPO & Casares Lab & Curro
 Description : Elecci�n del recinto y distribuci�n del n�mero fijado
               de puntos atractores aleatoriamente por su interior
 ============================================================================
*/

/*
 Dos inputs principales establecidos en las "opciones del usuario":
  1) N�mero de puntos de atracci�n que se desea generar: NPA
  2) Forma el�ptica o poligonal del contorno de la branquia: F
 Seg�n el valor de F, los elementos definitorios de la branquia:
  2.1) F = 0 -> centro y semiejes de la elipse: c1, c2, a, b
  2.2) F = 1 -> coordenadas de los v�rtices del pol�gono: ai, bi
*/
/*
 Dos outputs:
  1) El fichero "atrac.dat" almacena las coordenadas de los puntos atractores: xi, yi
  2) El fichero "elips.dat" almacena los elementos de la branquia: c1, c2, a, b / ai, bi
*/

/*
 Compilaci�n:
 C -> gcc puntos.c -lglut -lGL -lGLU -lm -o puntos.exe
 C++ -> g++ -O0 -g3 -Wall -c -fmessage-length=0 -o puntos.o puntos.cpp
 C++ -> g++ -o Puntos.exe puntos.o
*/

//-------- Librer�as ---------//

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <math.h>
# include <iostream> // C++

using namespace std; // C++

//--- Opciones del usuario ---//

int M = 512; // tama�o de la red cuadrada (ventana gr�fica M*M)
int NPA = 750; // n�mero de puntos de atracci�n que se desea generar
int F = 0; // F = 0 forma de circunferencia o elipse, F = 1 forma de pol�gono
// F = 0: circunferencias y elipses de centro (c1,c2) y semiejes a horizontal y b vertical
int c1 = 250, c2 = 280, a = 175, b = 210;
// F = 1: recinto poligonal de v�rtices con coordenadas (ai,bi), introducidos entre llaves
int poligono[][2] = {{119,461},{101,433},{90,404},{84,353},{84,273},{89,231},{98,191},{111,158},{136,128},{161,109},{194,91},{215,81},{259,73},{299,79},{329,94},{358,122},{384,155},{401,182},{415,212},{421,242},{425,271},{424,320},{419,349},{402,383},{385,404},{333,449},{294,466},{239,481},{190,483},{157,478}};

static unsigned int xk,yk,zk,ck;
unsigned int JKISS(); // generaci�n de n�meros aleatorios de 64 bits
void winInit(void);

//---- Programa principal ----//

int main(int argc, char **argv){

	winInit();

	return 0;
}

void winInit(){

	time_t t; // permite seleccionar diferentes semillas cada vez
	int contador = 0;

	FILE *fatrac = fopen("atrac.dat","w"); // almacena las coordenadas de los atractores
	FILE *felips = fopen("elips.dat","w"); // almacena los elementos de la elipse usada

	// semillas para generar los n�meros aleatorios
	// printf("Introduce las semillas de aleatorios xk, yk, zk, ck:\n");
	// scanf("%u\n%u\n%u\n%u", &xk,&yk,&zk,&ck);
	srand((unsigned)time(&t));
	xk = 1979*(rand()%1000); yk = 210111*(rand()%1500);
	zk = 1011*(rand()%2000); ck = 31570*(rand()%1700);
	printf("Semillas de aleatorios usadas:\n xk=%u\n yk=%u\n zk=%u\n ck=%u\n", xk,yk,zk,ck);

	// elecci�n por pantalla del n�mero de puntos de atracci�n
	// printf("Introduce el n�mero de atractores a generar:\n");
	// scanf("%d", &NPA);

	float x,y; // coordenadas de los puntos atractores

	int tam = sizeof(poligono)/sizeof(poligono[0]); // n�mero de v�rtices del pol�gono

	// generaci�n de los NPA puntos atractores
	do{
		label: x = JKISS()%M; y = JKISS()%M;

		if(F == 0){ // forma el�ptica
			if((x-c1)*(x-c1)/(a*a) + (y-c2)*(y-c2)/(b*b) > 1) goto label;
		}else if(F == 1){ // forma poligonal
			int i = 0, j = tam-1;
			int dentro = 0;
			for(i=0; i<tam; i++){
				if((poligono[i][1] < y && poligono[j][1] >= y) || (poligono[j][1] < y && poligono[i][1] >= y))
					if(poligono[i][0]+(y-poligono[i][1])/(poligono[j][1]-poligono[i][1])*(poligono[j][0]-poligono[i][0]) < x)
						dentro++; // corta al lado
				j = i;
			}
			if(dentro%2 == 0) goto label; // no est� dentro si es par
		}

		fprintf(fatrac,"%f %f\n",x,y);
		contador++;
	}while(contador < NPA);

	fclose(fatrac);

	if(F == 0) fprintf(felips,"%d %d %d %d\n", c1,c2,a,b);
	if(F == 1) for(int i=0; i<tam; i++) fprintf(felips, "%d %d\n", poligono[i][0],poligono[i][1]);

	fclose(felips);
}

//------ JKISS 64 bits -------//

unsigned int JKISS(){
	// generador de n�meros aleatorios a partir de las semillas xk, yk, zk, ck
	unsigned long long tk;

	xk = 314527869*xk + 1234567;
	yk ^= yk << 5; yk ^= yk >> 7; yk ^= yk << 22;
	tk = 429458439ULL*zk + ck;
	ck = (tk >> 32);
	zk = tk;

	return (xk + yk + zk);
}
