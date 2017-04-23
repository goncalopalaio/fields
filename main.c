#include <stdio.h>
#include <GLFW/glfw3.h>
#define M_MATH_IMPLEMENTATION
#include "m_math.h"
#include <string.h>
#include <stdlib.h>
#include "cube.h"
#define TRUE 1
#define FALSE 0
int WIN_W = 1000;
int WIN_H = 500;


typedef struct {
    float* vertices;
    float* colors;
    int vert_count;
    //
    GLuint shader_program;
    GLint uniform_picked_location;
    GLint uniform_time_location;
    GLint uniform_model_matrix_location;
    GLint uniform_view_matrix_location;
    GLint uniform_projection_matrix_location;
} colored_cube;

typedef struct {
	float prev_mouse_x;
	float prev_mouse_y;
    float3 delta_coordinates;
    float3 world;
} mouse_info;

typedef struct {
    float3 position;
    float3 direction;
    float3 up;
} camera_info;

typedef struct {
    float3 position;
    float3 direction;
} ray;

camera_info camera;
mouse_info mouse;

float prev_mouse_x = 0;
float pre_mouse_y = 0;
float view_matrix[] = M_MAT4_IDENTITY();
float projection_matrix[] = M_MAT4_IDENTITY();
float model_matrix[] = M_MAT4_IDENTITY();

#define NUMBER_OF_FLOAT3_TRACE_POINTS 8
GLuint ref_points_shader_program;
float* ref_points_vertices;
float* ref_points_colors;
int ref_points_index = 0;

#define RAY_SIDE_GRID 128
float* ray_grid_points_vertices;
float* ray_grid_points_colors;

int* closest_grid_point_idx;

float3 grid_min_pos;
float3 grid_max_pos;

#define NUMBER_HUMAN_POINTS 60
float* human_points_vertices;
float* human_points_colors;
ray* human_list;

void mouse_pick(double mouse_x, double mouse_y, float* view, float* projection, mouse_info* mouse) {
    // Normalized device coordinates: Ranges x(-1,1),y(-1,1),z(-1,1)
    float x = (2.0f * mouse_x) / WIN_W - 1.0f;
    float y = 1.0f - (2.0 * mouse_y) / WIN_H;
    float z = -1.0f;
    float4 ray_clip = {x,y,z, 1.0};

    // 4d Eye (Camera) Coordinates [-x:x, -y:y, -z:z, -w:w] -> inverse(projection_matrix) * ray_clip;
    float4 ray_eye = {0,0,0,0};
    float projection_matrix_inverse[] = M_MAT4_IDENTITY();
    m_mat4_inverse(projection_matrix_inverse, projection);
    m_mat4_transform4(&ray_eye, projection_matrix_inverse, &ray_clip);

    //Now, we only needed to un-project the x,y part, so let's manually set the z,w part to mean "forwards, and not a point".
    ray_eye.z = -1.0;
    ray_eye.w = 0.0;

    // 4d World Coordinates range [-x:x, -y:y, -z:z, -w:w] ray_world -> inverse(view_matrix) * ray_eye;
    // don't forget to normalise the vector at some point
    float4 ray_world = {0,0,0,0};
    float view_matrix_inverse[] = M_MAT4_IDENTITY();
    m_mat4_inverse(view_matrix_inverse, view);
    m_mat4_transform4(&ray_world, view_matrix_inverse, &ray_eye);

    // update mouse_info
    float3 ray_world_3 = {ray_world.x, ray_world.y, ray_world.z};
    M_NORMALIZE3(ray_world_3,ray_world_3);
    mouse->world = ray_world_3;

}

void l(char *s) {
    printf("%s\n", s);
}
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

void set_vec(float3* v, float x, float y, float z) {
    v->x = x;
    v->y = y;
    v->z = z;
}

void close_callback(GLFWwindow * window) {
    l("close_callback");
}

void size_callback(GLFWwindow * window, int width, int height) {
    l("size_callback");
}

void cursorpos_callback(GLFWwindow * window, double mx, double my) {
    mouse_pick(mx, my, view_matrix, projection_matrix, &mouse);

    
    mouse.delta_coordinates.x = mouse.prev_mouse_x - mx;
    mouse.delta_coordinates.y = mouse.prev_mouse_y - my;
    M_NORMALIZE3(mouse.delta_coordinates, mouse.delta_coordinates);
    mouse.prev_mouse_x = mx;
    mouse.prev_mouse_y = my;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    l("key_callback");
    glfwSetWindowShouldClose(window, 1);
}

