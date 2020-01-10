/*
 ============================================================================
 Name        : SCA.cpp
 Author      : Curro Jiménez & A. Ruiz
 Version     : 3.0 -- Algoritmo de Colonización Espacial
               Ramas -> array de memoria dinámica
               Atractores -> queue; movimiento aleatorio
 Mejoras     : Se puede introducir la inercia (dirección del padre)
               Generar archivo farbol íntegramente en volcado()
               En semilla.dat, atrac.dat coords. < M (si no, error)
 Copyright   : UPO & Casares Lab & Curro
 Description : Modelo SCA para traqueogénesis en Cloeon dipterum
 ============================================================================
*/

/*
 Cuatro inputs principales leídos del fichero "parametros.txt":
  1) Radio de influencia de los puntos de atracción en los puntos de rama: MAX
  2) Radio de alcance de un punto de atracción por parte de una rama: KILL
  3) Longitud del crecimiento de las ramas en cada iteración: CREC
  4) Velocidad de los puntos de atracción en su movimiento aleatorio: vel
 Dos inputs adicionales leídos del fichero "atrac.dat" y "elips.dat":
  5) Coordenadas de los puntos de atracción: cx, cy
  6) Elementos del recinto de generación de puntos de atracción
 Unos inputs iniciales leídos del fichero "semilla.dat":
  7) Datos de los puntos de rama iniciales: i, ipad, cx, cy, ddx, ddy
*/
/*
 Cinco outputs:
  1) Salida gráfica por ventana "Modelo SCA", grabada en el fichero "MAX_KILL_CREC_vel_grafico.pnm"
  2) El fichero "MAX_KILL_CREC_vel_aleator.dat" almacena las semillas de aleatorios: xk, yk, zk, ck
  3) El fichero "MAX_KILL_CREC_vel_atrac.dat" almacena los puntos de atracción utilizados: cx, cy
  4) El fichero "MAX_KILL_CREC_vel_elips.dat" almacena los elementos del recinto de atractores
  5) El fichero "MAX_KILL_CREC_vel_arbol.csv" guarda los puntos de rama: i, cx, cy, tg, ipad
*/

/*
 Compilación:
 C -> gcc SCA.c -lglut -lGL -lGLU -lm -o SCA.exe
 C++ -> g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -o SCA.o SCA.cpp
 C++ -> g++ -o SCA.exe SCA.o -lfreeglut -lglu32 -lopengl32
 Eclipse Properties > C/C++ Build >
  	Settings > GCC C++ Compiler: g++ / -O0 -g3 -Wall -c -fmessage-length=0
  	Settings > GCC C Compiler: gcc / -O0 -g3 -Wall -c -fmessage-length=0
  	Settings > MinGW C++ Linker > Libraries: -l / freeglut; glu32; opengl32
  	Tool Chain Editor: MinGW GCC / CDT Internal Builder
 En sistemas Linux, comentar la línea "# include <windows.h>"
 Considerar modificar las "User options" para sondear las diferentes opciones
 Para no mostrar la ventana de salida gráfica, ejecutar: SCA.exe 0
*/

//-------- Librerías ---------//

# include <windows.h> // para MS Windows
# include <math.h>
# include <stdio.h> // uso de ficheros FILE
# include <stdlib.h>
# include <GL/glut.h> // librería gráfica
# include <time.h> // semillas de aleatorios
//# include <unistd.h> // needed to sleep
//# include <cstdlib> // C++
# include <iostream> // C++
# include <string> // C++ tipo entero a string
# include <sstream> // C++ tipo entero a string
# include <cstring> // C++ tipo entero a string
# include <queue> // C++ colas y sus funciones

using namespace std; // C++

//-------- Constantes --------//

# define ESCAPE 27 // código ASCII de tecla
# define ESPACIO 32 // código ASCII de tecla
# define PI 3.141592654
# define UNI JKISS()/4294967296.0 // genera un número float aleatorio en (0,1), para probabilidades
# define dis(i1,j1,i2,j2) (sqrt((i2-i1)*(i2-i1)+(j2-j1)*(j2-j1))) // distancia entre dos puntos

//--- User options & Flags ---//

# define M 512 // tamaño de la ventana gráfica (M*M)
int V = 1; // ventana con la evolución de la estructura (muestra la ventana = 1; no la muestra = 0)
int F = 1; // fondo del recinto de puntos de atracción (dibuja el fondo = 1; no lo dibuja = 0)
int B = 1; // borde del recinto de puntos de atracción (dibuja el borde = 1; no lo dibuja = 0)
int T = 1; // tiempo de actualización de la salida gráfica (frecuencia con la que se dibuja)
int IG = 1; // espesor inicial de una rama recién creada (en píxeles)
int MG = 8; // máximo espesor que puede alcanzar una rama (en píxeles)
int C = 80; // semieje b inicial de la elipse o % de tamaño del polígono (crece > 0, no crece = 0)
int CE = 1; // expansión de las ramas con el crecimiento de la branquia (expande = 1, no expande = 0)
int FC = 1; // frecuencia de crecimiento de la branquia (crece cuando tg%FC == 0)
int S = 0; // surgencia aleatoria de nuevos puntos de atracción si CE > 0 (surgen = 1; no surgen = 0)
int R = 1; // regeneración de parte de la estructura ramificada (regenera = 1; no regenera = 0)
int E = 0; // captura la salida gráfica en cada tiempo tg para conservar la evolución (sí = 1; no = 0)
int TF = 5000; // tiempo máximo de ejecución del programa (en número de iteraciones)
float m = -0.1, n = 375; // recta de corte para la regeneración de la región y > mx + n
float P = 1./8; // coeficiente de difusión de atractores: probabilidad de permanecer estáticos

//---- Resto de variables ----//

