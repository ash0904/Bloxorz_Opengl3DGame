#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
#include <stdlib.h>

#include <thread>
#include <ao/ao.h>
#include <mpg123.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define BITS 8
using namespace std;

typedef struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
}VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices,Matrices2;

typedef struct COLOR {
    float r;
    float g;
    float b;
} COLOR;
int match_color(COLOR c1,COLOR c2)
{
  if(abs(c1.r-c2.r)<0.00001 &&
      abs(c1.g-c2.g)<0.00001 &&
        abs(c1.b-c2.b)<0.00001)
        return 1;
    else
    return 0;

}

COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
COLOR red = {255.0/255.0,51.0/255.0,51.0/255.0};
COLOR lightgreen = {57/255.0,230/255.0,0/255.0};
COLOR lightblue ={0/255.0, 170.0/255.0, 255/255.0};
COLOR darkgreen = {51/255.0,102/255.0,0/255.0};
COLOR black = {0,0,0};
COLOR blue = {0,0,1};
COLOR green = {1.0/255.0,255.0/255.0,1.0/255.0};
COLOR darkbrown = {46/255.0,46/255.0,31/255.0};
COLOR lightbrown = {95/255.0,63/255.0,32/255.0};
COLOR lightpink = {255/255.0,122/255.0,173/255.0};
COLOR darkpink = {255/255.0,51/255.0,119/255.0};
COLOR white = {255/255.0,255/255.0,255/255.0};

typedef struct Sprite {
    string name;          // name of object
    COLOR color;          // color of object
    float x,y,z;          // translation co-odinates
    float sx,sy,sz;       // scalling co-odinates
    VAO* object;
    VAO* boundary; // shape of object
    int key_press;           // doubt???
    int status;           // for objects to be hidden initially
    float height,width,depth;
    float dx,dy,dz;          // amount to be moved
    float rot_angle,anglez;
    int inAir;            // boolean 0 or 1
    int fixed;            // boolean 0 or 1
    int isMoving;         // boolean 0 or 1
    int horiz,horix;
} Sprite;


map <int, Sprite> sboard;
int do_rot,floor_fl0[10][10]={{1,0,0,1,0,0,0,0,0,0},
                              {1,13,13,1,0,0,0,0,0,0},
                              {1,1,1,5,1,1,0,0,0,0},
                              {1,1,1,5,1,1,1,1,0,0},
                              {1,0,0,0,1,1,1,1,0,0},
                              {1,0,0,0,0,1,2,1,0,0},
                              {1,1,1,3,0,1,1,1,0,0},
                              {0,0,0,0,0,1,1,1,1,1},
                              {0,0,0,0,0,0,0,1,1,1},
                              {0,0,0,0,0,0,0,1,1,1}},
            floor_fl1[10][10] = {{1,1,1,5,0,0,0,0,0,0},
                                {1,5,1,1,13,14,0,0,0,0},
                                {1,5,1,1,13,14,5,1,1,0},
                                {0,1,1,1,13,14,5,1,1,1},
                                {0,1,0,0,0,5,1,2,1,1},
                                {0,1,0,0,0,0,1,1,1,0},
                                {0,1,1,1,3,0,0,0,0,0},
                                {0,0,0,0,1,0,0,0,0,0},
                                {0,0,0,0,1,0,0,0,0,0},
                                {0,0,0,0,4,0,0,0,0,0}};
int rox=0,roy=0,roz=0,iangle=0,fangle=0,fl1=0,fl2=0,level=1;
GLuint programID;
double last_update_time, current_time;
float camera_fov=M_PI/2;
Sprite floor_mat[10][10],block,xblock,zblock;

// void* play_audio(string audioFile);
mpg123_handle *mh,*mh2;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;

int driver;
ao_device *dev;

ao_sample_format format;
int channels, encoding;
long rate;

