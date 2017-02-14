#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <time.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
} Matrices;

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


int do_rot,floor_fl0[10][10]= {{1,0,0,1,0,0,0,0,0,0},
                              {1,0,0,1,0,0,0,0,0,0},
                              {1,1,1,1,0,0,0,0,0,0},
                              {1,1,1,1,1,1,1,1,0,0},
                              {0,0,0,0,1,1,1,1,0,0},
                              {0,0,0,0,1,1,0,1,0,0},
                              {0,0,0,0,1,1,1,1,0,0},
                              {0,0,0,0,1,1,1,1,1,1},
                              {0,0,0,0,0,0,0,1,1,1},
                              {0,0,0,0,0,0,0,1,1,1}},
          floor_fl1[10][10] = {{1,1,1,0,0,0,0,0,0,0},
                              {1,1,1,1,1,1,0,0,0,0},
                              {1,1,1,1,1,1,1,1,1,0},
                              {0,1,1,1,1,1,1,1,1,1},
                              {0,0,0,0,0,1,1,0,1,1},
                              {0,0,0,0,0,0,1,1,1,0},
                              {0,0,0,0,0,0,0,0,0,0},
                              {0,0,0,0,0,0,0,0,0,0},
                              {0,0,0,0,0,0,0,0,0,0},
                              {0,0,0,0,0,0,0,0,0,0}};
int rox=0,roy=0,roz=0,iangle=0,fangle=0,fl1=0,fl2=0;
GLuint programID;
double last_update_time, current_time;
float rectangle_rotation = 0;
Sprite floor_mat[10][10],block,xblock,zblock;

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
int tpress=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
	case GLFW_KEY_C:
	    rectangle_rot_status = !rectangle_rot_status;
	    break;
	case GLFW_KEY_P:
	    break;
	case GLFW_KEY_T:
      tpress^=1;
	    break;
  case GLFW_KEY_RIGHT:
      // if(abs(int(block.anglez))%90==0)
      if( roz==0 && rox==0)
      {
        roz=-1;
        iangle=block.anglez;
      }
      break;
  case GLFW_KEY_LEFT:
      // if(abs(int(block.anglez))%90==0)
      if( roz==0 && rox==0)
      {
        roz=1;
        iangle=block.anglez;
      }
      break;
  case GLFW_KEY_UP:
      // if(abs(int(block.rot_angle))%90==0)
      if( roz==0 && rox==0)
      {
        rox=-1;
        iangle=block.rot_angle;
      }
      break;
  case GLFW_KEY_DOWN:
      // if(abs(int(block.rot_angle))%90==0)
      if( roz==0 && rox==0)
        {
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
    case 'a':
	     break;
    case 'd':
	     break;
    case 'w':
	     break;
    case 's':
	     break;
    case 'r':
	     break;
    case 'f':
	     break;
    case 'e':
	     break;
    case 'j':
	     break;
    case 'l':
	     break;
    case 'i':
	     break;
    case 'k':
	     break;
    case 'y':
	     break;
    case 'h':
	     break;
    case 'g':
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

    GLfloat fov = M_PI/2;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    Matrices.projection = glm::perspective(fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 1000.0f);
}

VAO *base, *baseBoundary, *floor_vao;

// Creates the rectangle object used in this sample code
void createCube (float wd,Sprite *obj)
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
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
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
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       0.0f, 1.0f, 1.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
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

float camera_rotation_angle = 90;

/* Render the scene with openGL */
/* Edit this function according to your assignment */

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

glm::mat4 topview()
{
  glm::vec3 eye ( 0, 10, 0);
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 0, -1);
  return glm::lookAt(eye, target, up);
}

glm::mat4 towerview()
{
  glm::vec3 eye ( -3, 8, 3);
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
   return glm::lookAt(eye, target, up);
}

glm::mat4 initialview()
{
  glm::vec3 eye ( -2, 8,4);
  glm::vec3 target (0, 0, -2);
  glm::vec3 up (0, 1, 0);
  return glm::lookAt(eye, target, up);
}

void check(GLFWwindow* window)
{
  float bx=block.x,by=block.y,bz=block.z,tilw=floor_mat[0][0].width;
  if(bx>=5*tilw || bx<=-5*tilw || by>=5*tilw || by<=-5*tilw || bz>=5*tilw || bz<=-5*tilw )
    quit(window);
}

void draw (GLFWwindow* window)
{
      // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram(programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    // glm::vec3 eye ( -5*cos(camera_rotation_angle*M_PI/180.0f), 0, -5*sin(camera_rotation_angle*M_PI/180.0f));
    glm::vec3 eye ( -3, 8, 3);
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);
    glm::lookAt(eye, target, up);
    // Compute Camera matrix (view)
    if(tpress)
    	Matrices.view = topview();  // Fixed camera for 2D (ortho) in XY plane
    else
      Matrices.view = initialview();
    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    glm::mat4 VP;
    VP = Matrices.projection * Matrices.view;
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    for(int i=0;i<10;i++)
    {
      for (int j = 0; j < 10; j++)
      {
         if(floor_fl1[i][j])
          display(floor_mat[i][j],VP);
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
        // block.x-=(3*(block.height)/4*sin(block.anglez*M_PI/180));
        if(block.horiz)
          block.x+=(1.5*floor_mat[0][0].width*abs(cos(block.anglez*M_PI/180)));
        else
          block.x+=(1.5*floor_mat[0][0].width*abs(sin(block.anglez*M_PI/180)));
          // cout<<block.x/floor_mat[0][0].width<<endl;

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

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    float side=2;
    float dispx=-4.5,dispz=-4.5;
    for(int i=0;i<10;i++)
    {
      dispx=-4.5;
      for (int j = 0; j < 10; j++)
      {
        Sprite *obj= &floor_mat[i][j];
        createCube (side,obj);
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
    // createBlock(side,&xblock);
    // block.sx = 2;
    // block.sy = 1;
    // block.sz = 1;
    // block.x = floor_mat[0][0].x;
    // block.y = (side*floor_mat[0][0].sy)/2+(side*block.sy)/2;
    // block.z = floor_mat[0][0].z;
    // block.rot_angle=0;
    // block.width=side*block.sx;
    // block.height=side*block.sy;
    // block.depth=side*block.sz;
    // createBlock(side,&zblock);
    // block.sx = 1;
    // block.sy = 2;
    // block.sz = 1;
    // block.x = floor_mat[0][0].x;
    // block.y = (side*floor_mat[0][0].sy)/2+(side*block.sy)/2;
    // block.z = floor_mat[0][0].z;
    // block.rot_angle=0;
    // block.width=side*block.sx;
    // block.height=side*block.sy;
    // block.depth=side*block.sz;

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

}

int main (int argc, char** argv)
{
    int width = 800;
    int height = 450;
    do_rot = 0;

    GLFWwindow* window = initGLFW(width, height);
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
  // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();
    }

    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}