int MAX,KILL,CREC; // radio de influencia de los atractores, radio de killing, longitud de crecimiento
int vel; // velocidad con la que se moverán aleatoriamente los puntos de atracción
int NPA,NPAL,PR,NV; // número de puntos de atracción, número de puntos de rama generados y vértices
int tg,tr; // tiempo transcurrido (en número de iteraciones)
int window; // ventana creada para la salida gráfica
int c1,c2,a,b; // elementos de la elipse usada como recinto de puntos de atracción
float aa,bb,ab,p11,p12,p21,p22,ox,oy; // auxiliares (con crecimiento y regeneración activados)
int *poligono,*cpoligono,*hpoligono; // listas de memoria dinámica con los vértices del recinto
float rhomo; // razón de la homotecia (con recinto poligonal y crecimiento activado)
int xhomo,yhomo; // centro de la homotecia (con recinto poligonal y crecimiento activado)
FILE *farbol; // fichero con las coordenadas y parámetros de los puntos de rama
FILE *faleator; // fichero con las semillas de aleatorios utilizadas y otros datos de interés
std::string fnombre,ffichero; // nombre y formato de los ficheros de salida del programa
char *farchivo; // cadena de caracteres del nombre completo de los archivos de salida del programa
stringstream sMAX,sKILL,sCREC,svel; // cambio de tipo de datos de los enteros MAX, KILL, CREC, vel
string cMAX,cKILL,cCREC,cvel; // cadena de caracteres de los valores enteros MAX, KILL, CREC, vel

static unsigned int xk,yk,zk,ck; // números semilla para la generación de aleatorios
unsigned int JKISS(); // generación de números aleatorios de 64 bits

struct punto_atrac{ // estructura de los atractores (cada uno afecta a un sólo punto de rama)
	float x,y; // coordenadas del punto de atracción
	int rama; // etiqueta (índice del array Ramas) del punto de rama más cercano
};
std::queue<punto_atrac> Atractores, AtractoresLimbo; // colas que albergan los puntos de atracción

struct punto_rama{ // estructura de los puntos de rama generados
	float x,y; // coordenadas del punto de rama
	int pad; // índice del punto de rama predecesor
	int con; // contador de atractores
	float vx,vy; // vector de crecimiento
	float dx,dy; // dirección heredada
	int edad; // tiempo en el que se genera
	float gord; // grosor de la rama
	int nod; // tipo de nodo (ramificaciones que se originan de él)
};
punto_rama *Ramas, *cRamas, *hRamas; // listas de memoria dinámica con los puntos de rama

//----------------------------//
//     PROGRAMA PRINCIPAL     //
//----------------------------//

void inicio(void); // carga los parámetros de entrada y registra los inputs empleados

void display(void); // efectúa la salida gráfica del programa en cada iteración

void atraccion(void); // asocia a cada punto de atracción el punto de rama más próximo
void ramificacion(void); // induce el crecimiento de las ramas en la dirección pertinente
void saciador(void); // desactiva el crecimiento de las ramas, una vez que se ha producido
void surgencia(void); // incorpora aleatoriamente a la branquia nuevos puntos de atracción
void movimiento(void); // lleva a cabo el movimiento aleatorio de los puntos de atracción
void crecimiento(void); // implementa el crecimiento en tamaño del recinto de la branquia
void regeneracion(void); // destruye y regenera parte de la estructura ramificada

bool dentropoli(float,float); // comprueba si un punto pertenece al interior del polígono
void captura(void); // exporta la salida gráfica del programa como archivo de imagen
void volcado(void); // finaliza la ejecución del programa guardando la salida gráfica

void keyPressed(unsigned char,int,int); // comportamiento del programa al pulsar una tecla

//----- Rutinas gráficas -----//

//void reshape(int width,int height){
//	// remodela la salida gráfica al redimensionar la ventana
//	glViewport(0,0,width,height);
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glOrtho(-1,1,-1,1,-1,1);
//	glMatrixMode(GL_MODELVIEW);
//}

void keyPressed(unsigned char key,int x,int y){
	// función llamada cada vez que se pulsa una tecla
	if(key == ESCAPE) volcado(); // "Escape" termina la ejecución del programa
	else if(key == ESPACIO) system("pause"); // "Espacio" pausa la ejecución del programa
}

//-------- Principal ---------//

int main(int argc,char **argv){

	if(argc == 2) V = atoi(argv[1]); // toma el valor V de la entrada por línea de comandos
	if(V == 0){ // ventana de evolución gráfica desactivada
		inicio(); // inicializa el programa
		while(NPA != 0) display(); // ejecuta mientras haya atractores
		V = 2; // cambia de valor para generar la salida gráfica
	}

	glutInit(&argc,argv); // inicializa la librería GLUT
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA); // se usa un único buffer
	// glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB); // para doble buffer
	glutInitWindowPosition(5,5); // posición de la ventana (desde la esquina superior izquierda)
	glutInitWindowSize(M,M); // tamaño de la ventana M*M (ancho por alto)
	glEnable(GL_LINE_SMOOTH); // permite cambiar el grosor de las líneas
	glEnable(GL_LINE_STIPPLE); // permite dibujar las líneas de diferente color

	window = glutCreateWindow("Modelo SCA"); // crea la ventana con el nombre dado

	if(V == 2){ // aquí termina la ejecución del programa si se fijó V = 0
		gluOrtho2D(0,M,0,M); // establece el plano de visualización
		display(); // genera la salida gráfica final
	}

	inicio(); // inicializa el programa

	glutDisplayFunc(display); // llama a "display" cada vez que se avanza una iteración
	// glutReshapeFunc(reshape); // remodela la gráfica al redimensionar la ventana
	glutKeyboardFunc(&keyPressed); // atiende a las instrucciones por teclado
	glutIdleFunc(display);
	glutMainLoop(); // entra en bucle de procesamiento

	return 0; // finalización normal del programa
}

