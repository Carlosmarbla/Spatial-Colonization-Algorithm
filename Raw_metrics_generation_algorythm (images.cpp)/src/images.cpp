/*
 ============================================================================
 Name        : images.cpp
 Author      : Curro Jiménez & A. Ruiz
 Version     : 4.0 -- Procesamiento de muestras y simulaciones gráficas
               Binarización y exportación de diferentes métricas
 Copyright   : UPO & Casares Lab & Curro
 Description : Extracción y tratamiento de datos de ficheros gráficos
               generados por SCA.cpp y muestras biológicas de Cloeon
 ============================================================================
*/

/*
 Inputs:
  1) Imagen en formato .pnm, RGB color y esqueletonizada (sin nudos >= 2x2 píxeles
     ni bucles, y la raíz en la parte más inferior). Con el contorno convexo de la
     branquia (será suprimido automáticamente para estimar la dimensión fractal)
*/
/*
 Outputs:
  1) El fichero "conteoramas.csv" almacena métricas y datos extraídos de la imagen
  2) El fichero "genearamas.csv" añade como métrica los ascendentes de cada rama
  3) El fichero "info.txt" registra la información del procesamiento de la imagen
  4) Salida gráfica por ventana, grabada en "graficocompu.pnm" y "graficonodos.pnm"
*/

/*
 Compilación:
 Mismas configuraciones que en SCA.cpp
 Considerar modificar las "User options" para sondear las diferentes opciones
*/

//-------- Librerías ---------//

# include <windows.h> // para MS Windows
# include <math.h>
# include <stdio.h> // uso de ficheros FILE
# include <stdlib.h>
# include <GL/glut.h> // librería gráfica
//# include <cstdlib> // C++
# include <iostream> // C++
# include <string> // C++ tipo entero a string
# include <sstream> // C++ tipo entero a string
# include <cstring> // C++ tipo entero a string
# include <fstream>
# include <conio.h> // C++ sleep para pausas
# include <queue> // C++ colas y sus funciones
# include <stack> // C++ pilas y sus funciones

using namespace std; // C++

//--- Opciones del usuario ---//

//int M = 512; // tamaño de la red cuadrada (ventana gráfica M*M)
std::string fruta = "C:\\Users\\tomas\\Desktop\\CABD\\Proyectos\\Proyecto 3, métricas Antonio\\Proyecto2\\Proyecto\\Obtención de métricas (images.cpp)\\Cloeon dipterum\\Experimentos Regeneración\\Regenerado control y regenerado antagonista FGF\\FiguraD";
std::string fprefijo = "FiguraD_4_gse"; //"7_L_M_se"; //"85_15_2_1";
std::string fnombre = ".pnm"; //".pnm";
int N = 1; // conteo de ramas en las filas múltiplo de N
int S = 0; // registra desde la fila cero (S = 1) / sólo en presencia de branquia (S = 0)
int T = 0; // computa sólo cambios de color (T = 1) / todos los píxeles de rama (T = 0) [grosor]
int mili = 100; // tiempo de espera entre cada cómputo de rama, en milisegundos

//---- Resto de variables ----//

int window; // ventana creada para la salida gráfica
int ancho, alto, tam, color; // datos de la imagen cargada
ifstream imagen; // tipo para lectura de datos
std::string ffichero; // nombre y formato de los ficheros de salida del programa
FILE *finfo; // fichero de información del procesamiento de la imagen
char *farchivo; // cadena de caracteres del nombre completo de los archivos de salida del programa
char p[5]; // palabra mágica del formato (.pnm -> P6)
queue<int*> Ramas; // cola que albergará el cómputo de filas, rama y branquia
queue<int*> Nodpun; // cola que albergará el número de nodos de ramificación y de puntas de ramas
queue<int*> GenRam; // cola que albergará la generación y longitud estimada de cada rama
queue<int> pica; // cola de enteros para almacenar el código binario de la imagen
const int e = 9; // máxima precisión para la estimación de la dimensión fractal
float* dfrac[e]; // guarda los pares de valores que estimarán la dimensión fractal (hasta 2^e)
float dimfrac; // variable que almacena la estimación del valor de la dimensión fractal

