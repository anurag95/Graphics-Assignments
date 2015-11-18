#include <iostream>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>
#include <cmath>
#include <vector>
#include <cstring>
#include <GL/glut.h>
using namespace std;

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define REST 0.98f
#define FRICTION 0.95f
#define TURN_TIME 30
// Function Declarations
void drawScene();
void update(int value);
void initRendering();
void handleResize(int w, int h);
void handleKeypress1(unsigned char key, int x, int y);
void handleKeypress2(int key, int x, int y);
void mouse_click(int btn,int state,int x,int y);
void output(float x, float y, float r, float g, float b, char *string);
void draw(float x, float y, float r, float g, float b);
void drawrack();


// Global Variables
int isgameover = 0;
int isclicked = 0;
int isstrikermoving = 0;
struct timeval tv, mytime;
int game_score0 = 30;
int game_score1 = 30;
int timer = TURN_TIME;
int turn = 0;
int ispocketed = 0; 
float box_len = 6.0f;

class striker;

class ball {
	protected:
		float centre_x;
		float centre_y;
		float vel_x;
		float vel_y;
		float rad;
	public:
		ball(void) {
		}

		ball(float x, float y, float radius)
		{
			centre_x = x;
			centre_y = y;
			vel_x = 0.0f;
			vel_y = 0.0f;
			rad = radius;

		}
		void draw(float r, float g, float b)
		{
    		glTranslatef(centre_x, centre_y, 0.0f);
    		glColor3f(r, g, b);
			glBegin(GL_TRIANGLE_FAN);
			for(int i=0 ; i<360 ; i++) {
				glVertex2f(rad * cos(DEG2RAD(i)), rad * sin(DEG2RAD(i)));
			}
			glEnd();
			glColor3f(0.0f, 0.0f, 0.0f);
			static const double inc = PI / 12;
			static const double max = 2 * PI;
		}

		void reducespeed()
		{
			if(abs(vel_y)>1e-3 && abs(vel_x) < 1e-3*abs(vel_x/vel_y))
				vel_x = 0.0f;
			else if(abs(vel_y)<=1e-3 && abs(vel_x) < 1e-3)
				vel_x = 0.0f;
			else vel_x = FRICTION*vel_x;

			if(abs(vel_y) < 1e-3)
				vel_y = 0;
			else vel_y = FRICTION*vel_y;
		}

		void pos_update(float box_len) {
			
			//reduce the speed of ball
			reducespeed();

			// Handle ball collisions with box
			if(centre_x + rad > box_len/2 || centre_x - rad < -box_len/2)
				vel_x *= -REST*1.0f;
			if(centre_y + rad > box_len/2 || centre_y - rad < -box_len/2)
				vel_y *= -REST*1.0f;
			
			// Update position of ball
			centre_x += vel_x;
			centre_y += vel_y;
		}
		
		int pocket(float box_len)
		{
			float rad = 0.13f;
			float offset = box_len*(1.29f/3.0f);
			if(sqrt(pow(centre_x - offset,2)+pow(centre_y-offset,2)) < rad)
			{
		//		vel_x = vel_y = 0.0f;
				return 1;
			}
			if(sqrt(pow(centre_x + offset,2)+pow(centre_y-offset,2)) < rad)
			{
		//		vel_x = vel_y = 0.0f;
				return 1;
			}
			if(sqrt(pow(centre_x - offset,2)+pow(centre_y+offset,2)) < rad)
			{
		//		vel_x = vel_y = 0.0f;
				return 1;
			}
			if(sqrt(pow(centre_x + offset,2)+pow(centre_y+offset,2)) < rad)
			{
		//		vel_x = vel_y = 0.0f;
				return 1;
			}
			return 0;
		}

		float getx() {return vel_x; }
		float gety() {return vel_y; }
		float getceny() {return centre_y; }
		float getcenx() {return centre_x; }
		
};

class coin : public ball {
	private:
		int score0;
		int score1;
		int type; //1 white, 0 black, 2 red	
	public:
		coin(void) : ball() {
		}