//---------- Inicio ----------//

void inicio(void){
	// carga los parámetros de entrada y registra los inputs empleados en diferentes ficheros
	int i,ipad;
	float cx,cy,ddx,ddy;
	time_t t; // permite seleccionar diferentes semillas cada vez

	if(R != 2){ // R == 2 cuando se entra en proceso de regeneración
		tg = 0; tr = -1; // inicializa el "tiempo"

		FILE *fparam = fopen("parametros.txt","r"); // fichero de entrada de los parámetros principales
		if(fscanf(fparam, "%d %d %d %d", &MAX,&KILL,&CREC,&vel) != 4){
			printf("Error al procesar el archivo 'parametros.txt'.\n");
			exit(-1); // finalización anómala del programa
		}
		fclose(fparam); // cierra el archivo "parametros.txt"

		srand((unsigned)time(&t)); // genera las cuatro semillas de números aleatorios
		xk = 1979*(rand()%1000); yk = 210111*(rand()%1500);
		zk = 1011*(rand()%2000); ck = 31570*(rand()%1700);

		sMAX << MAX; sKILL << KILL; sCREC << CREC; svel << vel; // desde tipo entero
		sMAX >> cMAX; sKILL >> cKILL; sCREC >> cCREC; svel >> cvel; // a tipo string

		fnombre = "aleator.dat"; // fichero de salida con las semillas aleatorias y opciones usadas
		ffichero = cMAX + "_" + cKILL + "_" + cCREC + "_" + cvel + "_" + fnombre;
		farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
		faleator = fopen(farchivo,"w"); delete[] farchivo;
		fprintf(faleator, "xk = %u\nyk = %u\nzk = %u\nck = %u\nIG = %d\nMG = %d\n C = %d\nCE = %d\nFC = %d\n S = %d\n P = %f\n", xk,yk,zk,ck,IG,MG,C,CE,FC,S,P);
		if(R != 1) fclose(faleator); // cierra el archivo "MAX_KILL_CREC_vel_aleator.dat"

		fnombre = "elips.dat"; // fichero de salida con los elementos del recinto de atractores
		ffichero = cMAX + "_" + cKILL + "_" + cCREC + "_" + cvel + "_" + fnombre;
		farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
		FILE *felips = fopen(farchivo,"w"); delete[] farchivo;
		FILE *felipse = fopen("elips.dat","r"); // fichero de entrada del recinto (elipse o polígono)
		if(fscanf(felipse, "%d %d %d %d\n", &c1,&c2,&a,&b) != 4){
			printf("Error al procesar el archivo 'elips.dat'.\n");
			exit(-1); // finalización anómala del programa
		}
		if(!feof(felipse)){ // no ha llegado al final del fichero, no es una elipse
			rewind(felipse); // regresa al principio del fichero
			c1 = -1; NV = 1; // número de vértices
			poligono = (int*)malloc(NV*2*sizeof(int)); // reserva memoria para los vértices
			while(!feof(felipse)){
				if(fscanf(felipse, "%d %d\n", &a,&b) != 2){
					printf("Error al procesar el archivo 'elips.dat'.\n");
					free(poligono); // libera la reserva de memoria
					exit(-1); // finalización anómala del programa
				}
				poligono[2*(NV-1)] = a; poligono[2*NV-1] = b; // coordenadas del vértice
				fprintf(felips, "%d %d\n", a,b);
				NV++; // aumenta el contador de vértices
				cpoligono = (int*)realloc(poligono, sizeof(int)*2*NV); // amplía la reserva de memoria
				poligono = cpoligono;
			}
			if(C > 0){ // recinto poligonal y activado el crecimiento
				hpoligono = (int*)malloc(NV*2*sizeof(int)); // reserva memoria para los vértices
				memcpy(hpoligono, poligono, 2*NV*sizeof(int)); // copia de los vértices del recinto
				rhomo = (float)C/100, xhomo = M, yhomo = M; // razón y centro de la homotecia
			}
		}else fprintf(felips, "%d %d %d %d\n", c1,c2,a,b); // sí es una elipse
		fclose(felips); // cierra el archivo "MAX_KILL_CREC_vel_elips.dat"
		fclose(felipse); // cierra el archivo "elips.dat"

		fnombre = "arbol.csv"; // fichero de salida que grabará los puntos de rama
		ffichero = cMAX + "_" + cKILL + "_" + cCREC + "_" + cvel + "_" + fnombre;
		farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
		farbol = fopen(farchivo,"w"); delete[] farchivo;
		fprintf(farbol, "#; x; y; Tiempo; Padre\n"); // cabecera de "MAX_KILL_CREC_vel_arbol.csv"

		PR = 2; // el primer punto inicial se almacena en Ramas[1] (Ramas[0] queda vacío...)

		FILE *fsemilla = fopen("semilla.dat","r"); // fichero de entrada de los puntos de rama iniciales
		Ramas = (punto_rama*)malloc(PR*sizeof(punto_rama)); // reserva memoria para PR puntos de rama
		while(!feof(fsemilla)){
			if(fscanf(fsemilla, "%d %d %f %f %f %f\n", &i,&ipad,&cx,&cy,&ddx,&ddy) != 6 || i != PR-1 || ipad >= i){
				printf("Error al procesar el archivo 'semilla.dat'.\n");
				free(Ramas); // libera la reserva de memoria
				exit(-1); // finalización anómala del programa
			}
			Ramas[i].pad = ipad; Ramas[i].x = cx; Ramas[i].y = cy; Ramas[i].dx = ddx; Ramas[i].dy = ddy;
			Ramas[i].gord = IG; Ramas[i].con = Ramas[i].vx = Ramas[i].vy = Ramas[i].edad = Ramas[i].nod = 0;
			fprintf(farbol, "%d; %d; %d; %d; %d\n", i,(int)cx,(int)cy,tg,ipad); // graba el punto de rama
			PR++; // aumenta el contador de puntos de rama
			cRamas = (punto_rama*)realloc(Ramas, sizeof(punto_rama)*PR); // amplía la reserva de memoria
			Ramas = cRamas;
		}
		if(c1 == -1 && C > 0){ // recinto poligonal y activado el crecimiento
			hRamas = (punto_rama*)malloc(PR*sizeof(punto_rama)); // reserva memoria para los puntos de rama
			memcpy(hRamas, Ramas, PR*sizeof(punto_rama)); // copia de los puntos de rama
		}
		fclose(fsemilla); // cierra el archivo "semilla.dat"
	}

	fnombre = R != 2 ? "atrac.dat" : "atracreg.dat" ; // fichero de salida con los puntos de atracción
	ffichero = cMAX + "_" + cKILL + "_" + cCREC + "_" + cvel + "_" + fnombre;
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	FILE *fatrac = fopen(farchivo,"w"); delete[] farchivo;
	FILE *fatractores = fopen(R != 2 ? "atrac.dat" : "atracreg.dat","r"); // fichero de entrada de los puntos atractores
	while(!feof(fatractores)){
		if(fscanf(fatractores, "%f %f\n", &cx,&cy) != 2){
			printf("Error al procesar el archivo 'atrac.dat'/'atracreg.dat'.\n");
			exit(-1); // finalización anómala del programa
		}
		punto_atrac A; // punto de atracción
		A.x = cx; A.y = cy; // coordenadas del punto de atracción
		if(R != 2 || A.y - m*A.x - n > 0){ // descarta puntos atractores sólo cuando entra en regeneración
			fprintf(fatrac, "%d %d\n", (int)cx,(int)cy);
			Atractores.push(A); // punto de atracción añadido a la cola
		}
	}
	fclose(fatrac); // cierra el archivo "MAX_KILL_CREC_vel_atrac.dat"
	fclose(fatractores); // cierra el archivo "atrac.dat"

	NPA = Atractores.size(); NPAL = 0; // número total de puntos de atracción

	if(V == 1 && R != 2) gluOrtho2D(0,M,0,M); // establece el plano de visualización
}

