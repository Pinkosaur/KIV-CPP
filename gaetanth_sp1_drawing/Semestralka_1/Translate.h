#pragma once
#include "Instruction.h"

class Translate : public Instruction {
private:
	int x;
	int y;
	bool ok;
public:
	Translate(std::string s) {
		// ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
		ok = true;
		try {
			// parsovani vstupu
			std::istringstream iss(s);
			if (!(iss >> x >> y)) {
				std::cerr << "Neplatny vstup pro translaci" << std::endl;
				ok = false;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Doslo k chybe pri parsovani instrukce 'translate'. Uziti: translate <x> <y>" << std::endl;
			ok = false;
		}
	}

	// indikace spravnosti instrukce
	bool is_ok() const override {
		return ok;
	}

	// Modifikace matice transformace pro bitmapu
	int write_pgm(std::vector<int>& bitmap, const int height, const int width, const Matrix3x3& transform) const override {
		if (!ok) return 0;
		// vytvoreni matice translace
		const Matrix3x3 translation_matrix = {
			1, 0, static_cast<double>(x),
			0, 1, static_cast<double>(y),
			0, 0, 1
		};

		// aplikace transformace
		const_cast<Matrix3x3&>(transform) = transform * translation_matrix;

		return 1;
	}

	// Psani SVG tagu translace
	std::string write_svg() const override {
		if (!ok) return "";
		std::string tag = "<g transform=\"translate(" + std::to_string(x) + "," + std::to_string(y) + ")\">\n";
		return tag;
	}
};