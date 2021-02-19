
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

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

    /* Placed inside new scope so Buffers are destroyed before glfwTerminate when the glfw context is destroyed */
    /* Best to heap allocate buffers and destroy before glfwTerminate. Rare case here as making vBuffers in main func scope */
    {

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

        VertexArray va; 
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));
        VertexBufferLayout layout; 
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

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
            va.Bind();
            /* binding index buffer*/
            ib.Bind();

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
    }

    glfwTerminate();
    return 0;
}