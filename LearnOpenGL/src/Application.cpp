
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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
* 
* GPUs use triangles as their drawing primatives. Therefore, rectangles are composed of two triangles. However, 
* this causes vertices to be stored more than once (duplicated). Can avoid this memory waste using index buffers. 
* 
* Index buffers: Allow one to reuse vertices. 
*/

struct ShaderProgramSource
{
    std::string VertexSource; 

    std::string FragmentSource; 
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::fstream stream(filepath);

    /* Using type to act as index into correct array */
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE; 

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT; 
            
        }
        else
        {
            ss[(int)type] << line << '\n'; 
        }
    }

    return { ss[0].str(), ss[1].str() };

}

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

    float positions[] = {
        -0.5f, -0.5f, 
        0.5f, -0.5f, 
        0.5f, 0.5f, 
        -0.5f, 0.5f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    /* id of generated buffer */
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    /* selecting buffer */
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    /* give OpenGL the data, can do this later buffer just needs to be bound */
    /* cannot use a signed type for index buffer*/
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), positions, GL_STATIC_DRAW);


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

    /* index buffer */
    unsigned int ibo;
    glGenBuffers(1, &ibo);
    /* selecting buffer */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    /* give OpenGL the data, can do this later buffer just needs to be bound */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    /* bind shader */
    glUseProgram(shader); 

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* draw call used w/o index buffers */
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        /* draw call used w/ index buffers */
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}