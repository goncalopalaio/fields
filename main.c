#include <stdio.h>
#include <GLFW/glfw3.h>
#define M_MATH_IMPLEMENTATION
#include "m_math.h"
#include <string.h>
#include <stdlib.h>
float MOUSE_X = 0;
float MOUSE_Y = 0;

#include "cube.h"

void l(char *s) {printf("%s\n", s);}
void l3(char *s, float3 f) {
	printf("%s %f %f %f\n", s,f.x,f.y,f.z);
}
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

	glBindAttribLocation(prog_object, 1, "color");

	l("Linking shader program");
	glLinkProgram(prog_object);

	return prog_object;
}

GLuint line_shader_program;
GLuint trace_points_shader_program;

float view_matrix[] = M_MAT4_IDENTITY();
float projection_matrix[] = M_MAT4_IDENTITY();
float model_matrix[] = M_MAT4_IDENTITY();

float vertices[8] = {0, 0, 0, 0.1, 0.1, 0, 0.1, 0.1};

#define NUMBER_OF_FLOAT3_TRACE_POINTS 32
float* trace_points;
int trace_points_current_index = 0;

void write_trace_point(float* trace_points, float3 point) {
	trace_points[trace_points_current_index] = point.x;
	trace_points[trace_points_current_index + 1] = point.y;
	trace_points[trace_points_current_index + 2] = point.z;
	printf("current index: %d\n", trace_points_current_index);	
	printf("T: %f %f %f \n",trace_points[trace_points_current_index], trace_points[trace_points_current_index+1], trace_points[trace_points_current_index+2]);

	trace_points_current_index += 3;
	trace_points_current_index = trace_points_current_index % (NUMBER_OF_FLOAT3_TRACE_POINTS * 3);
}

void write_trace_point_xyz(float* trace_points, float x, float y, float z) {
	float3 a = {x,y,z};
	write_trace_point(trace_points, a);
}

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
	"gl_FragColor = vec4 (1.0, 0.7, 0.1, 1.0);"
	"}";

	char str_trace_points_vert_shader[] =
	"attribute vec3 position;"
	"attribute vec3 color;"
	"varying vec3 v_color;"
	"uniform mat4 u_view_matrix;"
	"uniform mat4 u_projection_matrix;"
	"void main(){"
	"v_color = color;"
	"gl_Position =  u_projection_matrix * u_view_matrix * vec4(position, 1.0);"
	"}";

	char str_trace_points_frag_shader[] =
	"varying vec3 v_color;"
	"void main() {"
	"gl_FragColor = vec4(v_color, 1.0);"
	"}";

	// alloc trace points
	trace_points = (float*) malloc(NUMBER_OF_FLOAT3_TRACE_POINTS * 3 * sizeof(float)); // multiply by three to have space for 3 floats each
	// todo: memcpy this?
	// reset trace points
	for(int i = 0; i< NUMBER_OF_FLOAT3_TRACE_POINTS; i++) {
		trace_points[i] = 0;
		trace_points[i + 1] = 0;
		trace_points[i + 2] = 0;
	}


	
	m_mat4_ortho(projection_matrix, -1.0, 1.0, -1.0, 1.0, 1, 550);
	lm("Ortho projection? ", projection_matrix);	

	line_shader_program = compile_shader_program(str_line_vert_shader, str_line_frag_shader);
	trace_points_shader_program = compile_shader_program(str_trace_points_vert_shader, str_trace_points_frag_shader);

	if (line_shader_program == 0){
		l("ERROR creating shader program");
		return 1;
	}

	if (trace_points_shader_program == 0) {
		l("ERROR creating Point Trace shader program");
		return 1;	
	}


	glPointSize(8.0);
	float reference_offset = 4.5;

		write_trace_point_xyz(trace_points, 0,0,0);
		write_trace_point_xyz(trace_points, 0,0,0);

		write_trace_point_xyz(trace_points, -reference_offset,0,0);
		write_trace_point_xyz(trace_points, reference_offset,0,0);

		write_trace_point_xyz(trace_points, 0,-reference_offset,0);
		write_trace_point_xyz(trace_points, 0,reference_offset,0);

		write_trace_point_xyz(trace_points, 0,0,-reference_offset);
		write_trace_point_xyz(trace_points, 0,0,reference_offset);
	

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	float time = 0;
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	


		// camera
		m_mat4_identity(view_matrix);
		//float3 camera_position = {4 * sin(time) ,sin(time), 4 * cos(time)};
		float3 camera_position = {4 ,2*cos(time), 4};
		float3 camera_direction = {0 - camera_position.x ,0 - camera_position.y , 0 - camera_position.z};
		float3 camera_up = {0,1,0};
		m_mat4_lookat(view_matrix, &camera_position, &camera_direction, &camera_up);
		//lm("CameraLookat? ", view_matrix);


		glUseProgram(line_shader_program);
		glUniform1f(glGetUniformLocation(line_shader_program, "u_time"), time);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_model_matrix"), 1, GL_FALSE, model_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glUseProgram(0);

		// Draw trace points
		glUseProgram(trace_points_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, trace_points);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_LINES, 0, NUMBER_OF_FLOAT3_TRACE_POINTS);
		glUseProgram(0);
		
		glUseProgram(trace_points_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(line_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, cube_vertices);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, cube_colors);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glUseProgram(0);
		
		// todo clean all these calls so it is easier to handle

		// end
		glfwSwapBuffers(window);
		glfwPollEvents();
		time +=0.01;
	}

	glfwMakeContextCurrent(NULL);
	glfwDestroyWindow(window);
	return 0;
}

