
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
            -0.5f, -0.5f, 0.0f, 0.0f, //Bottom left, last two floats are text coords
            0.5f, -0.5f, 1.0f, 0.0f, //Bottom right
            0.5f, 0.5f, 1.0f, 1.0f, // Top right
            -0.5f, 0.5f, 0.0f, 1.0f //Top left
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
        * ortho projecting because we are dealing with a 2D world
        * The first four values specify the boundries of our window. 
        * */
        glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f); 

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);
        shader.SetUniformMat4f("u_MVP", proj);

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