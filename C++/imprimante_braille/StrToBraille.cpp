#include "StrToBraille.h"

#include <QString>
#include <QDebug>
#include <iostream>
#include <cwchar>
#include <wchar.h>

using namespace std;

StrToBraille::StrToBraille()
{
	_initCharToBrailleMap();
	_initBrailleToCharMap();
}

vector<bitset<12>> StrToBraille::strToBitsetVector(const string &text)
{
	vector<bitset<12>> bitset_vector;

    string const text_modified = text + "x"; //ajout d'une lettre minuscule non prise en compte dans la suite pour assure le fonctionnement du système de maj si 'text' se finit par une maj

	unsigned int nbr_capitals_in_a_row = 0,
		nbr_char_processed = 0;

	for (wchar_t c : text_modified)
    {
		if (_isUppercase(c) || (_isSpecialCharacter(c) && (nbr_capitals_in_a_row != 0)))  //(si 'c' est une maj) OU [(c'est un caractère spécial) ET (le caractère précédent était une maj)]
		{
			nbr_capitals_in_a_row += 1;
		}
		else if (nbr_capitals_in_a_row != 0) //si on vient de tomber sur une min après une ou plusieurs maj, on appelle _manageUppercaseSymbol()
		{
			_manageUppercaseSymbol(text_modified, bitset_vector, nbr_capitals_in_a_row, nbr_char_processed);
			nbr_capitals_in_a_row = 0;
		}

		bitset_vector.push_back(bitset<12>(_char_to_braille_map[tolower(c)]));
		nbr_char_processed++;
	}

	bitset_vector.pop_back(); //on supprime la lettre 'x' ajoutée au début

	return bitset_vector;
}

vector<char> StrToBraille::getNumberOfBrailleCharNeeded(const string &text)
{
    vector<bitset<12>> bitset_vct = strToBitsetVector(text);

	const bitset<6> null_bitset("000000");
	bitset<6> first_bitset_part;

    char current_line_char_nbr = 0,
		longest_line = -1, //valeur initiale pour savoir si une première est passée à la fin de for
		lines_nbr = 1;

	for (bitset<12> full_bitset : bitset_vct)
	{
		current_line_char_nbr += 2; //chaque paquet de 12 bits correspond à 2 caractères

		for (unsigned char i = 0; i < 6; i++)
			first_bitset_part[i] = full_bitset[i + 6];

		if (first_bitset_part == null_bitset) //si la première partie est 000000, elle ne sera pas imprimée, on peut donc la supprimer.
			current_line_char_nbr--;

		else if (full_bitset == bitset<12>("001101001101")) //si c'est un retour à la ligne
		{
			current_line_char_nbr -= 2; //on ne compte pas le caractère \n sur cette ligne
			lines_nbr++;

			if (current_line_char_nbr > longest_line)
				longest_line = current_line_char_nbr;

			current_line_char_nbr = 0;
		}
	}

	if (longest_line == -1)							//si il n'y a qu'une ligne, c'est la plus longue
		longest_line = current_line_char_nbr;

	else if (current_line_char_nbr > longest_line)	//sinon, on vérifie que la dernière ligne n'est pas la plus longue
		longest_line = current_line_char_nbr;

    vector<char> msgSize(2);

    msgSize[0] = longest_line;
    msgSize[1] = lines_nbr;

	return msgSize;
}