//------- Iteraciones --------//

void display(void){
	// efectúa la salida gráfica del programa en cada iteración y llama a las funciones de evolución
	int i,th;
	float x1,y1,x2,y2;

	if(C > 0) crecimiento(); // está activado el crecimiento

	if(V != 0 && tg%T == 0){ // actualiza la ventana de salida gráfica periódicamente
		glClearColor(1,1,1,0); // fija el color de fondo a blanco y opaco
		glClear(GL_COLOR_BUFFER_BIT); // borra la pantalla
		// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // para doble buffer
		// glPushMatrix(); // guarda la matriz actual en una pila

		if(F == 1){ // dibuja el fondo del recinto donde se distribuyen los puntos de atracción
			glColor3f(0.95,0.95,0.95); // color del fondo
			glBegin(GL_POLYGON); // pinta el polígono determinado por los vértices (x1,y1)
			if(c1 != -1) for(th=0; th<360; th++){ // elipse
				x1 = c1 + a*cos(th*PI/180); y1 = c2 + b*sin(th*PI/180);
				glVertex2f(x1,y1);
			}else for(i=1; i<NV; i++) glVertex2f(poligono[2*(i-1)],poligono[2*i-1]); // polígono
			glEnd(); // fin del polígono

			if(R == 2){ // dibuja la recta de corte cuando está activada la regeneración
				glColor3f(0.5,0.5,0.5); // color de la recta
				glLineWidth(1); // grosor de la recta de corte
				glBegin(GL_LINES); // pinta el segmento determinado por (x1,y1) y (x2,y2)
				x1 = p11+20/sqrt(m*m+1); y1 = m*x1+n; x2 = p21-20/sqrt(m*m+1); y2 = m*x2+n;
				glVertex2f(x1,y1); glVertex2f(x2,y2);
				glEnd(); // fin del segmento de recta

				// dibuja los puntos de corte cuando está activada la regeneración
				glColor3f(0.0,1.0,0.0); // color de los puntos
				glPointSize(3); // tamaño de los puntos
				glBegin(GL_POINTS); // pinta los puntos de corte
				glVertex2f(ox,oy); glVertex2f(p11,p12); glVertex2f(p21,p22);
				glEnd(); // fin de los puntos
			}
		}

		if(B == 1){ // dibuja el borde del recinto donde se distribuyen los puntos de atracción
			glColor3f(0.5,0.5,0.5); // color del borde
			glLineWidth(1); // grosor de los segmentos de recta
			glBegin(GL_LINE_LOOP); // pinta los segmentos determinados por los vértices (x1,y1)
			if(c1 != -1) for(th=0; th<360; th++){ // elipse
				x1 = c1 + a*cos(th*PI/180); y1 = c2 + b*sin(th*PI/180);
				glVertex2f(x1,y1);
			}else for(i=1; i<NV; i++) glVertex2f(poligono[2*(i-1)],poligono[2*i-1]); // polígono
			glEnd(); // fin de los segmentos de recta
		}

		// dibuja los puntos de atracción
		glColor3f(1.0,0.0,0.0); // color de los atractores
		glPointSize(2); // tamaño de los puntos atractores
		glBegin(GL_POINTS); // pinta los puntos determinados por las coordenadas (x1,y1)
		for(i=0; i<NPA; i++){
			punto_atrac A = Atractores.front(); // punto de atracción de la cabecera
			Atractores.pop(); // punto de atracción extraído de la cabecera
			x1 = A.x; y1 = A.y; // coordenadas del punto de atracción
			Atractores.push(A); // punto de atracción añadido a la cola
			glVertex2f(x1,y1);
		}
		glEnd(); // fin de los puntos

		// dibuja las ramas generadas por los puntos de rama
		for(i=1; i<PR-1; i++){
			if(Ramas[i].pad > 0 && Ramas[Ramas[i].pad].pad != -1){ // existe punto de rama predecesor
				float a1,b1,c1;
				float colnor = (tg != 0 ? (float)Ramas[i].edad/tg : 0);
				if(colnor >= 0 && colnor <= 0.5){ // degradado de marino (0 0 0.5) a azul (0 0 1)
					a1 = 0; b1 = 0; c1 = 0.5+1*colnor;
				}else if(colnor > 0.5 && colnor <= 1){ // degradado de azul (0 0 1) a cielo (0.5 0.8 1)
					a1 = 0+1*(colnor-0.5); b1 = 0+1.6*(colnor-0.5); c1 = 1;
				}
				glColor3f(a1,b1,c1); // color de las ramas
				glLineWidth(Ramas[i].gord < MG ? Ramas[i].gord : MG); // engrosa las ramas
				glBegin(GL_LINE_STRIP); // pinta el segmento determinado por (x1,y1) y (x2,y2)
				x1 = Ramas[Ramas[i].pad].x; y1 = Ramas[Ramas[i].pad].y; // coordenadas del predecesor
				x2 = Ramas[i].x; y2 = Ramas[i].y; // coordenadas del punto de rama actual
				glVertex2f(x1,y1); glVertex2f(x2,y2);
				glEnd(); // fin de la rama
			}
		}

		// glPopMatrix(); // recupera la matriz guardada en la pila
		// glutSwapBuffers(); // para doble buffer
		glFlush();

		if(E == 1) captura(); // exporta la salida gráfica actual en tiempo tg

		if(NPA == 0 && NPAL == 0) (R == 1) ? regeneracion() : volcado(); // sin atractores, fin del programa
	}

	tg++; // incrementa el tiempo en número de iteraciones

	atraccion(); // actualiza el punto de rama más próximo a cada atractor
	ramificacion(); // crea los nuevos puntos de rama e induce el crecimiento
	saciador(); // aborta el crecimiento de las ramas una vez se ha efectuado
	if(S == 1 && CE > 0) surgencia(); // incorpora nuevos puntos de atracción
	movimiento(); // ejerce el movimiento aleatorio de los puntos de atracción

	if(tg == TF){ // detiene la ejecución del programa en el tiempo TF fijado
		printf("Alcanzado el límite de tiempo fijado (%d iteraciones).\n",TF);
		volcado(); // finaliza el programa
	}
}

