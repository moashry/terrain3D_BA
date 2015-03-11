#include <string>

#include "windows.h"
#include "GL/GL.h"

#include "stdio.h"

namespace testCompiler {
	std::string vertexShaderInitCode;
	std::string geometryShaderInitCode;
	std::string fragmentShaderInitCode;

	std::string filename;

	bool init();
	void shutdown();

	bool compile(const std::string &vertexShaderCode, const std::string &geometryShaderCode, const std::string &fragmentShaderCode);
}

using namespace testCompiler;

static HWND dummyWindow;
static HDC dummyWindowDC;
static HGLRC glContext;

// from http://svn.assembla.com/svn/imperfectcircle/gfx/RenderSystems/GL/src/OgreWin32GLSupport.cpp
static bool initDummyWindow() {
	LPCSTR dummyText = "WglDummy";
	HINSTANCE hinst = GetModuleHandle( NULL );
		
	WNDCLASS dummyClass;
	memset(&dummyClass, 0, sizeof(WNDCLASS));
	dummyClass.style = CS_OWNDC;
	dummyClass.hInstance = hinst;
	dummyClass.lpfnWndProc = DefWindowProc;
	dummyClass.lpszClassName = dummyText;
	RegisterClass(&dummyClass);

	HWND hwnd = CreateWindow(dummyText, dummyText,
		WS_POPUP | WS_CLIPCHILDREN,
		0, 0, 32, 32, 0, 0, hinst, 0);

	// if a simple CreateWindow fails, then boy are we in trouble...
	if (hwnd == NULL)
		return false;

	// no chance of failure and no need to release thanks to CS_OWNDC
	dummyWindowDC = GetDC(hwnd); 

	// assign a simple OpenGL pixel format that everyone supports
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.cColorBits = 16;
	pfd.cDepthBits = 15;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	
	// if these fail, wglCreateContext will also quietly fail
	int format;
	if ((format = ChoosePixelFormat(dummyWindowDC, &pfd)) != 0) {
		SetPixelFormat(dummyWindowDC, format, &pfd);

		glContext = wglCreateContext(dummyWindowDC);
		if (glContext)
		{
			// if wglMakeCurrent fails, wglGetProcAddress will return null
			wglMakeCurrent(dummyWindowDC, glContext);
			return true;
		}
	}

	DestroyWindow(dummyWindow);
	UnregisterClass(dummyText, hinst);
	return false;
}

static void destroyDummyWindow() {
	LPCSTR dummyText = "WglDummy";
	HINSTANCE hinst = GetModuleHandle( NULL );

	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( glContext );

	// clean up our dummy window and class
	DestroyWindow( dummyWindow );
	UnregisterClass(dummyText, hinst);
}

void (APIENTRY *glCompileShader) (GLuint shader);
GLuint (APIENTRY *glCreateProgram) (void);
GLuint (APIENTRY *glCreateShader) (GLenum type);
void (APIENTRY *glDeleteProgram) (GLuint program);
void (APIENTRY *glDeleteShader) (GLuint shader);

typedef char GLchar;

void (APIENTRY *glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
void (APIENTRY *glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void (APIENTRY *glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
void (APIENTRY *glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

void (APIENTRY *glAttachShader) (GLuint program, GLuint shader);

void (APIENTRY *glLinkProgram) (GLuint program);
void (APIENTRY *glShaderSource) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
void (APIENTRY *glUseProgram) (GLuint program);

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_GEOMETRY_SHADER                0x8DD9

static bool initGLSLFunctions() {
	bool success = true;

#define INIT( name ) success = ((*(void**)&(name)) = wglGetProcAddress( #name )) && success
	INIT( glCompileShader );
	INIT( glCreateProgram );
	INIT( glCreateShader );
	INIT( glDeleteProgram );
	INIT( glDeleteShader );

	INIT( glGetProgramiv );
	INIT( glGetProgramInfoLog );
	INIT( glGetShaderiv );
	INIT( glGetShaderInfoLog );

	INIT( glAttachShader );
	INIT( glLinkProgram );
	INIT( glShaderSource );
	INIT( glUseProgram );
#undef INIT

	return success;
}

static const int logSize = 2048;

static bool compileShader( int shader, const char **codes, int numCodes, std::string &logMessage ) {
	glShaderSource(shader, numCodes, codes, NULL );
	glCompileShader(shader);

	char log[ logSize ];
	int logLength;
	glGetShaderInfoLog(shader, logSize, &logLength, log );

	if( logLength > 0 ) {
		logMessage = log;
	}

	int compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	return compileStatus == GL_TRUE;
}

static bool compileShaderAndDumpMessages( int shader, const char **codes, int numCodes, const std::string &headerMessage ) {
	std::string logMessage;
	bool compiled = compileShader(shader, codes, numCodes, logMessage);

	if( !logMessage.empty() ) {
		printf( "%s\n%s\n", headerMessage.c_str(), logMessage.c_str() );
	}

	return compiled;
}

bool testCompiler::compile(const std::string &vertexShaderCode, const std::string &geometryShaderCode, const std::string &fragmentShaderCode) {
	GLuint program = glCreateProgram();

	GLuint vertexShader = 0;
	if( !vertexShaderCode.empty() )
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

	GLuint geometryShader = 0;
	if( !geometryShaderCode.empty() )
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

	GLuint fragmentShader = 0;
	if( !fragmentShaderCode.empty() )
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	bool allCompiledAndLinked = true;

	// no early out, dump all the dirty details
	if( vertexShader != 0 ) {
		const char *codes[] = {
			vertexShaderInitCode.c_str(), "", vertexShaderCode.c_str()
		};
		allCompiledAndLinked = compileShaderAndDumpMessages(vertexShader, codes, 3, "Vertex Shader:") && allCompiledAndLinked;

		glAttachShader(program, vertexShader);
	}
	if( geometryShader != 0 ) {
		const char *codes[] = {
			geometryShaderInitCode.c_str(), "", geometryShaderCode.c_str()
		};

		allCompiledAndLinked = compileShaderAndDumpMessages(geometryShader, codes, 3, "Geometry Shader:") && allCompiledAndLinked;

		glAttachShader(program, geometryShader);
	}
	if( fragmentShader != 0 ) {
		const char *codes[] = {
			fragmentShaderInitCode.c_str(), "", fragmentShaderCode.c_str()
		};
		allCompiledAndLinked = compileShaderAndDumpMessages(fragmentShader, codes, 3, "Fragment Shader:") && allCompiledAndLinked;

		glAttachShader(program, fragmentShader);
	}

	if( allCompiledAndLinked ) {
		glLinkProgram(program);
		char log[ logSize ];
		int logLength;
		glGetProgramInfoLog(program, logSize, &logLength, log);

		if( logLength > 0 ) {
			printf( "Linking:\n" );
			printf( "%s\n", log );
		}

		int linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		allCompiledAndLinked = (linkStatus == GL_TRUE) && allCompiledAndLinked;
	}

	if( vertexShader != 0 )
		glDeleteShader(vertexShader);
	if( geometryShader != 0 )
		glDeleteShader(geometryShader);
	if( fragmentShader != 0 )
		glDeleteShader(fragmentShader);

	glDeleteProgram(program);

	return allCompiledAndLinked;
}

bool testCompiler::init() {
	if( !initDummyWindow() ) {
		return false;
	}

	if( !initGLSLFunctions() ) {
		destroyDummyWindow();
		return false;
	}

	return true;
}

void testCompiler::shutdown() {
	destroyDummyWindow();
}