void audio_init() {
    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    mh2 = mpg123_new(NULL, &err);
    buffer_size= 3000;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    /* open the file and get the decoding format */
    mpg123_open(mh, "./back1.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);
    mpg123_open(mh2, "./back.mp3");
    mpg123_getformat(mh2, &rate, &channels, &encoding);


    /* set the output format and open the output device */
    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}

void audio_play(int lev) {
    /* decode and play */
    if(lev==1)
    {
      if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
          ao_play(dev, (char*) buffer, done);
      else mpg123_seek(mh, 0, SEEK_SET);
    }
    else if(lev==2)
    {
      if (mpg123_read(mh2, buffer, buffer_size, &done) == MPG123_OK)
          ao_play(dev, (char*) buffer, done);
      else mpg123_seek(mh2, 0, SEEK_SET);
    }
}

void audio_close() {
    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_close(mh2);
    mpg123_delete(mh2);
    mpg123_exit();
    ao_shutdown();
}


mpg123_handle *mh1;
unsigned char *buffer1;
size_t buffer_size1;
size_t done1;
int err1;

int driver1;
ao_device *dev1;

ao_sample_format format1;
int channels1, encoding1;
long rate1;

void audio1_init() {
    /* initializations */
    ao_initialize();
    driver1 = ao_default_driver_id();
    mpg123_init();
    mh1 = mpg123_new(NULL, &err1);
    buffer_size1= 3000;
    buffer1 = (unsigned char*) malloc(buffer_size1 * sizeof(unsigned char));

    /* open the file and get the decoding format */
	    mpg123_open(mh1,"./sound.mp3");
    mpg123_getformat(mh1, &rate1, &channels1, &encoding1);

    /* set the output format and open the output device */
    format1.bits = mpg123_encsize(encoding1) * BITS;
    format1.rate = rate1;
    format1.channels = channels1;
    format1.byte_format = AO_FMT_NATIVE;
    format1.matrix = 0;
    dev1 = ao_open_live(driver1, &format1, NULL);
}

void audio1_play() {
    /* decode and play */
    if (mpg123_read(mh1, buffer1, buffer_size1, &done1) == MPG123_OK)
        ao_play(dev1, (char*) buffer1, done1);
    else mpg123_seek(mh1, 0, SEEK_SET);
}

void audio1_close() {
    /* clean up */
    free(buffer1);
    ao_close(dev1);
    mpg123_close(mh1);
    mpg123_delete(mh1);
    mpg123_exit();
    ao_shutdown();
}

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
	{
	    std::string Line = "";
	    while(getline(VertexShaderStream, Line))
		VertexShaderCode += "\n" + Line;
	    VertexShaderStream.close();
	}

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
	std::string Line = "";
	while(getline(FragmentShaderStream, Line))
	    FragmentShaderCode += "\n" + Line;
	FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    //    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    //    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    //    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void initGLEW(void){
    glewExperimental = GL_TRUE;
    if(glewInit()!=GLEW_OK){
	fprintf(stderr,"Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
    }
    if(!GLEW_VERSION_3_3)
	fprintf(stderr, "3.3 version not available\n");
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
  struct VAO* vao = new struct VAO;
  vao->PrimitiveMode = primitive_mode;
  vao->NumVertices = numVertices;
  vao->FillMode = fill_mode;

  // Create Vertex Array Object
  // Should be done after CreateWindow and before any other GL calls
  glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
  glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
  glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

  glBindVertexArray (vao->VertexArrayID); // Bind the VAO
  glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
  glVertexAttribPointer(
               0,                  // attribute 0. Vertices
               3,                  // size (x,y,z)
               GL_FLOAT,           // type
               GL_FALSE,           // normalized?
               0,                  // stride
               (void*)0            // array buffer offset
               );

  glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
  glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
  glVertexAttribPointer(
               1,                  // attribute 1. Color
               3,                  // size (r,g,b)
               GL_FLOAT,           // type
               GL_FALSE,           // normalized?
               0,                  // stride
               (void*)0            // array buffer offset
               );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float rectangle_rot_dir = 1;
bool rectangle_rot_status = true;
int tp=0,bp=0,vp=0,fp=0,hp=0,cp=0,steps=0;
float camera_rotation_angle = 90;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
      if(key!=GLFW_KEY_H)
        camera_rotation_angle=0;
        switch (key) {
	case GLFW_KEY_B:
      if(bp<4)
      {tp=0;bp++;vp=0;fp=0;hp=0;cp=0;}
      else
      {
        tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;
      }
	    break;
	case GLFW_KEY_T:
      if(tp==1)
      {tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;}
      else
      {
        tp=1;bp=0;vp=0;fp=0;hp=0;cp=0;
      }
	    break;
  case GLFW_KEY_V:
      if(vp==1)
      {tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;}
      else{
        tp=0;bp=0;vp=1;fp=0;hp=0;cp=0;
      }
      break;
  case GLFW_KEY_F:
      if(fp==1)
      {tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;}
      else{
        tp=0;bp=0;vp=0;fp=1;hp=0;cp=0;
      }
      break;
  case GLFW_KEY_H:
      if(hp==1)
      {tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;}
      else{
        tp=0;bp=0;vp=0;fp=0;hp=1;cp=0;
      }
      break;
  case GLFW_KEY_C:
      if(cp==1)
      {tp=0;bp=0;vp=0;fp=0;hp=0;cp=0;}
      else{
        tp=0;bp=0;vp=0;fp=0;hp=0;cp=1;
      }
      break;
  case GLFW_KEY_RIGHT:
      if( roz==0 && rox==0)
      {
        audio1_init();
        steps++;
        roz=-1;
        iangle=block.anglez;
      }
      break;
  case GLFW_KEY_LEFT:
      if( roz==0 && rox==0)
      {
        audio1_init();
        steps++;
        roz=1;
        iangle=block.anglez;
      }
      break;
  case GLFW_KEY_UP:
      if( roz==0 && rox==0)
      {
        audio1_init();
        steps++;
        rox=-1;
        iangle=block.rot_angle;
      }
      break;
  case GLFW_KEY_DOWN:
      if( roz==0 && rox==0)
        {
          audio1_init();
          steps++;
          rox=1;
          iangle=block.rot_angle;
        }
      break;
	default:
	    break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
        	case GLFW_KEY_ESCAPE:
        	    quit(window);
        	    break;
        	default:
        	    break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
    case 'Q':
    case 'q':
	     quit(window);
	     break;
    case ' ':
	// do_rot ^= 1;
	   break;
    default:
	     break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
    case GLFW_MOUSE_BUTTON_RIGHT:
	if (action == GLFW_RELEASE) {
	    rectangle_rot_dir *= -1;
	}
	break;
    default:
	break;
    }
}




/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = camera_fov;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 1000.0f);
    Matrices2.projection = glm::ortho((float)(-500.0f), (float)(500.0f), (float)(-350.0f), (float)(350.0f), 0.1f, 500.0f);
}
void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset==-1) {
        camera_fov*=1.1;
    }
    else if(yoffset==1){
        camera_fov/=1.1; //make it bigger than current size
    }
    if(camera_fov>=2){
    	camera_fov=2;
    }
    if(camera_fov<=0.5){
    	camera_fov=0.5;
    }
    reshapeWindow(window,800,450);
}

