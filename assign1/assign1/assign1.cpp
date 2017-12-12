// assign1.cpp : Defines the entry point for the console application.

/*
  CS 485 Computer Graphics
  Assignment 5: Height Fields
  Gabriel Wohlwend
*/

#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>

float** heightArray;

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

//swaps between monochromatic and color mapping
int colorSwitcher = 0;


typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

float defaultModifierR = 1.0;
float defaultModifierG = 1.0;
float defaultModifierB = 1.0;

//overall color modifier. This  is what's randomized when pressing 'c'
float modifierR = defaultModifierR;
float modifierG = defaultModifierG;
float modifierB = defaultModifierB;

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

//access pic's pixel brightness from 0-1 with "(float)(PIC_PIXEL(g_pHeightData, x, y, 0) / 255);"

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
	int i;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}


//initialization method
void myinit(void)
{
  /* setup gl view here */
	
	//enable Depth
	glEnable(GL_DEPTH_TEST);

	//enable alpha
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//select depth function
	glDepthFunc(GL_LESS);

	//set shading to smooth
	glShadeModel(GL_SMOOTH);

	//clear color then set up matrixmodes
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(10.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, 640.0 / 480.0, 1, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

//next 3 methods returns an altered color range to color the height map in temp map mode (press 'm' to use)
float redColor(int x, int y)
{
	if (heightArray[x][y] == 0.5 || heightArray[x][y] < 0.5)
	{
		return 0;
	}
	else //if (heightArray[x][y] > 0.5)
	{
		return (heightArray[x][y] * 2) - 1;
	}
}
float blueColor(int x, int y)
{
	if (heightArray[x][y] == 0.5 || heightArray[x][y] > 0.5)
	{
		return 0;
	}
	else //if (heightArray[x][y] < 0.5)
	{
		return 1 - ((heightArray[x][y] *2) - 1);
	}
}
float greenColor(int x, int y)
{
	//testing code
	//if (1 - (fabs((heightArray[x][y] * 2) - 1)) > 1 || 1 - (fabs((heightArray[x][y] * 2) - 1)) < 0 || 1 - (fabs((heightArray[x][y] * 2) - 1)))
	//{
	//	std::cout << "green1: " << heightArray[x][y] << std::endl;
	//	std::cout << "green2: " << (heightArray[x][y] * 2) << std::endl;
	//	std::cout << "green3: " << (heightArray[x][y] * 2) - 1 << std::endl;
	//	std::cout << "green4: " << fabs((heightArray[x][y] * 2) - 1) << std::endl;
	//	std::cout << "green5: " << 1 - (fabs((heightArray[x][y] * 2) - 1)) << std::endl << std::endl;
	//}
	
	return  1 - ( fabs( (heightArray[x][y] * 2) - 1) );
}

//call this to set the color of glColorf() it checks color mode then sets at xy depending on mode
void setColor(int x, int y)
{
	if (colorSwitcher == 0) //monochromatic
	{
		glColor3f(heightArray[x][y] * modifierR, heightArray[x][y] * modifierG, heightArray[x][y] * modifierB);
	}
	else //color mapped
	{
		glColor3f(redColor(x, y) * modifierR, greenColor(x, y) * modifierG, blueColor(x, y) * modifierB);
	}
}

//this methodis called by display and draws the heightmap
void drawHeightMap(float units, float variance)
{
	//these are used to center the heightmap
	float offsetX = (g_pHeightData->nx * units) / 2;
	float offsetY = (g_pHeightData->ny * units) / 2;
	

	for (int i = 0; i < g_pHeightData->nx - 1; i++)
	{
		for (int j = 0; j < g_pHeightData->ny - 1; j++)
		{
			glBegin(GL_TRIANGLE_STRIP);
			
			setColor(i, j);
			glVertex3f(i*units - offsetX, j * units - offsetY, heightArray[i][j] * variance);

			setColor(i + 1, j);
			glVertex3f((i + 1)*units - offsetX, j * units - offsetY, heightArray[i + 1][j] * variance);

			setColor(i, j + 1);
			glVertex3f(i*units - offsetX, (j + 1) * units - offsetY, heightArray[i][j + 1] * variance);

			setColor(i + 1, j + 1);
			glVertex3f((i + 1)*units - offsetX, (j + 1) * units - offsetY, heightArray[i + 1][j + 1] * variance);

			glEnd();
		}
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	//step back 5 units so you can see
	glTranslatef(0.0f, 0.0f, -5.0f);

	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

	glRotatef(g_vLandRotate[0], 1, 0, 0);
	glRotatef(g_vLandRotate[1], 0, 1, 0);
	glRotatef(g_vLandRotate[2], 0, 0, 1);
	
	
	/* replace this code with your height field implementation */
	/* you may also want to precede it with your rotation/translation/scaling */
	
	//i just call the method instead
	drawHeightMap(0.01, 0.4);
	

	//glFlush();
	glutSwapBuffers();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void keyFunc(unsigned char Key, int x, int y)
{
	//randomly generate 3 color modifiers between 0 and 1.0
	float red = rand() % (1 + 1);
	float green = rand() % (1 + 1);
	float blue = rand() % (1 + 1);

	switch (Key)
	{
	case 'm' :
		//swap colorSwitcher back and forth between 0 and 1 when M is pressed and when switching reset color modifiers
		if (colorSwitcher == 1) \
		{
			modifierR = defaultModifierR;
			modifierG = defaultModifierG;
			modifierB = defaultModifierB;
			colorSwitcher = 0; 
		}
		else 
		{
			modifierR = defaultModifierR;
			modifierG = defaultModifierG;
			modifierB = defaultModifierB;
			colorSwitcher = 1; }

		break;
	case 'c' :
		//randomize color modifiers when C is pressed
		//we divide by 2 and add 0.5 so it's not too dim.
		modifierR = (red / 2) + 0.5; 
		modifierG = (green / 2) + 0.5;
		modifierB = (blue / 2) + 0.5;

		break;
	}
}

void doIdle(void)
{
    /*do some stuff... */

	//didn't need an idle method so just left this here

    /*make the screen update */
	glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void createHeightArray(void)
{
	//instantiate height array
	heightArray = new float*[g_pHeightData->nx];
	for (int i = 0; i < g_pHeightData->nx; i++)
	{
		heightArray[i] = new float[g_pHeightData->ny];
	}

	//fill height array (this way we only do this once. not every frame)
	for (int i = 0; i < g_pHeightData->nx; i++)
	{
		for (int j = 0; j < g_pHeightData->ny; j++)
		{
			
			heightArray[i][j] = (((float)PIC_PIXEL(g_pHeightData, i, j, 0) +
								  (float)PIC_PIXEL(g_pHeightData, i, j, 1) +
							 	  (float)PIC_PIXEL(g_pHeightData, i, j, 2)) / 3) / 255;

			
			//heightArray[i][j] = ((float)PIC_PIXEL(g_pHeightData, i, j, 0) / 255);
		}
	}
}

int main(int argc, char* argv[])
{
	// I've set the argv[1] to spiral.jpg.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your texture name for the "Command Arguments"
	

	if (argc<2)
	{  
		printf ("usage: %s heightfield.jpg\n", argv[0]);
		exit(1);
	}

	g_pHeightData = jpeg_read((char*)argv[1], NULL);
	if (!g_pHeightData)
	{
	    printf ("error reading %s.\n", argv[1]);
	    exit(1);
	}

	createHeightArray();


	glutInit(&argc,(char**)argv);
  
	/*
		create a window here..should be double buffered and use depth testing
	*/
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutCreateWindow("HeightMap");


	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
  
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit",0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
  
	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* keyboard function */
	glutKeyboardFunc(keyFunc);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	

	/* do initialization */
	myinit();

	glutMainLoop();

	return 0;
}
