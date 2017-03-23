#include <stdio.h>
#include <GLFW/glfw3.h>
#define M_MATH_IMPLEMENTATION
#include "m_math.h"
#include <string.h>
#include <stdlib.h>
float MOUSE_X = 0;
float MOUSE_Y = 0;
float PREV_MOUSE_X = 0;
float PREV_MOUSE_Y = 0;
float MOUSE_DX = 0;
float MOUSE_DY = 0;

#include "cube.h"
int win_w = 1000;
int win_h = 500;


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

void cursorpos_callback(GLFWwindow * window, double mx, double my){
	MOUSE_DX = PREV_MOUSE_X - mx;
	MOUSE_DY = PREV_MOUSE_Y - my;

	
	MOUSE_X = mx;
	MOUSE_Y = my;

	PREV_MOUSE_X = mx;
	PREV_MOUSE_Y = my;
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

int compile_shader_program(const char* str_vert_shader, const char* str_frag_shader, const char* attrib_name_0, const char* attrib_name_1) {
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

	if (attrib_name_0 != NULL){
		l("Binding attrib 0");
		glBindAttribLocation(prog_object, 0, attrib_name_0);	
	}

	if (attrib_name_1 != NULL) {
		l("Binding attrib 1");
		glBindAttribLocation(prog_object, 1, attrib_name_1);	
	}
	
	l("Linking shader program");
	glLinkProgram(prog_object);

	return prog_object;
}

GLuint trace_points_shader_program;
GLuint colored_cube_shader_program;

float view_matrix[] = M_MAT4_IDENTITY();
float projection_matrix[] = M_MAT4_IDENTITY();
float model_matrix[] = M_MAT4_IDENTITY();

float vertices[8] = {0, 0, 0, 0.41, 0.41, 0, 0.41, 0.41};

#define NUMBER_OF_FLOAT3_TRACE_POINTS 256
float* trace_points_vertices;
float* trace_points_colors;


int trace_points_current_index = 0;

void write_trace_point(float* trace_points, float3 point) {
	trace_points[trace_points_current_index] = point.x;
	trace_points[trace_points_current_index + 1] = point.y;
	trace_points[trace_points_current_index + 2] = point.z;
	printf("current index: %d\n", trace_points_current_index);	
	printf("T: %f %f %f \n",trace_points[trace_points_current_index], trace_points[trace_points_current_index+1], trace_points[trace_points_current_index+2]);

	trace_points_current_index += 3;

	if (trace_points_current_index > NUMBER_OF_FLOAT3_TRACE_POINTS * 3){
		// keep first 8 points
		trace_points_current_index = 3 * 8; 
	}
}

void write_trace_point_xyz(float* trace_points, float x, float y, float z) {
	float3 a = {x,y,z};
	write_trace_point(trace_points, a);
}

int main(int argc, char const *argv[]){

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

	char str_colored_cube_vert_shader[] =
	"attribute vec3 position;"
	"attribute vec3 color;"
	"uniform mat4 u_model_matrix;"
	"uniform mat4 u_view_matrix;"
	"uniform mat4 u_projection_matrix;"
	"uniform float u_picked;"
	"uniform float u_time;"
	"varying vec3 v_color;"
	"varying float v_picked;"
	"void main(){"
	"v_color = color;"
	"v_picked = u_picked;"
	"gl_Position =  u_projection_matrix * u_view_matrix * u_model_matrix * vec4(position.x, position.y + u_picked,position.z, 1.0);"
	"}";

	char str_colored_cube_frag_shader[] =
	"varying vec3 v_color;"
	"varying float v_picked;"
	"void main() {"
	"gl_FragColor = vec4(v_color * v_picked, 1.0);"
	"}";
	// alloc trace points
	trace_points_vertices = (float*) malloc(NUMBER_OF_FLOAT3_TRACE_POINTS * 3 * sizeof(float)); // multiply by three to have space for 3 floats each
	trace_points_colors = (float*) malloc(NUMBER_OF_FLOAT3_TRACE_POINTS * 3 * sizeof(float));
	// todo: memcpy this?
	// reset trace points
	for(int i = 0; i< NUMBER_OF_FLOAT3_TRACE_POINTS; i++) {
		trace_points_vertices[i] = 0;
		trace_points_vertices[i + 1] = 0;
		trace_points_vertices[i + 2] = 0;

		// Setup random color for each point initially
		trace_points_colors[i] = m_randf();
		trace_points_colors[i + 1] = m_randf();
		trace_points_colors[i + 2] = m_randf();

	}


	
	// Picking is not working with ortho. Maybe it is more simple than I thought. Think about how to project the
	// mouse ray with this type of projection. Why is it working with perspective?

	// m_mat4_ortho(projection_matrix, -2.0, 2.0, -1.0, 1.0, 1, 950);
	// lm("Ortho projection? ", projection_matrix);	

	float aspect = win_w / (float)win_h;
	m_mat4_perspective(projection_matrix, 10.0, aspect, 0.1, 100.0);


	trace_points_shader_program = compile_shader_program(str_trace_points_vert_shader, str_trace_points_frag_shader, "position", "color");
	colored_cube_shader_program = compile_shader_program(str_colored_cube_vert_shader, str_colored_cube_frag_shader, "position", "color");


	glPointSize(8.0);
	glLineWidth(8.0);
	float reference_offset = 4.5;

	write_trace_point_xyz(trace_points_vertices, 0,0,0);
	write_trace_point_xyz(trace_points_vertices, 0,0,0);

	write_trace_point_xyz(trace_points_vertices, -reference_offset,0,0);
	write_trace_point_xyz(trace_points_vertices, reference_offset,0,0);

	write_trace_point_xyz(trace_points_vertices, 0,-reference_offset,0);
	write_trace_point_xyz(trace_points_vertices, 0,reference_offset,0);

	write_trace_point_xyz(trace_points_vertices, 0,0,-reference_offset);
	write_trace_point_xyz(trace_points_vertices, 0,0,reference_offset);
	

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	float time = 0;
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	


		// camera
		m_mat4_identity(view_matrix);
		float3 camera_position = {4*sin(time) ,1+cos(time*0.1), 4*cos(time)};
		float3 camera_direction = {0 - camera_position.x ,0 - camera_position.y , 0 - camera_position.z};		

		float3 camera_up = {0,1,0};
		m_mat4_lookat(view_matrix, &camera_position, &camera_direction, &camera_up);
		//lm("CameraLookat? ", view_matrix);


		/////// Mouse Picking

		// Normalized device coordinates: Ranges [x(-1,1),y(-1,1),z(-1,1)]
		float x = (2.0f * MOUSE_X) / win_w - 1.0f;
		float y = 1.0f - (2.0 * MOUSE_Y) / win_h;
		float z = -1.0f;
		printf("Mouse NDC: %f %f %f \n", x,y,z);

		float4 ray_clip = {x,y,z, 1.0};

		// 4d Eye (Camera) Coordinates [-x:x, -y:y, -z:z, -w:w]

		//vec4 ray_eye = inverse (projection_matrix) * ray_clip;
		float4 ray_eye = {0,0,0,0};
		float projection_matrix_inverse[] = M_MAT4_IDENTITY();
		m_mat4_inverse(projection_matrix_inverse, projection_matrix);
		m_mat4_transform4(&ray_eye, projection_matrix_inverse, &ray_clip);

		//Now, we only needed to un-project the x,y part, so let's manually set the z,w part to mean "forwards, and not a point".
		ray_eye.z = -1.0;
		ray_eye.w = 0.0;

		// 4d World Coordinates range [-x:x, -y:y, -z:z, -w:w]
		
		//vec3 ray_wor = (inverse (view_matrix) * ray_eye).xyz;
		// don't forget to normalise the vector at some point
		//ray_wor = normalise (ray_wor);
		float4 ray_world = {0,0,0,0};
		float view_matrix_inverse[] = M_MAT4_IDENTITY();
		m_mat4_inverse(view_matrix_inverse, view_matrix);
		m_mat4_transform4(&ray_world, view_matrix_inverse, &ray_eye);

		//printf("Ray world: %f %f %f %f\n", ray_world.x, ray_world.y, ray_world.z, ray_world.w);

		float3 ray_world_3 = {ray_world.x, ray_world.y, ray_world.z};
		float3 ray_world_3_normalized = {0,0,0};
		M_NORMALIZE3(ray_world_3_normalized,ray_world_3);
		printf("Normalized Ray world: %f %f %f \n", ray_world_3_normalized.x, ray_world_3_normalized.y, ray_world_3_normalized.z);
		
		// Draw trace points
		glUseProgram(trace_points_shader_program);
		glUniformMatrix4fv(glGetUniformLocation(trace_points_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(trace_points_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, trace_points_vertices);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, trace_points_colors);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glDrawArrays(GL_LINES, 0, NUMBER_OF_FLOAT3_TRACE_POINTS);
		glUseProgram(0);
		
		glUseProgram(colored_cube_shader_program);
		glUniform1f (glGetUniformLocation(colored_cube_shader_program, "u_time"), time);	
		
		int grid_size = 10;
		for (float i = -grid_size; i < grid_size; i+=0.3) {
			for (float j = -grid_size; j < grid_size; j+=0.3) {
			
			//m_mat4_identity(model_matrix);
			float3 transl = {i, 0, j};
			float transl_matrix[] = M_MAT4_IDENTITY();
			m_mat4_translation(transl_matrix, &transl);

			//float3 rot_axis = {0,0,1};
			//float rotation_matrix[] = M_MAT4_IDENTITY();
			//m_mat4_rotation_axis(rotation_matrix, &rot_axis, time);
			//m_mat4_mul(model_matrix,  transl_matrix, rotation_matrix);

			
			memcpy(model_matrix, transl_matrix, 16 * sizeof(float));

		
			
			float3 sphere_origin = {i,0,j};
			float intersection_in = 0;
			float intersection_out = 0;
			int intersection_res = m_3d_ray_sphere_intersection_in_out(&camera_position, &ray_world_3_normalized, &sphere_origin, 0.01, &intersection_in, &intersection_out);
			if (intersection_res > 0){
				glUniform1f (glGetUniformLocation(colored_cube_shader_program, "u_picked"), 1.0f);	
			} else {
				glUniform1f (glGetUniformLocation(colored_cube_shader_program, "u_picked"), 0.2f);	
			}
			
			glUniformMatrix4fv(glGetUniformLocation(colored_cube_shader_program, "u_model_matrix"), 1, GL_FALSE, model_matrix);
			glUniformMatrix4fv(glGetUniformLocation(colored_cube_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
			glUniformMatrix4fv(glGetUniformLocation(colored_cube_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, cube_vertices);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, cube_colors);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			}
		}
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