//---- Programa principal ----//

void carga(void); // abre el archivo de imagen, detecta su formato y sus dimensiones
void procesa(void); // codifica la imagen, la analiza y obtiene los datos de interés
void dimension(void); // estima la dimensión fractal de la imagen
void nodos(void); // computa los puntos de ramificación y puntas de rama existentes
void captura(void); // exporta la salida gráfica como archivo de imagen
void volcado(void); // finaliza el programa guardando la salida gráfica y el cómputo de ramas

int main(int argc,char **argv){

	carga(); // carga la imagen indicada

	glutInit(&argc,argv); // inicializa la librería GLUT
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA); // se usa un único buffer
	glutInitWindowPosition(5,5); // posición de la ventana (desde la esquina superior izquierda)
	glutInitWindowSize(ancho,alto); // tamaño de la ventana ancho*alto

	window = glutCreateWindow("Imagen procesada"); // crea la ventana con el nombre dado

	gluOrtho2D(0,ancho,0,alto); // establece el plano de visualización

	glClearColor(0,0,0,0); // fija el color de fondo a negro y opaco
	glClear(GL_COLOR_BUFFER_BIT); // borra la pantalla

	procesa(); // procesa la imagen cargada
	glFlush();

	Sleep(1000); // pausa de 3 segundos

	dimension(); // estima la dimensión fractal
//	glFlush();
	captura(); // exporta la salida gráfica

	Sleep(1000); // pausa de 3 segundos

	nodos(); // detecta puntos de ramificación
//	glFlush();
	captura(); // exporta la salida gráfica

	volcado(); // vuelca los datos en los ficheros de salida

	return 0; // finalización normal del programa
}

void carga(void){
	// abre el archivo de imagen, detecta su formato y sus dimensiones
	ffichero = fruta + fprefijo + "_info.txt"; // fichero de salida
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	finfo = fopen(farchivo,"w"); delete[] farchivo; // registra la info del proceso

	ffichero = fruta + fprefijo + fnombre;
	const char *ruta = ffichero.c_str(); // directorio y nombre de la imagen

	cout << "Control de ruta: " << ruta << endl;
	fprintf(finfo, "Control de ruta: %s\n", ruta);

	imagen.open(ruta); // ios::binary // abre la imagen de la ruta indicada

	if(!imagen){ // no se encuentra el archivo indicado en la ruta dada
		cout << "Error de lectura del archivo de imagen." << endl;
		fprintf(finfo, "Error de lectura del archivo de imagen.\n");
		imagen.close(); // cierra la imagen
		exit(-1); // finalización anómala del programa
	}

	char aux[100]; // variable auxiliar para leer los datos de la imagen

	imagen >> p; // almacena la palabra mágica (formato)
	imagen >> aux; // lee la siguiente palabra

	if(aux[0] == '#'){ // si el primer carácter es # (línea de comentario)
		imagen.getline(aux,100); // captura la línea para saltarla
		imagen >> aux; // lee la siguiente palabra
	}

	ancho = atoi(aux); // almacena el ancho en un tipo entero
	imagen >> aux; // lee la siguiente palabra
	alto = atoi(aux); // almacena el alto en un tipo entero
	imagen >> aux; // lee la siguiente palabra
	color = atoi(aux); // almacena el número de colores en un tipo entero
}

