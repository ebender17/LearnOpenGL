
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/* using msvc compiler specific func debug break */
#define ASSERT(x) if (!(x)) __debugbreak();
/* # converts int to string, __FILE__ & __LINE__ are intrinsics, should work on all compilers */
#define GLCall(x) GLClearError();\
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__))

/* call clear before gl func to clear errors to ensure we are not getting errors from other functions */
static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

/* call after GLClearError and gl func to only get errors from the above func */
/*
* @param function   name of func called 
* @param file   c++ src file where func was called from
* @param line   line func was called from
*/
static bool GLLogCall(const char* function, const char* file, int line)
{
    /* while error does not equal 0 */
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "):" << function << " " << file << ":" << line << std::endl; 
        return false;
    }
    return true; 
}

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
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str(); // &source[0]
    /* specified shader source code */
    GLCall(glShaderSource(id, 1, &src, nullptr)); 
    GLCall(glCompileShader(id));

    int result; 
    /* iv - i specifies we are dealing with an int, v specifies we want a vector (array) or in this case a ptr */
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result)); 
    if (result == GL_FALSE)
    {
        /* get error message*/
        /* query error length */
        int length; 
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        /* alloca is a c func that allows you to allowcate on the stack dynamically */
        char* message = (char*)alloca(length * sizeof(char)); 
        GLCall(glGetShaderInfoLog(id, length, &length, message)); 

        std::cout << "Failed to compile "<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl; 
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id)); 
        return 0; 
    }

    return id; 
}

/* Need to provide OpenGL with srings source code to read in shaders */
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    /* returns id to shader */
    /* can use GLUint as well as unsigned int to store id */
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs)); 
    GLCall(glAttachShader(program, fs)); 
    // Consult docs
    GLCall(glLinkProgram(program)); 
    GLCall(glValidateProgram(program));

    /* delete after being linked into program (stored in program), think deleting intermediates */
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs)); 

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create window and context with CORE profile*/
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Practice", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* synchronizes with our monitor's frame rate */
    glfwSwapInterval(1);

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

    /* generate vao */
    unsigned int vao; 
    GLCall(glGenVertexArrays(1, &vao)); 
    GLCall(glBindVertexArray(vao));

    /* id of generated buffer */
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    /* selecting buffer */
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    /* give OpenGL the data, can do this later buffer just needs to be bound */
    /* cannot use a signed type for index buffer*/
    GLCall(glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(float), positions, GL_STATIC_DRAW));


    /* To enable and disable index in vertex attribute array */
    GLCall(glEnableVertexAttribArray(0));
    /* glVertexAttribPointer info:
    * Tells OpenGL how to read data. Specifies layout. 
    * @param index - index of attribute in buffer
    * @param size -  number of components per generic vertex attribute (x, y coords? 2)
    * @param type - type of data we are providing (float)
    * @param normalized - specifies whether fixed-point data values shold be normalized (GL_TRUE)
    * @param stride - amount of bytes between each vertex, how many bytes to go forward to next vertex
    * @param pointer - how many bytes to go forward to next attribute, bytes to attributes from vertex ptr
    */
    /* Binds vao to currently bound vertex buffer */
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    /* index buffer */
    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    /* selecting buffer */
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    /* give OpenGL the data, can do this later buffer just needs to be bound */
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    /* bind shader */
    GLCall(glUseProgram(shader)); 

    /* Shader must be binded as above before setting uniform as below */
    /* Retreiving location of color variable */
    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    /* OpenGL strips uniforms not used by shader, therefore location == -1 is not necesarrily a fatal error */
    ASSERT(location != -1);
    /* Set uniform in shader */
    GLCall(glUniform4f(location, 0.1f, 0.9f, 1.0f, 1.0f));

    /* clearing everything so I can try reseting later */
    GLCall(glBindVertexArray(0)); 
    GLCall(glUseProgram(0)); 
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); 

    float r = 0.0f;
    float increment = 0.05f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
       
        GLCall(glUseProgram(shader));
        GLCall(glUniform4f(location, r, 0.9f, 1.0f, 1.0f));

        /* binding vertex array*/
        /* no longer need to bind vertex buffer and set layout, just bind vao as vao and vbo are linked */
        GLCall(glBindVertexArray(vao));
        /* binding index buffer*/
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

        /* draw call used w/ index buffers */
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment; 

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }

    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}