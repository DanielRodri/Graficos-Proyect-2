// CPP program to illustrate 
// Scanline Polygon fill Algorithm
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h> 
#include <conio.h>
#include <math.h> 
#include <GL/glut.h> 
#include <iostream>
using namespace std;
#define maxHt 500 
#define maxWd 500 
#define maxVer 3500 


FILE *fp; 
int modo = 0;
// Start from lower left corner 
typedef struct edgebucket 
{ 
	int ymax; //max y-coordinate of edge 
	float xofymin; //x-coordinate of lowest edge point updated only in aet 
	float slopeinverse; 
}EdgeBucket; 

typedef struct edgetabletup 
{ 
	// the array will give the scanline number 
	// The edge table (ET) with edges entries sorted 
	// in increasing y and x of the lower end 
	
	int countEdgeBucket; //no. of edgebuckets 
	EdgeBucket buckets[maxVer]; 
}EdgeTableTuple; 

EdgeTableTuple EdgeTable[maxHt], ActiveEdgeTuple; 

typedef GLubyte Pixel[3]; /*represents red green blue*/


int Width, Height; /*of image*/

/*array of pixels*/
Pixel *Image;

/*name of image file*/
char *Filename = "default.ppm";

int allowedSize(int x)
/*returns max power of 2 <= x*/
{
	int r;

	r = 1;
	while (r < x) r = (r << 1);

	if (r == x) return r;
	else return r >> 1;
}

void readImage(void)
/*reads the image file assumes ppm format*/
{
	
	int w, h, max;
	int i, j;
	unsigned int r, g, b;
	int k;
	char ch;
	FILE *fp;

	fp = fopen(Filename, "r");

	printf("filename = %s\n", Filename);

	/*read the header*/

	fscanf(fp, "P%c\n", &ch);
	if (ch != '3') {
		fprintf(stderr, "Only ascii mode 3 channel PPM files");
		exit(-1);
	}

	/*strip comment lines*/
	ch = getc(fp);
	while (ch == '#') {
		do {
			ch = getc(fp);
		} while (ch != '\n');
		ch = getc(fp);
	}
	ungetc(ch, fp);

	/*read the width*/
	fscanf(fp, "%d", &w);

	/*read the height*/
	fscanf(fp, "%d", &h);

	/*max intensity - not used here*/
	fscanf(fp, "%d", &max);

	/*width and height must be powers of 2 - taking the simple option
	here of finding the max power of 2 <= w and h*/

	Width = allowedSize(w);
	Height = allowedSize(h);

	printf("Width = %d, Height = %d\n", Width, Height);

	Image = (Pixel *)malloc(Width*Height * sizeof(Pixel));


	for (i = 0; i < Height; ++i) {
		for (j = 0; j < Width; ++j) {
			fscanf(fp, "%d %d %d", &r, &g, &b);
			k = i * Width + j; /*ok, can be more efficient here!*/
			(*(Image + k))[0] = (GLubyte)r;
			(*(Image + k))[1] = (GLubyte)g;
			(*(Image + k))[2] = (GLubyte)b;
		}
		/*better scan to the end of the row*/
		for (j = Width; j < w; ++j) fscanf(fp, "%c %c %c", &r, &g, &b);
	}
	fclose(fp);
}


void initialiseTextures(void)
{
	GLint level = 0;      /*only one level - no level of detail*/
	GLint components = 3; /*3 means R, G, and B components only*/
	GLint border = 0;     /*no border around the image*/

	/*read the image file*/
	readImage();

	/*each pixelrow on a byte alignment boundary*/
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*define information about the image*/
	glTexImage2D(GL_TEXTURE_2D, level, components,
		(GLsizei)Width, (GLsizei)Height,
		border, GL_RGB, GL_UNSIGNED_BYTE, Image);

	/*ensures that image is not wrapped*/
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	/*chooses mapping type from texels to pixels*/
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*this says for minification and magnfication choose texel that
	GL_NEAREST chooses the texel nearest the centre of the pixel
	is nearest to the centre of the pixel, rather than GL_LINEAR which
	performs a linear interpolation on the 4 surrounding texels*/

	/*GL_DECAL - this says overwrite pixel with texture colour*/
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	/*an alternative is GL_MODULATE which modulates the lighting
	by the texel value by multiplication*/

	/*this enables texturing*/
	glEnable(GL_TEXTURE_2D);
}


