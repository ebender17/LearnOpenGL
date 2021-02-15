
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

/*
* OpenGL Notes: 
* OpenGL is a state machine. Must bind (or select) buffer before it can be rendered. Think about how 
* layers operated in Photoshop. Layer must be selected to do work on said layer. 
* 
* Basic rendering pipeline: Draw call -> Vertex Shader -> Fragment Shader
* 
* Vertex is composed of many attributes: position, colors, normal
* 
* Shaders: Programs ran on GPU. 
* 
* Vertex Shader: Called for each vertex (ex. called three times for a triangle). 
* Primary purpose is to tell OpenGL where you want the vertex
* to be in the window. Also used to pass attributes into next stage (Fragment Shader)
* 
* Fragment Shader: Runs once for each pixel that needs to be rasterized. Decides the correct color the 
* pixel needs to be. 
* 
* Keep in mind: Better to perform calculations in vertex shader if possible (as it is less called less). 
* Sometime cannot avoid calculations in Fragment Shader however (such as lighting). 
*/

static unsigned int CompileShader( unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); // &source[0]
    /* specified shader source code */
    glShaderSource(id, 1, &src, nullptr); 
    glCompileShader(id);

    int result; 
    /* iv - i specifies we are dealing with an int, v specifies we want a vector (array) or in this case a ptr */
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    if (result == GL_FALSE)
    {
        /* get error message*/
        /* query error length */
        int length; 
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        /* alloca is a c func that allows you to allowcate on the stack dynamically */
        char* message = (char*)alloca(length * sizeof(char)); 
        glGetShaderInfoLog(id, length, &length, message); 

        std::cout << "Failed to compile "<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl; 
        std::cout << message << std::endl;

        glDeleteShader(id); 
        return 0; 
    }

    return id; 
}

/* Need to provide OpenGL with srings source code to read in shaders */
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    /* returns id to shader */
    /* can use GLUint as well as unsigned int to store id */
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs); 
    glAttachShader(program, fs); 
    // Consult docs
    glLinkProgram(program); 
    glValidateProgram(program);

    /* delete after being linked into program (stored in program), think deleting intermediates */
    glDeleteShader(vs);
    glDeleteShader(fs); 

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Practice", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* 
    * Initialization succeeded
    * Can use the available extensions as well as core OpenGL functionality 
    */
    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    /* Print OpenGL version */ 
    std::cout << glGetString(GL_VERSION) << std::endl; 

    float positions[6] = {
        -0.5f, -0.5f, 
        0.0f, 0.5f, 
        0.5f, -0.5f
    };

    /* id of generated buffer */
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    /* selecting buffer */
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    /* give OpenGL the data, can do this later buffer just needs to be bound */
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    /* To enable and disable generic vertex attribute array */
    glEnableVertexAttribArray(0);
    /* glVertexAttribPointer info:
    * index - index of attribute in buffer
    * size -  number of components per generic vertex attribute (x, y coords? 2)
    * type - type of data we are providing (float)
    * normalized - specifies whether fixed-point data values shold be normalized (GL_TRUE)
    * stride - amount of bytes between each vertex, how many bytes to go forward to next vertex
    * pointer - how many bytes to go forward to next attribute, bytes to attributes from vertex ptr
    */
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    /* Using GLSL, OpenGL Shading Language */
    /* location correlates to attribute position */
    std::string vertexShader = 
        "#version 330 core\n"
        "\n"
        "layout(location = 0) in vec4 position;"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = position;\n"
        "}\n"; 

    std::string fragmentShader =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) out vec4 color;"
        "\n"
        "void main()\n"
        "{\n"
        "   color = vec4(0.1, 0.9, 1.0, 1.0);\n"
        "}\n";

    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    /* bind shader */
    glUseProgram(shader); 

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* draw call used w/o index buffers */
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}