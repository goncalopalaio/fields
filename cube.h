
 #define CUBE_SIZE 0.1f
 float cube_vertices[] = {
      ////////////////////////////////////////////////////////////////////
      // FRONT
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // top-left
      -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-left
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      // Triangle 2
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // top-right
      -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // top-left
      ////////////////////////////////////////////////////////////////////
      // BACK
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // bottom-left
      CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // bottom-right
      // Triangle 2
      CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // bottom-right
      CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-right
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left

      ////////////////////////////////////////////////////////////////////
      // LEFT
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // bottom-left
      -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      // Triangle 2
      -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // top-right
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      ////////////////////////////////////////////////////////////////////
      // RIGHT
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // bottom-left
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      // Triangle 2
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // top-right
      CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left

      ////////////////////////////////////////////////////////////////////
      // TOP
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      -CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // bottom-left
      CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // bottom-right
      // Triangle 2
      CUBE_SIZE, CUBE_SIZE, CUBE_SIZE, // bottom-right
      CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-right
      -CUBE_SIZE, CUBE_SIZE, -CUBE_SIZE, // top-left
      ////////////////////////////////////////////////////////////////////
      // BOTTOM
      ////////////////////////////////////////////////////////////////////
      // Triangle 1
      -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // top-left
      -CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-left
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      // Triangle 2
      CUBE_SIZE, -CUBE_SIZE, CUBE_SIZE, // bottom-right
      CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE, // top-right
      -CUBE_SIZE, -CUBE_SIZE, -CUBE_SIZE // top-left
    };

float cube_colors[] = {
      // Triangle 1
      1, 0, 0,
      1, 0, 0,
      1, 0, 0,

      // Triangle 2
      1, 0, 0,
      1, 0, 0,
      1, 0, 0,


      // Triangle 1
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,

      // Triangle 2
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,

      // Triangle 1
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,

      // Triangle 2
      0, 0, 1,
      0, 0, 1,
      0, 0, 1,

      // Triangle 1
      1, 1, 0,
      1, 1, 0,
      1, 1, 0,

      // Triangle 2
      1, 1, 0,
      1, 1, 0,
      1, 1, 0,

      // Triangle 1
      1, 0, 1,
      1, 0, 1,
      1, 0, 1,

      // Triangle 2
      1, 0, 1,
      1, 0, 1,
      1, 0, 1,

      // Triangle 1
      1, 0.5, 0,
      1, 0.5, 0,
      1, 0.5, 0,

      // Triangle 2
      1, 0.5, 0,
      1, 0.5, 0,
      1, 0.5, 0,
    };