void mousebutton_callback(GLFWwindow * window, int button, int action, int mods) {
}

void char_callback(GLFWwindow * window, unsigned int key) {
    l("char_callback");
}
void error_callback(int error, const char* description) {
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
    if(vert_shader == 0) {
        l("Error compiling vert shader");
        return 1;
    }

    frag_shader = compile_shader(GL_FRAGMENT_SHADER, str_frag_shader);
    if(frag_shader == 0) {
        l("Error compiling frag shader");
        return 1;
    }

    l("Creating shader program");

    prog_object = glCreateProgram();
    glAttachShader(prog_object, vert_shader);
    glAttachShader(prog_object, frag_shader);

    if (attrib_name_0 != NULL) {
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

void write_trace_point(float* trace_points, float3 point) {
    trace_points[ref_points_index] = point.x;
    trace_points[ref_points_index + 1] = point.y;
    trace_points[ref_points_index + 2] = point.z;
    ref_points_index += 3;
}

void write_trace_point_xyz(float* trace_points, float x, float y, float z) {
    float3 a = {x,y,z};
    write_trace_point(trace_points, a);
}

float3 random_in_unit_sphere() {
    float3 res;
    float len = 0;
    do {
        res.x = 2.0 * m_randf() - 1;
        res.y = 2.0 * m_randf() - 1;
        res.z = 2.0 * m_randf() - 1;
        //squared length
        len = res.x*res.x + res.y*res.y + res.z*res.z;
    } while(len >= 1.0);

    return res;
}

float distance(float3* a, float3* b) {
    float x2 = (a->x - b->x);
    float y2 = (a->y - b->y);
    float z2 = (a->z - b->z);
    x2 *= x2;
    y2 *= y2;
    z2 *= z2;


    return sqrt(x2 + y2 + z2);
}

void influence_ray_grid_mouse() {
    int idx = 0;
    float3 tmp_dir;

    for (int x = 0; x < RAY_SIDE_GRID * RAY_SIDE_GRID; ++x) {
        ray* current = (ray*)&ray_grid_points_vertices[idx];
        float intersection_in = 0;
        float intersection_out = 0;
        int intersection_res = m_3d_ray_sphere_intersection_in_out(&camera.position, &mouse.world, &current->position, 0.1, &intersection_in, &intersection_out);

        if (intersection_res != FALSE) {
            //current->direction = random_in_unit_sphere();
      		current->direction.x = - mouse.delta_coordinates.y;
      		current->direction.y = 0;
      		current->direction.z = mouse.delta_coordinates.x;

      		//draw shorter lines
			tmp_dir.x = current->direction.x * 0.1;
			tmp_dir.y = current->direction.y * 0.1;
			tmp_dir.z = current->direction.z * 0.1;

            M_ADD3(current->direction, tmp_dir, current->position);
        }
        idx+=6;
    }
}

void build_grid() {
    float scale_between_points = 0.9;
    float offset_start_position = RAY_SIDE_GRID/2 * scale_between_points;

    int n_points_in_grid = RAY_SIDE_GRID * RAY_SIDE_GRID;
    int idx = 0;
    for (int x = 0; x < RAY_SIDE_GRID; ++x) {
        for (int y = 0; y < RAY_SIDE_GRID; ++y) {
            float x_pos = x * scale_between_points - offset_start_position;
            float y_pos = 0.1;
            float z_pos = y * scale_between_points - offset_start_position;

            if(grid_min_pos.x > x_pos) grid_min_pos.x = x_pos;
            if(grid_min_pos.y > y_pos) grid_min_pos.y = y_pos;
            if(grid_min_pos.z > z_pos) grid_min_pos.z = z_pos;

            if(grid_max_pos.x < x_pos) grid_max_pos.x = x_pos;
            if(grid_max_pos.y < y_pos) grid_max_pos.y = y_pos;
            if(grid_max_pos.z < z_pos) grid_max_pos.z = z_pos;

            ray_grid_points_vertices[idx] = 	x_pos;
            ray_grid_points_vertices[idx + 1] = y_pos;
            ray_grid_points_vertices[idx + 2] = z_pos;
            ray_grid_points_vertices[idx + 3] = x_pos + 0.5;
            ray_grid_points_vertices[idx + 4] = y_pos + 0;
            ray_grid_points_vertices[idx + 5] = z_pos + 0.5;

            // colors
            ray_grid_points_colors[idx + 0] = m_randf();
            ray_grid_points_colors[idx + 1] = m_randf();
            ray_grid_points_colors[idx + 2] = m_randf();
            ray_grid_points_colors[idx + 3] = m_randf();
            ray_grid_points_colors[idx + 4] = m_randf();
            ray_grid_points_colors[idx + 5] = m_randf();

            idx+=6;
        }
    }
    printf("GRID: %d\n",idx );

//    printf("GRID_STATE: \nmin: %f %f %f\nmax: %f %f %f\n", grid_min_pos.x,grid_min_pos.y,grid_min_pos.z,grid_max_pos.x,grid_max_pos.y,grid_max_pos.z);
}

void build_humans() {

    int idx = 0;
    for (int i = 0; i < NUMBER_HUMAN_POINTS; ++i) {
        human_points_vertices[idx + 0] = grid_min_pos.x + (grid_max_pos.x-grid_min_pos.x) * m_randf() ;
        human_points_vertices[idx + 1] = grid_min_pos.y + (grid_max_pos.y-grid_min_pos.y) * m_randf() ;
        human_points_vertices[idx + 2] = grid_min_pos.z + (grid_max_pos.z-grid_min_pos.z) * m_randf() ;

        human_points_colors[idx + 0] = m_randf();
        human_points_colors[idx + 1] = m_randf();
        human_points_colors[idx + 2] = m_randf();

        idx+=3;
    }
}

void update_humans() {
    float accumulation_factor = 0.1;
    float distance_threshold = 1.2;
    int n_points_in_grid = RAY_SIDE_GRID * RAY_SIDE_GRID;
    for (int i = 0; i < NUMBER_HUMAN_POINTS; ++i) {
        closest_grid_point_idx[i] = -1;
    }

    int idx_humans = 0;

    for (int h = 0; h < NUMBER_HUMAN_POINTS; ++h) {
        float3* curr_human_point = (float3*)&human_points_vertices[idx_humans];
        int idx_grid = 0;
        for (int i = 0; i < n_points_in_grid; ++i) {
            ray* curr_grid_point = (ray*)&ray_grid_points_vertices[idx_grid];


            //printf("%f %f %f    -> %f %f %f\n",curr_grid_point->position.x,curr_grid_point->position.y, curr_grid_point->position.z, curr_human_point->x, curr_human_point->y,curr_human_point->z );
            float dist = distance(&curr_grid_point->position, curr_human_point);
            //printf("distance: %f\n", dist);

            if (dist < distance_threshold) {
                float dix = curr_grid_point->direction.x - curr_grid_point->position.x;
                float diy = curr_grid_point->direction.y - curr_grid_point->position.y;
                float diz = curr_grid_point->direction.z - curr_grid_point->position.z;
                //printf("-->> %f %f %f\n",dix,diy,diz );

                human_points_vertices[idx_humans + 0] += (dix) * accumulation_factor;
                human_points_vertices[idx_humans + 1] += (diy) * accumulation_factor;
                human_points_vertices[idx_humans + 2] += (diz) * accumulation_factor;
                break;
            }

            idx_grid += 6;
            //printf("idx_grid %d \n", idx_grid );
        }
        idx_humans += 3;
    }
}


int main(int argc, char const *argv[]) {
    // Test pointers
    float a[6];
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;
    a[3] = 3;
    a[4] = 4;
    a[5] = 5;

    printf("%f %f %f %f %f %f \n", a[0],a[1],a[2],a[3],a[4],a[5]);
    float3 *p1 = (float3*)a;
    float3 *p2 = (float3*)(a + 3);
    printf("1: float3 %f %f %f\n", p1->x, p1->y, p1->z);
    printf("2: float3 %f %f %f\n", p2->x, p2->y, p2->z);


    grid_min_pos.x = 1000000;
    grid_min_pos.y = 1000000;
    grid_min_pos.z = 1000000;
    float3 grid_max_pos;
    grid_max_pos.x = 0;
    grid_max_pos.y = 0;
    grid_max_pos.z = 0;

    GLFWwindow* window;
    if(!glfwInit()) {
        l("glfw init failed");
    }
    else {
        l("Glfw initialized");
    }
    window = glfwCreateWindow(WIN_W, WIN_H, "Fields", NULL, NULL);
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
    ref_points_vertices = (float*) malloc(NUMBER_OF_FLOAT3_TRACE_POINTS * 3 * sizeof(float)); // multiply by three to have space for 3 floats each
    ref_points_colors = (float*) malloc(NUMBER_OF_FLOAT3_TRACE_POINTS * 3 * sizeof(float));
    // reset trace points
    for(int i = 0; i< NUMBER_OF_FLOAT3_TRACE_POINTS; i++) {
        ref_points_vertices[i] = 0;
        ref_points_vertices[i + 1] = 0;
        ref_points_vertices[i + 2] = 0;

        // Setup random color for each point initially
        ref_points_colors[i] = m_randf();
        ref_points_colors[i + 1] = m_randf();
        ref_points_colors[i + 2] = m_randf();
    }

    // Picking is not working with ortho. Maybe it is more simple than I thought. Think about how to project the
    // mouse ray with this type of projection. Why is it working with perspective?
    // m_mat4_ortho(projection_matrix, -2.0, 2.0, -1.0, 1.0, 1, 950);
    // lm("Ortho projection? ", projection_matrix);

    float time = 0;
    float aspect = WIN_W / (float)WIN_H;
    m_mat4_perspective(projection_matrix, 10.0, aspect, 0.1, 100.0);
    m_mat4_identity(view_matrix);

    colored_cube cube;
    cube.vertices = cube_vertices;
    cube.colors = cube_colors;
    cube.vert_count = (sizeof(cube_vertices) / sizeof(float)) / 3;
    cube.shader_program = compile_shader_program(str_colored_cube_vert_shader, str_colored_cube_frag_shader, "position", "color");
    cube.uniform_picked_location = glGetUniformLocation(cube.shader_program, "u_picked");
    cube.uniform_time_location = glGetUniformLocation(cube.shader_program, "u_time");
    cube.uniform_model_matrix_location = glGetUniformLocation(cube.shader_program, "u_model_matrix");
    cube.uniform_view_matrix_location = glGetUniformLocation(cube.shader_program, "u_view_matrix");
    cube.uniform_projection_matrix_location = glGetUniformLocation(cube.shader_program, "u_projection_matrix");

    ref_points_shader_program = compile_shader_program(str_trace_points_vert_shader, str_trace_points_frag_shader, "position", "color");

    glPointSize(16.0);
    glLineWidth(1.0);
    float reference_offset = 1.0;

    write_trace_point_xyz(ref_points_vertices, 0,0,0);
    write_trace_point_xyz(ref_points_vertices, reference_offset,reference_offset,reference_offset);

    write_trace_point_xyz(ref_points_vertices, -reference_offset,0,0);
    write_trace_point_xyz(ref_points_vertices, reference_offset,0,0);

    write_trace_point_xyz(ref_points_vertices, 0,-reference_offset,0);
    write_trace_point_xyz(ref_points_vertices, 0,reference_offset,0);

    write_trace_point_xyz(ref_points_vertices, 0,0,-reference_offset);
    write_trace_point_xyz(ref_points_vertices, 0,0,reference_offset);



    int number_lines = RAY_SIDE_GRID * RAY_SIDE_GRID;
    ray_grid_points_vertices = (float*) malloc(number_lines * 2 * 3 * sizeof(float));
    ray_grid_points_colors = (float*) malloc(number_lines * 2 * 3 * sizeof(float));
    human_list = (ray*) malloc(NUMBER_HUMAN_POINTS * sizeof(ray));
    human_points_vertices = (float*) malloc(NUMBER_HUMAN_POINTS * 3 * sizeof(float));
    human_points_colors = (float*) malloc(NUMBER_HUMAN_POINTS * 3 * sizeof(float));

    closest_grid_point_idx = (int*) malloc(NUMBER_HUMAN_POINTS * sizeof(int));


    build_grid();
    build_humans();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float cam_speed = 20;
        set_vec(&camera.position, 20 ,20, 0);
        set_vec(&camera.direction, 0 - camera.position.x ,0 - camera.position.y , 0 - camera.position.z);
        set_vec(&camera.up, 0,1,0);
        m_mat4_lookat(view_matrix, &camera.position, &camera.direction, &camera.up);
    	


        //Draw trace points
        glUseProgram(ref_points_shader_program);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, ref_points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ref_points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_LINES, 0, NUMBER_OF_FLOAT3_TRACE_POINTS);
        glUseProgram(0);

        glUseProgram(ref_points_shader_program);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, ref_points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ref_points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_POINTS, 0, NUMBER_OF_FLOAT3_TRACE_POINTS);
        glUseProgram(0);

        // todo:
        // -x- Create list of points to serve as a grid with a ray
        // Create list of humans to draw
        // Influence position of human based on the values in the proximity


        influence_ray_grid_mouse();
        update_humans();

        glUseProgram(ref_points_shader_program);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, ray_grid_points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, ray_grid_points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_LINES, 0, (number_lines)  * 2);
        glUseProgram(0);


        glUseProgram(ref_points_shader_program);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_view_matrix"), 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(glGetUniformLocation(ref_points_shader_program, "u_projection_matrix"), 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, human_points_vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, human_points_colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_POINTS, 0, NUMBER_HUMAN_POINTS);
        glUseProgram(0);


        /*
        glUseProgram(cube.shader_program);
        float transl_matrix2[] = M_MAT4_IDENTITY();

        int n_points = SIDE_GRID * SIDE_GRID;

        for(int gx = 0; gx < n_points; ++gx ) {
        for(int gy = 0; gy < n_points; ++gy ) {
        int idx = gx;

        float scale = 1.1;
        float x = gx * scale;
        float z = gy * scale;
        float3 transl2 = {x, 0, z};
        m_mat4_translation(transl_matrix2, &transl2);

        float3 sphere_origin = {x,0,z};
        float intersection_in = 0;
        float intersection_out = 0;
        int intersection_res = m_3d_ray_sphere_intersection_in_out(&camera.position, &mouse.world, &sphere_origin, 0.01, &intersection_in, &intersection_out);
        float picked = 0.2 + intersection_res;

        glUniform1f (cube.uniform_time_location, time);
        glUniform1f (cube.uniform_picked_location, picked);

        glUniformMatrix4fv(cube.uniform_model_matrix_location, 1, GL_FALSE, transl_matrix2);
        glUniformMatrix4fv(cube.uniform_view_matrix_location, 1, GL_FALSE, view_matrix);
        glUniformMatrix4fv(cube.uniform_projection_matrix_location, 1, GL_FALSE, projection_matrix);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, cube.vertices);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, cube.colors);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, cube.vert_count); // 36
        }
        }
        */
        /*

        float transl_matrix2[] = M_MAT4_IDENTITY();

        glUseProgram(cube.shader_program);
        for (float i = -5; i < 5; i+=0.3) {
        	for (float j = -5; j < 5; j+=0.3) {
        	float3 transl2 = {i, 0, j};
        	m_mat4_translation(transl_matrix2, &transl2);

        	float3 sphere_origin = {i,0,j};
        	float intersection_in = 0;
        	float intersection_out = 0;
        	int intersection_res = m_3d_ray_sphere_intersection_in_out(&camera.position, &mouse.world, &sphere_origin, 0.01, &intersection_in, &intersection_out);
        	float picked = 0.2 + intersection_res;


        	glUniform1f (cube.uniform_time_location, time);
        	glUniform1f (cube.uniform_picked_location, picked);
        	glUniformMatrix4fv(cube.uniform_model_matrix_location, 1, GL_FALSE, transl_matrix2);
        	glUniformMatrix4fv(cube.uniform_view_matrix_location, 1, GL_FALSE, view_matrix);
        	glUniformMatrix4fv(cube.uniform_projection_matrix_location, 1, GL_FALSE, projection_matrix);
        	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, cube.vertices);
        	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, cube.colors);
        	glEnableVertexAttribArray(0);
        	glEnableVertexAttribArray(1);
        	glDrawArrays(GL_TRIANGLES, 0, cube.vert_count); // 36

        	}
        }

        */

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