// Scanline Function 
void initEdgeTable() 
{ 
	int i; 
	for (i=0; i<maxHt; i++) 
	{ 
		EdgeTable[i].countEdgeBucket = 0; 
	} 
	
	ActiveEdgeTuple.countEdgeBucket = 0; 
} 





/* Function to sort an array using insertion sort*/
void insertionSort(EdgeTableTuple *ett) 
{ 
	int i,j; 
	
	EdgeBucket temp; 

	for (i = 1; i < ett->countEdgeBucket; i++) 
	{ 
		temp.ymax = ett->buckets[i].ymax; 
		temp.xofymin = ett->buckets[i].xofymin; 
		temp.slopeinverse = ett->buckets[i].slopeinverse; 
		j = i - 1; 

	while ((temp.xofymin < ett->buckets[j].xofymin) && (j >= 0)) 
	{ 
		ett->buckets[j + 1].ymax = ett->buckets[j].ymax; 
		ett->buckets[j + 1].xofymin = ett->buckets[j].xofymin; 
		ett->buckets[j + 1].slopeinverse = ett->buckets[j].slopeinverse; 
		j = j - 1; 
	} 
	ett->buckets[j + 1].ymax = temp.ymax; 
	ett->buckets[j + 1].xofymin = temp.xofymin; 
	ett->buckets[j + 1].slopeinverse = temp.slopeinverse; 
	} 
} 


void storeEdgeInTuple (EdgeTableTuple *receiver,int ym,int xm,float slopInv) 
{ 
	// both used for edgetable and active edge table.. 
	// The edge tuple sorted in increasing ymax and x of the lower end. 
	(receiver->buckets[(receiver)->countEdgeBucket]).ymax = ym; 
	(receiver->buckets[(receiver)->countEdgeBucket]).xofymin = (float)xm; 
	(receiver->buckets[(receiver)->countEdgeBucket]).slopeinverse = slopInv; 
			
	// sort the buckets 
	insertionSort(receiver); 
		
	(receiver->countEdgeBucket)++; 
	
	
} 

void storeEdgeInTable (int x1,int y1, int x2, int y2) 
{ 
	float m,minv; 
	int ymaxTS,xwithyminTS, scanline; //ts stands for to store 
	
	if (x2==x1) 
	{ 
		minv=0.000000; 
	} 
	else
	{ 
	m = ((float)(y2-y1))/((float)(x2-x1)); 
	
	// horizontal lines are not stored in edge table 
	if (y2==y1) 
		return; 
		
	minv = (float)1.0/m; 
	
	} 
	
	if (y1>y2) 
	{ 
		scanline=y2; 
		ymaxTS=y1; 
		xwithyminTS=x2; 
	} 
	else
	{ 
		scanline=y1; 
		ymaxTS=y2; 
		xwithyminTS=x1;	 
	} 
	// the assignment part is done..now storage.. 
	storeEdgeInTuple(&EdgeTable[scanline],ymaxTS,xwithyminTS,minv); 
	
	
} 

void removeEdgeByYmax(EdgeTableTuple *Tup,int yy) 
{ 
	int i,j; 
	for (i=0; i< Tup->countEdgeBucket; i++) 
	{ 
		if (Tup->buckets[i].ymax == yy) 
		{ 
		
			
			for ( j = i ; j < Tup->countEdgeBucket -1 ; j++ ) 
				{ 
				Tup->buckets[j].ymax =Tup->buckets[j+1].ymax; 
				Tup->buckets[j].xofymin =Tup->buckets[j+1].xofymin; 
				Tup->buckets[j].slopeinverse = Tup->buckets[j+1].slopeinverse; 
				} 
				Tup->countEdgeBucket--; 
			i--; 
		} 
	} 
}	 


