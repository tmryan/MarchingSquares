/*
	Marching Squares - CS116B
	Author: Tom Ryan
	Last Modified: 02/18/17
*/

#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <vector>
#include <queue>

#ifdef __APPLE__
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#elif __linux__
	#include <GL/gl.h>
	#include <GLUT/glut.h>
#elif _WIN32
	#include <GLUT/glut.h>
#endif

#ifndef PI
	#define PI 3.14159265358979323846
#endif

/////////////////////////
// Scene & Window Const
/////////////////////

const GLint WIDTH = 800;
const GLint HEIGHT = 800;
const GLfloat ASPECT = (GLfloat)WIDTH / (GLfloat)HEIGHT;
const GLfloat FOV = 70.0f;

const GLfloat DIMENSION = 100;
const GLfloat SQUARE_WIDTH = 2.0f;
const GLfloat VIEW_SCALAR = (FOV / (100.0f * DIMENSION));

//////////////////////////////
// Vector Maths Declarations
//////////////////////////

typedef struct vec3 {
	GLfloat x;
	GLfloat y;
	GLfloat z;
} vec3;

typedef struct vec4 {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
} vec4;

GLfloat magnitude(const vec3 &vec);
vec3 normalize(const vec3 &vec);
vec3 cross(const vec3 &u, const vec3 &v);
vec3 operator/(const vec3 &vec, const GLfloat &scalar);
vec3 operator*(const vec3 &vec, const GLfloat &scalar);
vec3 operator+(const vec3 &u, const vec3 &v);
vec3 operator-(const vec3 &u, const vec3 &v);

//////////////////////////////////
// Marching Squares Declarations
//////////////////////////////

typedef enum Direction {
	NE,
	N,
	NW,
	W,
	SW,
	S,
	SE,
	E
} Direction;

typedef enum MarchingSquareState {
	TOP_LEFT,
	BOT_LEFT,
	LEFT,
	BOT_RIGHT,
	NEG_DIAG,
	BOTTOM,
	INV_TOP_RIGHT,
	TOP_RIGHT,
	UPPER,
	POS_DIAG,
	INV_BOT_RIGHT,
	RIGHT,
	INV_BOT_LEFT,
	INV_TOP_LEFT,
	FILLED,
	EMPTY
} MarchingSquareState;

class Ball {
	private:
		GLfloat radius;
		GLfloat speed;
		vec3 position;
		vec3 facing;
		vec4 color;
		bool outOfBounds;

	public:
		Ball(GLfloat radius, GLfloat speed = 3.0f, vec3 position = vec3{ 0.0f, 0.0f, 0.0f },
			vec3 facing = vec3{ 1.0f, 1.0f, 0.0f }, vec4 color = vec4{ 1.0f, 1.0f, 1.0f });
		bool contains(vec3 point);
		void move();
		void bounce(const vec3 &normal);
		GLfloat getRadius();
		vec3 getPosition();
		vec3 getFacing();
		vec4 getColor();
		bool isOutOfBounds();
		void setOutOfBounds();
		void clearOutOfBounds();
};

class MarchingSquare {
	private:
		/*
		*  p3----p2
		*  |      |
		*  |      |
		*  p0----p1
		*/
		vec3 p0;
		vec3 p1;
		vec3 p2;
		vec3 p3;
		vec3 center;
		vec4 color;
		int row;
		int col;
		MarchingSquareState state;
	
	public:
		MarchingSquare(int row, int col, vec3 p0, 
						vec4 color = vec4{ 0.2f, 0.29f, 0.82f, 1.0f }, 
						MarchingSquareState state = EMPTY);
		bool contains(vec3 point);
		void activate(const vec4 &color, MarchingSquareState state);
		void emptyState();
		vec3 getPosition();
		vec3 botLeft();
		vec3 botRight();
		vec3 topRight();
		vec3 getCenter();
		vec4 getColor();
		MarchingSquareState getState();
		int getRow();
		int getCol();
};