		coin(int t, float x, float y, float radius) : ball(x, y, radius)
		{
			type = t;
			if(type == 2)
				score1 = score0 = 50;
			else if(type == 1)
			{
				score0 = 10;  
				score1 = -5;
			}
			else {
				score0 = -5;
				score1 = 10;
			}
		}
		void drawcoin()
		{
			if(type == 1)
			{
				draw(0.847f, 0.694f, 0.392f);
    //			draw(0.48f, 0.20f, 0.11f);
			}
			else if(type == 0) 
				draw(0, 0, 0);
			else draw(1.0, 0, 0);
		}
		int gettype() { return type; }

		pair<int, int> getscore(void) { return make_pair(score0, score1); }

		friend void ccollision(coin &c1, coin &c2);
		friend void scollision(striker &c1, coin &c2);
};

class striker : public ball {
	
	private:	
		float mag;
		float angle;

	public:

		striker(float x, float y, float radius) : ball(x, y, radius)
		{
			angle = (float)(PI/2);
			mag = 0.3f;
		}
		
		void setball(float x) 
		{ 
			if(centre_x<(-2.0f) && x<0.0f)
				return ;
			else if(centre_x>2.0f && x>0.0f)
				return ;
			centre_x += x; 
			return ;
		}

		void resetball() 
		{ 
			turn = turn^1;
			if(centre_x<(-2.0f))
				centre_x = -2.0f;
			else if(centre_x>2.0f)
				centre_x = 2.0f;
			if(turn == 0)
			{
				centre_y = -box_len*0.325f;
			}
			else centre_y = box_len*0.325f;
			if(turn == 0)
				angle = (float)(PI/2);
			else angle = (float)(3*PI/2);
			vel_x = 0.0f;
			vel_y = 0.0f;
			mag = 0.3f;
		}
		
		void drawcoin()
		{
			draw(0.1f, 0.8f, 0.0f);
		}

		void drawline() 
		{
    //		glTranslatef(centre_x, centre_y, 0.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			glPushAttrib(GL_ENABLE_BIT); 
			glLineStipple(6, 0xAAAA); 
			glEnable(GL_LINE_STIPPLE);
			glBegin(GL_LINES);
			glVertex2f(0.0f, 0.0f);
//			glVertex2f(2*(0.15f+mag)*cos(angle), 2*(0.15f+mag)*sin(angle));
			glVertex2f(6.0f*cos(angle), 6.0f*sin(angle));
			glEnd();
			glPopAttrib();
		}

		void impulse() 
		{
			vel_x = (float)mag*cos(angle);
			vel_y = (float)mag*sin(angle);
		}

		void incmag(float m, int increase)
		{
			if(increase && mag<= 0.60f){
				mag += m;
			}
			else if(!increase && mag>=0.01f){
				mag -= m;
			}
		}
		void incang(int increase)
		{ 
			if(turn == 0) {
				if(increase && angle < (PI - 15*PI/180)){
					angle = (angle+2*PI/180);
				}
				else if(!increase && angle > (15*PI/180)){
					angle = (angle-(2*PI)/180); 
				}
			}
			else {
				if(increase && angle < (2*PI - 15*PI/180)){
					angle = (angle+2*PI/180);
				}
				else if(!increase && angle > (PI+15*PI/180)){
					angle = (angle-(2*PI)/180); 
				}
			}
		}
		void setang(float a) {
			angle = a;
		}

		void setx(float a) {
			centre_x = a;
		}

		void setvel(float x, float y) {
			vel_x = x;
			vel_y = y;
		
		}
		float getmag() { return mag; }

		void setmag(float m) {
			mag = m;
		}

		friend void scollision(striker &c1, coin &c2);
};

class board {
	private:
		float len;
	public:
		board(float box_len){
			len = box_len;
		}
		void draw_layout()
		{
    //		glTranslatef(0.0f, 0.0f, -8.0f);
    		glColor3f(0.48f, 0.20f, 0.11f);
			draw_rec(len, 1);
    		glColor3f(1.0f, 1.0f, 0.80f);
			draw_rec(len*0.91, 1);
    		glColor3f(1.0f, 0.0f, 0.0f);
		//	draw_rec(len*0.70, 0);
			boundary(len);
			pockets(len);
			centre_circle(len);
			circles(len);
		}
		void boundary(float length)
		{
			glBegin(GL_LINES); //top
			glVertex2f(-len*1.0/3,len*1.05/3);
			glVertex2f(len*1.0/3,len*1.05/3);
			glEnd();

			glBegin(GL_LINES); //bottom
			glVertex2f(-len*1.0/3,-len*1.05/3);
			glVertex2f(len*1.0/3,-len*1.05/3);
			glEnd();

			glBegin(GL_LINES); //left
			glVertex2f(-len*1.05/3,len*1.0/3);
			glVertex2f(-len*1.05/3,-len*1.0/3);
			glEnd();
		
			glBegin(GL_LINES); //right
			glVertex2f(len*1.05/3,len*1.0/3);
			glVertex2f(len*1.05/3,-len*1.0/3);
			glEnd();
		}

