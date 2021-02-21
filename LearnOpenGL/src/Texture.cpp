#include "Texture.h"

// can include vendor folder in include path for complier if creating more serious app

#include "vendor/stb_image/stb_image.h"

Texture::Texture(const std::string& path)
	: m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), 
	m_Width(0), m_Height(0), m_BPP(0)
{
	/*
	* Flips our texture vertically
	* OpenGL expects texture pixel to start at bottom left, not top left
	* Depends on texture format!
	*/
	stbi_set_flip_vertically_on_load(1); 
	// last param is desired channels, we want 4 for rgba
	m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4); 
	// now have all texture data in this local buffer

	GLCall(glGenTextures(1, &m_RendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID)); 

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	// Do not want to tile on x
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	// Do not want to tile on y
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	//Give OpenGL the texture that was read in
	/*
	* @param Internal format - how OpenGL will store texture data 
	* @param Format - the texture data's format
	*/
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	/* 
	* In more complicated setups may want to retain a copy of the pixel data on CPU
	* b/c you may want to sample or do something else with it
	* */
	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);
}

Texture::~Texture()
{
	GLCall(glDeleteTextures(1, &m_RendererID));
}

void Texture::Bind(unsigned int slot) const
{
	/* 
	* Set glActiveTexture to a slot. The next texture bond will be bound to slot set unti
	* glActiveTexture is called again with a different slot. 
	* GLTexture0 is an enum. Therefore, adding slot to the enum will choose the slot wanted. 
	* */
	GLCall(glActiveTexture(GL_TEXTURE0 + slot)); 
	GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
}

void Texture::UnBind() const
{
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
