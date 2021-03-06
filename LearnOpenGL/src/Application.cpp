
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
    window = glfwCreateWindow(960, 540, "OpenGL Practice", NULL, NULL);
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
            100.0f, 100.0f, 0.0f, 0.0f, //Bottom left, last two floats are text coords
            200.0f, 100.0f, 1.0f, 0.0f, //Bottom right
            200.0f, 200.0f, 1.0f, 1.0f, // Top right
            100.0f, 200.0f, 0.0f, 1.0f //Top left
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        //Enables transparency
        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexArray va; 
        VertexBuffer vb(positions, 4 * 4 * sizeof(float));
        VertexBufferLayout layout; 
        layout.Push<float>(2);
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        /* 
        * Projection Matrix 
        * 4 by 3 aspect ratio
        * The first four values specify the bounds 
        */
        glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
        /* 
        * view matrix 
        * moving "camera" to the right
        */
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));
        /* 
        * moderl matrix 
        * moving moderl 200 to the right and 200 upwards
        */
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));

        // in reverse order due to OpenGL expecting data in column major order
        glm::mat4 mvp = proj * view * model; 

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", mvp);

        Texture texture("res/textures/Emily_D&P_NoBG.png");
        texture.Bind();
        // Pass in 0 b/c we have bound texture to slot 0
        shader.SetUniform1i("u_Texture", 0);

        /* clearing everything so I can try reseting later */
        va.UnBind(); 
        vb.UnBind();
        ib.UnBind();

        Renderer renderer;

        float r = 0.0f;
        float increment = 0.05f;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            renderer.Clear();

            /* 
            * Renderer typically takes in a material instead of a shader. 
            * Therefore, uniforms would not have to be handled as below. 
            * Will get to materials (shaders + uniforms) in the future. 
            */
            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            renderer.Draw(va, ib, shader);

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

    }

    glfwTerminate();
    return 0;
}