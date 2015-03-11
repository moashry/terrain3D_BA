// Andreas Kirsch
#include <cstdio>
#include <vector>
#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <fstream>

#include "textSource.h"

// change this if another way should be used to convert an arbitrary expression to std::String
std::string getExpressionToStringCode( const std::string &expressionCode ) {
	return 	"((std::ostringstream*)&(std::ostringstream() << (" + expressionCode + ")))->str()";
}

bool checkLiteral( str::TextIterator &text, const std::string &literal ) {
	return std::strncmp( text.getPosition().getPointer(), literal.c_str(), literal.size() ) == 0;
}

bool matchLiteral( str::TextIterator &text, const std::string &literal ) {
	if( std::strncmp( text.getPosition().getPointer(), literal.c_str(), literal.size() ) == 0 ) {
		text += literal.size();
		return true;
	}
	return false;
}

bool matchStringLiteral( str::TextIterator &text ) {
	if( matchLiteral(text, "\"" ) ) {
		// parse quotation
		while( !matchLiteral( text, "\"" ) ) {
			if( *text == '\\' ) {
				++text;
			}
			++text;
		}
		return true;
	}
	return false;
}

// read text and escape everything in-between <<< >>> markers and also between @"..." - only avoid doing this inside quotation marks
void processText( std::ostream &out, str::TextIterator &text, bool useExitToken = false, const std::string &exitToken = "" ) {
	str::TextPosition begin = text.getPosition();
	while( (!useExitToken && !text.isEOF() ) || (useExitToken && !checkLiteral(text, exitToken)) ) {
		if( matchStringLiteral( text ) ) {
			// pass
		}
		if( matchLiteral(text, "\"" ) ) {
			// parse quotation
			while( !matchLiteral( text, "\"" ) ) {
				if( *text == '\\' ) {
					++text;
				}
				++text;
			}
		}
		else if( checkLiteral( text, "@\"" ) ) {
			// emit skipped text
			out << str::TextFragment( begin, text ).getText();
			text += 2;

			std::string rawQuotation;
			rawQuotation.reserve( 256 );
			// parse quotation
			while( true ) {
				if( *text == '"' ) {
					if( *++text != '"' ) {
						break;
					}
					else {
						rawQuotation += "\\";
					}
				}

				rawQuotation += *text++;
			}
			out << "\"" << rawQuotation << "\"";
			begin = text;
		}
		else if( checkLiteral( text, "<<<" ) ) {
			// emit skipped text
			out << str::TextFragment( begin, text ).getText();
			text += 3;
			
			std::string rawText;
			rawText.reserve( 512 );
			rawText = "\"";
			while( !matchLiteral( text, ">>>" ) ) {
				if( *text == '\n' ) {
					rawText += "\\n\"\n\"";
					text++;
				}
				else if( *text == '\t' ) {
					rawText += "\\t";
					text++;
				}
				else if( *text == '\"' ) {
					rawText += "\\\"";
					text++;
				}
				else if( matchLiteral( text, "\\>>>") ) {
					rawText += ">>>";
				}
#if 1
				else if( matchLiteral( text, "\\<(") ) {
					rawText += "<(";
				}
				else if( matchLiteral( text, "<(" ) ) {
					std::ostringstream subExpression;
					processText( subExpression, text, true, ")>" );

					rawText += "\" + " + getExpressionToStringCode( subExpression.str() ) + " + \"";

					text += 2;
				}
#endif
				else if( *text == '\\' ) {
					rawText += "\\\\";
					text++;
				}
				else {
					rawText += *text;
					text++;
				}
			}
			rawText += "\"";
			out << rawText;

			begin = text;
		}
		else {
			++text;
		}
	}
	// emit skipped text
	out << str::TextFragment( begin, text ).getText();
}

std::string readFile( const char *filename ) {
	FILE *file = fopen( filename, "rt" );

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

std::string escapePathString( const std::string &in ) {
	std::string escaped = in;

	int ofs = 0;

	while( (ofs = escaped.find( '\\', ofs )) != std::string::npos ) {
		escaped.replace( ofs, 1, "\\\\" );
		ofs += 2;
	}

	return escaped;
}

#if 1
int main( int argc, char **argv ) {
	if( argc == 1 || argc > 3 ) {
		std::cout << "ctp <infile> [<outfile>]" << std::endl;
		return 0;
	}
	std::string inFile = argv[1];
	std::string outFile;
	if( argc == 3 ) {
		outFile = argv[2];
	}
	else /* if( argc == 2 ) */ {
		outFile = inFile;
		int extensionDot = outFile.find_last_of( '.' );
		if( extensionDot == std::string::npos ) {
			outFile += ".cpp";
		}
		else {
			outFile.replace( outFile.begin() + extensionDot, outFile.end(), ".cpp" );
		}
	}

	str::TextSource inTextSource( inFile, readFile( inFile.c_str() ) );
	std::ofstream outFileStream( outFile.c_str() );
	outFileStream << "#line 1 \"" << escapePathString( inFile ) << "\"\n";
	str::TextIterator text = inTextSource.getIterator();
	processText( outFileStream, text );

	return 0;
}
#else
int main( int argc, char **argv ) {
	std::cout << escapePathString( "c:\\test\\anotherfile" );
	std::string testBlock = "<<<\t\n\"\\\\>>>>>>\n@\"hello\\world\\this is a test: c:\\test\\:-) \"\"hello world\"\"\"\n\"hello world this is another\\ntest\"\n";
	str::TextSource inTextSource( "test", testBlock );
	processText( std::cout, inTextSource.getIterator() );

	return 0;
}
#endif