VAO *base, *baseBoundary, *floor_vao;

// Creates the rectangle object used in this sample code
void createCube (float wd,Sprite *obj,COLOR c1)
{
    // GL3 accepts only Triangles. Quads are not supported
     GLfloat vertex_buffer_data [] = {
  -wd/2, wd/2, wd/2,
	-wd/2, -wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	-wd/2, wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	wd/2, wd/2, wd/2,
	wd/2, wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, wd/2, wd/2,
	wd/2, wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	wd/2, wd/2, wd/2,
	wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, -wd/2, -wd/2,
	wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, -wd/2, wd/2,
    };

     GLfloat color_buffer_data [] = {
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
       c1.r, c1.g, c1.b,
    };
    GLfloat boundary_buffer_data [] = {
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
   };

    // create3DObject creates and returns a handle to a VAO that can be used later
    base = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
    baseBoundary = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, boundary_buffer_data, GL_LINE);
    obj->object = base;
    obj->boundary = baseBoundary;
}
void createBlock (float wd,Sprite *obj)
{
    // GL3 accepts only Triangles. Quads are not supported
     GLfloat vertex_buffer_data [] = {
  -wd/2, wd/2, wd/2,
	-wd/2, -wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	-wd/2, wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	wd/2, wd/2, wd/2,
	wd/2, wd/2, wd/2,
	wd/2, -wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	-wd/2, wd/2, wd/2,
	wd/2, wd/2, wd/2,
	-wd/2, wd/2, -wd/2,
	wd/2, wd/2, wd/2,
	wd/2, wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	-wd/2, -wd/2, -wd/2,
	wd/2, -wd/2, -wd/2,
	-wd/2, -wd/2, wd/2,
	wd/2, -wd/2, -wd/2,
	wd/2, -wd/2, wd/2,
    };

     GLfloat color_buffer_data [] = {
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 1.0f, 0.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 1.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
    };
    GLfloat boundary_buffer_data [] = {
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f,
   };

    // create3DObject creates and returns a handle to a VAO that can be used later
    base = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
    baseBoundary = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_LINE);
    obj->object = base;
    obj->boundary = baseBoundary;
}