		void draw_rec(float length, int fill)
		{
			if(fill)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	   		glBegin(GL_QUADS);
			glVertex2f(-length / 2, -length / 2);
			glVertex2f(length / 2, -length / 2);
			glVertex2f(length / 2, length / 2);
			glVertex2f(-length / 2, length / 2);
			glEnd();
		}
		void pockets(float length)
		{
			float rad = 0.16f;
			float offset = len*(1.29f/3.0f);
			
    		glColor3f(0.48f, 0.20f, 0.11f);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	   		glBegin(GL_QUADS);
			glVertex2f(offset, offset);
			glVertex2f(offset+rad, offset+rad);
			glVertex2f(offset+rad, offset);
			glVertex2f(offset, offset+rad);
			glEnd();
	   		glBegin(GL_QUADS);
			glVertex2f(-offset, -offset);
			glVertex2f(-offset-rad, -offset-rad);
			glVertex2f(-offset-rad, -offset);
			glVertex2f(-offset, -offset-rad);
			glEnd();
	   		glBegin(GL_QUADS);
			glVertex2f(offset, -offset);
			glVertex2f(offset+rad, -offset-rad);
			glVertex2f(offset+rad, -offset);
			glVertex2f(offset, -offset-rad);
			glEnd();
	   		glBegin(GL_QUADS);
			glVertex2f(-offset, offset);
			glVertex2f(-offset-rad, offset+rad);
			glVertex2f(-offset-rad, offset);
			glVertex2f(-offset, offset+rad);
			glEnd();
			

			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable( GL_BLEND );
			glColor4f(0.38f, 0.38f, 0.38f, 0.1f);
			glBegin(GL_TRIANGLE_FAN);
			for(double d = 0; d < 360; d += 1)
				glVertex2f(cos(d)*rad+offset, sin(d)*rad+offset);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			for(double d = 0; d < 360; d += 1)
				glVertex2f(cos(d)*rad-offset, sin(d)*rad+offset);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			for(double d = 0; d < 360; d += 1)
				glVertex2f(cos(d)*rad+offset, sin(d)*rad-offset);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			for(double d = 0; d < 360; d += 1)
				glVertex2f(cos(d)*rad-offset, sin(d)*rad-offset);
			glEnd();

//			glTranslatef(0.0f, 0.0f, 0.0f);
		}
		void circles(float length)
		{
			float rad = 0.12f;
			float offset = len*(0.99f/3.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			static const double inc = PI / 12;
			static const double max = 2 * PI;
			
			glBegin(GL_LINE_LOOP);
			for(double d = 0; d < max; d += inc)
				glVertex2f(cos(d)*rad+offset, sin(d)*rad+offset);
			glEnd();
			glBegin(GL_LINE_LOOP);
			for(double d = 0; d < max; d += inc)
				glVertex2f(cos(d)*rad-offset, sin(d)*rad+offset);
			glEnd();
			glBegin(GL_LINE_LOOP);
			for(double d = 0; d < max; d += inc)
				glVertex2f(cos(d)*rad-offset, sin(d)*rad-offset);
			glEnd();
			glBegin(GL_LINE_LOOP);
			for(double d = 0; d < max; d += inc)
				glVertex2f(cos(d)*rad+offset, sin(d)*rad-offset);
			glEnd();
//			glTranslatef(0.0f, 0.0f, 0.0f);
		}
		void centre_circle(float length)
		{
			float rad = (float)len/12;
			glColor3f(1.0f, 0.0f, 0.0f);
			static const double inc = PI / 12;
			static const double max = 2 * PI;
			glBegin(GL_LINE_LOOP);
			for(double d = 0; d < max; d += inc)
				glVertex2f(cos(d)*rad, sin(d)*rad);
			glEnd();
		}

};

void ccollision(coin &c1, coin &c2)
{
	if(sqrt(pow(c1.centre_x - c2.centre_x,2)+pow(c1.centre_y-c2.centre_y,2)) > 2*c1.rad)
	{
		if(sqrt(pow(c1.centre_x+c1.vel_x - c2.centre_x-c2.vel_x,2)+pow(c1.centre_y+c1.vel_y-c2.centre_y-c2.vel_y,2)) > 2*c1.rad)
			return ;
	}
	pair<float, float> dist = make_pair(c2.centre_x-c1.centre_x, c2.centre_y-c1.centre_y);
	pair<float, float> vel = make_pair(c2.vel_x-c1.vel_x, c2.vel_y-c1.vel_y);
	
	float vxcm = (c1.vel_x+c2.vel_x)/2;
	float vycm = (c1.vel_y+c2.vel_y)/2;

	float var;
	if(dist.first!=0)
		var=dist.second/dist.first;
	else
		var=0.1f;
	float dv = -2*(vel.first+var*vel.second)/(2*(1+var*var));

	c2.vel_x += dv;
	c2.vel_y += var*dv;
	c1.vel_x -= dv;
	c1.vel_y -= var*dv;

	//correction for inelastic collision
	c2.vel_x = (c2.vel_x - vxcm)*REST + vxcm;
	c2.vel_y = (c2.vel_y - vycm)*REST + vycm;
	c1.vel_x = (c1.vel_x - vxcm)*REST + vxcm;
	c1.vel_y = (c1.vel_y - vycm)*REST + vycm;

}

void scollision(striker &c1, coin &c2)
{
	if(sqrt(pow(c1.centre_x - c2.centre_x,2)+pow(c1.centre_y-c2.centre_y,2)) > c2.rad+c1.rad)
	{
		if(sqrt(pow(c1.centre_x+c1.vel_x - c2.centre_x-c2.vel_x,2)+pow(c1.centre_y+c1.vel_y-c2.centre_y-c2.vel_y,2)) > c2.rad+c1.rad)
			return ;
	}

	pair<float, float> dist = make_pair(c2.centre_x-c1.centre_x, c2.centre_y-c1.centre_y);
	pair<float, float> vel = make_pair(c2.vel_x-c1.vel_x, c2.vel_y-c1.vel_y);

	float vxcm = (2*c1.vel_x+c2.vel_x)/3;
	float vycm = (2*c1.vel_y+c2.vel_y)/3;

	// taken care of mass ratio
	float m21 = 0.50f;

	float var;
	if(dist.first!=0)
		var=dist.second/dist.first;
	else
		var=0.1f;
	float dv = -2*(vel.first+var*vel.second)/((1.0f+m21)*(1+var*var));

	c2.vel_x += dv;
	c2.vel_y += var*dv;
	c1.vel_x -= m21*dv;
	c1.vel_y -= m21*var*dv;
	
	//correction for inelastic collision
	c2.vel_x = (c2.vel_x - vxcm)*REST + vxcm;
	c2.vel_y = (c2.vel_y - vycm)*REST + vycm;
	c1.vel_x = (c1.vel_x - vxcm)*REST + vxcm;
	c1.vel_y = (c1.vel_y - vycm)*REST + vycm;
}

void drawpower(float mag)
{
    glTranslatef(0.0f, 0.0f, -8.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
    glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(3.5f,  -2.0f);
	glVertex2f(4.4f,  -2.0f);
    glColor3f(1.0f, 1.0f, 0.0f);
//	glVertex2f(4.9f,  7.84*mag-2.0f);
	glVertex2f(4.4f, 0.0f);
	glVertex2f(3.5f, 0.0f);
	glEnd();
	glBegin(GL_QUADS);
    glColor3f(1.0f, 1.0f, 0.0f);
	glVertex2f(3.5f,  0.0f);
	glVertex2f(4.4f,  0.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(4.4f, 2.0f);
	glVertex2f(3.5f, 2.0f);
	glEnd();

	glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0, 0.0f);
	glVertex2f(3.45f,  6.54*mag-2.0f);
	glVertex2f(4.45f,  6.54*mag-2.0f);
	glVertex2f(4.45f,  6.50*mag-1.96f);
	glVertex2f(3.45f,  6.50*mag-1.96f);
	glEnd();

}

void drawrec()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(3.0f,  -3.2f);
	glVertex2f(4.4f,  -3.2f);
	glVertex2f(5.89f, 3.2f);
	glVertex2f(3.0f, 3.2f);
	glEnd();
}

striker b(0.0f, -1.95f, 0.15f);
board myboard(6.0f);

vector<coin> coins;
vector<int> types, types2;
vector<coin> :: iterator it, it2, itend;

int main(int argc, char **argv) {

	gettimeofday(&mytime,NULL);
	coins.push_back(coin(1, 0.25f, 0.0f, 0.1f));
	coins.push_back(coin(1, -0.125f, 0.216f, 0.1f));
	coins.push_back(coin(1, -0.125f, -0.216f, 0.1f));
	coins.push_back(coin(0, -0.25f, 0.0f, 0.1f));
	coins.push_back(coin(0, 0.125f, 0.216f, 0.1f));
	coins.push_back(coin(0, 0.125f, -0.216f, 0.1f));
	coins.push_back(coin(2, 0.0f, 0.0f, 0.1f));
	vector<coin>::iterator it = coins.begin();

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    int w = glutGet(GLUT_SCREEN_WIDTH);
    int h = glutGet(GLUT_SCREEN_HEIGHT);
    int windowWidth = w * 4 / 5;
    int windowHeight = h * 4 / 5;

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition((w - windowWidth) / 2, (h - windowHeight) / 2);

    glutCreateWindow("CSE251_sampleCode");  // Setup the window
    initRendering();

    // Register callbacks
    glutDisplayFunc(drawScene);
    glutIdleFunc(drawScene);
    glutKeyboardFunc(handleKeypress1);
    glutSpecialFunc(handleKeypress2);
	glutMouseFunc(mouse_click);
//	glutMotionFunc(mouse_drag);
    glutReshapeFunc(handleResize);
    glutTimerFunc(1, update, 0);
    glutMainLoop();
    return 0;
}

void mouse_drag(int x, int y)
{
	if(!isclicked)
		return ;
	float px=((float)x);
	float py=((float)y);
	float maxh = (float)glutGet(GLUT_WINDOW_HEIGHT);
	float maxw = (float)glutGet(GLUT_WINDOW_WIDTH);
	float a = (py-(maxh/2))/(maxh);
	a*=6.62f;
	float ba = (px-(maxw/2))/(maxw);
	ba*=11.78f;
	if(ba < -2.0f || ba > 2.0f)
		return ;
	b.setx(ba);	
}

void mouse_click(int btn,int state,int x,int y)
{
	float px=((float)x);
	float py=((float)y);
	float maxh = (float)glutGet(GLUT_WINDOW_HEIGHT);
	float maxw = (float)glutGet(GLUT_WINDOW_WIDTH);
	py = (py-(maxh/2))/(maxh);
	py*=6.62f;
	px = (px-(maxw/2))/(maxw);
	px*=11.78f;
	float c = b.getcenx();
	if(btn==GLUT_RIGHT_BUTTON && state==GLUT_DOWN) 
	{
		if(turn == 0)
		{
			if(py>=1.80f && py<=2.12f && px<=(c+0.16f) && px>=(c-0.16f))
			{
				isclicked = 1;
				glutMotionFunc(mouse_drag);
			}
			else isclicked = 0;
		}
		else if(turn == 1)
		{
			if(py<=-1.80f && py>=-2.12f && px<=(c+0.16f) && px>=(c-0.16f))
			{
				isclicked = 1;
				glutMotionFunc(mouse_drag);
			}
			else isclicked = 0;
		}
	}
	else if(btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN)
		isclicked = 0;
	else if(btn==GLUT_LEFT_BUTTON && state==GLUT_UP)
	{
		isclicked = 0;
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		isstrikermoving = 1;
		if(turn == 0)
		{
			if(py<=1.81f && py>=-2.73f && px>=-2.73f && px<=2.73f)
			{
				float mag = sqrt(pow(1.95f-py, 2) + pow(px-c, 2));
				float diffy = 1.95f-py;
				float ang = asin(diffy/mag);
				if(px<=c)
					ang = (float)PI - ang;
				mag/=10.97f;
				b.setang(ang);
				b.setmag(mag);
				b.impulse();
			//	b.setvel(mag*cos(ang), mag*sin(ang));
			}
		}
		else if(turn == 1)
		{
			if(py>=-1.81f && py<=2.73f && px>=-2.73f && px<=2.73f)
			{
				float mag = sqrt(pow(1.95f-(-py), 2) + pow(px-c, 2));
				float diffy = 1.95f-(-py);
				float ang = asin(diffy/mag);
				if(px<=c)
					ang = (float)PI - ang;
				ang = 2*PI-ang; 
				mag/=10.97f;
				b.setang(ang);
				b.setmag(mag);
				b.impulse();
			//	b.setvel(mag*cos(ang), mag*sin(ang));
			}
		}
	}
}

// Function to draw objects on the screen
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glPushMatrix();
	float mag = b.getmag();
	drawpower(mag);
//	drawrec();
	drawrack();
    // Draw Box
    myboard.draw_layout();
    
