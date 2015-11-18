#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imageloader.h"
#include "vec3f.h"

#define pi 3.14156

using namespace std;

static const int conepoints = 362; //need 362, wrote 340 so that rotation is seen

int gamescore = 20;
int cameratype = 1; //for different camera views

class Target { //concentric circles with alternating red and white circles
	public:
		int circles = 9; // number of circles in the target
		uint vao[10], vbo[10];
		float sc = 0.5f, centerheight; //scalefactor
		float radius[10];
		GLfloat cone[10][conepoints*3];
		Target() 
		{
			for(int j=0;j<circles;j++)
			{
				radius[j] = sc*(-(float)j+circles);
				cone[j][0] = j/100.0f; //top point
				cone[j][2] = 0.0f;  //along z
				cone[j][1] = 0.0f; //fixed center as they are concentric
				for(int i=1;i<conepoints;i++)
				{
					cone[j][3*i+2] = radius[j]*cos(i*pi/180); //z
					cone[j][3*i+1] = radius[j]*sin(i*pi/180); //y
					cone[j][3*i+0] = j/100.0f; //x
				}
			}	
	
		}
		void init()
		{
			glGenVertexArrays(circles, &vao[0]);

			for(int j=0;j<circles;j++)
			{
				glBindVertexArray(vao[j]);
				glGenBuffers(1, vbo+j);

				// VBO for cone1
				glBindBuffer(GL_ARRAY_BUFFER, vbo[j]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(cone[j]), cone[j], GL_STATIC_DRAW);
				glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
				glEnableVertexAttribArray(0);	
			}
		}
		void draw()
		{
			for(int j=0;j<circles;j++)
			{
				glBindVertexArray(vao[j]);	// First VAO
				if(j%2 == 0)
					glVertexAttrib3f((GLuint)1, 1.0, 0.0, 0.0); // set constant color attribute
				else 
					glVertexAttrib3f((GLuint)1, 1.0, 1.0, 1.0); // set constant color attribute
				glDrawArrays(GL_TRIANGLE_FAN, 0, conepoints);	// draw first object
			}
		}
}gametarget;

class terrain {
	public:
		int w, l, counter;  //width, length, number of points 
		GLfloat *arr, *color, **height;  //height, colour at each point
		Vec3f** normals; // normals
		float weight; //how much weightage to give to height
		unsigned int vao[2], vbo[2];
		bool computedNormals = false; //Whether normals is up-to-date