void check_score(GLFWwindow* window)
{
  int i,o,t,nf=0;
  for(i=1;i<=15;i++)
    sboard[i].status=0;
  o=steps%10;
  t=steps/10;
  if(o==0 || o==2 || o==3 || o==5 || o==6 || o==7 || o==8 || o==9)
    sboard[1].status=1;
  if(o==0 || o==1 || o==2 || o==3 || o==4 || o==7 || o==8 || o==9)
    sboard[2].status=1;
  if(o==0 || o==1 || o==3 || o==4 || o==5 || o==6 || o==7 || o==8 || o==9)
    sboard[3].status=1;
  if(o==0 ||o==2 || o==3 || o==5 || o==6 || o==8 || o==9)
    sboard[4].status=1;
  if(o==0 || o==2 || o==6 || o==8)
    sboard[5].status=1;
  if(o==0  || o==4 || o==5 || o==6 || o==8 || o==9)
    sboard[6].status=1;
  if( o==2 || o==3 || o==4 || o==5 || o==6 || o==8 || o==9 )
    sboard[7].status=1;
  if(t==0 || t==2 || t==3 || t==5 || t==6 || t==7 || t==8 || t==9)
    sboard[8].status=1;
  if(t==0 || t==1 || t==2 || t==3 || t==4 || t==7 || t==8 || t==9)
    sboard[9].status=1;
  if(t==0 || t==1 || t==3 || t==4 || t==5 || t==6 || t==7 || t==8 || t==9)
    sboard[10].status=1;
  if(t==0 ||t==2 || t==3 || t==5 || t==6 || t==8 || t==9)
    sboard[11].status=1;
  if(t==0 || t==2 || t==6 || t==8)
    sboard[12].status=1;
  if(t==0 || t==4 || t==5 || t==6 || t==8 || t==9)
    sboard[13].status=1;
  if( t==2 || t==3 || t==4 || t==5 || t==6 || t==8 || t==9 )
    sboard[14].status=1;
}

void display(Sprite obj,glm::mat4 VP)
{
  glm::mat4 MVP;	// MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3( obj.x, obj.y, obj.z));        // glTranslatef
  glm::mat4 scaleMat = glm::scale(glm::vec3 (obj.sx,obj.sy,obj.sz));
  Matrices.model *= (translateRectangle * scaleMat);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj.object);
  draw3DObject(obj.boundary);
}

void display_block(Sprite obj,glm::mat4 VP)
{
  glm::mat4 MVP;	// MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3( obj.x, obj.y, obj.z));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(0.0f), glm::vec3(0,0,1));
  if(abs(roz) || obj.horiz)
    rotateRectangle = glm::rotate((float)(obj.anglez*M_PI/180.0f), glm::vec3(0,0,1));
  if(abs(rox) || obj.horix)
    rotateRectangle = glm::rotate((float)(obj.rot_angle*M_PI/180.0f), glm::vec3(1,0,0));

  glm::mat4 scaleMat = glm::scale(glm::vec3 (obj.sx,obj.sy,obj.sz));
  Matrices.model *= (translateRectangle *rotateRectangle* scaleMat);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj.object);
  draw3DObject(obj.boundary);
}