void updatexbyslopeinv(EdgeTableTuple *Tup) 
{ 
	int i; 
	
	for (i=0; i<Tup->countEdgeBucket; i++) 
	{ 
		(Tup->buckets[i]).xofymin =(Tup->buckets[i]).xofymin + (Tup->buckets[i]).slopeinverse; 
	} 
} 

int * limits(char x[10]) {
	int * limites= new int[4] ;
	errno_t err;
	if ((err = fopen_s(&fp, x, "r")) != 0)
	{
		printf("Could not open file");
		return {};
	}

	glColor3f(1.0f, 0.0f, 0.0f);
	int count = 0, x1, y1;
	rewind(fp);
	while (!feof(fp)) {
		count++;
		if (count == 1)
		{
			fscanf_s(fp, "%d,%d", &limites[0], &limites[1]);
			limites[2] = limites[0];
			limites[3] = limites[1];
		}
		else
		{
			fscanf_s(fp, "%d,%d", &x1, &y1);
			if (x1 > limites[2]) {
				limites[2] = x1;
			}
			else if (x1< limites[0]) {
				limites[0] = x1;
			}
			if (y1 > limites[3]) {
				limites[3] = y1;
			}
			else if (y1 < limites[1]) {
				limites[1] = y1;
			}
		}
	}
	return limites;
}

void TextureFill(char provincia[10])
{
	int * limites = limits(provincia);
	//cout << "minX: " << limites[0] << "minY: " << limites[1] << "maxX: " << limites[2] << "maxY: " << limites[3];
	float largoX = limites[2] - limites[0];
	float largoY = limites[3] - limites[1];
	/* Follow the following rules:
	1. Horizontal edges: Do not include in edge table
	2. Horizontal edges: Drawn either on the bottom or on the top.
	3. Vertices: If local max or min, then count twice, else count
		once.
	4. Either vertices at local minima or at local maxima are drawn.*/

	int i, j, x1, ymax1, x2, ymax2, FillFlag = 0, coordCount;

	// we will start from scanline 0; 
	// Repeat until last scanline: 
	for (i = 0; i < maxHt; i++)//4. Increment y by 1 (next scan line) 
	{

		// 1. Move from ET bucket y to the 
		// AET those edges whose ymin = y (entering edges) 
		for (j = 0; j < EdgeTable[i].countEdgeBucket; j++)
		{
			storeEdgeInTuple(&ActiveEdgeTuple, EdgeTable[i].buckets[j].
				ymax, EdgeTable[i].buckets[j].xofymin,
				EdgeTable[i].buckets[j].slopeinverse);
		}

		// 2. Remove from AET those edges for 
		// which y=ymax (not involved in next scan line) 
		removeEdgeByYmax(&ActiveEdgeTuple, i);

		//sort AET (remember: ET is presorted) 
		insertionSort(&ActiveEdgeTuple);



		//3. Fill lines on scan line y by using pairs of x-coords from AET 
		j = 0;
		FillFlag = 0;
		coordCount = 0;
		x1 = 0;
		x2 = 0;
		ymax1 = 0;
		ymax2 = 0;
		while (j < ActiveEdgeTuple.countEdgeBucket)
		{
			if (coordCount % 2 == 0)
			{
				x1 = (int)(ActiveEdgeTuple.buckets[j].xofymin);
				ymax1 = ActiveEdgeTuple.buckets[j].ymax;
				if (x1 == x2)
				{
					/* three cases can arrive-
						1. lines are towards top of the intersection
						2. lines are towards bottom
						3. one line is towards top and other is towards bottom
					*/
					if (((x1 == ymax1) && (x2 != ymax2)) || ((x1 != ymax1) && (x2 == ymax2)))
					{
						x2 = x1;
						ymax2 = ymax1;
					}

					else
					{
						coordCount++;
					}
				}

				else
				{
					coordCount++;
				}
			}
			else
			{
				x2 = (int)ActiveEdgeTuple.buckets[j].xofymin;
				ymax2 = ActiveEdgeTuple.buckets[j].ymax;

				FillFlag = 0;

				// checking for intersection... 
				if (x1 == x2)
				{
					/*three cases can arive-
						1. lines are towards top of the intersection
						2. lines are towards bottom
						3. one line is towards top and other is towards bottom
					*/
					if (((x1 == ymax1) && (x2 != ymax2)) || ((x1 != ymax1) && (x2 == ymax2)))
					{
						x1 = x2;
						ymax1 = ymax2;
					}
					else
					{
						coordCount++;
						FillFlag = 1;
					}
				}
				else
				{
					coordCount++;
					FillFlag = 1;
				}


				if (FillFlag)
				{
					//drawing actual lines... 
						glBegin(GL_LINES);
						/*
						cout << "\n";
						cout << x1 << "-" << x2 <<"\n";
						cout << limites[2] << "-" << limites[0] << "\n";
						cout << ((float)(limites[2] - x1) / (float)(limites[2] - limites[0])) << "<->" << 1.0-((float)(limites[3] - i) / (float)(limites[3] - limites[1])) << "\n";
						cout << ((float)(limites[2] - x2) / (float)(limites[2] - limites[0])) << "<->" << 1.0-((float)(limites[3] - i) / (float)(limites[3] - limites[1])) << "\n";
						*/
						glTexCoord2f(1.0-((float)(limites[2] - x1) / largoX), ((float)(limites[3] - i) / largoY));
						glVertex2i(x1, i);

						glTexCoord2f(1.0-((float)(limites[2] - x2) / largoX), ((float)(limites[3] - i) / largoY));
						glVertex2i(x2, i);

						glEnd();
						glFlush();
					


				}

			}

			j++;
		}


		// 5. For each nonvertical edge remaining in AET, update x for new y 
		updatexbyslopeinv(&ActiveEdgeTuple);
	}



}


