#pragma once

#include <exception>
#include <string>
#include <stack>
#include <sstream>

#include "textSource.h"
#include "strprintf.h"

namespace str {
	struct RecognizerException : public std::exception {
		RecognizerException( const TextPosition &position, const std::string &what )
			: std::exception(), recognizerWhat( what ), position( position ) {}

		virtual const char *what() const {
			LineFileMapper::LineFile lineFile = position.getMappedLineFile();
			formattedWhat = strprintf( "%s(%i) : error: %s", lineFile.filename.c_str(), lineFile.line, recognizerWhat.c_str() );
			return formattedWhat.c_str();
		}

	private:
		mutable std::string formattedWhat;

		std::string recognizerWhat;
		TextPosition position;
	};

	/* TODO:
		- add outside control for short parsers (ie add another class that wraps BaseRecognizer and changes match, ignore, etc to public
			without a parse() implementation
		NOTE: might be better to derive BaseRecognizer from it instead (parse and recognize add functionality..)
		- runtime members like textPosition, skipRule, etc. should be part of a *static Context stack* that is managed by _parse and accessed
			by member methods - this would reduce the memprint
	*/
	class BaseRecognizer {
	public:
		BaseRecognizer() {}

		// get the whole fragment or as much as we've read
		const TextFragment getFragment() const {
			if( !parserState.textPosition || parserState.endMarkerSet )
				return TextFragment( startMarker, endMarker );
			else
				return TextFragment( startMarker, *parserState.textPosition );
		}

		void recognize( TextFragment &textFragment, BaseRecognizer *skipRule ) {
			TextIterator &textPosition = textFragment.getIterator();
			_parse( &textPosition, skipRule );
		}

		template< typename SkipRule >
		void recognize( TextFragment &textFragment ) {
			SkipRule skipRule;
			recognize( textFragment, &skipRule );
		}

	protected:
		virtual void parse() = 0;

		void setSkipRule( BaseRecognizer &skipRule ) {
			parserState.skipRule = &skipRule;
		}

		BaseRecognizer * getSkipRule() const {
			return parserState.skipRule;
		}

		// try to match rule and save the result in rule
		bool match( BaseRecognizer &rule ) {
			skip();

			try {
				rule._parse( parserState.textPosition, parserState.skipRule );
			}
			catch( const RecognizerException & ) {
				return false;
			}

			skip();
			return true;
		}

		// only check if the rule matches anything
		template< typename SubRecognizer >
		bool check( const SubRecognizer &rule = SubRecognizer() ) {
			SubRecognizer _rule = rule;

			skip();

			try {
				TextIterator checkTextPosition = *parserState.textPosition;
				
				_rule._parse( &checkTextPosition, parserState.skipRule );

				return true;
			}
			catch( const RecognizerException & ) {
				return false;
			}
		}

		// evaluate rule and save the result (also return the reference) - throw otherwise
		template< typename SubRecognizer >
		SubRecognizer & evaluate( SubRecognizer &rule ) {
			skip();
			rule._parse( parserState.textPosition, parserState.skipRule );
			skip();

			return rule;
		}

		// try to match rule but ignore the result
		template< typename SubRecognizer >
		bool ignore( const SubRecognizer &rule = SubRecognizer() ) {
			SubRecognizer _rule = rule;
			return match( _rule );		
		}

		// inefficient because it copies the result
		// expect rule otherwise throw - return a copy of the rule (inefficient)
		template< typename SubRecognizer >
		SubRecognizer expect( const SubRecognizer &rule = SubRecognizer() ) {
			SubRecognizer _rule = rule;
			return evaluate( _rule );
		}

		void error( const char *fmt, ... ) {
			va_list list;
			va_start( list, fmt);
			std::string output = strvprintf( fmt, list );
			
			throw RecognizerException( *parserState.textPosition, output );

			va_end( list );
		}

		void skip() {
			if(  parserState.skipRule ) {
				try {
					parserState.skipRule->_parse( parserState.textPosition, NULL );
				}
				catch( const RecognizerException & ) {
				}		
			}
		}

		void setStartMarker() {
			startMarker = *parserState.textPosition;
		}

		void setEndMarker() {
			endMarker = *parserState.textPosition;
			parserState.endMarkerSet = true;
		}

		// textPosition interface
		static char peek() { return parserState.textPosition->peek(); }

		static char next( int count = 1 ) { char old = parserState.textPosition->peek(); parserState.textPosition->next( count ); return old; }
		static void previous( int count = 1 ) { return  parserState.textPosition->prev( count ); }

		static bool isEOF() { return  parserState.textPosition->isEOF(); }
		static bool isSOF() { return  parserState.textPosition->isSOF(); }

		static int getLine() { return parserState.textPosition->getPosition().getLine(); }
		static int getLinePos() { return parserState.textPosition->getPosition().getLinePos(); }

		static const TextPosition & getTextPosition() { return parserState.textPosition->getPosition(); }

		static const char * getPointer() { return  parserState.textPosition->getPosition().getPointer(); }

	private:
		void _parse( TextIterator *_textPosition, BaseRecognizer *_skipRule ) {
			struct StateScope {
				State oldState;

				StateScope() {
					oldState = parserState;
				}

				~StateScope() {
					parserState = oldState;
				}
			} scope;

			// copy the textPosition
			TextIterator subPosition = *_textPosition;

			parserState.textPosition = &subPosition;
			parserState.skipRule = _skipRule;

			try {
				parserState.endMarkerSet = false;

				startMarker = *parserState.textPosition;
				parse();
				if( !parserState.endMarkerSet ) {
					endMarker = *parserState.textPosition;
				}

				// update the parent position, too
				*_textPosition = subPosition;
			}
			catch( EOFException& ) {
				// use _textPosition instead of the EOF position [4/25/2010 Andreas Kirsch]
				subPosition = *_textPosition;
				error( "Unexpected EOF during parsing!" );
			}
			/*
			catch( SOFException &sofException ) {
				error( "Unexpected SOF!" );
			}*/
		}

	private:
		struct State {
			BaseRecognizer *skipRule;
			bool endMarkerSet;
			TextIterator *textPosition;
		};
		static __declspec( thread ) State parserState;
	
		TextPosition startMarker, endMarker;
	};

	// Recognizer class that defines some helper methods
	class Recognizer : public BaseRecognizer {
	protected:
		bool checkLiteral( const char *literal ) {
			skip();

			return !strncmp( getPointer(), literal, strlen( literal ) );
		}

		bool matchLiteral( const char *literal ) {
			skip();

			int length = strlen( literal );
			if( !strncmp( getPointer(), literal, length ) ) {
				next( length );
				return true;
			}
			else {
				return false;
			}
		}

		void expectLiteral( const char *literal ) {
			if( !matchLiteral( literal ) ) {
				error( "Expected '%s'!", literal );
			}
		}

		std::string readUpToLiteral( const char *literal ) {
			std::ostringstream remainder;

			int length = strlen( literal );

			while( strncmp( getPointer(), literal, length ) != 0 ) {
				char c = next();
				remainder << c;
			}

			return remainder.str();
		}
	};
}