void procesa(void){
	// codifica la imagen, la analiza y obtiene los datos de interés
	tam=ancho*alto;
	int x,y,lon=tam*3; float R,G,B;
	char img[lon]; // array de caracteres para almacenar el código RGB de la imagen
	int pic[tam]; // array de enteros para almacenar el código binario de la imagen

	imagen.read(img, lon); // almacena la imagen en el array de caracteres
	imagen.close(); // cierra la imagen

	cout << "Imagen: pm = " << p << "; ancho = " << ancho << "; alto = " << alto << "; colores = " << color << endl;
	fprintf(finfo, "\nImagen: pm = %s; ancho = %d; alto = %d; colores = %d\n", p,ancho,alto,color);

	img[lon]=0;
	int i = lon, ind = tam-1, nramas = 0, largot = 0; // acumulador de ramas y de branquia
	for(y=0; y<alto; y++){ // codificación de color RGB
		int ini = 1, pri=0, fin=0, largo = 0, mramas = 0, tramas = 0; // contador de ramas detectadas
		for(x=1; x<=ancho; x++){ // va de img[lon-1] a img[0] => se pintarán (ancho-x,y)
			R = (int)img[i-2]; G = (int)img[i-1]; B = (int)img[i]; // 0,-1
			(R || G || B != 0) ? R = G = B = color, pic[ind] = 1 : pic[ind] = 0; // RGB = 0,255 (-1=255)

			if(y%N == 0 && ind < tam){ // computa esta fila
				if(pic[ind] != pic[ind+1]){
					mramas++; // cambio de color
					if(mramas == 2){ pri = ind+1; ini = x;} // coordenada de inicio de branquia
					largo = x - ini; // ancho de branquia en la altura y
				}
				if(pic[ind] == 1) tramas++; // píxel blanco (rama o contorno de branquia)
				if(R == 0){ R = 255; G = 0; B = 0;} // dibuja la línea roja
			}

		//	if(S+largo != 0){
				glColor3f(R,G,B); // color para dibujar
				glPointSize(1); // tamaño del punto
				glBegin(GL_POINTS); // inicio del dibujo de puntos
				glVertex2f(ancho-x,y); // pinta las coordenadas (x,y) evitando el reflejo
		//	}

			ind--; i = i-3;
		}

		fin = pri-largo; // coordenada de final de branquia en esa fila

		while(pic[pri] == 1){ pic[pri] = 0; tramas--; pri++;} // elimina el contorno de la branquia
		while(pic[fin] == 1){ pic[fin] = 0; tramas--; fin++;} // elimina el contorno de la branquia

		if(y%N == 0 && ind < tam){
			mramas = largo > 0 ? mramas - 4 : mramas; // resta los bordes de la branquia
			nramas += T==0 ? tramas : mramas/2; // acumula las ramas detectadas en la fila
			largot += largo; // acumula los anchos de la branquia
			int *ra = (int*)malloc(5*sizeof(int)); // reserva memoria para cinco enteros
			ra[0] = y; ra[1] = T==0 ? tramas : mramas/2; ra[2] = nramas; ra[3] = largo; ra[4] = largot;
			Ramas.push(ra); // agrega a la cola: fila, cantidad de rama y ancho de branquia
		}
	}

	glEnd(); // fin del dibujo de puntos

	const char *resultado = (i == 0) ? "correctamente." : "erróneamente.";
	cout << "Impresión de la imagen realizada " << resultado << endl;
	fprintf(finfo, "Impresión de la imagen realizada %s\n", resultado);

	for(ind=tam-1; ind>=0; ind--) pic[ind] == 1 ? pica.push(1) : pica.push(0); // graba en la cola el array sin contorno de branquia
}