void check(GLFWwindow* window)
{
  float bx=block.x,by=block.y,bz=block.z,tilw=floor_mat[0][0].width;
  if(bx>=5*tilw || bx<=-5*tilw || by>=5*tilw || by<=-5*tilw || bz>=5*tilw || bz<=-5*tilw )
    quit(window);
  int ax=bx/tilw,ay=by/tilw,az=bz/tilw;

  for(ax=0;ax<10;ax++)
  {
    for(az=0;az<10;az++)
    {
      int tem;
      if(level==1)
      {
        tem=floor_fl0[ax][az];
      }
      else if(level==2)
      {
        tem=floor_fl1[ax][az];
      }

      if(tem==0 || tem==13)
        if(abs(floor_mat[ax][az].x-bx)<=tilw/2 && abs(floor_mat[ax][az].z-bz)<=tilw/2)
          {
            cout<<"Oops!\n";
            quit(window);
          }
      if(tem==2 && block.horiz==0 && block.horix==0)
        if(abs(floor_mat[ax][az].x-bx)<=tilw/2 && abs(floor_mat[ax][az].z-bz)<=tilw/2)
        {
          if(level != 2)
          {
            steps=0;
            cout<<"Congrats promoted to next level\n";
            block.x = floor_mat[0][0].x;
            block.z = floor_mat[0][0].z;
            level++;
            for(int i=0;i<10;i++)
              for(int j=0;j<10;j++)
              {
                createCube(2.0f,&floor_mat[i][j],blue);
              }
          }
          else
          {
            cout<<"You won\n";
            quit(window);
          }
        }
      if(tem==5 && block.horiz==0 && block.horix==0)
        if(abs(floor_mat[ax][az].x-bx)<=tilw/2 && abs(floor_mat[ax][az].z-bz)<=tilw/2)
        {
          cout<<"Oops!\n";
          quit(window);
        }
      if((tem==3 || tem==4)  && block.horiz==0 && block.horix==0)
        if(abs(floor_mat[ax][az].x-bx)<=tilw/2 && abs(floor_mat[ax][az].z-bz)<=tilw/2)
        {
          for(int i=0;i<10;i++)
            for(int j=0;j<10;j++)
            {
              int *upd;
              if(level==1)
              {
                upd=&floor_fl0[i][j];
              }
              else if(level==2)
              {
                upd=&floor_fl1[i][j];
              }
              if(*upd==10+tem)
              {
                *upd=1;
                createCube(2.0f,&floor_mat[i][j],lightpink);
              }
            }
          }
    }
  }
}

glm::mat4 topview()
{
  glm::vec3 eye ( 0, 18, 0);
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 0, -1);
  return glm::lookAt(eye, target, up);
}

glm::mat4 blockview()
{
  if(bp==1)
  {
    glm::vec3 eye ( block.x+block.width/2, block.y, block.z);
    glm::vec3 target ( block.x+block.width/2+2, block.y, block.z);
    glm::vec3 up (0, 1, 0);
    return glm::lookAt(eye, target, up);
  }
   if(bp==2)
   {
     glm::vec3 eye ( block.x, block.y, block.z-block.depth/2);
     glm::vec3 target  ( block.x, block.y, block.z-block.depth/2-2);
     glm::vec3 up (0, 1, 0);
     return glm::lookAt(eye, target, up);
   }
   if(bp==3)
   {
       glm::vec3 eye ( block.x-block.width/2, block.y, block.z);
       glm::vec3 target  ( block.x-block.width/2-2, block.y, block.z);
       glm::vec3 up (0, 1, 0);
       return glm::lookAt(eye, target, up);
   }
   if(bp==4)
   {
     glm::vec3 eye ( block.x, block.y, block.z+block.depth/2);
     glm::vec3 target  ( block.x, block.y, block.z+block.depth/2+2);
     glm::vec3 up (0, 1, 0);
     return glm::lookAt(eye, target, up);
   }
}