//-------- Evolución ---------//

void atraccion(void){
	// mide las distancias y asocia a cada punto de atracción el punto de rama más próximo
	int i,j,jmin;
	float d,dmin;

	for(i=0; i<NPA; i++){
		dmin = 2*M; // inicializa la distancia mínima con un valor máximo
		punto_atrac A = Atractores.front(); // punto de atracción de la cabecera
		Atractores.pop(); // punto de atracción extraído de la cabecera
		for(j=1; j<PR-1; j++){
			if(Ramas[j].pad >= 0){ // existe punto de rama predecesor si > 0
				d = dis(A.x,A.y,Ramas[j].x,Ramas[j].y); // distancia
				dmin = (d != 0 && d < dmin) ? d : dmin; // distancia mínima
				jmin = (dmin == d) ? j : jmin; // índice de la rama más cercana
			}
		}
		if(dmin == 2*M){ // no hay ningún punto de rama próximo
			printf("Error al existir un punto de atracción sin rama próxima.\n");
			volcado(); // finaliza el programa
			exit(-1); // finalización anómala del programa
		}

		if(dmin > KILL){ // el punto de atracción sigue activo
			Atractores.push(A); // punto de atracción añadido a la cola
			if(dmin < MAX){ // el atractor ejerce influencia en el punto de rama
				A.rama = jmin; // asocia el atractor con su punto de rama más próximo
				Ramas[jmin].vx = Ramas[jmin].vx + (A.x - Ramas[jmin].x)/dmin; // vector normalizado
				Ramas[jmin].vy = Ramas[jmin].vy + (A.y - Ramas[jmin].y)/dmin; // vector normalizado
				Ramas[jmin].con++; // incremento del contador de crecimiento del punto de rama jmin
			}
		}
	}

	NPA = Atractores.size(); // actualiza el número de puntos de atracción activos
}