void dimension(){
	// estima la dimensión fractal de la imagen
	int pic[tam]; // array de enteros para almacenar el código binario de la imagen
	int ind = tam-1;

	while(ind >= 0){ // !pica.empty()
		int bi = pica.front(); // elemento actual en la cabecera de la cola
		pic[ind] = bi; // pasa los elementos de la cola al array
		pica.pop(); // extrae el elemento de la cabecera de la cola
		pica.push(bi); // introduce el elemento de nuevo en la cola
		ind--;
	}

	int x,y,R,G,B; ind = tam-1; // dibuja la estructura sin branquia
	for(y=0; y<alto; y++){ // codificación de color binario a RGB
		for(x=1; x<=ancho; x++){ // va de pic[tam-1] a pic[0] => se pintarán (ancho-x,y)
			pic[ind] == 1 ? R = G = B = 1 : R = G = B = 0;
			glColor3f(R,G,B); // color para dibujar
			glPointSize(1); // tamaño del punto
			glBegin(GL_POINTS); // inicio del dibujo de puntos
			glVertex2f(ancho-x,y); // pinta las coordenadas (x,y) evitando el reflejo
			ind--;
		}
	}

	glEnd(); // fin del dibujo de puntos

	if(ancho == alto){ // estimación de la dimensión fractal
		float r=1, logirm=0, logdfm=0, lognum=0, logden=0; int k;
		for(k=0; k<e; k++){
			r = r/2;
			float *logs = (float*)malloc(2*sizeof(float)); // reserva memoria para dos reales
			int M=ancho,c=M*r,nc=1/(r*r),j,m,n,o=0; // tamaño (c) y número (nc) de cuadrados
			// cout << "Test: " << nc << endl;
			int df = 0; // número de cuadrados con al menos un píxel blanco

			if(k==8){ // cada píxel es un cuadrado
				for(j=tam-1; j>-1; j--) if(pic[j]!=0) df+=1;
			}else{
				int pixb[nc]; // array para contabilizar píxeles blancos en cada cuadrado
				ind = tam-1;

				while(ind > 0){ // cuenta el número de píxeles blancos de cada cuadrado
					pixb[o]=0;
					for(m=0; m<c; m++) for(n=0; n<c; n++) pixb[o] += pic[ind-n-M*m];
					o++; ind = ind-c;
					if((ind+1-(c-1)*M)%(c*M) == 0) ind = ind-M*(c-1); // evita recontar píxeles
				}

				for(j=nc-1; j>-1; j--) if(pixb[j]!=0) df+=1; // cuadrados con píxeles blancos
			}

			float logir = log(1/r), logdf = log(df);
			logs[0] = logir; logs[1] = logdf;
			dfrac[k] = logs;
		}

		for(k=0; k<e; k++){
			logirm = logirm + dfrac[k][0]/e; // media de los logir
			logdfm = logdfm + dfrac[k][1]/e; // media de los logdf
		}
		for(k=0; k<e; k++){
			lognum = lognum + (dfrac[k][0]-logirm)*(dfrac[k][1]-logdfm);
			logden = logden + (dfrac[k][0]-logirm)*(dfrac[k][0]-logirm);
		}
		dimfrac = lognum/logden; // pendiente de la recta de regresión

	}else{
		cout << "La imagen debe ser cuadrada para estimar la dimensión fractal." << endl;
		fprintf(finfo, "\nLa imagen debe ser cuadrada para estimar la dimensión fractal.\n");
	}
}

