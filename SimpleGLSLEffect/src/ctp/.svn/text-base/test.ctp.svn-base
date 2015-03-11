#include <iostream>
#include <sstream>

void main() {
	std::cout <<
<<<
This is a test of a raw block:

"Inner quotation"
	Tabs
	
You begin a text block by using <<< and \>>> markers around it.
It's possible to escape \>>> by using an additional \ before it: \\>>>.
Everything in between will be converted to an escaped C string literal.

Moreover it's possible to include code expressions more easily by enclosing an expression in
\<( and )> markers (escape \<( by using \\<():
1 + 1 = <( <<<<(1 + 1)>>>> )> \o/
>>>
	<< @"It's also possible to use raw strings enclosed in @""..."". You escape "" by using double quotation marks """".\n"
	;
}