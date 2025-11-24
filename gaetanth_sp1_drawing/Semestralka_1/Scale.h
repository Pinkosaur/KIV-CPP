#pragma once
#include "Instruction.h"

class Scale : public Instruction {
private:
	int x;
	int y;
	double k;
    bool ok;
public:
	Scale(std::string s) {
        // ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
        ok = true;
		try {
            // parsovani vstupu
            std::istringstream iss(s);
            if (!(iss >> x >> y >> k)) {
                std::cerr << "Neplatny vstup pro zmenu meritka" << std::endl;
                ok = false;
            }
			// nevalidni faktor
            if (k == 0) {
                std::cerr << "Meritku nelze zmenit na 0" << std::endl;
                ok = false;
            }
		}
		catch (const std::exception& e) {
            std::cerr << "Doslo k chybe pri parsovani instrukce 'scale'. Uziti: scale <x> <y> <f> (x, y cela cisla, f realne nenulove)" << std::endl;
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
		// Vytvoreni matice translace do stredu
        const Matrix3x3 translate_to_origin = {
            1, 0, static_cast<double>(-x),
            0, 1, static_cast<double>(-y),
            0, 0, 1
        };

		// Vytvoreni matice meritka
        const Matrix3x3 scale_matrix = {
            static_cast<double>(k), 0, 0,
            0, static_cast<double>(k), 0,
            0, 0, 1
        };

		// Translace spatky do pocatku
        const Matrix3x3 translate_back = {
            1, 0, static_cast<double>(x),
            0, 1, static_cast<double>(y),
            0, 0, 1
        };

        // Kombinace transofrmaci
        const Matrix3x3 scale_transform = translate_back * scale_matrix * translate_to_origin;

        const_cast<Matrix3x3&>(transform) = transform * scale_transform;

        return 1;
    }

	// Psani SVG tagu zmeny meritka
    std::string write_svg() const override {
        if (!ok) return "";
        std::string tag = "<g transform=\"translate(" + std::to_string(x) + "," + std::to_string(y) + ") scale(" + std::to_string(k) + ") translate(" + std::to_string(-x) + "," + std::to_string(-y) + ")\">\n";
        return tag;
    }


};