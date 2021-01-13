#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <windows.h>
#include <gl/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "cube.h"
#include "QuadMesh.h"

const int vWidth  = 1100;    // Viewport width in pixels
const int vHeight = 700;    // Viewport height in pixels

// Variables for rotating wheels
float rightWheel = 0.0;
float leftWheel = 0.0;

// Control the 
float flip = 0.0;

// Move the bot forward/backward along its relative x-axis
float drive = 0.0;

// Turn bot left or right
float robotAngle = 0.0;

// Spin Flag created using Cube Mesh
float flagAngle = 0.0;

// Lighting/shading and material properties for robot (copied)
// Robot RGBA material properties
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };

GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat wheelsAndBotSupportParts_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat wheelsAndBotSupportParts_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat wheelsAndBotSupportParts_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat wheelsAndBotSupportParts_mat_shininess[] = { 100.0F };

GLfloat botBaseAndArm_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat botBaseAndArm_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat botBaseAndArm_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat botBaseAndArm_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// A template cube mesh
CubeMesh *cubeMesh = createCubeMesh();
bool stop = false;
// A flat open mesh
QuadMesh *groundMesh = NULL;

// Default Mesh Size
int meshSize = 8;

// Function prototypes
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void flagAnimationHandler(int param);
void drawBattleBot();
void drawBase();
void drawArm();
void drawWheels();
void drawArmSupport();
void drawArmSupportCylinders();
void drawFlag();

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Arm Flipper Subzero 2020");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 5, 15) looking at the origin, up along positive y axis
	gluLookAt(0.0, 5.0, 15.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Battle Bot
	drawBattleBot();

	// Drawing additional object using cube mesh
	drawFlag();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -2.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
	
}

void drawFlag()
{
	glPushMatrix();
	// spin flag
	glTranslatef(10.0, 0, -8.0);
	glRotatef(flagAngle, 0.0, 1.0, 0.0);
	glTranslatef(-10.0, 0, 8.0);

	glPushMatrix();
	glTranslatef(10.0, 0, -8.0);

	//draw flag
	glPushMatrix();
	glTranslatef(1.0, 1.0, 0.0);
	glScalef(1.0,0.8,0.1);
	drawCubeMesh(cubeMesh);
	glPopMatrix();

	//draw flag pole
	glPushMatrix();
	glScalef(0.1,2.0,0.1);
	drawCubeMesh(cubeMesh);
	glPopMatrix();

	glPopMatrix();

	glPopMatrix();
}

void drawBattleBot()
{
	 
	glPushMatrix();

	 glRotatef(robotAngle, 0.0, 0.1, 0.0); // Turns bot left or right
	 glTranslatef(drive, 0.0, 0.0); // Moves bot forwards or backwards
	 drawBase();
	 drawWheels();
	 drawArm();
	 drawArmSupport();
	 drawArmSupportCylinders();

	glPopMatrix();


}

void drawBase()
{
	// Lighting and shading for base (copied)
	glMaterialfv(GL_FRONT, GL_AMBIENT, botBaseAndArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, botBaseAndArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, botBaseAndArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, botBaseAndArm_mat_shininess);
	
	//Draw middle body
	glPushMatrix();
	glTranslatef(-1.56,0.0,0.0);
	glScalef(4.2, 1.25, 5.0);
	glutSolidCube(1.0);
	glPopMatrix();

	//Draw middle body (flat part in the front)
	glPushMatrix();
	glTranslatef(3.75,0.0,0.0);
	glScalef(0.125, 0.25, 3.0);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw support bar at the front of the bot
	glPushMatrix();
	glTranslatef(1.8,-0.5,0.0);
	glScalef(4.0, 0.25, 3.0);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw left side
	glPushMatrix();
	glTranslatef(1.8,0.0,-1.8);
	glScalef(4.0, 1.25, 1.2);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw left side between wheels
	glPushMatrix();
	glTranslatef(0.0,0.0,-2.6);
	glScalef(3.8, 1.25, 0.6);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw right side
	glPushMatrix();
	glTranslatef(1.8,0.0,1.8);
	glScalef(4.0, 1.25, 1.2);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw right side between wheels
	glPushMatrix();
	glTranslatef(0.0,0.0,2.6);
	glScalef(3.8, 1.25, 0.6);
	glutSolidCube(1.0);
	glPopMatrix();
	
}