	// Draw Striker
	if(ispocketed == 0)
	{
		glPushMatrix();
		b.drawcoin();
		int drawline = 1;
		for(int i=0;i<coins.size();i++)
		{
			if(abs(coins[i].getx()) > 1e-4 || abs(coins[i].gety()) > 1e-4)
			{
				drawline=0;
				break;
			}
		}
		if(abs(b.getx())>1e-4 || abs(b.gety())>1e-4)
			drawline = 0;
		if(drawline == 1)
			//	if(abs(b.getx())<1e-4 && abs(b.gety())<1e-4)
			b.drawline();
		glPopMatrix();
	}
	
	//Draw Coins
	it = coins.begin();
	itend = coins.end();
	for(int i=0;i<coins.size();i++)
	{
		glPushMatrix();
		coins[i].drawcoin();
		//coins[it].drawcoin();
		glPopMatrix();
	}

	char buffer[50];

	if(!isgameover)
	{
		sprintf(buffer, "Timer: %d sec", timer);	
		output(-4.9f, 0.0f, 0.0f, 0.0f, 0.0f, buffer);
	}

	sprintf(buffer, "Player 2: %d", game_score1);	
	output(-4.7f, 2.5f, 0.0f, 0.0f, 0.0f, buffer);
	sprintf(buffer, "(Black)");	
	output(-4.7f, 2.2f, 0.0f, 0.0f, 0.0f, buffer);
	
