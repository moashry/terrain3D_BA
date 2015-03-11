#pragma once

#include <assert.h>
#include <exception>
#include <string>
#include <vector>
#include <algorithm>

#include "lineFileMapper.h"

namespace str {
	/* TODOs/Thoughts:
		1. could use composition in TextIterator instead of inheritance
			+ simplifies operator = some more
		2. move the implementation of advance and stepBack to TextSource
			+ centralizes the implementation
			- adds a ping-pong level to everything
			+ code would work on markers directly
		+ increases similarity to Memento and Iterator pattern because TextIterator only contains state
		+ makes it easy to 'templatize' TextIterator based on TextSource!
		+ add support for sub-textsources!

		* add match method that uses strcmp or stricmp to advance by more than one character
	*/
	struct EOFException : std::exception {
		EOFException() : std::exception( "Unexpected EOF" ) {}
	};

	struct SOFException : std::exception {
		SOFException() : std::exception( "Unexpected SOF" ) {}
	};

	class TextFragment;
	class TextSource;

	class TextPosition {
		friend class TextIterator;
		friend class TextFragment;
		friend class TextSource;
	public:
		int getLine() const;
		int getLinePos() const;
		const std::string & getSourceName() const;

		LineFileMapper::LineFile getMappedLineFile() const;

		const char * getPointer() const { return position; }

		friend bool operator ==( const TextPosition &a, const TextPosition &b );

		// get text source iterator
		/*
		TextIterator getIterator() const {
			TextIterator iterator = getSource().getIterator();
			iterator.setPosition( *this );
			return iterator;
		}*/

		const TextSource & getSource() const;

		void addLineFileMapping( int line, const std::string &filename ) const;

		TextPosition() : line( 0 ), linePos( 0 ), position( NULL ), source( NULL ) {}

	protected:
		TextPosition( const TextSource *source, const char *position, int line, int linePos );

	protected:
		int line;
		int linePos;
		const char *position;

		const TextSource *source;
	};

	class TextIterator {
		friend class TextSource;
		friend class TextFragment;
	public:
		bool isEOF() const;
		bool isSOF() const;

		char peek() const;
		void next( int count = 1 );
		void prev( int count = 1 );

		TextIterator &operator ++() {
			next();
			return *this;
		}

		TextIterator &operator --() {
			prev();
			return *this;
		}

		TextIterator &operator +=( int count ) {
			next( count );
			return *this;
		}

		TextIterator &operator -=( int count ) {
			prev( count );
			return *this;
		}

		const char * const operator ++( int ) {
			const char *pointer = position.getPointer();
			next();
			return pointer;
		}

		const char * const operator --( int ) {
			const char *pointer = position.getPointer();
			prev();
			return pointer;
		}
		
		char operator * () {
			return *position.getPointer();
		}

		void setPosition( const TextPosition &position ) {
			assert( begin <= position.getPointer() && position.getPointer() <= end );
			assert( &position.getSource() == &this->position.getSource() );

			this->position = position;
		}

		const TextPosition & getPosition() const {
			return position;
		}

		operator const TextPosition &() const {
			return getPosition();
		}

		// get fragment in which the iterator moves
		TextFragment getFragment() const;

		const TextSource & getSource() const;

		TextIterator( const TextPosition &begin, const TextPosition &end );
		TextIterator() : begin( NULL ), end( NULL ) {}

	protected:
		TextIterator( const TextSource *source, const char *begin, const char *end );

	private:
		TextPosition position;
		const char *begin, *end;
	};

	class TextFragment {
		friend class TextSource;
		friend class TextIterator;
	public:
		const std::string & getText() const;

		TextIterator getIterator() const;

		TextPosition getBegin() const;
		TextPosition getEnd() const;

		bool empty() const {
			return begin == end;
		}

		const TextSource & getTextSource() const;

		TextFragment( const TextPosition &begin, const TextPosition &end );
		TextFragment() : begin( NULL ), end( NULL ), source( NULL ) {}

	protected:
		TextFragment( const TextSource *source, const char *begin, const char *end );
		
	private:
		const char *begin, *end;
		mutable std::string subText;

		const TextSource *source;
	};

	class TextSource {
		friend class TextPosition;
		friend class TextIterator;
		friend class TextFragment;

	public:
		TextSource( const std::string &sourceName, const std::string &text )
			: text( text ), sourceName( sourceName ), baseLine( START_LINE ), baseLinePos( START_LINEPOS ) {
				initLineMarkers();
		}

		const std::string & getText() const {
			return text;
		}

		const std::string & getSourceName() const {
			return sourceName;
		}
		
		TextFragment getFragment() const {
			return TextFragment( this, text.c_str(), text.c_str() + text.length() );
		}

		TextIterator getIterator() const {
			return TextIterator( this, text.c_str(), text.c_str() + text.length() );
		}

	protected:
		void next( TextPosition &position ) const;
		void prev( TextPosition &position ) const;

		LineFileMapper::LineFile mapLine( int line ) const {
			return lineFileMapper.mapLine( line );
		}

		TextPosition getPosition( const char *pointer ) const {
			std::vector< const char * >::const_iterator nextLine = std::upper_bound( lineMarkers.begin(), lineMarkers.end(), pointer );
			if( nextLine == lineMarkers.begin() ) {
				assert( false );
			}
			std::vector< const char * >::const_iterator actualLine = nextLine - 1;

			int line = actualLine - lineMarkers.begin();
			int linePos = pointer - *actualLine;
			line += baseLine;
			if( line == 0 ) {
				linePos += baseLinePos;
			}
			else {
				linePos += START_LINEPOS;
			}

			return TextPosition( this, pointer, line, linePos );
		}

	protected:
		void initLineMarkers() {
			// only support \n (convert \r\n and \r to \n before passing text!
			assert( strstr( text.c_str(), "\r\n" ) == NULL );
			assert( strchr( text.c_str(), '\r' ) == NULL );

			// fill the lineMarker vector

			// start the first line
			lineMarkers.push_back( text.c_str() );
			// find the other line starts
			for( const char *p = text.c_str() ; *p ; ) {
				if( *p == '\n' ) {
					lineMarkers.push_back( ++p );
				}
				else {
					++p;
				}
			}

			lineFileMapper.addMapping( baseLine, baseLine, sourceName );
		}

		std::vector< const char * > lineMarkers;
		mutable LineFileMapper lineFileMapper;

		int baseLinePos;
		int baseLine;

		const std::string text;
		const std::string sourceName;

		static const int START_LINE = 1, START_LINEPOS = 1;
	};

	struct TextArtifact {
		std::string text;
		TextPosition origin;

		operator std::string &() {
			return text;
		}

		operator const std::string &() const {
			return text;
		}

		const std::string & getText() const {
			return text;
		}

		TextArtifact() {}

		TextArtifact( const TextFragment &fragment )
			: text( fragment.getText() ), origin( fragment.getBegin() ) {}
	};

	inline void TextSource::next( TextPosition &textPos ) const {
		char character = *textPos.position++;
		if( character == '\n' ) {
			textPos.line++;
			textPos.linePos = START_LINEPOS;
		}
	}

	inline void TextSource::prev( TextPosition &textPos ) const {
		char character = *--textPos.position;
		if( character == '\n' ) {
			textPos = getPosition( textPos.position );
		}
	}

	inline bool operator==( const TextPosition &a, const TextPosition &b ) {
		return a.getPointer() == b.getPointer();
	}
}