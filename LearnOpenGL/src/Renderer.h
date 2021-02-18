#pragma once
#include <GL/glew.h>

/* using msvc compiler specific func debug break */
#define ASSERT(x) if (!(x)) __debugbreak();

/* # converts int to string, __FILE__ & __LINE__ are intrinsics, should work on all compilers */
#define GLCall(x) GLClearError();\
x;\
ASSERT(GLLogCall(#x, __FILE__, __LINE__))

/* call clear before gl func to clear errors to ensure we are not getting errors from other functions */
void GLClearError();

/* call after GLClearError and gl func to only get errors from the above func */
/*
* @param function   name of func called
* @param file   c++ src file where func was called from
* @param line   line func was called from
*/
bool GLLogCall(const char* function, const char* file, int line);