void nodos(){
	// computa los puntos de ramificación y puntas de ramas existentes
	int pic[tam]; // array de enteros para almacenar el código binario de la imagen
	int ind = tam-1;
	int x,y,R,G,B;

	while(ind >= 0){ // !pica.empty()
		int bi = pica.front(); // elemento actual en la cabecera de la cola
		pic[ind] = bi; // pasa los elementos de la cola al array
		pica.pop(); // extrae el elemento de la cabecera de la cola
		pica.push(bi); // introduce el elemento de nuevo en la cola
		ind--;
	}

	int i = tam-1, raiz = 0, d = 3; // distancia mínima entre distintos nodos de ramificación
	int aux, a, b, tnodos = 0, tpuntas = 0; // acumuladores de nodos y de puntas de ramas
	for(y=0; y<alto; y++){ // codificación de color RGB
		int mnodos = 0, mpuntas = 0; // contador de puntos de ramificación y de puntas de ramas
		for(x=1; x<=ancho; x++){ // va de pic[tam-1] a pic[0] => se pintarán (ancho-x,y)

			if(y%N == 0 && i < tam && pic[i] == 1){ // computa esta fila (píxeles blancos)

				// detecta los puntos de ramificación
				aux = i-1-ancho, a = -1, b = -1;
				int camcol = 0, nodcer = 0; // cambios de color y presencia de nodos cercanos
				for(int j=0; j<8; j++){
					j<2 ? b++ : j<4 ? a++ : j<6 ? b-- : a--; // recorre las posiciones vecinas a i
					if(i < tam-ancho && i > ancho && i%ancho >= 1 && i%ancho < ancho-1)
						camcol += pic[aux]!=pic[i+a+b*ancho]; // cambios de color sin desbordar la ventana
					aux = i+a+b*ancho; // vecinos recorridos en sentido antihorario
				}
				a = 0, b = 0; // comprueba la presencia de nodos cercanos a distancia d
				while(a*a+b*b <= d*d && nodcer == 0){
					while(a*a+b*b <= d*d && nodcer == 0){
						for(int j=-1; j<=1; j+=2) // recorre el entorno sin desbordar la ventana
							if(i < tam-d*ancho && i > d*ancho && i%ancho >= d && i%ancho < ancho-d)
								if(pic[i+j*a+j*b*ancho] == 2 || pic[i+j*a-j*b*ancho] == 2) nodcer = 1;
						b++;
					}
					a++; b = 0;
				}

				// representa los nodos de ramificación
				if(camcol > 4 && nodcer == 0){
					pic[i] = 2; // nodo de ramificación
					mnodos++; R=0,G=1,B=0; // computa el nodo y lo dibuja
					glColor3f(R,G,B); // color para dibujar
					glPointSize(3); // tamaño del punto
					glBegin(GL_POINTS); // inicio del dibujo de puntos
					glVertex2f(ancho-x,y); // pinta las coordenadas (x,y) evitando el reflejo
				}

				// detecta las puntas de las ramas
				camcol = 0, aux = i-1-ancho, a = -1, b = -1;
				for(int j=0; j<8; j++){
					j<2 ? b++ : j<4 ? a++ : j<6 ? b-- : a--; // recorre las posiciones vecinas a i
					if(i < tam-ancho && i > ancho && i%ancho >= 1 && i%ancho < ancho-1)
						camcol += pic[aux]!=pic[i+a+b*ancho]; // cambios de color sin desbordar la ventana
					aux = i+a+b*ancho; // vecinos recorridos en sentido antihorario
				}

				// representa las puntas de las ramas
				if(camcol == 2){
					pic[i] = 3; // punta de rama
					if(raiz != 0){ // evita representar la raíz
						mpuntas++; R=0,G=0,B=1; // computa la punta y la dibuja
						glColor3f(R,G,B); // color para dibujar
						glPointSize(3); // tamaño del punto
						glBegin(GL_POINTS); // inicio del dibujo de puntos
						glVertex2f(ancho-x,y); // pinta las coordenadas (x,y) evitando el reflejo
					}else raiz = i;
				}
			}
			i--;
		}

		if(y%N == 0 && i < tam){
			tnodos += mnodos; // número total de puntos de ramificación
			tpuntas += mpuntas; // número total de puntas de ramas
			int *np = (int*)malloc(4*sizeof(int)); // reserva memoria para cuatro enteros
			np[0] = mnodos; np[1] = tnodos; np[2] = mpuntas; np[3] = tpuntas;
			Nodpun.push(np); // agrega a la cola: número de nodos de ramificación y de puntas
		}
	}

	glEnd(); // fin del dibujo de puntos

	cout << "Detectados " << tnodos << " puntos de ramificación." << endl;
	fprintf(finfo, "\nDetectados %d puntos de ramificación.", tnodos);
	cout << "Detectadas " << tpuntas << " puntas de ramas." << endl;
	fprintf(finfo, "\nDetectadas %d puntas de ramas.", tpuntas);

	// calcula la generación de cada punta de rama
	int inpu=1, inno=0, indnod[tnodos], para=0; // índices de los nodos detectados
	for(int t=0; t<tam; t++) if(pic[t] == 2){ indnod[inno] = t; inno++;}

	int v=0, n=0, c=0, l=0; // vueltas, nodo vecino, contador de vecinos, longitud de rama
	pic[raiz] = 1, i = raiz, aux = i-1-ancho, a = -1, b = -1;
	stack<int> Nudos; // pila que almacena todos los nodos existentes entre raíz y punta

	label: int j=0; while(inpu <= tpuntas && j < 8 && para < 13){
		// debe llegar a todas las puntas; si no, se aplica el criterio de parada Nudos.size() != 0
		j<2 ? b++ : j<4 ? a++ : j<6 ? b-- : a--; // recorre las posiciones vecinas a i
		if(i < tam-ancho && i > ancho && i%ancho >= 1 && i%ancho < ancho-1){
			n = pic[aux]==2 ? aux : n; // índice donde hay un vecino nodo
			if(pic[aux] != 0){
				if(v == 1){
					pic[i] = c>1 ? 2 : pic[i]; // más de un vecino = nudo != nodo = cambios de color
					switch(pic[i]){
						case 2: R=1,G=0,B=1; Nudos.push(i); break; // nodo o nudo
						case 3: R=0,G=0,B=1; break; // punta de rama
						default: R=1,G=0,B=0; break; // simple rama
					}
					if(pic[i] != 3) pic[i] = 0;
					x = i%ancho; y = i/ancho; // coordenadas
					i = n!=0 ? n : aux, j = -1, a = -1, b = -1, v = 0, n = 0, c = 0, l++;

					// representa las ramas recorridas
					glColor3f(R,G,B); // color para dibujar
					glPointSize(1); // tamaño del punto
					glBegin(GL_POINTS); // inicio del dibujo de puntos
					glVertex2f(x,alto-1-y); // pinta las coordenadas (x,y) evitando el reflejo
					glEnd(); // fin del dibujo de puntos
				}else c++; // (v = 0) cuenta el número de vecinos
			}
		}
		aux = i+a+b*ancho, j++; // vecinos recorridos en sentido antihorario

		if(j == 8){
			if(v == 1){
				if(pic[i] == 3){ // hallada una punta, de vuelta al nodo inmediatamente anterior
					Sleep(mili); // pausa la ejecución
					glFlush(); // refresca la salida gráfica
					int gene = 0, innu = 0, nudo, aa, bb; // cuenta los nodos entre raíz y punta de rama
					queue<int> NodCon,iNudos; stack<int> cNudos = Nudos; // NodCon: nodos contabilizados
					while(!cNudos.empty()){ nudo=cNudos.top(); cNudos.pop(); pic[nudo]=2; iNudos.push(nudo);}

					while(innu < (int)iNudos.size()){ // recuento de los nodos para esta punta
						nudo = iNudos.front(); // elemento actual en la cabecera de la cola
						iNudos.pop(); // extrae el elemento de la cabecera de la cola
						aa = 0, bb = 0; // comprueba la presencia de nodos cercanos a distancia d
						while(aa*aa+bb*bb <= d*d){
							while(aa*aa+bb*bb <= d*d){
								for(int jj=-1; jj<=1; jj+=2) // recorre el entorno sin desbordar la ventana
									if(nudo < tam-d*ancho && nudo > d*ancho && nudo%ancho >= d && nudo%ancho < ancho-d)
										for(int t=0; t<tnodos; t++)
											if(indnod[t] == nudo+jj*aa+jj*bb*ancho || indnod[t] == nudo+jj*aa-jj*bb*ancho){
												int incon = NodCon.size(); // número de nodos ya contabilizados
												while(incon >= 1){ // evita acumular dos veces el mismo nodo
													int noco = NodCon.front(); NodCon.pop(); NodCon.push(noco);
													if(noco == indnod[t]) goto label2;
													incon--;
												}
												if(incon == 0){ NodCon.push(indnod[t]); gene++;} // acumula un nuevo nodo
											}
								bb++;
							}
							aa++; bb = 0;
						}
						label2:	iNudos.push(nudo); // introduce el elemento de nuevo en la cola
						innu++;
					}

					int *gr = (int*)malloc(3*sizeof(int)); // reserva memoria para tres enteros
					gr[0] = inpu; gr[1] = gene; gr[2] = l;
					GenRam.push(gr); // agrega a la cola: número, generación y longitud de la rama
					inpu++; l=0; // longitud de rama a cero
				}
				pic[i] = 0, v = 0, n = 0, c = 0;
				i = Nudos.top(); Nudos.pop(); // por si se queda sin salida sin ser punta (que puede pasar)
			}else v = 1; // (v = 0) completada una vuelta
			goto label;
		}
		if(Nudos.size() == 0 && inpu > 1) para++; // criterio de parada si no alcanza todas las puntas
	}
	cout << "Eficiencia de alcance: " << inpu-1 << "/" << tpuntas << "." << endl;
	fprintf(finfo, "\nEficiencia de alcance: %d/%d.\n", inpu-1,tpuntas);
}