glm::mat4 towerview()
{
    glm::vec3 eye ( -10, 12, 12);
    glm::vec3 target (0, 0, -2);
    glm::vec3 up (0, 1, 0);
    return glm::lookAt(eye, target, up);
}

glm::mat4 followview()
{
  glm::vec3 eye ( block.x+block.width/2+2, block.y+ block.height/2+2, block.z+block.depth/2+2);
  glm::vec3 target  ( block.x-block.width/2-1, block.y, block.z-block.depth/2-1);
  glm::vec3 up (0, 1, 0);
  return glm::lookAt(eye, target, up);
}

glm::mat4 heliview()
{
  glm::vec3 eye ( 14*sin(camera_rotation_angle*M_PI/180.0f), 14, -14*cos(camera_rotation_angle*M_PI/180.0f));
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  return glm::lookAt(eye, target, up);
}

glm::mat4 initialview()
{
  if(level==1)
  {
      glm::vec3 eye ( -2, 10, 12);
      glm::vec3 target (0, 0, -2);
      glm::vec3 up (0, 1, 0);
      return glm::lookAt(eye, target, up);
  }
  if(level==2)
  {
    glm::vec3 eye ( -1, 10,12);
    glm::vec3 target (0, 0, -2);
    glm::vec3 up (0, 1, 0);
    return glm::lookAt(eye, target, up);
  }
}

