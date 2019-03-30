// CPP program to illustrate 
// Scanline Polygon fill Algorithm 

#include <stdio.h> 
#include <math.h> 
#include <GL/glut.h> 
#define maxHt 350 
#define maxWd 350 
#define maxVer 500 

FILE *fp; 

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
				glVertex2i(x1,i); 
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
} 

void drawPolyDino(char x[10]) 
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
				glVertex2i( x1, y1); 
				glVertex2i( x2, y2); 
			glEnd(); 
			storeEdgeInTable(x1, y1, x2, y2);//storage of edges in edge table. 
			
			
			glFlush(); 
		} 
	} 
		
		
} 

void menu(int i) 
{ 
	if (i == 1) {
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		drawPolyDino("costarica.txt");
		
		
	}
	else if(i == 2) {
		initEdgeTable(); 
		drawPolyDino("alajuela.txt");
		ScanlineFill(1.0,0.0,0.0);
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
	else if (i == 3) {
	
	}
	else if (i == 4) {
		exit(1);
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
	glutCreateMenu(menu);
	glutAddMenuEntry(" Mapa sin colorear", 1);
	glutAddMenuEntry(" Mapa coloreado", 2);
	glutAddMenuEntry(" Mapa con texturas", 3);
	glutAddMenuEntry(" Salir", 4);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	 
	
	glutMainLoop(); 
	fclose(fp); 
} 
