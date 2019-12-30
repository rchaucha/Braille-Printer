#define NOMINMAX

#include <string>
#include <vector>
#include <windows.h> 
#include <iostream>
#include <bitset>
#include <limits> //pour cin.ignore(std::numeric_limits<streamsize>::max(),'\n');
#include "StrToBraille.h"
#include "ArduinoConnectionManager.h"
#include "tserial.h"

using namespace std;

int main()
{
	UINT default_codepage = GetConsoleOutputCP();
	SetConsoleOutputCP(1252);

	CONSOLE_FONT_INFOEX ConsoleCurrentFontEx;
	ConsoleCurrentFontEx.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	ConsoleCurrentFontEx.dwFontSize.Y = 18;
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), 0, &ConsoleCurrentFontEx);
	
	StrToBraille stb;
	ArduinoConnectionManager acm;

	string text_to_print = "";
	vector<bitset<12>> bitset_vector;

	if (acm.connect()) //si la connection est bien établie
	{
		cin.ignore();

		printf("Entrez le texte à imprimer: ");
		getline(cin, text_to_print);

		bitset_vector = stb.strToBitsetVector(text_to_print);

		vector<float> vect = stb.getNumberOfBrailleCharNeeded(text_to_print);

		cout << "longest_line : " << vect[0] << endl <<
			"nbr_lines : " << vect[1] << endl;

		Sleep(500);

		for (const bitset<12> &bitset_12 : bitset_vector)
		{
			cout << bitset_12.to_string() << " ";
			acm.sendBitSet(bitset_12);

			Sleep(2000);
		}

		acm.sendBitSet(bitset<12>("000100000100"));

		acm.disconnect();
	}

	printf("Appuyez sur entrer pour fermer le programme");
	cin.ignore(std::numeric_limits<streamsize>::max(),'\n');

	SetConsoleOutputCP(default_codepage);
	return 0;
}