void drawWheels(){

	// Lighting and shading for wheels (copied)
	glMaterialfv(GL_FRONT, GL_AMBIENT, wheelsAndBotSupportParts_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, wheelsAndBotSupportParts_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelsAndBotSupportParts_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, wheelsAndBotSupportParts_mat_shininess);

	// Draw front-right wheel
	glPushMatrix(); 
	glTranslatef(2.8, 0.0, 2.8);
	glRotatef(rightWheel,0.0,0.0,1.0); // Control roation of wheel
	glScalef(0.85,0.85,0.85);
	glutSolidTorus(0.4,0.8,10,10);
	glPopMatrix();

	// Draw back-right wheel
	glPushMatrix();
	glTranslatef(-2.8, 0.0, 2.8);
	glRotatef(rightWheel,0.0,0.0,1.0); // Control roation of wheel
	glScalef(0.85,0.85,0.85);
	glutSolidTorus(0.4,0.8,10,10);
	glPopMatrix();

	// Draw front-left wheel
	glPushMatrix();
	glTranslatef(2.8, 0.0, -2.8);
	glRotatef(leftWheel,0.0,0.0,1.0); // Control roation of wheel
	glScalef(0.85,0.85,0.85);
	glutSolidTorus(0.4,0.8,10,10);
	glPopMatrix();

	// Draw back-left wheel
	glPushMatrix();
	glTranslatef(-2.8, 0.0, -2.8);
	glRotatef(leftWheel,0.0,0.0,1.0); // Control roation of wheels
	glScalef(0.85,0.85,0.85);	
	glutSolidTorus(0.4,0.8,10,10);
	glPopMatrix();	
}

void drawArm()
{
	// Lighting and shading for arm (copied)
	glMaterialfv(GL_FRONT, GL_AMBIENT, botBaseAndArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, botBaseAndArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, botBaseAndArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, botBaseAndArm_mat_shininess);

	glPushMatrix();
	// Apply transformations to allow  to rotate in a 'flicking' action
	glRotatef(flip,0.0,0.0,1.0); 
	glRotatef(-5.0,0.0,0.0,1.0);
	glTranslatef(-0.7,-0.3,0.0);

	// Draw tip of the arm
	glPushMatrix();
	glTranslatef(6.28,0.8,0.0);
	glRotatef(5.0,0.0,0.0,1.0);
	glScalef(1.0,0.5,1.0);
	glutSolidCube(1.5);
	glPopMatrix();

	// Draw the middle parts of the arm
	glPushMatrix();
	glTranslatef(4.85,1.335,0.0);
	glRotatef(55.0,0.0,0.0,1.0);
	glScalef(0.5,1.5,1.0);
	glutSolidCube(1.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.02,1.91,0.0);
	glScalef(1.5,0.5,1.0);
	glutSolidCube(1.5);
	glPopMatrix();

	// Draw part of arm closest to the middle of the base
	glPushMatrix();
	glTranslatef(0.8,1.2,0.0);
	glRotatef(-59.5,0.0,0.0,1.0);
	glScalef(0.5,2.0,1.0);
	glutSolidCube(1.5);
	glPopMatrix();

	glPopMatrix();

}