void ScanlineFill(float R, float G, float B)
{ 
	
	/* Follow the following rules: 
	1. Horizontal edges: Do not include in edge table 
	2. Horizontal edges: Drawn either on the bottom or on the top. 
	3. Vertices: If local max or min, then count twice, else count 
		once. 
	4. Either vertices at local minima or at local maxima are drawn.*/

	int i, j, x1, ymax1, x2, ymax2, FillFlag = 0, coordCount; 
	
	// we will start from scanline 0; 
	// Repeat until last scanline: 
	for (i=0; i<maxHt; i++)//4. Increment y by 1 (next scan line) 
	{ 
		
		// 1. Move from ET bucket y to the 
		// AET those edges whose ymin = y (entering edges) 
		for (j=0; j<EdgeTable[i].countEdgeBucket; j++) 
		{ 
			storeEdgeInTuple(&ActiveEdgeTuple,EdgeTable[i].buckets[j]. 
					ymax,EdgeTable[i].buckets[j].xofymin, 
					EdgeTable[i].buckets[j].slopeinverse); 
		} 
	
		// 2. Remove from AET those edges for 
		// which y=ymax (not involved in next scan line) 
		removeEdgeByYmax(&ActiveEdgeTuple, i); 
		
		//sort AET (remember: ET is presorted) 
		insertionSort(&ActiveEdgeTuple); 
		

		
		//3. Fill lines on scan line y by using pairs of x-coords from AET 
		j = 0; 
		FillFlag = 0; 
		coordCount = 0; 
		x1 = 0; 
		x2 = 0; 
		ymax1 = 0; 
		ymax2 = 0; 
		while (j<ActiveEdgeTuple.countEdgeBucket) 
		{ 
			if (coordCount%2==0) 
			{ 
				x1 = (int)(ActiveEdgeTuple.buckets[j].xofymin); 
				ymax1 = ActiveEdgeTuple.buckets[j].ymax; 
				if (x1==x2) 
				{ 
				/* three cases can arrive- 
					1. lines are towards top of the intersection 
					2. lines are towards bottom 
					3. one line is towards top and other is towards bottom 
				*/
					if (((x1==ymax1)&&(x2!=ymax2))||((x1!=ymax1)&&(x2==ymax2))) 
					{ 
						x2 = x1; 
						ymax2 = ymax1; 
					} 
				
					else
					{ 
						coordCount++; 
					} 
				} 
				
				else
				{ 
						coordCount++; 
				} 
			} 
			else
			{ 
				x2 = (int)ActiveEdgeTuple.buckets[j].xofymin; 
				ymax2 = ActiveEdgeTuple.buckets[j].ymax; 
			
				FillFlag = 0; 
				
				// checking for intersection... 
				if (x1==x2) 
				{ 
				/*three cases can arive- 
					1. lines are towards top of the intersection 
					2. lines are towards bottom 
					3. one line is towards top and other is towards bottom 
				*/
					if (((x1==ymax1)&&(x2!=ymax2))||((x1!=ymax1)&&(x2==ymax2))) 
					{ 
						x1 = x2; 
						ymax1 = ymax2; 
					} 
					else
					{ 
						coordCount++; 
						FillFlag = 1; 
					} 
				} 
				else
				{ 
						coordCount++; 
						FillFlag = 1; 
				} 
			
			
				if(FillFlag) 
				{ 
					//drawing actual lines... 
					glColor3f(R,G,B); 
				
					glBegin(GL_LINES);
					glTexCoord2f(0.0, i);
					glVertex2i(x1,i); 

					glTexCoord2f(1.0, i);
					glVertex2i(x2,i); 
				
					glEnd(); 
					glFlush();		 
				
			
				} 
			
			} 
			
			j++; 
		} 
			
		
		// 5. For each nonvertical edge remaining in AET, update x for new y 
		updatexbyslopeinv(&ActiveEdgeTuple); 
	} 



} 


