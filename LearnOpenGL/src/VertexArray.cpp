#include "VertexArray.h"

#include "VertexBufferLayout.h"
#include "Renderer.h"

VertexArray::VertexArray()
{
    GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
    /* bind vertex array */
    Bind();
    /* bind vertex buffer */
	vb.Bind();
    /* setup layout */
    const auto& elements = layout.GetElements();
    for (unsigned int i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i]; 
        unsigned int offset = 0;

        /* To enable and disable index in vertex attribute array */
        GLCall(glEnableVertexAttribArray(i));

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
        GLCall(glVertexAttribPointer(i, element.count, element.type, 
            element.normalized, layout.GetStride(), (const void*)offset));
        offset += element.count * VertexBufferElement::GetSizeOfType(element.type); 

    }
    
}

void VertexArray::Bind() const
{
    GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::UnBind() const
{
    GLCall(glBindVertexArray(0));
}
