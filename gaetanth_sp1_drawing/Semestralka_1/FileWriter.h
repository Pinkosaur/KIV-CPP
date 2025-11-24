#pragma once
#include "Instruction.h"
#include<iostream>
#include<fstream>
#include<string>

class FileWriter {
private:
	std::string OUTPUT_FILE;
	int fwidth;
	int fheight;
	std::vector<std::unique_ptr<Instruction>> instr_vect;
	int processed_count = 0;
public:
	void setOutputFile(const std::string filename) {
		OUTPUT_FILE = filename;
	}
	void setWidth(int w) {
		fwidth = w;
	}
	void setHeight(int h) {
		fheight = h;
	}
	int getProcessedCount() const {
		return processed_count;
	}
	int getVectLenght() const {
		return static_cast<int>(instr_vect.size());
	}

	bool try_add_instruction(const std::string line) {
		auto instr = Interpret_Row(line);
		if (instr) { // Nesmi byt nullptr
			instr_vect.push_back(std::move(instr));
			return true;
		}
		else return false;
	}

	// Zapis do souboru podle pripony
	bool write() {
		std::string extension = OUTPUT_FILE.substr(OUTPUT_FILE.size() - 4);
		if (extension == ".svg") {
			return make_svg_file();
		}
		if (extension == ".pgm") {
			return make_pgm_file();
		}
	}

	// Vytvoreni a vyplneni SVG souboru
	// vrati true pokud se podarily vsechny operace 
	bool make_svg_file() {
		std::ofstream svg_file(OUTPUT_FILE);

		// Genericly SVG header
		svg_file << "<html>\n<body>\n<svg width=\"" << fwidth << "\" height=\"" << fheight << "\" xmlns=\"http://www.w3.org/2000/svg\" style=\"background - color:white\">\n";
		
		int groups = 0; // pro vypocet zaviracich tagu
		std::string line;
		for (const std::unique_ptr<Instruction>& instr : instr_vect) {
			// psani jednotlivych instrukci do souboru (pokud jsou platne)
			if (instr->is_ok()) {
				line = instr->write_svg();
				processed_count += 1;
				svg_file << line;
			}
			else return false;

			// Pokud je instrukce Translate, Rotate nebo Scale, pridame zaviraci tag
			if (dynamic_cast<Translate*>(instr.get()) != nullptr or dynamic_cast<Rotate*>(instr.get()) != nullptr or dynamic_cast<Scale*>(instr.get()) != nullptr) {
				groups += 1;
			}
		}
		for (int i = 0; i < groups; i++) svg_file << "</g>\n";

		svg_file << "</svg>\n</body>\n</html>";
		svg_file.close();
		return true;
	}

	// Vytvoreni a vyplneni PGM souboru
	// vrati true pokud se podarily vsechny operace 
	bool make_pgm_file() {
		if (fwidth > 500 or fheight > 500) {
			std::cout << "Upozorneni: rozmery nad 500x500 ve formatu PGM mohou zpusobit ztratu kvality obrazku" << std::endl;
		}
		// Vytvoreni bitmapy pro logicke kresleni a matici transformace souradneho systemu
		std::vector<int> bitmap(fwidth * fheight, 1);
		Matrix3x3 transform; // Jednotkova matice

		for (const std::unique_ptr<Instruction>& instr : instr_vect) {

			if (!instr->is_ok()) {
				return false;
			}
			// Psani do bitmapu
			instr->write_pgm(bitmap, fwidth, fheight, transform);
			processed_count++;

		}

		// Zapis do souboru
		std::ofstream pgm_file(OUTPUT_FILE);
		pgm_file << "P2\n" << fwidth << " " << fheight << "\n1\n";
		for (int i = 0; i < fheight; ++i) {
			for (int j = 0; j < fwidth; ++j) {
				pgm_file << bitmap[i * fwidth + j] << " ";
			}
			pgm_file << "\n";
		}
		pgm_file.close();
		return true;
	}

	

private:
	// Rozpoznavani instrukci
	std::unique_ptr<Instruction> Interpret_Row(std::string row) const {
		size_t word_end = row.find(' ');
		std::string type = row.substr(0, word_end);
		if (type == "translate") {
			return std::make_unique<Translate>(row.substr(word_end + 1));
		}
		if (type == "rotate") {
			return std::make_unique<Rotate>(row.substr(word_end + 1));
		}
		if (type == "scale") {
			return std::make_unique<Scale>(row.substr(word_end + 1));
		}
		if (type == "circle") {
			return std::make_unique<Circle>(row.substr(word_end + 1));
		}
		if (type == "line") {
			return std::make_unique<Line>(row.substr(word_end + 1));
		}
		if (type == "rect") {
			return std::make_unique<Rectangle>(row.substr(word_end + 1));
		}
		std::cerr << "Neplatna instrukce" << std::endl;
		return nullptr;
	}
};