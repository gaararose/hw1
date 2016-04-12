// Author: Rose Phannavong
// Program: Hw 1
// Purpose: Waterfall Model.
//
//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

extern "C" 
{
#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 2000
#define GRAVITY 0.1

#define rnd() (float)rand()/(float)RAND_MAX
#define PI 3.1415926f

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec 
{
	float x, y, z;
};

struct Shape 
{
	float width, height;
	float radius;
	Vec center;
};

struct Particle 
{
	Shape s;
	Vec velocity;
};

struct Game 
{
	Shape box[5];
	Shape circle;
	Particle *particle;
	//Particle particle[MAX_PARTICLES];
	int n;
	Game() {
		particle = new Particle[MAX_PARTICLES];
		n = 0;

		//box shape
		for (int i = 0; i < 5; i++) {
			box[i].width = 100;
			box[i].height = 10;
			box[i].center.x = 120 + i*65;
			box[i].center.y = 500 - i*60;
		}	

		//circle shape
		circle.radius = 270;
		circle.center.x = 190 + 5*80;
		circle.center.y = 200 - 5*50;
	}

	~Game() { 
		delete [] particle; 
	}	

};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e);
void movement(Game *game);
void render(Game *game);
void bubbles(Game *game);

int bubble = 0;

int main(void)
{
	int done = 0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n = 0;

	//declare a box shape
	//game.box.width = 100;
	//game.box.height = 10;
	//game.box.center.x = 120 + 5*65;
	//game.box.center.y = 500 - 5*60;

	//start animation
	while (!done) {
		while (XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e);
		}
		movement(&game);
		render(&game);
		bubbles(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) 
{
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) 
{
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

void makeParticle(Game *game, int x, int y) 
{
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = rnd() - 0.5;
	p->velocity.x = rnd() - 0.5;
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
	    return;
	}
	if (e->type == ButtonPress) {
	    if (e->xbutton.button == 1) {
		//Left button was pressed
		int y = WINDOW_HEIGHT - e->xbutton.y;
		makeParticle(game, e->xbutton.x, y);
		return;
	    }
	    if (e->xbutton.button == 3) {
		//Right button was pressed
		return;
	    }
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
	    savex = e->xbutton.x;
	    savey = e->xbutton.y;
	    if (++n < 10)
		return;
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    makeParticle(game, e->xbutton.x, y);
	
	}
}

void bubbles(Game *game)
{
    if (bubble) {
	if (game->n < MAX_PARTICLES) {
	    for (int i = 0; i < 1; i++) {
		makeParticle(game, 120, 550);
	    }
	}
    }
}

int check_keys(XEvent *e)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
	    int key = XLookupKeysym(&e->xkey, 0);
	    if (key == XK_Escape) {
		return 1;
	    }
	    //You may check other keys here.
	    if(key == XK_b) {
		    bubble ^= 1;
	    }
	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for (int i = 0; i < game->n; i++) {
	    p = &game->particle[i];
	    p->s.center.x += p->velocity.x;
	    p->s.center.y += p->velocity.y;

	    //gravity
	    p->velocity.y -= GRAVITY;

	    //check for collision with shapes...
	    //Shape *s;
	    //s = &game->box;
	    for (int k = 0; k < 5; k++){
		//Shape *s = &game->box[k];
	    	if (p->s.center.y >= game->box[k].center.y - (game->box[k].height) &&
                	p->s.center.y <= game->box[k].center.y + (game->box[k].height) &&
	        	p->s.center.x >= game->box[k].center.x - (game->box[k].width) &&
	        	p->s.center.x <= game->box[k].center.x + (game->box[k].width)) {
	       		p->velocity.y *= rnd() * -0.5;
			p->velocity.x += rnd() * 0.05;
			p->s.center.y = game->box[k].center.y + game->box[k].height + 0.1;
	   	}	 
	    }

	    //circle collision
	    float d0, d1, dist = 0;
	    d0 = p->s.center.x - game->circle.center.x;
	    d1 = p->s.center.y - game->circle.center.y;
	    dist = sqrt(d0 * d0 + d1 * d1);
	    if (dist <= game->circle.radius) {
		    p->s.center.x = game->circle.center.x + d0/dist *
			    game->circle.radius * 1.01;
		    p->s.center.y = game->circle.center.y + d1/dist *
			    game->circle.radius * 1.01;
		    p->velocity.x += (d0/dist) * 0.50;
		    p->velocity.y += (d1/dist) * 0.05;
	    }

	    //check for off-screen
	    if ((p->s.center.y < 0.0) || (p->s.center.y > WINDOW_HEIGHT)) {
		memcpy (&game->particle[i], &game->particle[game->n-1],
			sizeof(Particle));
		std::cout << "Off screen!" << std::endl;
		//game->particle[i] = game->particle[game->n-1];
		game->n--;
	    }	
        }
}