void ramificacion(void){
	// induce el crecimiento de las ramas en la dirección media de los atractores influyentes
	int i,j,nr;
	float n;

	for(i=PR-2; i>0; i--){
		if(Ramas[i].con > 0){ // tiene influencia de atractores, se crea un nuevo punto de rama
			nr = PR-1; // índice del nuevo punto de rama
			Ramas[nr].pad = i; // predecesor del nuevo punto de rama
			Ramas[i].nod++; // punta: nod = 0; punto de rama: nod = 1; nodo de ramificación: nod > 1
			Ramas[nr].edad = tg; // tiempo de creación
			Ramas[nr].gord = IG; // grosor inicial de la rama
			Ramas[nr].con = Ramas[nr].nod = 0;

			if(Ramas[i].nod > 1){ // el nuevo punto implica la creación de una nueva rama
				j = i;
				do{ // engrosa el tramo de rama predecesor
					Ramas[j].gord = sqrt(Ramas[j].gord*Ramas[j].gord + IG*IG);
					j = Ramas[j].pad;
				}while(j != 0);
			}

			Ramas[i].vx = Ramas[i].vx/Ramas[i].con; Ramas[i].vy = Ramas[i].vy/Ramas[i].con; // media...
			n = dis(0,0,Ramas[i].vx,Ramas[i].vy); // módulo del vector de crecimiento
			if(vel == 0 && n < 0.3){ // corrección del vector de crecimiento por ser casi nulo
				Ramas[i].vx = -(Ramas[i].y - Ramas[Ramas[i].pad].y); // nuevo vector de crecimiento
				Ramas[i].vy = Ramas[i].x - Ramas[Ramas[i].pad].x; // ortogonal a la rama de partida
				printf("Corrección de un vector de crecimiento por ser casi nulo (%f).\n",n);
				n = dis(0,0,Ramas[i].vx,Ramas[i].vy); // módulo del nuevo vector de crecimiento
			}
			Ramas[i].vx = Ramas[i].vx/n; Ramas[i].vy = Ramas[i].vy/n; // vector normalizado
			Ramas[nr].x = Ramas[i].x + CREC*Ramas[i].vx; Ramas[nr].y = Ramas[i].y + CREC*Ramas[i].vy;
			Ramas[nr].vx = Ramas[nr].vy = Ramas[nr].dx = Ramas[nr].dy = 0;

			fprintf(farbol, "%d; %d; %d; %d; %d\n", nr,(int)Ramas[nr].x,(int)Ramas[nr].y,tg,i);

			PR++; // aumenta el contador de puntos de rama
			cRamas = (punto_rama*)realloc(Ramas, sizeof(punto_rama)*PR); // amplía la reserva de memoria
			Ramas = cRamas;

			if(c1 == -1 && C > 0){ // recinto poligonal y activado el crecimiento
				hRamas[nr].x = Ramas[nr].x, hRamas[nr].y = Ramas[nr].y;
				cRamas = (punto_rama*)realloc(hRamas, sizeof(punto_rama)*PR); // amplía la reserva de memoria
				hRamas = cRamas;
			}
		}
	}
}

void saciador(void){
	// desactiva el crecimiento de las ramas, una vez que se ha producido una iteración
	int i;

	for(i=1; i<PR-1; i++) Ramas[i].con = Ramas[i].vx = Ramas[i].vy = 0;
}

void surgencia(void){
	// incorpora aleatoriamente nuevos puntos de atracción dentro de la branquia
	int c=0,t=50; // número de oportunidades para generar atractores
	float x,y,n; punto_atrac A;

	do{
		n = UNI; // número aleatorio en (0,1)
		if(n > P){
			x = JKISS()%M, y = JKISS()%M;
			bool elip = (x-c1)*(x-c1)/(a*a)+(y-c2)*(y-c2)/(b*b) <= 1;
			bool poli = dentropoli(x,y);
			if((c1 != -1 && elip && b < bb) || (c1 == -1 && poli && rhomo <1)){
				A.x = x; A.y = y;
				Atractores.push(A);
				NPA++;
			} // se generan atractores sólo si la branquia está en crecimiento
		}
		c++;
	}while(c < t);
}

void movimiento(void){
	// lleva a cabo el movimiento aleatorio de los puntos de atracción a posiciones vecinas
	int i,x,y,v=0;
	float Q = (1-P)*(2-sqrt(2))/4; // probabilidad de movimiento ortogonal

	while(v < vel){
		for(i=0; i<NPA; i++){
			punto_atrac A = Atractores.front();
			Atractores.pop();

			punto_atrac Aux = A;
			float aux = UNI; // aleatorio en (0,1)
			int mov = aux < Q ? 1 : (aux < 2*Q ? 2 : (aux < 3*Q ? 3 : (aux < 4*Q ? 4 : (aux < (4+sqrt(2)/2)*Q ?
			5 : (aux < (4+sqrt(2))*Q ? 6 : (aux < (4+3*sqrt(2)/2)*Q ? 7 : (aux < (4+2*sqrt(2))*Q ? 8 : 9)))))));

			switch(mov){
				case 1: x = 1; y = 0; break; // movimiento derecha
				case 2: x = 0; y = 1; break; // movimiento arriba
				case 3: x = -1; y = 0; break; // movimiento izquierda
				case 4: x = 0; y = -1; break; // movimiento abajo
				case 5: x = 1; y = 1; break; // diagonal superior-derecha
				case 6: x = -1; y = 1; break; // diagonal superior-izquierda
				case 7: x = -1; y = -1; break; // diagonal inferior-izquierda
				case 8: x = 1; y = -1; break; // diagonal inferior-derecha
				default: x = 0; y = 0; break; // no hay movimiento
			}

			A.x = A.x + x; A.y = A.y + y;
			// if(A.x <= M-1 && A.x >= 0 && A.y <= M-1 && A.y >= 0) Atractores.push(A);
			if(c1 != -1){ // elipse
				if((A.x-c1)*(A.x-c1)/(a*a)+(A.y-c2)*(A.y-c2)/(b*b) <= 1) Atractores.push(A);
				else Atractores.push(Aux);
			}else // polígono
				dentropoli(A.x,A.y) ? Atractores.push(A) : Atractores.push(Aux); // no está dentro si es par
		}
		v++;
	}
}