void StrToBraille::_manageUppercaseSymbol(const string &text, vector<bitset<12>> &bitset_vector, unsigned int nbr_capitals_in_a_row, unsigned int nbr_char_processed)
{
	/* Source : http://www.braillelog.net/majuscules-et-mises-en-evidence.php
	Propriétés:
	 1 - Le signe de majuscule(combinaison des points 4 et 6), employé seul, n'affecte que le caractère qui le suit.
	 2 - Pour indiquer qu'un mot est entièrement écrit en majuscule, on le fait précéder de 2 signes de majuscule consécutifs. Par exemple,
	 l'écriture en majuscule du mot "PARIS" est la suivante : signe de majuscule, signe de majuscule, p, a, r, i, s.
	 3 - Si une suite d'au moins quatre mots est en majuscule, on l'indique en plaçant, devant le premier mot de cette suite, le symbole 
	 constitué du caractère "deux-points" (combinaison 2 5) suivi du signe de majuscule. La fin de cette séquence en majuscule se matérialise 
	 par la présence d'un seul signe de majuscule devant son dernier mot.
	*/
	
	unsigned int nbr_spaces = 0;

	for (unsigned int i = 1; i <= nbr_capitals_in_a_row; i++) //on compte le nombre d'espaces pour savoir le nombres de mots en capitales d'affilée
	{
		if (text[nbr_char_processed - i] == ' ') //on part de la fin (équivalente à nbr_char_processed) en reculant
			nbr_spaces++;
	}

	if (nbr_spaces == 0)	//si aucun espace -> propriété 1
	{
		stack<string> temp_lifo_stack;

		for (unsigned int i = 0; i < nbr_capitals_in_a_row; i++)	//on ajoute le caratère modifié à une pile lifo (last in first out) pour pouvoir les replacer ensuite
		{
			string old_bitset_str = bitset_vector[bitset_vector.size() - 1].to_string(); //on prend le dernier car on a enlevé les précédents

			if (!_isSpecialCharacter(_braille_to_char_map[old_bitset_str])) //si ce n'est pas un caractère spécial, on lui assigne le symbole maj
				old_bitset_str.replace(old_bitset_str.begin(), old_bitset_str.begin() + 6, "000101");
			
			temp_lifo_stack.push(old_bitset_str);

			bitset_vector.pop_back(); //on supprime le dernier élément maintenant stocké dans la pile
		}
		while(!temp_lifo_stack.empty())	//on défait la pile et on les replace dans le vecteur tant que la pile n'est pas vide
		{
			bitset_vector.push_back(bitset<12>(temp_lifo_stack.top()));
			temp_lifo_stack.pop();
		}
	}
	else if (nbr_spaces <= 3) //si espaces compris dans [1;3] -> propriété 2
	{
		stack<vector<string>> temp_lifo_stack;
		vector<string> temp_vect;

		temp_vect.push_back(bitset_vector[bitset_vector.size() - 1].to_string()); //on passe la dernière espace en la plaçant directement dans temp_vect
		bitset_vector.pop_back();

		for (unsigned int i = 0; i < (nbr_capitals_in_a_row - 1); i++)	//on ajoute le caratère modifié à une pile lifo (last in first out) pour pouvoir les replacer ensuite
		{
			string old_bitset_str = bitset_vector[bitset_vector.size() - 1].to_string(); //on prend le dernier car on a enlevé les précédents
			
			if (old_bitset_str != _char_to_braille_map[' ']) //si le caractère n'est pas une espace, on l'ajoute au vecteur-mot
			{
				temp_vect.push_back(old_bitset_str);
			}
			else
			{
				_addUppercaseSymbolAtFisrtChar(temp_vect);

				temp_vect.push_back("000000000101");
				temp_vect.push_back("000000000000");

				temp_lifo_stack.push(temp_vect);	
				temp_vect.clear();
			}

			bitset_vector.pop_back(); //on supprime le dernier élément maintenant stocké dans le vecteur
		}

		_addUppercaseSymbolAtFisrtChar(temp_vect);

		temp_vect.push_back("000000000101");

		temp_lifo_stack.push(temp_vect);
		temp_vect.clear();

		_undoStringVectorStack(bitset_vector, temp_lifo_stack);
	}
	else	//sinon -> propriété 3
	{
		stack<vector<string>> temp_lifo_stack;
		vector<string> temp_vect;

		unsigned int last_word_letters_nbr = 0;

		do	//tant qu'on est au dernier mot, on ajoute chaque caractère au vecteur
		{
			last_word_letters_nbr++;

			temp_vect.push_back(bitset_vector[bitset_vector.size() - 1].to_string());	//on prend le dernier car on a enlevé les précédents

			bitset_vector.pop_back(); //on supprime le dernier élément maintenant stocké dans le vecteur
		} while (bitset_vector[bitset_vector.size() - 1].to_string() != _char_to_braille_map[' ']);

		_addUppercaseSymbolAtFisrtChar(temp_vect);

		temp_lifo_stack.push(temp_vect);	//on ajoute le dernier mot à la pile, assigné de son symbole maj signalant que c'est le dernier de la phrase
		temp_vect.clear();

		for (unsigned int i = 0; i < (nbr_capitals_in_a_row - last_word_letters_nbr); i++) //tant qu'on n'est pas remonté à la 1ere maj, on ajoute les mots dans des vecteurs ajoutés à une pile
		{
			string old_bitset_str = bitset_vector[bitset_vector.size() - 1].to_string(); //on prend le dernier car on a enlevé les précédents

			if (old_bitset_str != _char_to_braille_map[' ']) //si le caractère n'est pas une espace, on l'ajoute au vecteur-mot
			{
				temp_vect.push_back(old_bitset_str);
			}
			else
			{
				temp_vect.push_back(_char_to_braille_map[' ']);
				temp_lifo_stack.push(temp_vect);
				temp_vect.clear();
			}

			bitset_vector.pop_back(); //on supprime le dernier élément maintenant stocké dans le vecteur
		}

		_addUppercaseSymbolAtFisrtChar(temp_vect);
											 //						 0 0
		temp_vect.push_back("000000010010"); //on ajoute le symbole  1 1 devant la phrase
											 //						 0 0
		temp_lifo_stack.push(temp_vect);
		temp_vect.clear();

		_undoStringVectorStack(bitset_vector, temp_lifo_stack);
	}
}