void myInit(void) 
{ 

	glClearColor(1.0,1.0,1.0,0.0); 
	glMatrixMode(GL_PROJECTION); 
	
	glLoadIdentity(); 
	gluOrtho2D(0,maxHt,0,maxWd); 
	glClear(GL_COLOR_BUFFER_BIT); 
	//initialiseTextures();
} 

void drawPolyDino(char x[100]) 
{ 

	errno_t err;
	if ((err = fopen_s(&fp, x, "r")) != 0)
	{
		printf("Could not open file");
		return;
	}

	glColor3f(1.0f,0.0f,0.0f); 
	int count = 0,x1,y1,x2,y2; 
	rewind(fp); 
	while(!feof(fp) ) 
	{ 
		count++; 
		if (count>2) 
		{ 
			x1 = x2; 
			y1 = y2; 
			count=2; 
		} 
		if (count==1) 
		{ 
			fscanf_s(fp, "%d,%d", &x1, &y1); 
		} 
		else
		{ 
			fscanf_s(fp, "%d,%d", &x2, &y2); 
			
			glBegin(GL_LINES); 
			glVertex2i(x1, y1);

			glVertex2i(x2, y2);
			glEnd(); 
			storeEdgeInTable(x1, y1, x2, y2);//storage of edges in edge table. 
			
			
			glFlush(); 
		} 
	} 
		
		
} 
void dibujarColor() {
	initEdgeTable();
	drawPolyDino("alajuela.txt");
	ScanlineFill(1.0, 0.0, 0.0);
	initEdgeTable();
	drawPolyDino("heredia.txt");
	ScanlineFill(1.0, 1.0, 0.0);
	initEdgeTable();
	drawPolyDino("sanjose.txt");
	ScanlineFill(1.0, 0.0, 1.0);
	initEdgeTable();
	drawPolyDino("cartago.txt");
	ScanlineFill(0.0, 1.0, 1.0);
	initEdgeTable();
	drawPolyDino("limon.txt");
	ScanlineFill(0.5, 1.0, 0.5);
	initEdgeTable();
	drawPolyDino("puntarenas.txt");
	ScanlineFill(1.0, 0.5, 0.0);
	initEdgeTable();
	drawPolyDino("Guanacaste.txt");
	ScanlineFill(0.0, 1.0, 0.0);
}

void dibujarSinColor() {

	drawPolyDino("alajuela.txt");

	drawPolyDino("heredia.txt");

	drawPolyDino("sanjose.txt");

	drawPolyDino("cartago.txt");

	drawPolyDino("limon.txt");

	drawPolyDino("puntarenas.txt");

	drawPolyDino("Guanacaste.txt");

}

