#include <stdio.h>
#include <GLFW/glfw3.h>
#define M_MATH_IMPLEMENTATION
#include "m_math.h"

float MOUSE_X = 0;
float MOUSE_Y = 0;

void l(char *s) {printf("%s\n", s);}
void lm(char*name, float matrix[]) {
	int index = 0;
	printf("%s:\n", name);
	printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
	index += 4;
	printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
	index += 4;
	printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);
	index += 4;
	printf("%f %f %f %f\n", matrix[index+0], matrix[index+1],matrix[index+2],matrix[index+3]);

}

/**************************************************************************/
/*
-- Draw a field with boundary lines



*/
/***************************************************************************/

void close_callback(GLFWwindow * window){
   l("close_callback");
}

void size_callback(GLFWwindow * window, int width, int height){
	l("size_callback");
}

void cursorpos_callback(GLFWwindow * window, double x, double y){
	MOUSE_Y = y;
	MOUSE_X = x;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	l("key_callback");
	glfwSetWindowShouldClose(window, 1);
}

void mousebutton_callback(GLFWwindow * window, int button, int action, int mods){
}

void char_callback(GLFWwindow * window, unsigned int key){
	l("char_callback");
}
void error_callback(int error, const char* description){
	printf("%s\n", description);
}

GLuint compile_shader(GLenum type, const char *src) {
	GLuint shader;
	GLint compiled;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if(!compiled) {
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar strInfoLog[infoLogLength];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		printf("Compilation error in shader %s\n", strInfoLog);
		glDeleteShader(shader);
		return 0;
	}
	printf("Success!\n");
	return shader;
}

int compile_shader_program(const char* str_vert_shader, const char* str_frag_shader) {
	GLuint vert_shader;
	GLuint frag_shader;
	GLuint prog_object;

	vert_shader = compile_shader(GL_VERTEX_SHADER, str_vert_shader);
	if(vert_shader == 0) {l("Error compiling vert shader"); return 1;}

	frag_shader = compile_shader(GL_FRAGMENT_SHADER, str_frag_shader);
	if(frag_shader == 0) {l("Error compiling frag shader"); return 1;}
	
	l("Creating shader program");

	prog_object = glCreateProgram();
	glAttachShader(prog_object, vert_shader);
	glAttachShader(prog_object, frag_shader);

	l("Binding 0 -> position");
	glBindAttribLocation(prog_object, 0, "position");

	l("Linking shader program");
	glLinkProgram(prog_object);

	return prog_object;
}

GLuint line_vert_shader;
GLuint line_frag_shader;
GLuint line_shader_program;

float view_matrix[] = M_MAT4_IDENTITY();
float projection_matrix[] = M_MAT4_IDENTITY();
float model_matrix[] = M_MAT4_IDENTITY();

float vertices[8] = {0, 0, 0, 0.1, 0.1, 0, 0.1, 0.1};

int main(int argc, char const *argv[]){
	int win_w = 500;
	int win_h = 300;

	GLFWwindow* window;
	if(!glfwInit()) {l("glfw init failed");} else {l("Glfw initialized");}
	window = glfwCreateWindow(win_w, win_h, "Fields", NULL, NULL);
	if(!window) {
		l("Create window failed");
		glfwTerminate();
		return -1;
	}
	l("Setting callbacks");
	glfwSetWindowCloseCallback(window, close_callback);
	glfwSetWindowSizeCallback(window, size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mousebutton_callback);
	glfwSetCharCallback(window, char_callback);
	glfwSetCursorPosCallback(window, cursorpos_callback);
	glfwSetErrorCallback(error_callback);
	l("Setting context");
	glfwMakeContextCurrent(window);
	// End of window setup

	char str_line_vert_shader[] =
	"attribute vec3 position;"
	"uniform float u_time;"
	"uniform mat4 u_model_matrix;"
	"uniform mat4 u_view_matrix;"
	"uniform mat4 u_projection_matrix;"
	"varying float v_time;"
	"void main(){"
	"v_time = u_time;"
	"gl_Position =  u_projection_matrix * u_view_matrix * u_model_matrix * vec4(position,1.0);"
	"}";

	char str_line_frag_shader[] =
	"varying float v_time;"
	"void main() {"
	"gl_FragColor = vec4 (clamp(sin(v_time/12.0),0.0,1.0), 0.7, 0.1, 1.0);"
	"}"
	;

	
	m_mat4_ortho(projection_matrix, -1.0, 1.0, -1.0, 1.0, 1, 150);
	lm("Ortho projection? ", projection_matrix);	




	line_shader_program = compile_shader_program(str_line_vert_shader, str_line_frag_shader);

	if (line_shader_program == 0){
		l("ERROR creating shader program");
		return 1;
	}

	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	float time = 0;
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
	


		// camera
		m_mat4_identity(view_matrix);
		float3 camera_position = {0,0,-10};
		float3 camera_direction = {(MOUSE_X-win_w/2.0)/win_w,0,1};
		float3 camera_up = {0,1,0};
		m_mat4_lookat(view_matrix, &camera_position, &camera_direction, &camera_up);
		lm("CameraLookat? ", view_matrix);



		float3 translation = {-0.5,sin(time),-9.0};
		float3 axis = {0,0,1};
		
		//m_mat4_rotation_axis(model_matrix, &axis, time);
		//m_mat4_translation(model_matrix, &translation);
		


		glUseProgram(line_shader_program);
		glUniform1f(glGetUniformLocation(line_shader_program, "u_time"), time);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_model_matrix"), 1, GL_FALSE, model_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glUseProgram(0);


		// end
		glfwSwapBuffers(window);
		glfwPollEvents();
		time +=0.001;
	}

	glfwMakeContextCurrent(NULL);
	glfwDestroyWindow(window);
	return 0;
}

