#pragma once
#include "Instruction.h"

class Rotate : public Instruction {
private:
	int center_x;
	int center_y;
	float angle;
	bool ok;
public:
	Rotate(std::string s) {
		// ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
		ok = true;
		try {
			// parsovani vstupu
			std::istringstream iss(s);
			if (!(iss >> center_x >> center_y >> angle)) {
				std::cerr << "Neplatny vstup pro rotaci" << std::endl;
				ok = false;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Doslo k chybe pri parsovani instrukce 'rotate'. UzitiL rotate <x> <y> <uhel> (x, y cela cisla, uhel realne cislo)" << std::endl;
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
		const double rad_angle = angle * M_PI / 180.0;

		// Vypocet komponentu matice rotace
		const double cos_theta = std::cos(rad_angle);
		const double sin_theta = std::sin(rad_angle);

		// Translace stredoveho bodu do stredu rotace
		const Matrix3x3 translate_to_origin = {
			1, 0, static_cast<double>(-center_x),
			0, 1, static_cast<double>(-center_y),
			0, 0, 1
		};

		// Matice rotace
		Matrix3x3 rotation = {
			cos_theta, -sin_theta, 0,
			sin_theta, cos_theta,  0,
			0,         0,          1
		};

		// Translace spatky
		const Matrix3x3 translate_back = {
			1, 0, static_cast<double>(center_x),
			0, 1, static_cast<double>(center_y),
			0, 0, 1
		};

		// Kombinace transformaci
		const Matrix3x3 rotation_transform = translate_back * rotation * translate_to_origin;

		const_cast<Matrix3x3&>(transform) = transform * rotation_transform;

		return 1;
	}

	// Psani SVG tagu rotace
	std::string write_svg() const override {
		if (!ok) return "";
		std::string tag = "<g transform=\"rotate(" + std::to_string(angle) + "," + std::to_string(center_x) + "," + std::to_string(center_x) + ")\">\n";
		return tag;
	}

};