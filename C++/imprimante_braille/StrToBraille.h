#ifndef DEF_STRTOBRAILLE
#define DEF_STRTOBRAILLE

#include <map>
#include <bitset>
#include <vector>
#include <stack>
#include <cwchar>
#include "tserial.h"

class StrToBraille
{
public:
	StrToBraille();

    std::vector<std::bitset<12>> strToBitsetVector(const std::string &text);
    std::vector<char> getNumberOfBrailleCharNeeded(const std::string &text);

private:
    void _manageUppercaseSymbol(const std::string &text, std::vector<std::bitset<12>> &bitset_vector, unsigned int nbr_capitals_in_a_row, unsigned int nbr_char_processed);
	void _addUppercaseSymbolAtFisrtChar(std::vector<std::string> &vect);
	void _undoStringVectorStack(std::vector<std::bitset<12>> &bitset_vector, std::stack<std::vector<std::string>> &lifo_stack);
	bool _isUppercase(wchar_t const c) const;
	bool _isSpecialCharacter(wchar_t const c) const;
	void _initCharToBrailleMap();
	void _initBrailleToCharMap();

	std::map<char, std::string> _char_to_braille_map;
	std::map<std::string, char> _braille_to_char_map;
};

#endif