void displaySteps(Sprite obj)
{
  Matrices2.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices2.projection * Matrices2.view;
  glm::mat4 MVP;
  Matrices2.model = glm::mat4(1.0f);
  glm::mat4 ObjectTransform;
  glm::mat4 translateObject = glm::translate (glm::vec3(obj.x,obj.y, 0.0f)); // glTranslatef
  glm::mat4  rotateTriangle=glm::mat4(1.0f);
  rotateTriangle = glm::rotate((float)(obj.rot_angle*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  ObjectTransform=translateObject*rotateTriangle;

  Matrices2.model *= ObjectTransform;
  MVP = VP * Matrices2.model; // MVP = p * V * M

  glUniformMatrix4fv(Matrices2.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(obj.object);
}


void draw (GLFWwindow* window)
{
    glUseProgram(programID);


    glm::vec3 eye ( -3, 8, 3);
    glm::vec3 target (0, 0, 0);
    glm::vec3 up (0, 1, 0);
    glm::lookAt(eye, target, up);
    if(tp)
    	Matrices.view = topview();  // Fixed camera for 2D (ortho) in XY plane
    else if(bp)
      	Matrices.view = blockview();  // Fixed camera for 2D (ortho) in XY plane
    else if(vp)
      	Matrices.view = towerview();  // Fixed camera for 2D (ortho) in XY plane
    else if(fp)
      	Matrices.view = followview();  // Fixed camera for 2D (ortho) in XY plane
    else if(hp)
      {
        camera_rotation_angle+=1;
        if(camera_rotation_angle>=360)
          camera_rotation_angle=0;
      	Matrices.view = heliview();
      }
    else
      Matrices.view = initialview();
    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    glm::mat4 VP;
    VP = Matrices.projection * Matrices.view;
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    check_score(window);
    for(int i=1;i<=15;i++)
    {
     if(sboard[i].status)
        displaySteps(sboard[i]);
    }

    for(int i=0;i<10;i++)
    {
      for (int j = 0; j < 10; j++)
      {
        int tem;
        if(level==1)
        {
          tem=floor_fl0[i][j];
        }
        else if(level==2)
        {
          tem=floor_fl1[i][j];
        }

         if(tem==1)
          display(floor_mat[i][j],VP);
        if(tem==3 || tem==4)
        {
          float side=2;
          createCube (side,&floor_mat[i][j],green);
          display(floor_mat[i][j],VP);
        }
        if(tem==5)
        {
          float side=2;
          createCube (side,&floor_mat[i][j],red);
          display(floor_mat[i][j],VP);
        }
      }
    }
    float tempx=block.x,tempy=block.y,tempz=block.z;

    if((block.horiz && rox))
    {
      if(rox==1)
        block.z+=floor_mat[0][0].width;
      if(rox==-1)
        block.z-=floor_mat[0][0].width;
      display_block(block,VP);
      rox=0;
    }
    else if((block.horix && roz))
    {
      if(roz==-1)
        block.x+=floor_mat[0][0].width;
      if(roz==1)
        block.x-=floor_mat[0][0].width;
      display_block(block,VP);
      roz=0;
    }
    else
    {
      if(roz==-1)
      {
        block.anglez+=3*roz;
        if(block.horiz)
          block.x+=(1.5*floor_mat[0][0].width*abs(cos(block.anglez*M_PI/180)));
        else
          block.x+=(1.5*floor_mat[0][0].width*abs(sin(block.anglez*M_PI/180)));
        block.y=((block.height)/2*abs(cos(block.anglez*M_PI/180))+abs((block.depth)/2*sin(block.anglez*M_PI/180))+floor_mat[0][0].height/2);
      }
      if(roz==1)
      {
        block.anglez+=3*roz;
        // block.x-=(3*(block.height)/4*sin(block.anglez*M_PI/180));
        if(block.horiz)
          block.x-=(1.5*floor_mat[0][0].width*abs(cos(block.anglez*M_PI/180)));
        else
          block.x-=(1.5*floor_mat[0][0].width*abs(sin(block.anglez*M_PI/180)));
        block.y=((block.height)/2*abs(cos(block.anglez*M_PI/180))+abs((block.depth)/2*sin(block.anglez*M_PI/180))+floor_mat[0][0].height/2);
      }

      if(rox==1)
      {
        block.rot_angle+=3*rox;
        // block.x-=(3*(block.height)/4*sin(block.rot_angle*M_PI/180));
        if(block.horix)
          block.z+=(1.5*floor_mat[0][0].width*abs(cos(block.rot_angle*M_PI/180)));
        else
          block.z+=(1.5*floor_mat[0][0].width*abs(sin(block.rot_angle*M_PI/180)));
        block.y=((block.height)/2*abs(cos(block.rot_angle*M_PI/180))+abs((block.depth)/2*sin(block.rot_angle*M_PI/180))+floor_mat[0][0].height/2);
        // cout<<block.y<<endl;
      }
      if(rox==-1)
      {
        block.rot_angle+=3*rox;
        if(block.horix)
          block.z-=(1.5*floor_mat[0][0].width*abs(cos(block.rot_angle*M_PI/180)));
        else
          block.z-=(1.5*floor_mat[0][0].width*abs(sin(block.rot_angle*M_PI/180)));
        block.y=((block.height)/2*abs(cos(block.rot_angle*M_PI/180))+abs((block.depth)/2*sin(block.rot_angle*M_PI/180))+floor_mat[0][0].height/2);
        // cout<<block.y<<endl;
      }

      display_block(block,VP);
      block.x=tempx;
      block.z=tempz;
      if(abs(block.anglez-iangle)>=90 && abs(roz)==1 )
      {
        if(roz==-1)
          block.x+=(1.5*floor_mat[0][0].width);
        if(roz==1)
          block.x-=(1.5*floor_mat[0][0].width);
        block.horiz^=1;
        roz=0;
        block.dx+=1;
      }

      if(abs(block.rot_angle-iangle)>=90 && abs(rox)==1 )
      {
        if(rox==1)
          block.z+=(1.5*floor_mat[0][0].width);
        if(rox==-1)
          block.z-=(1.5*floor_mat[0][0].width);
        block.horix^=1;
        rox=0;
        block.dz+=1;
      }
    }
    check(window);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Bloxorz ~harshit mahajan", NULL, NULL);
/* Nothing to Edit here */

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window, mousescroll); // mouse scroll

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */

VAO* createRectangle (COLOR color1, float height, float width)
{
  // GL3 accepts only Triangles. Quads are not supported
  float w=width/2,h=height/2;
  GLfloat vertex_buffer_data [] = {
      -w,-h,0, // vertex 1
      -w,h,0, // vertex 2
      w,h,0, // vertex 3

      w,h,0, // vertex 3
      w,-h,0, // vertex 4
      -w,-h,0  // vertex 1
  };

   GLfloat color_buffer_data [] = {
    color1.r,color1.g,color1.b, // color 1
    color1.r,color1.g,color1.b, // color 2
    color1.r,color1.g,color1.b, // color 3

    color1.r,color1.g,color1.b, // color 3
    color1.r,color1.g,color1.b, // color 4
    color1.r,color1.g,color1.b,  // color 1

  };
    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void create_board(int no)
{
  sboard[no].name="sboard";
  sboard[no].color=white;
  sboard[no].width=3;
  sboard[no].height=50;
  sboard[no].status=0;
  sboard[no].rot_angle=0;
  if(no==1)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=345;
  }
  if(no==2)
  {
    // sboard[no].rot_angle=0;
    sboard[no].x=495;
    sboard[no].y=323;
  }
  if(no==3)
  {
    sboard[no].x=495;
    sboard[no].y=268;
  }
  if(no==4)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=240;
  }
  if(no==5)
  {
    sboard[no].x=460;
    sboard[no].y=268;
  }
  if(no==6)
  {
    sboard[no].x=460;
    sboard[no].y=323;
  }
  if(no==7)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=478;
    sboard[no].y=295;
  }
  if(no==8)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=345;
  }
  if(no==9)
  {
    // sboard[no].rot_angle=0;
    sboard[no].x=445;
    sboard[no].y=323;
  }
  if(no==10)
  {
    sboard[no].x=445;
    sboard[no].y=268;
  }
  if(no==11)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=240;
  }
  if(no==12)
  {
    sboard[no].x=410;
    sboard[no].y=268;
  }
  if(no==13)
  {
    sboard[no].x=410;
    sboard[no].y=323;
  }
  if(no==14)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=35;
    sboard[no].x=428;
    sboard[no].y=295;
  }
  if(no==15)
  {
    sboard[no].rot_angle=90;
    sboard[no].height=20;
    sboard[no].x=388;
    sboard[no].y=293;
  }
  sboard[no].object = createRectangle (white, sboard[no].height,sboard[no].width);
}