void volcado(void){
	// finaliza el programa guardando la salida gráfica y el cómputo de ramas
	int ys=0,f,r,rt,b,bt; // fila, cantidad de rama y ancho de branquia
	int n,nt,p,pt; // número de nodos de ramificación y de puntas
	float rfa,rfi,rba,rbi; // ratios rama/fila y rama/branquia acumulado e incremental
	float nfa,nfi,nba,nbi; // ratios nodos/fila y nodos/branquia acumulado e incremental
	float pfa,pfi,pba,pbi; // ratios puntas/fila y puntas/branquia acumulado e incremental

	ffichero = fruta + fprefijo + "_conteoramas" + (S==0 ? "_sincro.csv" : ".csv"); // fichero de salida
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	FILE *framas = fopen(farchivo,"w"); delete[] farchivo; // cabecera del fichero "conteoramas.csv"
	fprintf(framas, "Fila; Branquia; Rama; Rama/Fila; iRama/iFila; Rama/Branquia; iRama/iBranquia; Nodos; Nodos/Fila; iNodos/iFila; Nodos/Branquia; iNodos/iBranquia; Puntas; Puntas/Fila; iPuntas/iFila; Puntas/Branquia; iPuntas/iBranquia; log 1/r; log N(r)\n");

	while(!Ramas.empty()){
		int* ra = (int*)Ramas.front(); // elemento actual en la cabecera de la cola
		int* np = (int*)Nodpun.front(); // elemento actual en la cabecera de la cola
		f = ys, r = ra[1], rt = ra[2], b = ra[3], bt = ra[4]; n = np[0], nt = np[1], p = np[2], pt = np[3];
		rfa = f!=0 ? (float)rt/f : 0, rfi = (float)r/N, rba = bt!=0 ? (float)rt/bt : 0, rbi = b!=0 ? (float)r/b : 0;
		nfa = f!=0 ? (float)nt/f : 0, nfi = (float)n/N, nba = bt!=0 ? (float)nt/bt : 0, nbi = b!=0 ? (float)n/b : 0;
		pfa = f!=0 ? (float)pt/f : 0, pfi = (float)p/N, pba = bt!=0 ? (float)pt/bt : 0, pbi = b!=0 ? (float)p/b : 0;

		if(S + b != 0){ // computa todas las filas o sólo en presencia de branquia
			fprintf(framas, "%d;%d;%d;%f;%f;%f;%f;%d;%f;%f;%f;%f;%d;%f;%f;%f;%f;%f;%f\n", f,bt,rt,rfa,rfi,rba,rbi,nt,nfa,nfi,nba,nbi,pt,pfa,pfi,pba,pbi,ys<e?dfrac[ys][0]:0,ys<e?dfrac[ys][1]:dimfrac);
			ys++; // graba los datos en el fichero de exportación
		}
		Ramas.pop(); Nodpun.pop(); // extrae los elementos de las cabeceras de las colas
	}

	fclose(framas); // cierra el archivo "conteoramas.csv"

	int num,gene,lonr; // número, generación y longitud de la rama
	float medi=0,vari=0; // media y varianza del número de generación

	ffichero = fruta + fprefijo + "_genearamas" + (S==0 ? "_sincro.csv" : ".csv"); // fichero de salida
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	FILE *fgenea = fopen(farchivo,"w"); delete[] farchivo; // cabecera del fichero "genearamas.csv"
	fprintf(fgenea, "Núm. Rama; Generación; Longitud; Media Gen.; Varianza Gen.\n");

	while(!GenRam.empty()){
		int* gr = (int*)GenRam.front(); // elemento actual en la cabecera de la cola
		num = gr[0], gene = gr[1], lonr = gr[2]; medi += gene; vari += gene*gene;
		fprintf(fgenea, "%d;%d;%d;;\n", num,gene,lonr); // graba los datos en el fichero
		GenRam.pop(); // extrae los elementos de la cabecera de la cola
	}

	medi = medi/num; vari = vari/num-medi*medi; //(cvari/num-medi*medi)*num/(num-1);
	fprintf(fgenea, ";;;%f;%f\n", medi,vari); // graba los datos en el fichero

	fclose(fgenea); // cierra el archivo "genearamas.csv"

	cout << "Dimensión fractal estimada: " << dimfrac << "." << endl;
	fprintf(finfo, "\nDimensión fractal estimada: %f.\n", dimfrac);
	cout << "Cantidad de rama: " << rt << " píxeles." << endl;
	fprintf(finfo, "Cantidad de rama: %d.\n", rt);
	cout << "Área de la branquia: " << bt << " píxeles." << endl;
	fprintf(finfo, "Área de la branquia: %d.\n", bt);
	cout << "Densidad de rama: " << (float)rt/bt << "." << endl;
	fprintf(finfo, "Densidad de rama: %f.\n", (float)rt/bt);
	cout << "Generación media: " << medi << ", varianza: " << vari << "." << endl;
	fprintf(finfo, "Generación media: %f, varianza: %f.\n", medi,vari);

	system("pause"); // a la espera de que se pulse una tecla
	glutDestroyWindow(window); // cierra la ventana generada
	printf("Las métricas se han computado en las filas indicadas.\n");
	fprintf(finfo, "\nLas métricas se han computado en las filas indicadas.\n");
	fclose(finfo); // cierra el archivo "info.txt"
	exit(0); // finalización normal del programa
}