class SceneBounds {
	private:
		GLfloat maxX;
		GLfloat minX;
		GLfloat maxY;
		GLfloat minY;

	public:
		SceneBounds(GLfloat maxX, GLfloat minX, GLfloat maxY, GLfloat minY);
		bool outOfBounds(Ball &ball);
		vec3 getWallNormal(Ball &ball);
};

MarchingSquare* findSquare(const vec3 &pos);
void resolveSquareStates(Ball &ball, MarchingSquare &square);
void activateSquare(MarchingSquare &square, Ball &ball, int state);
Direction generateDirection();
void populateGrid();
void generateShapes(int numShapes);

////////////////////////
// OpenGL Declarations
////////////////////

typedef struct CamDirection {
	bool up;
	bool down;
	bool left;
	bool right;
	bool forward;
	bool backward;
} CamDirection;

typedef struct Camera {
	vec3 position;
	vec3 facing;
	vec3 up;
} Camera;

void initOpenGL();
void resetProjection();
void resizeViewport(GLint width, GLint height);
void draw();
void driver();

void keyboardHandler(unsigned char key, int x, int y);

//////////////////
// Lookup Tables
//////////////

std::vector<std::vector<GLfloat> > squareStateLookup{
	// TOP_LEFT
	{ -0.1f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f },

	// BOT_LEFT
	{ -1.0f, -1.0f, -1.0f,
	-0.1f, -1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f },

	// LEFT
	{ -1.0f, -1.0f, -1.0f,
	-0.1f, -1.0f, -1.0f,
	-1.0f, 0.0f, -1.0f,

	-0.1f, -1.0f, -1.0f,
	-0.1f, 1.0f, -1.0f,
	-1.0f, 0.0f, -1.0f,

	-1.0f, 0.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-0.1f, 1.0f, -1.0f },

	// BOT_RIGHT
	{ 0.1f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f },

	// NEG_DIAG
	{ 0.1f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,

	-0.1f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f,

	-1.0f, 0.1f, -1.0f,
	0.1f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,

	1.0f, -0.1f, -1.0f,
	-0.1f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f },

	// BOTTOM
	{ 0.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,

	1.0f, -0.1f, -1.0f,
	-1.0f, -0.1f, -1.0f,
	0.0f, -1.0f, -1.0f,

	-1.0f, -0.1f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	0.0f, -1.0f, -1.0f },

	// INV_TOP_RIGHT
	{ -1.0f, -1.0f, -1.0f,
	-0.1f, -1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f,

	-0.1f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 0.1f, -1.0f,

	-0.1f, -1.0f, -1.0f,
	1.0f, 0.1f, -1.0f,
	-1.0f, -0.1f, -1.0f,

	-1.0f, -0.1f, -1.0f,
	1.0f, 0.1f, -1.0f,
	0.1f, 1.0f, -1.0f,

	0.1f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f },

	// TOP_RIGHT
	{ 1.0f, 0.1f, -1.0f,
	1.0f, 1.0f, -1.0f,
	0.1f, 1.0f, -1.0f },

	// UPPER
	{ 0.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f,

	-1.0f, 0.1f, -1.0f,
	1.0f, 0.1f, -1.0f,
	0.0f, 1.0f, -1.0f,

	1.0f, 0.1f, -1.0f,
	1.0f, 1.0f, -1.0f,
	0.0f, 1.0f, -1.0f },

	// POS_DIAG
	{ -1.0f, -1.0f, -1.0f,
	-0.1f, -1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f,

	-0.1f, -1.0f, -1.0f,
	1.0f, 0.1f, -1.0f,
	-1.0f, -0.1f, -1.0f,

	-1.0f, -0.1f, -1.0f,
	0.1f, 1.0f, -1.0f,
	1.0f, 0.1f, -1.0f,

	1.0f, 0.1f, -1.0f,
	1.0f, 1.0f, -1.0f,
	0.1f, 1.0f, -1.0f },

	// INV_BOT_RIGHT
	{ 1.0f, 1.0f, -1.0f,
	-0.1f, 1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,

	-0.1f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f,

	-1.0f, 0.1f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	0.1f, -1.0f, -1.0f,

	0.1f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,
	-1.0f, 0.1f, -1.0f,

	-1.0f, 0.1f, -1.0f,
	-0.1f, 1.0f, -1.0f,
	1.0f, -0.1f, -1.0f },

	// RIGHT
	{ 1.0f, 1.0f, -1.0f,
	0.1f, 1.0f, -1.0f,
	1.0f, 0.0f, -1.0f,

	0.1f, 1.0f, -1.0f,
	0.1f, -1.0f, -1.0f,
	1.0f, 0.0f, -1.0f,

	1.0f, 0.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	0.1f, -1.0f, -1.0f },

	// INV_BOT_LEFT
	{ 1.0f, 1.0f, -1.0f,
	0.1f, 1.0f, -1.0f,
	1.0f, 0.1f, -1.0f,

	0.1f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f,

	0.1f, 1.0f, -1.0f,
	-1.0f, -0.1f, -1.0f,
	1.0f, 0.1f, -1.0f,

	1.0f, 0.1f, -1.0f,
	-1.0f, -0.1f, -1.0f,
	-0.1f, -1.0f, -1.0f,

	-0.1f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 0.1f, -1.0f },

	// INV_TOP_LEFT
	{ -1.0f, -1.0f, -1.0f,
	0.1f, -1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f,

	0.1f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -0.1f, -1.0f,

	1.0f, -0.1f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-0.1f, 1.0f, -1.0f,

	-0.1f, 1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f,
	1.0f, -0.1f, -1.0f,

	1.0f, -0.1f, -1.0f,
	0.1f, -1.0f, -1.0f,
	-1.0f, 0.1f, -1.0f },

	// FILLED
	{ 0.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 0.0f, -1.0f,

	-1.0f, 0.0f, -1.0f,
	1.0f, 0.0f, -1.0f,
	0.0f, 1.0f, -1.0f,

	1.0f, 0.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	0.0f, 1.0f, -1.0f,

	0.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -0.0f, -1.0f,

	1.0f, -0.0f, -1.0f,
	-1.0f, -0.0f, -1.0f,
	0.0f, -1.0f, -1.0f,

	-1.0f, -0.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	0.0f, -1.0f, -1.0f }
};

