#include "Instruction.h"
#include "Translate.h"
#include "Rotate.h"
#include "Scale.h"
#include "Line.h"
#include "Rectangle.h"
#include "Circle.h"
#include "FileWriter.h"

#include<iostream>
#include<regex>
#include<string>
#include<fstream>
#include<vector>
#include<array>
#include<stdexcept>



int main(int argc, char** pArgv) {

	if (argc != 4) {
		std::cerr << "Uziti: drawing.exe <vstupni_soubor>.txt <vystupni_soubor>.svg|.pgm <sirka>x<vyska>\n";
		return 1;
	}

	// Inicializace objektu pro zapis do souboru
	FileWriter filewriter;

	// Parsovani vstupu
	const std::regex inputFormat(R"(^[\w.-\\]+\.txt$)");
	const std::regex outputFormat(R"(^[\w.-\\]+\.(svg|pgm)$)");
	const std::regex sizeFormat("[0-9]+x[0-9]+");

	std::string INPUT_FILE;
	int file_width;
	int file_height;

	// Vstupni soubor
	if (!std::regex_match(pArgv[1], inputFormat)) {
		std::cout << "Neplatny vstupny soubor" << std::endl;
		return 2;
	}
	else INPUT_FILE = pArgv[1];

	// Vystupni soubor
	if (!std::regex_match(pArgv[2], outputFormat)) {
		std::cout << "Neplatny vystupny soubor" << std::endl;
		return 2;
	}
	else filewriter.setOutputFile(pArgv[2]);

	// Velikost vystupu
	if (!std::regex_match(pArgv[3], sizeFormat)) {
		std::cout << "Neplatna velikost vystupu" << std::endl;
		return 2;
	}
	else {
		std::string size = pArgv[3];
		size_t x = size.find('x');
		int width = std::stoi(size.substr(0, x));
		int height = std::stoi(size.substr(x + 1));
		if (width > 0 && height > 0) {
			filewriter.setWidth(width);
			filewriter.setHeight(height);
		}
		else {
			std::cout << "Neplatna velikost vystupu" << std::endl;
			return 3;
		}	
	}

	std::ifstream input_file(INPUT_FILE);
	if (!input_file.is_open()) {
		std::cerr << "Nepodarilo se otevrit soubor " << INPUT_FILE << std::endl;
		return 4;
	}

	// Cteni radku souboru
	
	std::string instr_line;

	std::vector<std::unique_ptr<Instruction>> instr_vect;

	while (getline(input_file, instr_line)) {
		// Odstraneni bilych znaku na zacatku radku
		size_t first_non_whitespace = instr_line.find_first_not_of(" \t");
		if (first_non_whitespace != std::string::npos) {
			instr_line = instr_line.substr(first_non_whitespace);
		}
		else {
			// prazdny radek
			continue;
		}

		// Zjistit, zda radek neni prazdny nebo komentar
		if (!instr_line.empty() && instr_line[0] != '#') {
			bool check_success = filewriter.try_add_instruction(instr_line);
			if (!check_success) {
				std::cerr << "Nepodarilo se interpretovat instrukci '" << instr_line << "'" << std::endl;
				return 5;
			}
		}
	}

	input_file.close();

	// V pripade, ze nebyly nacteny zadne instrukce
	if (filewriter.getVectLenght() == 0) {
		std::cerr << "Zadna instrukce nebyla nalezena v souboru" << std::endl;
		return 6;
	}

	// Chyba pri zapisu do souboru
	if (!filewriter.write()) {
		std::cerr << "Chyba pri zpracovani souboru, zastaveni programu." << std::endl;
		return 7;
	}

	// Vse probehlo v poradku
	std::cout << "OK" << std::endl;
	std::cout << filewriter.getProcessedCount() << std::endl;

	return 0;
}