		void setHeight(int x, int y, float h) {
			arr[counter++] = x;
			arr[counter++] = h;
			arr[counter++] = y;	
        }
        void setColor(float h) {
        	counter -= 3;
        	// values range from -weight/2 to weight/2, so normalized it
			color[counter++] = (h+weight/2)/weight; 
			color[counter++] = (h+weight/2)/weight;
			color[counter++] = (h+weight/2)/weight;
			//	color[counter++] = 0;
        }
        float getHeight(int r, int c) { return height[r][c]; }
        int getWidth() { return w; }
        int getLength() { return l; }
		terrain()
		{
			counter = 0;
			weight = 5.0;

			Image* image = loadBMP("heightmap.bmp");
			l = image->height;
			w = image->width;

			float h;
			height = new float*[l];
			for(int i=0;i<l;i++)
				height[i] = new float[w];

			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;

			for(int i=0;i<l;i++) //reading the data into an array
				for(int j=0;j<w;j++)
				{
					unsigned char color = (unsigned char)image->pixels[3 * (i * image->width + j)];
					h = weight * ((color / 255.0f) - 0.5f); // -weight/2 to +weight/2
					height[i][j] = h;  //stores the height at x, y
				}
			arr = new GLfloat[2*l*w*3]; //stores the order for strip, 3 values for a vertice 
			color = new GLfloat[2*l*w*3];
            int turn = 0, rev = 0;
            int x=0;
            for(int y = 0; y < l-1; y++) 
            {
            	if(rev%2 == 0) {
            		for(x=0;x<w;x++)
            		{
            			h = (height[y][x]);	
                   		setHeight(x, y, h);
                   		setColor(h);

	        			h = (height[y+1][x]);	
                   		setHeight(x, y+1, h);
                   		setColor(h);
            		}
            	}
            	else if(rev%2) {
            		for(x=w-1;x>=0;x--)
            		{
            			h = (height[y][x]);	
                   		setHeight(x, y, h);
                   		setColor(h);

            			h = (height[y+1][x]);	
                   		setHeight(x, y+1, h);
                   		setColor(h);
            		}
            	}
            	rev = !rev;
             }
             delete image;
		}
		void init()
		{
			glGenVertexArrays(1, &vao[0]);

			glBindVertexArray(vao[0]);
			glGenBuffers(2, vbo);

			//vbo for terrain
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*w*l*3, arr, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(0);

			//colour data for terrain
			glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*2*w*l*3, color, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(1);
		}
		void draw()
		{
			glBindVertexArray(vao[0]);	
			glDrawArrays(GL_TRIANGLE_STRIP, 0, counter/3);	// draw first object
		}
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, height[z - 1][x] - height[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, height[z + 1][x] - height[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, height[z][x - 1] - height[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, height[z][x + 1] - height[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z)
		{
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
}ter;


class spinning_top {
	public:
		float magnitude, direction, vel_x, vel_y;
		float sc = 0.8f; //scalefactor
		float x, z, h; 
		float outrad = sc*2.0f, baselen = sc*2.0f, handlelen = sc*3.0f, inrad = sc*outrad*0.23f;
		GLfloat cone1[conepoints*3], cone2[conepoints*3], cone3[conepoints*3];
		unsigned int vao[3], vbo[3];
		spinning_top(void)
		{
			magnitude = direction = 0;
			z = 0;
			x = ter.getWidth()/2 - 1.1;
			h = ter.getHeight(x+ter.w/2, z+ter.l/2);
			cone1[0] = 0.0f; // cone top point
			cone1[1] = 0.0f;
			cone1[2] = 0.0f;

			for(int i=1;i<conepoints;i++)
			{
				cone1[3*i] = outrad*cos(i*pi/180); //x
				cone1[3*i+2] = outrad*sin(i*pi/180); //z
				cone1[3*i+1] = baselen; //y
			}	

			cone2[0] = 0.0f; // cone top point
			cone2[1] = baselen*1.3f;
			cone2[2] = 0.0f;

			for(int i=1;i<conepoints;i++)
			{
				cone2[3*i] = outrad*cos(i*pi/180); //x
				cone2[3*i+2] = outrad*sin(i*pi/180); //z
				cone2[3*i+1] = baselen; //y
			}	

			cone3[0] = 0.0f; // cone top point
			cone3[1] = handlelen+2*baselen;
			cone3[2] = 0.0f;

			for(int i=1;i<conepoints;i++)
			{
				cone3[3*i] = inrad*cos(i*pi/180); //x
				cone3[3*i+2] = inrad*sin(i*pi/180); //z
				cone3[3*i+1] = baselen; //y
			}			
		}
		void init()
		{
			glGenVertexArrays(3, &vao[0]);

			glBindVertexArray(vao[0]);
			glGenBuffers(1, vbo);

			// VBO for cone1
			glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cone1), cone1, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(0);

			glBindVertexArray(vao[1]);
			glGenBuffers(1, &vbo[1]);

			// VBO for cone2
			glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cone2), cone2, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(0);			

			glBindVertexArray(vao[2]);
			glGenBuffers(1, &vbo[2]);

			// VBO for cone3
			glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(cone3), cone3, GL_STATIC_DRAW);
			glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0); 
			glEnableVertexAttribArray(0);	
		}
		void draw(void)
		{
			for(int i=0;i<3;i++)
			{
				glBindVertexArray(vao[i]);	// First VAO
				if(!i)
					glVertexAttrib3f((GLuint)1, 1.0, 0.0, 0.0); // set constant color attribute
				else if(i&1) 
					glVertexAttrib3f((GLuint)1, 0.0, 1.0, 0.0); // set constant color attribute
				else 
					glVertexAttrib3f((GLuint)1, 0.0, 0.0, 1.0); // set constant color attribute
				glDrawArrays(GL_TRIANGLE_FAN, 0, conepoints);	// draw first object
			}
		}
		void update()
		{
			double frac, intp;
			x += vel_x;
			frac = abs(modf(x, &intp));
			if(x>=0)
			{
				h = (ter.getHeight(x+(ter.w)/2, z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(ceil(x)+(ter.w)/2, z+(ter.l)/2));
			}
			else {
				h = (ter.getHeight(x+(ter.w)/2, z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(floor(x)+(ter.w)/2, z+(ter.l)/2));
			}
			z += vel_y;
			frac = abs(modf(z, &intp));
			if(z>=0){
				h = (ter.getHeight(x+(ter.w)/2, ceil(z)+(ter.l)/2)*frac + (1-frac)*ter.getHeight(x+(ter.w)/2, z+(ter.l)/2));
			}
			else
				h = (ter.getHeight(x+(ter.w)/2, floor(z)+(ter.l)/2)*frac + (1-frac)*ter.getHeight(x+(ter.w)/2, z+(ter.l)/2));	
		}

};


struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

int haveshot = 0;

float camera_rotation_angle = 0, toprotation = 0;
uint programID;

void reshape(int, int);

#include "shaders.cpp"

spinning_top top = spinning_top();
//Target gametarget = Target();
//terrain ter = terrain();

int targetx, targetz; float targetrot;
	
void resetgame()
{
	haveshot = 0;
	top = spinning_top();
	top.init();
	gametarget = Target();
	gametarget.init();

	targetx = rand()%(ter.getWidth()-2*gametarget.circles)+gametarget.circles;
	targetz = 1.0f;
	targetrot = -90;

}

void init(void)
{
	targetx = rand()%(ter.getWidth()-2*gametarget.circles)+gametarget.circles;
	targetz = 1.0f;
	targetrot = -90;

	ter.init();
	top.init();
	gametarget.init();

	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	reshape(600, 600);

	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LESS);
}

void display(void)
{

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(programID);

	// check collision with target 
	if(abs(targetz - ter.getLength()/2 - top.x)<1e-1 && abs(targetx - ter.getWidth()/2 - top.z)<gametarget.circles){
		gamescore += 10;
		resetgame();
	}

	//camera views

	if(cameratype == 1) // static player view
	{			
		glm::vec3 eye ( 0, 40, 65 );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0); // lies in perpendicular plane to  the vector formed by eye and target
		Matrices.view = glm::lookAt( eye, target, up );
	}
	else if(cameratype == 2) // top view
	{
		glm::vec3 eye ( 0, 80, 0 );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 0, -1);
		Matrices.view = glm::lookAt( eye, target, up );
	}
	else if(cameratype == 3) // following the top
	{
		glm::vec3 eye ( top.z-6.0f, top.h+top.baselen*2+top.handlelen, top.x-6.0f );
		glm::vec3 target (top.z, top.h+top.handlelen, top.x);
		glm::vec3 up (0, 1, 0);
		Matrices.view = glm::lookAt( eye, target, up );
	}
	else if(cameratype == 4){ // looking at the target from the top													// Top view
		glm::vec3 eye (top.z, top.h+top.baselen*2+top.handlelen, top.x);
		glm::vec3 target (targetx-ter.getWidth()/2, 0, targetz-ter.getLength()/2);
		glm::vec3 up (0, 1, 0);
		Matrices.view = glm::lookAt( eye, target, up );
	}
	if(cameratype == 5) // camera rotating around the terrain
	{			
		glm::vec3 eye ( 50*sin(camera_rotation_angle*M_PI/180.0f), 35, 50*cos(camera_rotation_angle*M_PI/180.0f) );
		glm::vec3 target (0, 0, 0);
		glm::vec3 up (0, 1, 0); // lies in perpendicular plane to  the vector formed by eye and target
		Matrices.view = glm::lookAt( eye, target, up );
		camera_rotation_angle++;
	}
 	//camera views end

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	glm::mat4 VP = Matrices.projection * Matrices.view;
	glm::mat4 MVP;  // MVP = Projection * View * Model

	// transformation on target
	Matrices.model = glm::mat4(1.0f); // load identity matrix
	glm::mat4 translateTarget = glm::translate (glm::vec3(targetx-ter.getWidth()/2, (float)gametarget.sc*gametarget.circles+ter.getHeight(targetz, targetx), targetz-ter.getLength()/2) ); // glTranslatef 
	glm::mat4 rotateTarget = glm::rotate (targetrot, glm::vec3(0, 1, 0) ); 
    Matrices.model *= translateTarget * rotateTarget;
	MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	gametarget.draw();


	// transformation on terrain
	Matrices.model = glm::mat4(1.0f); // load identity matrix
	glm::mat4 translateTerrain = glm::translate (glm::vec3(-ter.getWidth()/2, -0.0f, -ter.getLength()/2) ); // glTranslatef
    Matrices.model *= translateTerrain;
	MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	ter.draw();


	//transformation on top	
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateTop = glm::translate (glm::vec3(top.z, top.h, top.x)); // glTranslatef
	glm::mat4 rotateTop = glm::rotate (toprotation, glm::vec3(0, 1, 0) ); 
	Vec3f v = ter.getNormal((int)top.x + (ter.w)/2, (int)top.z + (ter.l)/2);
	glm::vec3 normal = glm::normalize(glm::vec3(v[0], v[1], v[2]));
	glm::mat4 alignTop = glm::orientation(glm::vec3(0, 1, 0), normal); 
    Matrices.model *= translateTop * alignTop;// * rotateTop; // p * V * M;
	MVP = VP * Matrices.model; 
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	top.draw();

	// check whether it has reached the boundary
	if(abs(abs(top.x)-ter.getWidth()/2+1) < 1e-1)
	{
		gamescore-=1;
		printf("Game Over. Your score is %d.\n", gamescore);
		resetgame();
	}
	else if(abs(abs(top.z)-ter.getLength()/2+1) < 1e-1)
	{
		gamescore-=1;
		printf("Game Over. Your score is %d.\n", gamescore);
		resetgame();
	}
	top.update();

	glBindVertexArray(0);
	glutSwapBuffers();

	toprotation = 0;
}