void dibujarTextura() {
	initEdgeTable();
	drawPolyDino("alajuela.txt");
	TextureFill("alajuela.txt");

	initEdgeTable();
	drawPolyDino("heredia.txt");
	TextureFill("heredia.txt");

	initEdgeTable();
	drawPolyDino("sanjose.txt");
	TextureFill("sanjose.txt");

	initEdgeTable();
	drawPolyDino("cartago.txt");
	TextureFill("cartago.txt");

	initEdgeTable();
	drawPolyDino("limon.txt");
	TextureFill("limon.txt");

	initEdgeTable();
	drawPolyDino("puntarenas.txt");
	TextureFill("puntarenas.txt");

	initEdgeTable();
	drawPolyDino("Guanacaste.txt");
	TextureFill("Guanacaste.txt");
}

void menu(int i) 
{
	
	if (i == 1) {
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		dibujarSinColor();
		modo = 1;
		
	}
	else if(i == 2) {
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		dibujarColor();
		
		modo = 2;
	
	}
	else if (i == 3) {
		//gl(GL_TEXTURE_2D);
		glPushMatrix();
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		initialiseTextures();
		dibujarTextura();

		modo = 3;
	}
	else if (i == 4) {
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		glPopMatrix();
		if (modo == 1) {
			dibujarSinColor();
		}
		else if (modo == 2) {
			dibujarColor();
		}
		glPushMatrix();
	}
	else if (i == 4) {
		exit(1);
	}
	
	
} 



void teclado(unsigned char key, int x, int y) {
	
	if (modo != 0) {
		switch (key)
		{


		case 'a': //izquierda
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			glTranslatef(250, 20, 0.0f);
			glRotatef(65, 0.0f, 0.0f, 1.0f);

			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();


			printf("\r\n Presiono Flecha izquierda");
			break;
		case 'd':
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			glTranslatef(20, 250, 0.0f);
			glRotatef(-65, 0.0f, 0.0f, 1.0f);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();


			printf("\r\n Presiono Flecha izquierda");
			break;
		case 'z':
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();

			glTranslatef(100.0, 75.0, 0.0f);
			glScalef(0.5, 0.5, 0.0);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}

			glFlush();
			printf("\r\n Zoom out");
			break;

		case 'x':
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			glTranslatef(-100.0, -75.0, 0.0f);
			glScalef(1.5, 1.5, 0.0);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();
			printf("\r\n Zoom in");
			break;



		}
	}
}


void pan(int key, int x, int y) {

	if (modo != 0) {
		switch (key)
		{


		case GLUT_KEY_UP:
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			glTranslatef(0.0, 20, 0.0f);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();
			break;

		case GLUT_KEY_DOWN:
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();
			glTranslatef(0.0, -20, 0.0f);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();
			break;

		case GLUT_KEY_LEFT:
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();

			glTranslatef(-20, 0.0, 0.0f);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();
			break;

		case GLUT_KEY_RIGHT:
			glClear(GL_COLOR_BUFFER_BIT);
			glFlush();

			glTranslatef(20, 0.0, 0.0f);
			if (modo == 1) {
				dibujarSinColor();
			}
			else if (modo == 2) {
				dibujarColor();
			}
			else if (modo == 3) {
				dibujarTextura();
			}
			glFlush();
			break;

		}


	}
}



void main(int argc, char** argv) 
{ 
	
	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
	glutInitWindowSize(maxHt,maxWd); 
	glutInitWindowPosition(100, 150); 
	glutCreateWindow("Proyecto #2"); 
	myInit(); 
	glutSpecialFunc(pan);
	glutKeyboardFunc(teclado);
	glutCreateMenu(menu);
	glutAddMenuEntry(" Mapa sin colorear", 1);
	glutAddMenuEntry(" Mapa coloreado", 2);
	glutAddMenuEntry(" Mapa con texturas", 3);
	glutAddMenuEntry(" Reiniciar", 4);
	glutAddMenuEntry(" Salir", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	
	
	glutMainLoop(); 
	fclose(fp); 
} 
				

		