void captura(void){
	// exporta la salida gráfica del programa en ejecución como archivo de imagen
	int sx=0,sy=0,w=ancho,h=alto; // ancho y alto de la pantalla gráfica
	char out[3*w*h]; // reserva espacio para el fichero gráfico
	int P = Nodpun.size(); // sirve de flag para el nombre del gráfico

	ffichero = fruta + fprefijo + "_grafico" + (P==0 ? "compu" : "nodos") + (S==0 ? "_sincro.pnm" : ".pnm");
	farchivo = new char[ffichero.length() + 1]; std::strcpy(farchivo, ffichero.c_str());
	FILE *fgraf = fopen(farchivo,"wb"); delete[] farchivo; // fichero de salida con la imagen procesada

	if(!fgraf) return;

	glPixelStorei(GL_PACK_ALIGNMENT,1); // salida alineada de bytes
	glReadPixels(sx,sy,w,h,GL_RGB,GL_UNSIGNED_BYTE,out); // comando que obtiene la imagen

	fprintf(fgraf,"P6\n%d %d\n255\n",w,h); // cabecera del fichero "graficoproces.pnm"
	for(int y=0; y<h; y++) fwrite(&out[3*(h-1-y)*w],1,3*w,fgraf); // voltea la imagen de abajo a arriba

	fclose(fgraf); // cierra el archivo "graficoproces.pnm"
}