vec3 directionsLookup[]{
	vec3{ 1.0f, 1.0f, 0.0f },
	vec3{ 0.0f, 1.0f, 0.0f },
	vec3{ -1.0f, 1.0f, 0.0f },
	vec3{ -1.0f, 0.0f, 0.0f },
	vec3{ -1.0f, -1.0f, 0.0f },
	vec3{ 0.0f, -1.0f, 0.0f },
	vec3{ 1.0f, -1.0f, 0.0f },
	vec3{ 1.0f, 0.0f, 0.0f }
};

////////////
// Globals
////////

std::vector<std::vector<MarchingSquare> > grid;
std::queue<MarchingSquare*> activeSquares;
std::vector<Ball> balls;

Camera camera = { vec3{ 0.0f, 0.0f, 1.0f }, vec3{ 0.0f, 0.0f, 0.0f }, vec3{ 0.0f, 1.0f, 0.0f } };
SceneBounds sceneBounds(DIMENSION, -1.0f * DIMENSION + 4.0f, DIMENSION - 4.0f, -1.0f * DIMENSION);

MarchingSquare nullSqr(-1.0f, -1.0f, vec3{ 0.0f, 0.0f, 0.0f });
MarchingSquare *centerSquare;

bool activeSqrsEnabled = false;
bool centerSqrEnabled = false;
bool shapesEnabled = false;

///////////
// main()
///////