void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    for(int i=1;i<=15;i++)
      create_board(i);
    float side=2;
    float dispx=-4.5,dispz=-4.5;
    for(int i=0;i<10;i++)
    {
      dispx=-4.5;
      for (int j = 0; j < 10; j++)
      {
        Sprite *obj= &floor_mat[i][j];
        createCube (side,obj,blue);
        obj->x = dispx*side;dispx+=1;
        obj->y=0;
        obj->z = dispz*side;
        obj->sx=1;
        obj->sy=0.25;
        obj->sz=1;
        obj->height = side*obj->sy;
        obj->width = side*obj->sx;
        obj->depth = side*obj->sz;
      }
      dispz+=1;
    }
    createBlock(side,&block);
    block.sx = 1;
    block.sy = 2;
    block.sz = 1;
    block.x = floor_mat[0][0].x;
    block.y = (side*floor_mat[0][0].sy)/2+(side*block.sy)/2;
    block.z = floor_mat[0][0].z;
    block.rot_angle=0;
    block.anglez=0;
    block.width=side*block.sx;
    block.height=side*block.sy;
    block.depth=side*block.sz;
    block.horiz=0;
    block.horix=0;
    block.dx=0;
    block.dz=0;

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
    Matrices2.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

}


time_t old_time;


int main (int argc, char** argv)
{

    srand(time(NULL));

    old_time = time(NULL);
    // thread(play_audio,"./back1.mp3").detach();
    int width = 800;
    int height = 450;
    do_rot = 0;

    GLFWwindow* window = initGLFW(width, height);
    audio_init();
    initGLEW();
    initGL (window, width, height);

    last_update_time = glfwGetTime();
    /* Draw in loop */
  while (!glfwWindowShouldClose(window))
  {

	   // clear the color and depth in the frame buffer
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // OpenGL Draw commands
	current_time = glfwGetTime();
  // cout<< camera_rotation_angle<<endl;
	last_update_time = current_time;
	draw(window);
  if(level==1)
    audio_play(1);
  if(level==2)
    audio_play(2);

  if(abs(rox) || abs(roz))
    audio1_play();
  // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
    }

    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