void reshape(int w, int h)
{
	GLfloat fov = 45.0f;
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
	Matrices.projection = glm::perspective (fov, (GLfloat) w / (GLfloat) h, 0.1f, 500.0f);
}

void keyboardDown (unsigned char key, int x, int y)
{
	switch (key) {
		case 'Q':
		case 'q':
		case 27: //ESC
			exit (0);
			break;
		case '1':
			cameratype = 1;
			break;
		case '2':
			cameratype = 2;
			break;
		case '3':
			cameratype = 3;
			break;
		case '4':
			cameratype = 4;
			break;
		case '5':
			cameratype = 5;
			break;
	}
	if(haveshot) return;
	switch(key) {
		case 'w':
			top.magnitude -= 0.01;
			break;
		case 's': 
			top.magnitude += 0.01;
			break;
		case 'a': 
			top.direction += 4;
			break;
		case 'c':
			top.direction -= 4;
			break;
		case 32: // space
			haveshot = 1;
			top.vel_x = top.magnitude*cos(top.direction*pi/180);
			top.vel_y = top.magnitude*sin(top.direction*pi/180);
			break;
		default:
			break;
	}
}

void handleKeypress(int key, int x, int y)
{
	double frac, intp;
	switch (key) {
		case GLUT_KEY_UP: top.x -= 1*0.1;
				frac = abs(modf(top.x, &intp));
				if(top.x>=0)
				{
					top.h = (ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(ceil(top.x)+(ter.w)/2, top.z+(ter.l)/2));
				}
				else {
					top.h = (ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(floor(top.x)+(ter.w)/2, top.z+(ter.l)/2));
				}
				break;
		case GLUT_KEY_DOWN: top.x += 1*0.1;
				frac = abs(modf(top.x, &intp));
				if(top.x>=0){
					top.h = (ter.getHeight(ceil(top.x)+(ter.w)/2, top.z+(ter.l)/2)*frac + (1-frac)*ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2));
				}
				else 
					top.h = (ter.getHeight(floor(top.x)+(ter.w)/2, top.z+(ter.l)/2)*frac + (1-frac)*ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2));
				break;
		case GLUT_KEY_RIGHT: top.z += 1*0.1;
				frac = abs(modf(top.z, &intp));
				if(top.z>=0){
					top.h = (ter.getHeight(top.x+(ter.w)/2, ceil(top.z)+(ter.l)/2)*frac + (1-frac)*ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2));
				}
				else
					top.h = (ter.getHeight(top.x+(ter.w)/2, floor(top.z)+(ter.l)/2)*frac + (1-frac)*ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2));
				break;
		case GLUT_KEY_LEFT: top.z -= 1*0.1;
				frac = abs(modf(top.z, &intp));
				if(top.z>=0){
					top.h = (ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(top.x+(ter.w)/2, ceil(top.z)+(ter.l)/2));
				}   
				else
					top.h = (ter.getHeight(top.x+(ter.w)/2, top.z+(ter.l)/2)*(1-frac) + frac*ter.getHeight(top.x+(ter.w)/2, floor(top.z)+(ter.l)/2));
				break;
		default:
			break;
	}
}

int main (int argc, char* argv[])
{
	srand(time(NULL));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitContextVersion (3, 3); // Init GL 3.3
	glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
	glutInitWindowSize(600,600);
	glutCreateWindow("Game");
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << "glewInit failed, aborting." << endl; // Problem: glewInit failed, something is seriously wrong.
		exit (1);
	}
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "OpenGL version " << glGetString(GL_VERSION) << " supported" << endl;

	glutKeyboardFunc (keyboardDown);
 //   glutSpecialFunc(handleKeypress);


	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);	

	init();
	glutMainLoop();

	return 0;
}