void render(Game *game)
{
	//upper-left and lower-right corners of a rectangle
	Rect rct;
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	for (int i = 0; i < 5; i++) {		
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
			glVertex2i(-w,-h);
			glVertex2i(-w, h);
			glVertex2i( w, h);
			glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}

	const int n = 60;
	static Vec verts[n];
	static int firsttime = 1;
	float angle = 0.0;
	if (firsttime) {
		float incr = (PI * 2.0) / (float) n;
		for (int i = 0; i < n; i++) {
			verts[i].y = sin(angle) * game->circle.radius;
			verts[i].x = cos(angle) * game->circle.radius;
			angle += incr;
		}
		firsttime = 0;
	}

	//glColor3ub(90, 140, 90);
	//glPushMatric();
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < n; i++) {
		glVertex2i(game->circle.center.x + verts[i].x,
			game->circle.center.y + verts[i].y);
	}
	glEnd();
	//glPopMatrix();

	//draw all particles here
	//for (int i = 0; i < game->n; i++) { 
		glPushMatrix();
		//glColor3ub(rnd() * 150, rnd() * 160, rnd() * 220);
		for (int i = 0; i < game-> n; i++) {
		    	glColor3ub(rnd() * 150, rnd() * 160, rnd() * 220);
	    		Vec *c = &game->particle[i].s.center;
	    		w = 2;
	    		h = 2;
	    		glBegin(GL_QUADS);
				glVertex2i(c->x-w, c->y-h);
				glVertex2i(c->x-w, c->y+h);
				glVertex2i(c->x+w, c->y+h);
				glVertex2i(c->x+w, c->y-h);
	    		glEnd();
	   		glPopMatrix();
		}
		//glPopMatrix();
	//}		

	//texts/fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

	rct.bot = 585;
	rct.left = 90;
	ggprint8b (&rct, 16, 0x99CCFF, "Waterfall Model.");

	rct.bot = 570;
	rct.left = 100;
	ggprint8b (&rct, 16, 0x00ffff0, "Press b for bubbler.");

	rct.bot = 490;
	rct.left = 100;
	rct.center = 1;
	ggprint16 (&rct, 16, 0xFF0099, "Requirements.");

	rct.bot = 428;
	rct.left = 150;
	ggprint16 (&rct, 16, 0xFF0099, "Design.");

	rct.bot -= 45;
	rct.left += 70;
	ggprint16 (&rct, 16, 0xFF0099, "Coding.");

	rct.bot -= 45;
	rct.left += 70;
	ggprint16 (&rct, 16, 0xFF0099, "Testing.");

	rct.bot -= 40;
	rct.left += 90;
	ggprint16 (&rct, 16, 0xFF0099, "Maintenance.");

	//if(bubble) {
	    //if (game->n < MAX_PARTICLES) {
		//for (int i = 0; i < 10; i++) {
			//makeParticle(game, 120, 550);
		//}
	    //}
	//}
}



