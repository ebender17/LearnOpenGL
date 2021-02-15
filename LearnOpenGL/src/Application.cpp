
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

/*
* OpenGL Notes: 
* OpenGL is a state machine. Must bind (or select) buffer before it can be rendered. Think about how 
* layers operated in Photoshop. Layer must be selected to do work on said layer. 
* 
* Vertex is composed of many attributes: position, colors, normal
*/
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

    glfwTerminate();
    return 0;
}