int main(int argc, char *argv[]) {
	GLint window;

	srand(static_cast<unsigned int>(time(0)));

	// Initializing scene state
	populateGrid();
	generateShapes(3);

	// Initializing window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow("Marching Squares");

	// Setting renderer callback functions
	glutDisplayFunc(&draw);
	glutIdleFunc(&driver);
	glutReshapeFunc(&resizeViewport);

	// Setting input callback functions
	glutKeyboardFunc(&keyboardHandler);

	// Initializing OpenGL
	initOpenGL();

	glutMainLoop();
}

/////////////////////////////
// OpenGL Related Functions
/////////////////////////

void initOpenGL() {
	glEnable(GL_DEPTH_TEST);

	// Setting shading technique to interpolate colors
	glShadeModel(GL_SMOOTH);

	// Preparing projection matrix
	resetProjection();

	// Selecting modelview matrix stack
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Positioning camera to look down the z-axis
	gluLookAt(camera.position.x, camera.position.y, camera.position.z,
		camera.facing.x, camera.facing.y, camera.facing.z,
		camera.up.x, camera.up.y, camera.up.z);
}

// Resets projection matrix to initial values
void resetProjection() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(FOV, ASPECT, 0.1, 10.0f);
}

// Adjust OpenGL state on window resize
void resizeViewport(GLint width, GLint height) {
	// Avoiding divide by 0 error
	if (height < 10) {
		height = 10;
	}

	// Setting viewport to new window size
	glViewport(0, 0, width, height);

	// Preparing projection matrix
	resetProjection();

	// Selecting modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Positioning camera to look down the z-axis
	gluLookAt(camera.position.x, camera.position.y, camera.position.z,
		camera.facing.x, camera.facing.y, camera.facing.z,
		camera.up.x, camera.up.y, camera.up.z);
}

// Main "loop" since GLUT is event driven
void driver() {
	// Looking for out of bounds shapes
	for (unsigned int i = 0; i < balls.size(); i++) {
		balls.at(i).move();

		// Reorienting shapes if out of bounds
		if (!balls.at(i).isOutOfBounds() && sceneBounds.outOfBounds(balls.at(i))) {
			balls.at(i).setOutOfBounds();
			// Generating new facing from wall normal
			balls.at(i).bounce(sceneBounds.getWallNormal(balls.at(i)));
		} else if(balls.at(i).isOutOfBounds()) {
			// Clearing out of bounds flag once shapes return to scene
			balls.at(i).clearOutOfBounds();
		}
	}

	for (unsigned int j = 0; j < balls.size(); j++) {
		// Searching for squares containing centers of shapes
		MarchingSquare *epicenter = findSquare(balls.at(j).getPosition());
		
		if (epicenter != &nullSqr) {
			centerSquare = epicenter;
			// Testing vertices of intersected squares and setting state
			resolveSquareStates(balls.at(j), *epicenter);
		}
	}

	draw();
}

