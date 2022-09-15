#include "stdafx.h"
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "GL/freeglut_ext.h"
#include "InitShader.h"
#include <iostream>
#include <sstream>

static char* readShaderSource(const char* shaderFile)
{
	FILE* fp = fopen(shaderFile, "r");
	if ( fp == NULL ) 
	{ 
		return NULL; 
	}
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';
	fclose(fp);
	return buf;
}

GLuint InitShader(const char* vShaderFile, const char* fShaderFile)
{
	struct Shader 
	{
		const char*  filename;
		GLenum       type;
		GLchar*      source;
	} shaders[2] = 
	{
		{ vShaderFile, GL_VERTEX_SHADER, NULL },
		{ fShaderFile, GL_FRAGMENT_SHADER, NULL }
	};

	GLuint program = glCreateProgram();

	for ( int i = 0; i < 2; ++i ) 
	{
		Shader& s = shaders[i];
		s.source = readShaderSource( s.filename );
		if ( shaders[i].source == NULL ) 
		{
			std::stringstream err;
			err << "Failed to read " << s.filename;
			std::cerr << err.str() << std::endl;
			throw std::exception(err.str().c_str());
			exit( EXIT_FAILURE );
		}

		GLuint shader = glCreateShader( s.type );
		glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
		glCompileShader( shader );

		GLint  compiled;
		glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
		if ( !compiled ) 
		{
			std::stringstream err;
			err << s.filename << " failed to compile: ";
			GLint  logSize;
			glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
			char* logMsg = new char[logSize];
			glGetShaderInfoLog( shader, logSize, NULL, logMsg );
			err << logMsg;
			std::cerr << err.str() << std::endl;
			delete [] logMsg;
			throw std::exception(err.str().c_str());
			exit( EXIT_FAILURE );
		}
		delete [] s.source;
		glAttachShader(program, shader);
	}

	glLinkProgram(program);

	GLint  linked;
	glGetProgramiv( program, GL_LINK_STATUS, &linked );
	if ( !linked ) 
	{
		std::stringstream err;
		err << "Shader program failed to link: ";
		GLint  logSize;
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize);
		char* logMsg = new char[logSize];
		glGetProgramInfoLog( program, logSize, NULL, logMsg );
		err << logMsg;
		std::cerr << err.str() << std::endl;
		delete [] logMsg;
		throw std::exception(err.str().c_str());
		exit( EXIT_FAILURE );
	}

	glUseProgram(program);
	return program;
}