void StrToBraille::_addUppercaseSymbolAtFisrtChar(std::vector<std::string>& vect)
{
	string temp_first_char_of_word = vect[vect.size() - 1]; //on prend la dernière lettre ajoutée au vecteur-mot qui correspond à la 1ere lettre du mot

	vect.pop_back();

	temp_first_char_of_word.replace(temp_first_char_of_word.begin(), temp_first_char_of_word.begin() + 6, "000101"); //on lui assigne le symbole maj

	vect.push_back(temp_first_char_of_word);
}

void StrToBraille::_undoStringVectorStack(std::vector<std::bitset<12>>& bitset_vector, std::stack<std::vector<std::string>>& lifo_stack)
{
	while (!lifo_stack.empty())  //on défait la pile
	{
		vector<string> temp_vect = lifo_stack.top();

		while (!temp_vect.empty())
		{
			bitset_vector.push_back(bitset<12>(temp_vect[temp_vect.size() - 1]));
			temp_vect.pop_back();
		}

		lifo_stack.pop();
	}
}

bool StrToBraille::_isUppercase(wchar_t const c) const
{
	bool is_upper = false;

	switch (c)
	{
    case wchar_t('À') :
		is_upper = true; break;
	case wchar_t('Â') :
		is_upper = true; break;
	case wchar_t('Ç') :
		is_upper = true; break;
	case wchar_t('È') :
		is_upper = true; break;
	case wchar_t('É') :
		is_upper = true; break;
	case wchar_t('Ê') :
		is_upper = true; break;
	case wchar_t('Ë') :
		is_upper = true; break;
	case wchar_t('Î') :
		is_upper = true; break;
	case wchar_t('Ï') :
		is_upper = true; break;
	case wchar_t('Ô') :
		is_upper = true; break;
	case wchar_t('Œ') :
		is_upper = true; break;
	case wchar_t('Ù') :
		is_upper = true; break;
	case wchar_t('Û') :
		is_upper = true; break;
	case wchar_t('Ü') :
		is_upper = true; break;
	default:
		switch (c)
		{
		case wchar_t('à') :
			is_upper = false; break;
		case wchar_t('â') :
			is_upper = false; break;
		case wchar_t('ç') :
			is_upper = false; break;
		case wchar_t('è') :
			is_upper = false; break;
		case wchar_t('é') :
			is_upper = false; break;
		case wchar_t('ê') :
			is_upper = false; break;
		case wchar_t('ë') :
			is_upper = false; break;
		case wchar_t('î') :
			is_upper = false; break;
		case wchar_t('ï') :
			is_upper = false; break;
		case wchar_t('ô') :
			is_upper = false; break;
		case wchar_t('œ') :
			is_upper = false; break;
		case wchar_t('ù') :
			is_upper = false; break;
		case wchar_t('û') :
			is_upper = false; break;
		case wchar_t('ü') :
			is_upper = false; break;
		default:
			is_upper = isupper(c); break;
		} break;
	}

	return is_upper;
}

