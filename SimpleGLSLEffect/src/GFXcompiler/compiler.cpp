#include <stdio.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "gfxRecognizer.h"
#include "testCompiler.h"

std::string readFile( const char *filename ) {
	FILE *file = fopen( filename, "rt" );
	if( !file ) {
		return std::string();
	}

	fseek( file, 0, SEEK_END );
	int size = ftell( file );
	fseek( file, 0, SEEK_SET );

	std::string fileContent;
	fileContent.resize( size );

	int realSize = fread( &fileContent[0], 1, size, file );
	fclose( file );

	// remove space overhead caused by \r\n to \n conversion
	fileContent.resize( realSize );

	return fileContent;
}

void initOutputStage( const EffectFile *file, const std::string &className );
std::string headerFile();
std::string sourceFile();
std::string codeDump( const Pass &pass, SharedCode::TypeFunctor type );

int main(int argc, char **argv) {
	std::string originalFileName;
	bool testCompile = true;
	bool forceOutput = false;

	int filenamearg;
	for( filenamearg = 1 ; filenamearg < argc && strncmp( argv[filenamearg], "--", 2 ) == 0 ; filenamearg++ ) {
		if( stricmp( argv[filenamearg], "--originalFilename" ) == 0  ) {
			originalFileName = argv[++filenamearg];
		}
		else if( stricmp( argv[filenamearg], "--noCompile" ) == 0  ) {
			testCompile = false;
		}
		else if( stricmp( argv[filenamearg], "--forceOutput" ) == 0  ) {
			forceOutput = true;
		}
	}
	if( filenamearg == argc ) {
		printf( "gfxcompiler [--forceOutput] [--noCompile] [--originalFilename <filename>] <gfx path>\n" );
		return -1;
	}

	std::string gfxFilePath( argv[ filenamearg ] );
	if( originalFileName.empty() ) {
		originalFileName = gfxFilePath;
	}

	int lastslash = gfxFilePath.find_last_of( '/' );
	if( lastslash == std::string::npos ) {
		lastslash = 0;
	}
	else {
		lastslash++;
	}
	int lastbackslash = gfxFilePath.find_last_of( '\\' );
	if( lastbackslash == std::string::npos ) {
		lastbackslash = 0;
	}
	else {
		lastbackslash++;
	}
	int lastdirseparator = max( lastslash, lastbackslash );
	std::string gfxFileName = gfxFilePath.substr( lastdirseparator );
	std::string gfxPath = gfxFilePath.substr( 0, lastdirseparator );

	std::string gfxFileNameBase = gfxFileName;
	int extdot = gfxFileNameBase.find_last_of( '.' );
	if( extdot != std::string::npos ) {
		gfxFileNameBase.resize( extdot );
	}

	std::string gfxContent = readFile( gfxFilePath.c_str() );
	str::TextSource textSource( gfxFilePath, gfxContent );

	EffectFile effectFile;
	try {
		effectFile.recognize<CSkipRule>( textSource.getFragment() );
	}
	catch( std::exception &exception ) {
		printf( "Parsing Error:\n%s\n", exception.what() );
		return -1;
	}

	initOutputStage( &effectFile, gfxFileNameBase );

	bool allCompiled = true;
	if( testCompile ) {			
		testCompiler::init();
		// test compile each pass
		testCompiler::filename = originalFileName;

		// NOTE: this code should match the output of the string template
		std::string versionInfo = "";
		if( effectFile.version != EffectFile::VERSION_NOT_SPECIFIED ) {
			versionInfo = "#version " + strprintf( "%i", effectFile.version ) + "\r\n" /*+ "#line 1 \"" + shaderTestCompilation.filename + "\"\r\n"*/;
		}

		testCompiler::vertexShaderInitCode = versionInfo + "#define VERTEX_SHADER\r\n";
		testCompiler::geometryShaderInitCode = versionInfo + "#define GEOMETRY_SHADER\r\n";
		testCompiler::fragmentShaderInitCode = versionInfo + "#define FRAGMENT_SHADER\r\n";

		std::list< Pass >::const_iterator passIter = effectFile.passes.begin();
		for(size_t i = 0 ; i < effectFile.passes.size() ; i++, passIter++ ) {
			const Pass &pass = *passIter;

			std::string vertexShaderCode;
			if( pass.hasVertexShader ) {
				vertexShaderCode = codeDump( pass, &SharedCode::isForVertexShaders );
			}
			std::string geometryShaderCode;
			if( pass.hasGeometryShader ) {
				geometryShaderCode = codeDump( pass, &SharedCode::isForGeometryShaders );
			}
			std::string fragmentShaderCode;
			if( pass.hasFragmentShader ) {
				fragmentShaderCode = codeDump( pass, &SharedCode::isForFragmentShaders );
			}

			printf( "Pass %i:\n", i );
			//System.out.println(vertexShaderCode);
			//System.out.println(geometryShaderShared);
			//System.out.println(fragmentShaderShared);
			allCompiled = testCompiler::compile( vertexShaderCode, geometryShaderCode, fragmentShaderCode ) && allCompiled;
		}

		testCompiler::shutdown();
	}

	if( allCompiled || forceOutput ) {
		std::string sourceFileName = gfxPath + gfxFileNameBase + ".cpp";
		std::ofstream sourceFileStream( sourceFileName.c_str() );

		sourceFileStream << sourceFile();

		std::string headerFileName = gfxPath + gfxFileNameBase + ".h";

		// only write the header file if something has changed  [6/14/2010 Andreas Kirsch]
		std::string oldHeaderFileContent = readFile( headerFileName.c_str() );
		std::string headerFileContent = headerFile();

		printf( "Output written to %s", sourceFileName.c_str() );

		if( oldHeaderFileContent != headerFileContent ) {
			std::ofstream headerFileStream( headerFileName.c_str() );
			headerFileStream << headerFileContent;
			printf( " and %s", headerFileName.c_str() );
		}

		printf( "\n" );
	}
	else {
		printf( "\nNo output was written!\n");
	}
}