	sprintf(buffer, "Player 1: %d", game_score0);	
	output(-4.7f, -2.2f, 0.0f, 0.0f, 0.0f, buffer);
	sprintf(buffer, "(White)");	
	output(-4.7f, -2.5f, 0.0f, 0.0f, 0.0f, buffer);
	
	if(isgameover) 
	{
		sprintf(buffer, "Game Over!");	
		output(-5.2f, 0.0f, 0.0f, 0.0f, 0.0f, buffer);
		if(game_score0 > game_score1)
			sprintf(buffer, "Player 1 Wins");	
		else
			sprintf(buffer, "Player 2 Wins");	
		output(-5.27f, -0.3f, 0.0f, 0.0f, 0.0f, buffer);

	}
	
	glPopMatrix();

    glutSwapBuffers();
}

// Function to handle all calculations in the scene
// updated evry 10 milliseconds
void update(int value) 
{
	if(isgameover)
		return ;

	if(coins.size() == 0)
	{
		isgameover = 1;
	}
	int isonesecond = 0;
	gettimeofday(&tv,NULL);
	if(tv.tv_sec-mytime.tv_sec>=1 && abs(tv.tv_usec-mytime.tv_usec)<1e4)
	{
		isonesecond = 1;
		mytime.tv_sec = tv.tv_sec;
		mytime.tv_usec = tv.tv_usec;
		timer -= 1;
		if(timer < 0)
		{
		//	b.resetball();
		//	isstrikermoving = 0;
		//	goto A;
		}
	}
	int scored = 0;
	int resetstrk = 1;

	it = coins.begin();
	for(int i=0;i<coins.size();i++)
		scollision(b, coins[i]);
	for(int i=0;i<coins.size();i++)
		for(int j=i+1;j<coins.size();j++)
			ccollision(coins[i], coins[j]);

	b.pos_update(box_len*0.91f);
	if(b.pocket(box_len) && ispocketed==0)
	{
		if(turn == 0)
			game_score0 -= 10;
		else game_score1 -= 10;
		ispocketed = 1;
//		timer = 30;
//		resetball();
//		isstrikermoving = 0;
//		goto A;
	}
	for(int i=0;i<coins.size();i++)
	{
		coins[i].pos_update(box_len*0.91f);
		if(coins[i].pocket(box_len))
		{
			scored = 1;
			if(turn == 0) {
				game_score0 += coins[i].getscore().first;
				types.push_back(coins[i].gettype());
			}
			else {
				game_score1 += coins[i].getscore().second;
				types2.push_back(coins[i].gettype());
			}
			coins.erase(coins.begin()+i);
		}
	}
	if(scored == 1)
	{
	//	timer = 30;
	//	resetball();
	//	isstrikermoving = 0;
	//	goto A;
	}
	for(int i=0;i<coins.size();i++)
	{
		if(abs(coins[i].getx()) > 1e-3 || abs(coins[i].gety()) > 1e-3)
		{
			resetstrk=0;
			break;
		}
	}
	if(abs(b.getx())>1e-3 || abs(b.gety())>1e-3)
		resetstrk = 0;
	if(!isstrikermoving && resetstrk == 1 && isonesecond)
	{
		if(turn == 0)
			game_score0 = game_score0-1;
		else 
			game_score1 = game_score1-1;
	}
	if(resetstrk == 1 || timer < 0)
	{
		ispocketed = 0;
		if(isstrikermoving || timer < 0) {
			isstrikermoving = 0;
			b.resetball();
			timer = TURN_TIME;
			mytime.tv_sec = tv.tv_sec;
			mytime.tv_usec = tv.tv_usec;
		}
	}

	A:
	glutTimerFunc(1, update, 0);
}