bool StrToBraille::_isSpecialCharacter(wchar_t const c) const
{
	bool is_special = false;

	switch (c)
	{
	case wchar_t(',') :
		is_special = true; break;
	case wchar_t('\'') :
		is_special = true; break;
	case wchar_t(' ') :
		is_special = true; break;
	case wchar_t('#') :
		is_special = true; break;
	case wchar_t('/') :
		is_special = true; break;
	case wchar_t('@') :
		is_special = true; break;
	case wchar_t('&') :
		is_special = true; break;
	case wchar_t('€') :
		is_special = true; break;
	case wchar_t('$') :
		is_special = true; break;
	case wchar_t('£') :
		is_special = true; break;
	case wchar_t('¥') :
		is_special = true; break;
	case wchar_t('©') :
		is_special = true; break;
	case wchar_t('®') :
		is_special = true; break;
	case wchar_t('~') :
		is_special = true; break;
	case wchar_t('_') :
		is_special = true; break;
	default: break;
	}

	return is_special;
}

void StrToBraille::_initCharToBrailleMap()
{
	{//LETTRES			   			|123456123456| (position du point en braille)
		_char_to_braille_map['a'] = "000000100000";
		_char_to_braille_map['b'] = "000000110000";
		_char_to_braille_map['c'] = "000000100100";
		_char_to_braille_map['d'] = "000000100110";
		_char_to_braille_map['e'] = "000000100010";
		_char_to_braille_map['f'] = "000000110100";
		_char_to_braille_map['g'] = "000000110110";
		_char_to_braille_map['h'] = "000000110010";
		_char_to_braille_map['i'] = "000000010100";
		_char_to_braille_map['j'] = "000000010110";
		_char_to_braille_map['k'] = "000000101000";
		_char_to_braille_map['l'] = "000000111000";
		_char_to_braille_map['m'] = "000000101100";
		_char_to_braille_map['n'] = "000000101110";
		_char_to_braille_map['o'] = "000000101010";
		_char_to_braille_map['p'] = "000000111100";
		_char_to_braille_map['q'] = "000000111110";
		_char_to_braille_map['r'] = "000000111010";
		_char_to_braille_map['s'] = "000000011100";
		_char_to_braille_map['t'] = "000000011110";
		_char_to_braille_map['u'] = "000000101001";
		_char_to_braille_map['v'] = "000000111001";
		_char_to_braille_map['w'] = "000000010111";
		_char_to_braille_map['x'] = "000000101101";
		_char_to_braille_map['y'] = "000000101111";
		_char_to_braille_map['z'] = "000000101011";

		_char_to_braille_map['à'] = "000000111011";
		_char_to_braille_map['â'] = "000000100001";
		_char_to_braille_map['ç'] = "000000111101";
		_char_to_braille_map['è'] = "000000011101";
		_char_to_braille_map['é'] = "000000111111";
		_char_to_braille_map['ê'] = "000000110001";
		_char_to_braille_map['ë'] = "000000110101";
		_char_to_braille_map['î'] = "000000100101";
		_char_to_braille_map['ï'] = "000000110111";
		_char_to_braille_map['ô'] = "000000100111";
		_char_to_braille_map['œ'] = "000000010101";
		_char_to_braille_map['ù'] = "000000011111";
		_char_to_braille_map['û'] = "000000100011";
		_char_to_braille_map['ü'] = "000000110011";
	}

	{//CHIFFRES					 	|123456123456|
		_char_to_braille_map['0'] = "000001001111";
		_char_to_braille_map['1'] = "000001100001";
		_char_to_braille_map['2'] = "000001110001";
		_char_to_braille_map['3'] = "000001100101";
		_char_to_braille_map['4'] = "000001100111";
		_char_to_braille_map['5'] = "000001100011";
		_char_to_braille_map['6'] = "000001110101";
		_char_to_braille_map['7'] = "000001110111";
		_char_to_braille_map['8'] = "000001110011";
		_char_to_braille_map['9'] = "000001010101";
	}

	{//PONCTUATION					|123456123456|
		_char_to_braille_map['.'] = "000000010011";
		_char_to_braille_map[','] = "000000010000";
		_char_to_braille_map['?'] = "000000010001";
		_char_to_braille_map[';'] = "000000011000";
		_char_to_braille_map[':'] = "000000010010";
		_char_to_braille_map['!'] = "000000011010";
		_char_to_braille_map['('] = "000000011001";
		_char_to_braille_map[')'] = "000000001011";
		_char_to_braille_map['"'] = "000000011011";
		_char_to_braille_map['\'']= "000000001000";
		_char_to_braille_map['\n']= "001101001101";
	}

	{//SYMBOLES						|123456123456|
		_char_to_braille_map[' '] = "000000000000";
		_char_to_braille_map['#'] = "000000001111";
		_char_to_braille_map['/'] = "000000001100";
		_char_to_braille_map['@'] = "000000001110";
		_char_to_braille_map['&'] = "000010111111";
		_char_to_braille_map['€'] = "000110100010";
		_char_to_braille_map['$'] = "000110011100";
		_char_to_braille_map['£'] = "000110111000";
		_char_to_braille_map['¥'] = "000110101111";
		_char_to_braille_map['©'] = "000010100100";
		_char_to_braille_map['®'] = "000010111010";
		_char_to_braille_map['™'] = "000010011110";
		_char_to_braille_map['%'] = "000010001101";
		_char_to_braille_map['+'] = "000000011010";
		_char_to_braille_map['-'] = "000000001001";
		_char_to_braille_map['*'] = "000000001010";
		_char_to_braille_map['÷'] = "000000010010";
		_char_to_braille_map['<'] = "000010110001";
        _char_to_braille_map['>'] = "000010001110";
		_char_to_braille_map['['] = "000110011001";
		_char_to_braille_map[']'] = "001011110000";
		_char_to_braille_map['~'] = "000010111100";
		_char_to_braille_map['_'] = "000010001001";
	}
}