void drawArmSupportCylinders()
{
	// Lighting and shading for support parts (copied)
	glMaterialfv(GL_FRONT, GL_AMBIENT, wheelsAndBotSupportParts_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, wheelsAndBotSupportParts_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelsAndBotSupportParts_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, wheelsAndBotSupportParts_mat_shininess);

	glPushMatrix();
	// Apply transformations to align with 
	glTranslatef(-0.6,-0.2,0.0);
	glRotatef(-60.0,0.0,0.0,1.0);
	glRotatef(90.0,0.0,1.0,0.0);

	// Draw thin cylinders attached to 
	glPushMatrix();
	GLUquadricObj *quadratic5;
	quadratic5 = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluQuadricDrawStyle(quadratic5, GLU_LINE);
	glTranslatef(-0.3, 2.0, -0.8);
	glScalef(0.1,0.1,0.8);
	gluCylinder(quadratic5,1.0f, 1.0f, 1.0f,32,32);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj *quadratic4;
	quadratic4 = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluQuadricDrawStyle(quadratic4, GLU_LINE);
	glTranslatef(0.3, 2.0, -0.8);
	glScalef(0.1,0.1,0.8);
	gluCylinder(quadratic4,1.0f, 1.0f, 1.0f,32,32);
	glPopMatrix();

	// Draw rectangluar frame for cylinders
	glPushMatrix();
	glTranslatef(0.0,2.0,1.2);
	glScalef(1.7, 0.8, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0,2.0,0.0);
	glScalef(1.7, 0.8, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw Cylinders
	glPushMatrix();
	GLUquadricObj *quadratic3;
	quadratic3 = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluQuadricDrawStyle(quadratic3, GLU_LINE);
	glTranslatef(0.4, 2.0, 0.0);
	glScalef(0.4,0.4,1.2);
	gluCylinder(quadratic3,1.0f, 1.0f, 1.0f,32,32);
	glPopMatrix();

	glPushMatrix();
	GLUquadricObj *quadratic2;
	quadratic2 = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluQuadricDrawStyle(quadratic2, GLU_LINE);
	glTranslatef(-0.4, 2.0, 0.0);
	glScalef(0.4,0.4,1.2);
	gluCylinder(quadratic2,1.0f, 1.0f, 1.0f,32,32);
	glPopMatrix();

	glPopMatrix();
}

void drawArmSupport()
{
	// Lighting and shading for support parts (copied)
	glMaterialfv(GL_FRONT, GL_AMBIENT, wheelsAndBotSupportParts_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, wheelsAndBotSupportParts_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, wheelsAndBotSupportParts_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, wheelsAndBotSupportParts_mat_shininess);

	// Draw leftside diagonal bar
	glPushMatrix();
	glTranslatef(-0.5,0.76,-0.9);
	glRotatef(-64.5,0.0,0.0,1.0);
	glScalef(0.25, 1.4, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw leftside upright bar
	glPushMatrix();
	glTranslatef(0.0,0.9,-0.9);
	glScalef(0.25, 0.5, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw rightside diagonal bar
	glPushMatrix();
	glTranslatef(-0.5,0.76,0.9);
	glRotatef(-64.5,0.0,0.0,1.0);
	glScalef(0.25, 1.4, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw rightside upright bar
	glPushMatrix();
	glTranslatef(0.0,0.9,0.9);
	glScalef(0.25, 0.5, 0.125);
	glutSolidCube(1.0);
	glPopMatrix();

	// Draw cylinder to go through the 
	glPushMatrix();
	GLUquadricObj *quadratic1;
	quadratic1 = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluQuadricDrawStyle(quadratic1, GLU_LINE);
	glTranslatef(0.0, 1.0, -1.1);
	glScalef(0.1,0.1,2.2);
	gluCylinder(quadratic1,1.0f, 1.0f, 1.0f,32,32);
	glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 80.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 5, 15) looking at the origin, up along positive y axis
	gluLookAt(0.0, 5.0, 15.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
  switch(key){

  case 'q':
  case 'Q':
  case 27:  
    // Esc, q, or Q key = Quit 
    exit(0);
    break;  
  case 'f':
	// Pressing f moves battle bot forward
	rightWheel -= 10.0;
	leftWheel -= 10.0;
    drive += 0.2;
    glutPostRedisplay();
    break;
  case 'b':
	// Pressing b moves battle bot backwards
	rightWheel += 10.0;
	leftWheel += 10.0;
	drive -= 0.2;
	glutPostRedisplay();
	break;
  case 's':
	// Pressing s spins the flag
	stop = false;
	glutTimerFunc(10, flagAnimationHandler, 0);
	break;
  case 'S':
	// Pressing capital S stops spinning the flag
	stop = true;
	break;
  // Initialize the transformation variables 
  default:
    break;

  }


}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key - pressing F1 tells user how to control the battle bot (instructions are printed to console window)
	if (key == GLUT_KEY_F1)
	{
		printf("\n BATTLEBOT CONTROLS:");
		printf("\n f - move bot forwards \n b - move bot backwards \n up arrow - activate weapon");
		printf("\n left arrow - turn left \n right arrow - turn right");
		printf("\n s - spin flag\n S - stop spinning flag\n q - quit program\n\n");
	}

	// Left arrow key - when pressed, bot turns left
	if (key == GLUT_KEY_LEFT)
	{
		robotAngle += 3.0;
		leftWheel -= 10.0;
		glutPostRedisplay();
	}

	// Right arrow key - when pressed, bot turns right
	if (key == GLUT_KEY_RIGHT)
	{
		robotAngle -= 3.0;
		rightWheel -= 10.0;
		glutPostRedisplay();
	}

	// Up arrow key - when pressed, bots weapon activates
	if (key == GLUT_KEY_UP)
	{
		flip = 25.0;
		glutTimerFunc(200, animationHandler, 0);
		glutPostRedisplay();
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Handles animation of weapon flicking motion 
void animationHandler(int param)
{
  // Bring  back to initial position
  flip += 0.0;

  // If the  = 25 degrees, it's brought back to initial position
  if (flip >= 25.0)
  {
	flip = 0.0;
  }
  glutPostRedisplay();
}

// Handles spinning animation of flag 
void flagAnimationHandler(int param)
{
	if (!stop)
	{
		flagAngle -= 3.0;
		glutPostRedisplay();
		glutTimerFunc(10, flagAnimationHandler, 0);
	}
}