void crecimiento(void){
	// implementa el crecimiento en tamaño de la branquia, disponible para recinto elíptico y poligonal
	int i,cor,inc=1; // incremento de la elipse (semieje b) o del polígono (razón rhomo)
	float radio,tras,x,y,irh=(float)inc/1000,com=1-(float)C/100;

	if(tg == tr + 1){ // tiempo cero (tr = -1) o principio de la regeneración
		AtractoresLimbo = Atractores; // cola con los puntos de atracción pendientes de activar
		while(!Atractores.empty()) Atractores.pop(); // cola con los puntos de atracción activos
		if(tg == 0){
			if(c1 != -1 && C < b){ // recinto elíptico sin alcanzar su máximo tamaño
				aa = a; bb = b; ab = aa/bb; // tamaño máximo y razón de semiejes a/b
				b =	C; a = ceil(b*ab); c2 = c2-bb+b; // semiejes iniciales y altura del centro
			}else if(c1 == -1 && rhomo <= 1) // recinto poligonal sin alcanzar su máximo tamaño
				for(i=1; i<=NV; i++) if(yhomo > poligono[2*i-1]){
					yhomo = poligono[2*i-1], xhomo = poligono[2*(i-1)];
				} // calcula el vértice de menor altura
		}
	}

	if(tg%FC == 0){ // frecuencia con la que crece la branquia
		if(c1 != -1 && b < bb && tg != 0){ // recinto elíptico sin alcanzar su máximo tamaño
			b = b+inc; a = ceil(b*ab); c2 = c2+inc; // semiejes y altura del centro
			if(CE == 1) for(i=1; i<PR-1; i++){ // expansión de las ramas con la branquia
				Ramas[i].x += (Ramas[i].x-(float)c1)*(inc*(float)ab)/(2*(float)a);
				//Ramas[i].y += (Ramas[i].y-(float)c2+(float)b)*inc/(2*(float)b);
			}
		}else if(c1 == -1 && rhomo <= 1){ // recinto poligonal sin alcanzar su máximo tamaño
			rhomo = rhomo<0.998 ? rhomo+irh : 1; // razón de la homotecia
			tras = dis(0,rhomo*yhomo,0,yhomo); // traslación para la altura tras la homotecia
			for(i=1; i<=NV; i++){ // homotecia de razón rhomo y centro (xhomo,0)
				x = (float)hpoligono[2*(i-1)], y = (float)hpoligono[2*i-1];
				poligono[2*(i-1)] = rhomo*x + (1-rhomo)*(float)xhomo; // coordenada x
				poligono[2*i-1] = rhomo*y + tras; // coordenada y
			}
			if(CE == 1 && rhomo != 1) for(i=1; i<PR-1; i++){ // expansión de las ramas con la branquia
				cor = Ramas[i].edad/FC; // ajustado según la frecuencia de crecimiento de la branquia
				Ramas[i].x = (com+rhomo-cor*irh)*hRamas[i].x + (1-com-rhomo+cor*irh)*(float)xhomo;
				//Ramas[i].y = (com+rhomo-cor*irh)*hRamas[i].y;
			} // homotecia de razón rhomo corregida y completada y centro (xhomo,0)
		}
	}

	if(R == 2){ // crecimiento en regeneración
		ox = ox-inc*m/sqrt(m*m+1); oy = oy+inc/sqrt(m*m+1); radio = dis(ox,oy,p11,p12);
	} // centro y radio del arco

	NPAL = AtractoresLimbo.size(); // número de atractores pendientes de activar

	for(i=0; i<NPAL; i++){
		punto_atrac A = AtractoresLimbo.front();
		AtractoresLimbo.pop();
		if(R != 2){ // ¿dentro del polígono (c1 = -1) / elipse (c1 != -1) / circunferencia (R = 2)?
			if(c1 == -1) dentropoli(A.x,A.y) ? Atractores.push(A) : AtractoresLimbo.push(A);
			else (A.x-c1)*(A.x-c1)/(a*a)+(A.y-c2)*(A.y-c2)/(b*b) <= 1 ? Atractores.push(A) : AtractoresLimbo.push(A);
		}else (A.x-ox)*(A.x-ox)+(A.y-oy)*(A.y-oy) < radio*radio ? Atractores.push(A) : AtractoresLimbo.push(A);
	}

	NPA = Atractores.size(); // número de atractores activos
}

void regeneracion(void){
	// destruye y regenera parte de la estructura ramificada
	float d; int i,j,inc=1; // incremento del radio de la circunferencia activadora
	R = 2; tr = tg; // tiempo en el que se produce el corte de la estructura

	printf("Regeneración a partir de la iteración %d.\n",tr+1);

	fprintf(faleator, "\nRegeneración:\ntr = %d\ny = %fx + %f\n", tr,m,n);
	fclose(faleator); // cierra el archivo "MAX_KILL_CREC_vel_aleator.dat"

	for(i=1; i<PR-1; i++){ // elimina la parte de la estructura ramificada que se ha cortado
		if(Ramas[i].y - m*Ramas[i].x - n > 0){
			Ramas[i].pad = -1; // "anula" los puntos que quedan por encima del corte
			Ramas[i].x = Ramas[i].y = Ramas[i].dx = Ramas[i].dy = Ramas[i].con = 0;
			Ramas[i].vx = Ramas[i].vy = Ramas[i].edad = Ramas[i].gord = Ramas[i].nod = 0;
		}else{
			j = i;
			do{ // comprueba si la estructura restante es inconexa
				j = Ramas[j].pad;
				if(j == -1){ // el antecesor más antiguo está "anulado"
					j = i;
					do{ // elimina las estructuras ramificadas inconexas con los puntos de rama semilla
						Ramas[j].pad = -1; // "anula" los puntos no conexos con la raíz de las ramas
						Ramas[j].x = Ramas[j].y = Ramas[j].dx = Ramas[j].dy = Ramas[j].con = 0;
						Ramas[j].vx = Ramas[j].vy = Ramas[j].edad = Ramas[j].gord = Ramas[j].nod = 0;
						j = Ramas[j].pad;
					}while(j != -1);
				}
			}while(j != 0 && j != -1);
		}
	}

	if(c1 != -1){ // calcula los puntos de intersección de la recta con la elipse (p1 y p2)
		float fa=(float)a, fb=(float)b, fc1=(float)c1, fc2=(float)c2, eca=fb*fb+fa*fa*m*m, ecb=2*n*m*fa-2*m*fa*fc2+2*m*m*fc1*fa;
		float ecc=n*n+m*m*fc1*fc1+2*n*m*fc1+fc2*fc2-2*n*fc2-2*m*fc1*fc2-fb*fb, ecd=sqrt(ecb*ecb-4*eca*ecc);
		float cos1 = (-ecb+ecd)/(2*eca), cos2 = (-ecb-ecd)/(2*eca);
		p11 = fc1+fa*cos1; p12 = m*p11+n; p21 = fc1+fa*cos2; p22 = m*p21+n;
	}else{ // calcula los puntos de intersección de la recta con el polígono (p1 y p2)
		float x1,y1,x2,y2,mr,nr,p01=0,p02=0; p11=0,p12=0,p21=0,p22=0;
		for(i=1; i<NV; i++){ // corte con cada lado del polígono
			x1 = (float)poligono[2*(i-1)], y1 = (float)poligono[2*i-1];
			x2 = (float)poligono[(2*i)%(2*NV)], y2 = (float)poligono[(2*i+1)%(2*NV)];
			if(x1 != x2){ // no es un segmento de recta vertical
				mr = (y1-y2)/(x1-x2); nr = y1-mr*x1; // ecuación del segmento
				m != mr ? (p01 = (nr-n)/(m-mr), p02 = m*p01+n) : p01 = 0;
			}else{ p01 = x1; p02 = m*x1+n;} // segmento vertical
			if(p02 > max(y1,y2) || p02 < min(y1,y2) || p01 < 0 || p01 > M) p01 = p02 = 0;
			(p01 != 0 || p02 != 0) ? (p11 == 0 && p12 == 0) ? (p11=p01, p12=p02) : (p21=p01, p22=p02, i=NV) : p01=0;
		}
		if(p11 < p21){ p01 = p11, p11 = p21, p21 = p01, p02 = p12, p12 = p22, p22 = p02;}
	}

	d = dis(p11,p12,p21,p22); // distancia entre p1 y p2
	ox = (p11+p21)/2+(d+inc)*m/sqrt(m*m+1); oy = (p12+p22)/2-(d+inc)/sqrt(m*m+1); // centro del arco activador

	inicio(); // reinicializa el programa con nuevos puntos atractores (grabados en atracreg.dat)

	if(NPA == 0) volcado(); // no hay atractores, finaliza el programa
}