void draw() {
	std::vector<GLfloat> *verts;
	std::vector<GLfloat>::iterator vertIter;

	std::vector<Ball> *shapeVerts;
	std::vector<Ball>::iterator shapeIter;

	MarchingSquare *square;

	// Clearing color and depth buffers
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	while (!activeSquares.empty()) {
		// Grabbing next active square object
		square = activeSquares.front();
		activeSquares.pop();

		// Grabbing next block of vertices
		verts = &squareStateLookup.at(square->getState() - 1);

		// Vertex data attributes
		unsigned int vertexDataSize = 3;
		unsigned int locationBegin = 0;

		// Applying transformations
		glPushMatrix();
		glScalef(VIEW_SCALAR, VIEW_SCALAR, VIEW_SCALAR);
		glTranslatef(square->getPosition().x, square->getPosition().y, square->getPosition().z);

		// Pushing mesh slightly backward to prevent z-fighting with overlay
		glPolygonOffset(1.0f, 1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Begin rendering
		glBegin(GL_TRIANGLES);

		// Drawing marching squares
		for (vertIter = verts->begin(); vertIter < verts->end(); vertIter += vertexDataSize) {
			glColor4f(square->getColor().x, square->getColor().y, square->getColor().z, square->getColor().w);
			glVertex3f(*vertIter, *(vertIter + 1), *(vertIter + 2));
		}

		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL);

		glPopMatrix();

		if (activeSqrsEnabled) {
			// Drawing active square outlines
			glPushMatrix();
			glScalef(VIEW_SCALAR, VIEW_SCALAR, VIEW_SCALAR);

			// Bringing outline forward so no z-fighting with mesh
			glPolygonOffset(-1.0f, -1.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glBegin(GL_POLYGON);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f(square->getPosition().x, square->getPosition().y, square->getPosition().z);
			glVertex3f(square->botLeft().x, square->botLeft().y, square->botLeft().z);
			glVertex3f(square->botRight().x, square->botRight().y, square->botRight().z);
			glVertex3f(square->topRight().x, square->topRight().y, square->topRight().z);

			glEnd();
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPopMatrix();
		}

		// Clearing shape state
		square->emptyState();
	}

	if (shapesEnabled) {
		for (shapeIter = balls.begin(); shapeIter < balls.end(); shapeIter++) {
			// Applying transformations
			glPushMatrix();
			glScalef(VIEW_SCALAR, VIEW_SCALAR, VIEW_SCALAR);

			glPolygonOffset(-1.0f, -1.0f);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glBegin(GL_POLYGON);

			// Drawing circle outline
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glVertex3f((*shapeIter).getPosition().x + (*shapeIter).getRadius(), (*shapeIter).getPosition().y, 0.0f);
			for (GLfloat theta = 0; theta < 2 * PI; theta += PI / 12.0)
			{
				glVertex3f((*shapeIter).getPosition().x + cos(theta) * (*shapeIter).getRadius(),
					(*shapeIter).getPosition().y + sin(theta) * (*shapeIter).getRadius(), 0.0f);
			}

			glEnd();
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPopMatrix();
		}
	}

	if (centerSqrEnabled) {
		// Applying transformations
		glPushMatrix();
		glScalef(VIEW_SCALAR, VIEW_SCALAR, VIEW_SCALAR);

		glPolygonOffset(-1.0f, -1.0f);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBegin(GL_POLYGON);

		// Note: This only works for the square most recently returned by findSquare()
		// Drawing epicenter outline
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex3f(centerSquare->getPosition().x, centerSquare->getPosition().y, centerSquare->getPosition().z);
		glVertex3f(centerSquare->botLeft().x, centerSquare->botLeft().y, centerSquare->botLeft().z);
		glVertex3f(centerSquare->botRight().x, centerSquare->botRight().y, centerSquare->botRight().z);
		glVertex3f(centerSquare->topRight().x, centerSquare->topRight().y, centerSquare->topRight().z);

		glEnd();
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPopMatrix();
	}

	glutSwapBuffers();
}

void keyboardHandler(unsigned char key, int x, int y) {
	switch (key) {
		// Exit program if escape key pressed
		case 27:
			exit(0);
			break;
		case 'a':
			activeSqrsEnabled = !activeSqrsEnabled;
			break;
		case 's':
			centerSqrEnabled = !centerSqrEnabled;
			break;
		case 'd':
			shapesEnabled = !shapesEnabled;
			break;
		default:
			break;
	}
}

//////////////////////////////
// MarchingSquares functions
//////////////////////////

MarchingSquare* findSquare(const vec3 &pos) {
	MarchingSquare *foundSquare = &nullSqr;
	
	// Approximating the grid element containing point pos
	MarchingSquare originSqr = grid.at((int)(DIMENSION - pos.y) / 2).at((int)(pos.x + DIMENSION) / 2);
	
	// Searching nearby grid elements for pos
	for (int i = originSqr.getRow() - 4; i <= originSqr.getRow() + 4; i++) {
		for (int j = originSqr.getCol() - 4; j <= originSqr.getCol() + 4; j++) {
			if (i < DIMENSION + 1 && i > 0 && j < DIMENSION + 1 && j > 0) {
				if (grid.at(i).at(j).contains(pos)) {
					foundSquare = &grid.at(i).at(j);
				}

				// Breaking the loop after square found
				if (foundSquare != &nullSqr) {
					break;
				}
			}
		}
	}

	return foundSquare;
}

void resolveSquareStates(Ball &ball, MarchingSquare &square) {
	int state = 0;

	for (int i = square.getRow() - (ball.getRadius() * 2.0f); i <= square.getRow() + (ball.getRadius() * 2.0f); i++) {
		for (int j = square.getCol() - (ball.getRadius() * 2.0f); j <= square.getCol() + (ball.getRadius() * 2.0f); j++) {
			if (i < DIMENSION + 1 && i > 0 && j < DIMENSION + 1 && j > 0) {
				// Note: getPosition() returns topLeft point p0
				//		 Maybe should have called it topLeft()
				if (ball.contains(grid.at(i).at(j).getPosition())) {
					state = state | 1;
				}

				if (ball.contains(grid.at(i).at(j).botLeft())) {
					state = state | 2;
				}

				if (ball.contains(grid.at(i).at(j).botRight())) {
					state = state | 4;
				}

				if (ball.contains(grid.at(i).at(j).topRight())) {
					state = state | 8;
				}

				if (state != 0) {
					activateSquare(grid.at(i).at(j), ball, state);
					state = 0;
				}
			}
		}
	}
}

void activateSquare(MarchingSquare &square, Ball &ball, int state) {
	switch (state) {
		case 1:
			square.activate(ball.getColor(), TOP_LEFT);
			break;
		case 2:
			square.activate(ball.getColor(), BOT_LEFT);
			break;
		case 3:
			square.activate(ball.getColor(), LEFT);
			break;
		case 4:
			square.activate(ball.getColor(), BOT_RIGHT);
			break;
		case 5:
			square.activate(ball.getColor(), NEG_DIAG);
			break;
		case 6:
			square.activate(ball.getColor(), BOTTOM);
			break;
		case 7:
			square.activate(ball.getColor(), INV_TOP_RIGHT);
			break;
		case 8:
			square.activate(ball.getColor(), TOP_RIGHT);
			break;
		case 9:
			square.activate(ball.getColor(), UPPER);
			break;
		case 10:
			square.activate(ball.getColor(), POS_DIAG);
			break;
		case 11:
			square.activate(ball.getColor(), INV_BOT_RIGHT);
			break;
		case 12:
			square.activate(ball.getColor(), RIGHT);
			break;
		case 13:
			square.activate(ball.getColor(), INV_BOT_LEFT);
			break;
		case 14:
			square.activate(ball.getColor(), INV_TOP_LEFT);
			break;
		case 15:
			square.activate(ball.getColor(), FILLED);
			break;
		default:
			square.activate(ball.getColor(), FILLED);
			break;
	}

	activeSquares.push(&square);
}

Direction generateDirection() {
	int direction = rand() % (8 - 1 + 1) + 1;
	
	Direction facing = NW;

	switch (direction) {
	case 1:
		facing = NE;
		break;
	case 2:
		facing = N;
		break;
	case 3:
		facing = NW;
		break;
	case 4:
		facing = W;
		break;
	case 5:
		facing = SW;
		break;
	case 6:
		facing = S;
		break;
	case 7:
		facing = SE;
		break;
	case 8:
		facing = E;
		break;
	default:
		break;
	}

	return facing;
}

void populateGrid() {
	for (GLfloat i = DIMENSION; i >= -1 * DIMENSION; i -= SQUARE_WIDTH) {
		grid.push_back(std::vector<MarchingSquare>());

		for (GLfloat j = -1 * DIMENSION; j <= DIMENSION; j += SQUARE_WIDTH) {
			grid.back().push_back(MarchingSquare(grid.size(), grid.back().size(),
								  vec3{ j, i, -1.0f }, vec4{ 0.2f, 0.29f, 0.82f, 1.0f }, 
								  EMPTY));
		}
	}
}

void generateShapes(int numShapes) {
	vec4 colors[] = {
		// Blue
		{ 0.2f, 0.29f, 0.82f, 1.0f },
		// Orange
		{ 0.918f, 0.631f, 0.2f, 1.0f }
	};
	
	GLfloat speed = 2.0f;

	// Populating list of shapes
	for (int i = 0; i < numShapes; i++) {
		// Generating random shape settings
		GLfloat radius = rand() % ((int)DIMENSION / 5 - (int)DIMENSION / 8 + 1) + (int)DIMENSION / 8;

		bool negX = (rand() % 2 == 0);
		GLfloat x = rand() % (int)((DIMENSION / 2) - radius) + radius;
		if (negX) {
			x *= -1;
		}

		bool negY = (rand() % 2 == 1);
		GLfloat y = rand() % (int)((DIMENSION / 2) - radius) + radius;
		if (negY) {
			y *= -1;
		}

		int color = rand() % (1 + 1);

		balls.push_back(Ball(radius, speed, vec3{x, y, -1.0f}, directionsLookup[generateDirection()], colors[0]));
	}
}

////////////////
// class: Ball
////////////

Ball::Ball(GLfloat radius, GLfloat speed, vec3 position, vec3 facing, vec4 color) {
	this->radius = radius;
	this->speed = speed;
	this->position = position;
	this->facing = facing;
	this->color = color;
	outOfBounds = false;
}

bool Ball::contains(vec3 point) {
	bool contained = false;

	if (position.x != point.x && position.y != point.y) {
		if (pow(position.x - point.x, 2.0) + pow(position.y - point.y, 2.0) < (radius * radius)) {
			contained = true;
		}
	} else {
		if (position.x == point.x && abs(position.y - point.y) < radius) {
			contained = true;
		} else if (position.y == point.y && abs(position.x - point.x) < radius) {
			contained = true;
		}
	}

	return contained;
}

void Ball::move() {
	position = position + (facing * speed);
}

void Ball::bounce(const vec3 &normal) {
	int component = rand() % (3 - 1 + 1) + 1;
	vec3 vec;

	// Choosing i or j component
	switch (component) {
		case 1:
			if (normal.x == 0.0f) {
				vec = vec3{ 1.0f, 0.0f, 0.0f };
			} else if (normal.y == 0.0f) {
				vec = vec3{ 0.0f, 1.0f, 0.0f };
			}
			break;
		case 2:
			if (normal.x == 0.0f) {
				vec = vec3{ 0.0f, 0.0f, 0.0f };
			} else if (normal.y == 0.0f) {
				vec = vec3{ 0.0f, 0.0f, 0.0f };
			}
			break;
		case 3:
			if (normal.x == 0.0f) {
				vec = vec3{ -1.0f, 0.0f, 0.0f };
			} else if (normal.y == 0.0f) {
				vec = vec3{ 0.0f, -1.0f, 0.0f };
			}
			break;
		default:
			break;
	}

	// Adding wall normal plus randomized i or j component yields bounce
	facing = normal + vec;
}

GLfloat Ball::getRadius() {
	return radius;
}

vec3 Ball::getPosition() {
	return position;
}

vec3 Ball::getFacing() {
	return facing;
}

vec4 Ball::getColor() {
	return color;
}

void Ball::setOutOfBounds() {
	outOfBounds = true;
}

void Ball::clearOutOfBounds() {
	outOfBounds = false;
}

bool Ball::isOutOfBounds() {
	return outOfBounds;
}

//////////////////////////
// class: MarchingSquare
//////////////////////

MarchingSquare::MarchingSquare(int row, int col, vec3 p0, vec4 color, 
								MarchingSquareState state) {
	/*
	*  p0----p3
	*  |      |
	*  |      |
	*  p1----p2
	*/	
	this->p0 = p0;
	p1 = p0 + vec3{ 0.0f, -1 * SQUARE_WIDTH, 0.0f};
	p2 = p0 + vec3{ SQUARE_WIDTH, -1 * SQUARE_WIDTH, 0.0f };
	p3 = p0 + vec3{ SQUARE_WIDTH, 0.0f, 0.0f };
	center = (p0 + p2) / 2;
	center.z = -1.0f;
	this->color = color;
	this->state = state;
	this->row = row;
	this->col = col;
}

bool MarchingSquare::contains(vec3 point) {
	bool contained = false;

	if ((point.x >= this->p0.x) && (point.x <= this->p3.x) && (point.y >= this->p1.y) && (point.y <= this->p0.y)) {
		contained = true;
	}

	return contained;
}

void MarchingSquare::activate(const vec4 &color, MarchingSquareState state) {
	this->color = color;
	this->state = static_cast<MarchingSquareState>(static_cast<int>(this->state) | static_cast<int>(state));
}

void MarchingSquare::emptyState() {
	state = EMPTY;
}

vec3 MarchingSquare::getPosition() {
	return p0;
}

vec3 MarchingSquare::botLeft() {
	return p1;
}

vec3 MarchingSquare::botRight() {
	return p2;
}

vec3 MarchingSquare::topRight() {
	return p3;
}

vec3 MarchingSquare::getCenter() {
	return center;
}

vec4 MarchingSquare::getColor() {
	return color;
}

MarchingSquareState MarchingSquare::getState() {
	return state;
}

int MarchingSquare::getRow() {
	return row;
}

int MarchingSquare::getCol() {
	return col;
}

///////////////////////
// class: SceneBounds
///////////////////

SceneBounds::SceneBounds(GLfloat maxX, GLfloat minX, GLfloat maxY, GLfloat minY) {
	this->maxX = maxX;
	this->minX = minX;
	this->maxY = maxY;
	this->minY = minY;
}

bool SceneBounds::outOfBounds(Ball &ball) {
	bool escaped = false;

	if (ball.getPosition().x + ball.getRadius() > maxX || ball.getPosition().x - ball.getRadius() < minX || 
		ball.getPosition().y + ball.getRadius() > maxY || ball.getPosition().y - ball.getRadius() < minY) {
		escaped = true;
	}

	return escaped;
}

vec3 SceneBounds::getWallNormal(Ball &ball) {
	vec3 normal;

	if (ball.getPosition().x + ball.getRadius() > maxX) {
		normal = vec3{-1.0f, 0.0f, 0.0f};
	} else if (ball.getPosition().x - ball.getRadius() < minX) {
		normal = vec3{ 1.0f, 0.0f, 0.0f };
	} else if (ball.getPosition().y + ball.getRadius() > maxY) {
		normal = vec3{ 0.0f, -1.0f, 0.0f };
	} else if (ball.getPosition().y - ball.getRadius() < minY) {
		normal = vec3{ 0.0f, 1.0f, 0.0f };
	}

	return normal;
}

//////////////////////
// lib: Vector Maths
//////////////////

GLfloat magnitude(const vec3 &vec) {
	return sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

vec3 normalize(const vec3 &vec) {
	return vec / magnitude(vec);
}

vec3 cross(const vec3 &u, const vec3 &v) {
	return vec3{ (u.y * v.z) - (u.z * v.y), (u.z * v.x) - (u.x * v.z), (u.x * v.y) - (u.y * v.x) };
}

vec3 operator/(const vec3 &vec, const GLfloat &scalar) {
	return vec3{ vec.x / scalar, vec.y / scalar, vec.z / scalar };
}

vec3 operator*(const vec3 &vec, const GLfloat &scalar) {
	return vec3{ vec.x * scalar, vec.y * scalar, vec.z * scalar };
}

vec3 operator+(const vec3 &u, const vec3 &v) {
	return vec3{ u.x + v.x, u.y + v.y, u.z + v.z };
}

vec3 operator-(const vec3 &u, const vec3 &v) {
	return vec3{ u.x - v.x, u.y - v.y, u.z - v.z };
}