void StrToBraille::_initBrailleToCharMap()
{
	{//LETTRES			   	 |123456123456|	(position du point en braille)
		_braille_to_char_map["000000100000"] = 'a';
		_braille_to_char_map["000000110000"] = 'b';
		_braille_to_char_map["000000100100"] = 'c';
		_braille_to_char_map["000000100110"] = 'd';
		_braille_to_char_map["000000100010"] = 'e';
		_braille_to_char_map["000000110100"] = 'f';
		_braille_to_char_map["000000110110"] = 'g';
		_braille_to_char_map["000000110010"] = 'h';
		_braille_to_char_map["000000010100"] = 'i';
		_braille_to_char_map["000000010110"] = 'j';
		_braille_to_char_map["000000101000"] = 'k';
		_braille_to_char_map["000000111000"] = 'l';
		_braille_to_char_map["000000101100"] = 'm';
		_braille_to_char_map["000000101110"] = 'n';
		_braille_to_char_map["000000101010"] = 'o';
		_braille_to_char_map["000000111100"] = 'p';
		_braille_to_char_map["000000111110"] = 'q';
		_braille_to_char_map["000000111010"] = 'r';
		_braille_to_char_map["000000011100"] = 's';
		_braille_to_char_map["000000011110"] = 't';
		_braille_to_char_map["000000101001"] = 'u';
		_braille_to_char_map["000000111001"] = 'v';
		_braille_to_char_map["000000010111"] = 'w';
		_braille_to_char_map["000000101101"] = 'x';
		_braille_to_char_map["000000101111"] = 'y';
		_braille_to_char_map["000000101011"] = 'z';

		_braille_to_char_map["000000111011"] = 'à';
		_braille_to_char_map["000000100001"] = 'â';
		_braille_to_char_map["000000111101"] = 'ç';
		_braille_to_char_map["000000011101"] = 'è';
		_braille_to_char_map["000000111111"] = 'é';
		_braille_to_char_map["000000110001"] = 'ê';
		_braille_to_char_map["000000110101"] = 'ë';
		_braille_to_char_map["000000100101"] = 'î';
		_braille_to_char_map["000000110111"] = 'ï';
		_braille_to_char_map["000000100111"] = 'ô';
		_braille_to_char_map["000000010101"] = 'œ';
		_braille_to_char_map["000000011111"] = 'ù';
		_braille_to_char_map["000000100011"] = 'û';
		_braille_to_char_map["000000110011"] = 'ü';
	}

	{//CHIFFRES				 |123456123456|	
		_braille_to_char_map["000001001111"] = '0';
		_braille_to_char_map["000001100001"] = '1';
		_braille_to_char_map["000001110001"] = '2';
		_braille_to_char_map["000001100101"] = '3';
		_braille_to_char_map["000001100111"] = '4';
		_braille_to_char_map["000001100011"] = '5';
		_braille_to_char_map["000001110101"] = '6';
		_braille_to_char_map["000001110111"] = '7';
		_braille_to_char_map["000001110011"] = '8';
		_braille_to_char_map["000001010101"] = '9';
	}

	{//PONCTUATION			 |123456123456|	
		_braille_to_char_map["000000010011"] = '.';
		_braille_to_char_map["000000010000"] = ',';
		_braille_to_char_map["000000010001"] = '?';
		_braille_to_char_map["000000011000"] = ';';
		_braille_to_char_map["000000010010"] = ':';
		_braille_to_char_map["000000011010"] = '!';
		_braille_to_char_map["000000011001"] = '(';
		_braille_to_char_map["000000001011"] = ')';
		_braille_to_char_map["000000011011"] = '"';
		_braille_to_char_map["000000001000"] = '\'';
	}

	{//						 |123456123456|
		_braille_to_char_map["000000000000"] = ' ';
		_braille_to_char_map["000000001111"] = '#';
		_braille_to_char_map["000000001100"] = '/';
		_braille_to_char_map["000000001110"] = '@';
		_braille_to_char_map["000010111111"] = '&';
		_braille_to_char_map["000110100010"] = '€';
		_braille_to_char_map["000110011100"] = '$';
		_braille_to_char_map["000110111000"] = '£';
		_braille_to_char_map["000110101111"] = '¥';
		_braille_to_char_map["000010100100"] = '©';
		_braille_to_char_map["000010111010"] = '®';
		_braille_to_char_map["000010011110"] = '™';
		_braille_to_char_map["000010001101"] = '%';
		_braille_to_char_map["000000001101"] = '+';
		_braille_to_char_map["000000001001"] = '-';
		_braille_to_char_map["000000001010"] = '*';
		_braille_to_char_map["000000010010"] = '÷';
		_braille_to_char_map["000010110001"] = '<';
        _braille_to_char_map["000010001110"] = '>';
		_braille_to_char_map["000110011001"] = '[';
		_braille_to_char_map["001011110000"] = ']';
		_braille_to_char_map["000010111100"] = '~';
		_braille_to_char_map["000010001001"] = '_';
	}
}
