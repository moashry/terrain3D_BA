#include "textSource.h"

namespace str {
	int TextPosition::getLine() const {
		return getSource().mapLine( line );
	}

	int TextPosition::getLinePos() const {
		return linePos;
	}

	const std::string & TextPosition::getSourceName() const {
		static const std::string invalidTextPosition = "(invalid text position)";
		return source ? source->getSourceName() : invalidTextPosition;
	}

	const TextSource & TextPosition::getSource() const {
		assert( source );
		return *source;
	}

	TextPosition::TextPosition( const TextSource *source, const char *position, int line, int linePos )
		: line( line ), linePos( linePos ), position( position ), source( source ) {
	}

	bool TextIterator::isEOF() const {
		return position.getPointer() == end;
	}

	bool TextIterator::isSOF() const {
		return position.getPointer() == begin;
	}

	char TextIterator::peek() const {
		return *position.getPointer();
	}

	void TextIterator::next( int count /*= 1 */ ) {
		for( ; count > 0 ; count-- ) {
			if( isEOF() ) {
				throw EOFException();
			}	

			position.getSource().next( position );
		}
	}

	void TextIterator::prev( int count /*= 1 */ ) {
		for( ; count > 0 ; count-- ) {
			if( isSOF() ) {
				throw SOFException();
			}

			position.getSource().prev( position );
		}
	}

	str::TextFragment TextIterator::getFragment() const {
		return TextFragment( &position.getSource(), begin, end );
	}

	const TextSource & TextIterator::getSource() const {
		return position.getSource();
	}

	TextIterator::TextIterator( const TextPosition &begin, const TextPosition &end )
		: position( begin ), begin( begin.getPointer() ), end( end.getPointer() ) {
		assert( &begin.getSource() == &end.getSource() );
	}

	TextIterator::TextIterator( const TextSource *source, const char *begin, const char *end )
		: position( source->getPosition( begin ) ), begin( begin ), end( end ) {
	}

	const std::string & TextFragment::getText() const {
		if( begin != end && subText.empty() ) {
			subText = std::string( begin, end );
		}
		return subText;
	}

	str::TextIterator TextFragment::getIterator() const {
		assert( source );
		return TextIterator( source, begin, end );
	}

	TextPosition TextFragment::getBegin() const {
		assert( source );
		return source->getPosition( begin );
	}

	TextPosition TextFragment::getEnd() const {
		assert( source );
		return source->getPosition( end );
	}

	const TextSource & TextFragment::getTextSource() const {
		assert( source );
		return *source;
	}

	TextFragment::TextFragment( const TextPosition &begin, const TextPosition &end )
		: source( &begin.getSource() ), begin( begin.getPointer() ), end( end.getPointer() )
	{
		assert( &begin.getSource() == &end.getSource() );
	}

	TextFragment::TextFragment( const TextSource *source, const char *begin, const char *end )
		: source( source ), begin( begin ), end( end )
	{
	}
}