//------ JKISS 64 bits -------//

unsigned int JKISS(){
	// generador de números aleatorios a partir de las semillas xk, yk, zk, ck
	unsigned long long tk;

	xk = 314527869*xk + 1234567;
	yk ^= yk << 5; yk ^= yk >> 7; yk ^= yk << 22;
	tk = 429458439ULL*zk + ck;
	ck = (tk >> 32);
	zk = tk;

	return(xk + yk + zk);
}

bool dentropoli(float Ax,float Ay){
	// comprueba si un punto pertenece al interior del recinto poligonal o no
	int k, dentro = 0, j = NV-1;
	for(k=1; k<NV; k++){
		if((poligono[2*k-1] < Ay && poligono[2*j-1] >= Ay) || (poligono[2*j-1] < Ay && poligono[2*k-1] >= Ay))
			if(poligono[2*(k-1)]+(Ay-poligono[2*k-1])/(poligono[2*j-1]-poligono[2*k-1])*(poligono[2*(j-1)]-poligono[2*(k-1)]) < Ax)
				dentro++; // corta al lado del polígono
		j = k;
	}
	return dentro%2 == 1; // no está dentro si es par
}

//------- Finalización -------//

void volcado(void){
	// finaliza la ejecución del programa guardando la salida gráfica y los puntos de rama
	E = 0; captura(); // exporta la salida gráfica final

	fclose(farbol); // cierra el archivo "MAX_KILL_CREC_vel_arbol.csv"

	system("pause");
	glutDestroyWindow(window); // cierra la ventana generada
	free(Ramas); free(poligono); // libera la reserva de memoria
	printf("Finalización del programa en %d iteraciones.\n",tg);
	exit(0); // finalización normal del programa
}

void captura(void){
	// exporta la salida gráfica del programa en ejecución como archivo de imagen
	int i=0,j=0,sx=0,sy=0,w=M,h=M; // ancho y alto de la pantalla gráfica
	float T=TF,t=tg;
	char out[3*w*h]; // reserva espacio para el fichero gráfico

	stringstream stg; // cambio de tipo de datos del entero tg
	string ctg,ceros; // cadena de caracteres de tg y de ceros
	stg << tg; stg >> ctg; // desde tipo entero a tipo string

	while(T >= 1){T = T/10; i++;} // i = número de caracteres de TF
	while(t >= 1 || j == 0){t = t/10; j++;} // j = caracteres de tg
	while(i-j > 0){ceros = ceros + "0"; j++;} // ceros = i-j ceros

	fnombre = "grafico.pnm"; // fichero de salida con la imagen de la ventana gráfica generada
	ffichero = cMAX + "_" + cKILL + "_" + cCREC + "_" + cvel + "_" + fnombre;
	ffichero = E == 0 ? ffichero : "video\\" + ceros + ctg + ".pnm"; // se guarda en SCA/video
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	FILE *fgraf = fopen(farchivo,"wb"); delete[] farchivo;

	if(!fgraf) return;

	glPixelStorei(GL_PACK_ALIGNMENT,1); // salida alineada de bytes
	glReadPixels(sx,sy,w,h,GL_RGB,GL_UNSIGNED_BYTE,out); // comando que obtiene la imagen

	fprintf(fgraf,"P6\n%d %d\n255\n",w,h); // cabecera del fichero "MAX_KILL_CREC_vel_grafico.pnm"
	for(int y=0; y<h; y++) fwrite(&out[3*(h-1-y)*w],1,3*w,fgraf); // voltea la imagen de abajo a arriba

	fclose(fgraf); // cierra el archivo "MAX_KILL_CREC_vel_grafico.pnm"
}
