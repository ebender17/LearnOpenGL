#include "Shader.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"


Shader::Shader(const std::string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
	
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath)
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

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
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

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

/* Need to provide OpenGL with srings source code to read in shaders */
unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // can use GLUint as well as unsigned int to store id 
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    // Consult docs
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    // delete after being linked into program (stored in program), think deleting intermediates 
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::UnBind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}



unsigned int Shader::GetUniformLocation(const std::string& name)
{
    /* 
    * If uniform location is found do not have to retrieve again, saves on performance
    * when there are a lot of uniforms. 
    */
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    /* Shader must be binded before setting uniform as below */
    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    
    /* 
    * Not using assert as -1 could be a possible location if uniform is not used
    * and stripped from the shader. Therefore, we do not want app to have errors
    * if this occurs. 
    */
    if (location == -1)
        std::cout << "Warning: uniform " << name << " doesn't exist!" << std::endl;
    //save in cache 
    m_UniformLocationCache[name] = location;
    return location;
}