void output(float x, float y, float r, float g, float b, char *string)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void draw(float x, float y, float r, float g, float b)
{
	float rad = 0.12f;
	glColor3f(r, g, b);
	glBegin(GL_TRIANGLE_FAN);
	for(int i=0 ; i<360 ; i++) {
		glVertex2f(x + rad * cos(DEG2RAD(i)), y + rad * sin(DEG2RAD(i)));
	}
	glEnd();
}

void drawrack()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(5.0f, -2.0f);
	glVertex2f(5.4f, -2.0f);
	glVertex2f(5.4f, 0.0f);
	glVertex2f(5.0f, 0.0f);
	glEnd();

	for(int i=0;i<types.size();i++)
	{
		if(types[i] == 1)
			draw(5.2f, -1.8f+0.3f*i, 0.847f, 0.694f, 0.392f);
		else if(types[i] == 0) 
			draw(5.2f, -1.8f+0.3f*i, 0, 0, 0);
		else draw(5.2f, -1.8f+0.3f*i, 1.0, 0, 0);
	}
	
	glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 0.0f);
	glVertex2f(5.0f, 0.0f);
	glVertex2f(5.4f, 0.0f);
	glVertex2f(5.4f, 2.0f);
	glVertex2f(5.0f, 2.0f);
	glEnd();

	for(int i=0;i<types2.size();i++)
	{
		if(types2[i] == 1)
			draw(5.2f, 0.2f+0.3f*i, 0.847f, 0.694f, 0.392f);
		else if(types2[i] == 0) 
			draw(5.2f, 0.2f+0.3f*i, 0, 0, 0);
		else draw(5.2f, 0.2f+0.3f*i, 1.0, 0, 0);
	}
}

// Initializing some openGL 3D rendering options
void initRendering() {

    glEnable(GL_DEPTH_TEST);        // Enable objects to be drawn ahead/behind one another
    glEnable(GL_COLOR_MATERIAL);    // Enable coloring
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);   // Setting a background color
}

// Function called when the window is resized
void handleResize(int w, int h) {

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w / (float)h, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void handleKeypress1(unsigned char key, int x, int y) {

    if(key == 27)
        exit(0);     // escape key is pressed
	else if(key == 32)
	{
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.impulse();
		isstrikermoving = 1;
	}
	else if(key == 97)
	{
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.incang(1);
	}
	else if(key == 99)
	{
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.incang(0);
	}
}

void handleKeypress2(int key, int x, int y) 
{
	if (key == GLUT_KEY_LEFT)
	{
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.setball(-0.05f);
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.setball(0.05f);
	}
	else if (key == GLUT_KEY_UP){
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.incmag(0.01f, 1);
	}
	else if (key == GLUT_KEY_DOWN){
		if(abs(b.getx())>0.0f || abs(b.gety())>0.0f)
			return ;
		b.incmag(0.01f, 0);
	}
}

