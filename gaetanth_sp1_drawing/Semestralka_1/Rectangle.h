#pragma once
#include "Instruction.h"

class Rectangle : public Instruction {
private:
	int x;
	int y;
	int width;
	int height;
	bool ok;
public:
	Rectangle(std::string s) {
		// ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
		ok = true;
		try {
			// parsovani vstupu
			std::istringstream iss(s);
			if (!(iss >> x >> y >> width >> height)) {
				std::cerr << "Neplatny vstup pro obdelnik" << std::endl;
				ok = false;
			}
			// neplatna sirka nebo vyska
			if (width == 0 || height == 0) {
				std::cerr << "Rozmery obdelniku nemohou byt 0." << std::endl;
				ok = false;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Doslo k chybe pri parsovani instrukce 'rect'. Uziti: rect <x> <y> <sirka> <vyska> (cela cisla)" << std::endl;
			ok = false;
		}
	}

	// indikace spravnosti instrukce
	bool is_ok() const override {
		return ok;
	}


	// Vykresleni obdelniku do bitmapy pro PGM
	int write_pgm(std::vector<int>& bitmap, const int height, const int width, const Matrix3x3& transform) const override {
		if (!ok) return 0;

		// Transformace souradnic rohu obdelniku
		std::vector<std::pair<double, double>> corners = {
			{x, y},
			{x + this->width, y},
			{x + this->width, y + this->height},
			{x, y + this->height}
		};

		for (auto& corner : corners) {
			double tx = transform.m[0][0] * corner.first + transform.m[0][1] * corner.second + transform.m[0][2];
			double ty = transform.m[1][0] * corner.first + transform.m[1][1] * corner.second + transform.m[1][2];
			corner.first = tx;
			corner.second = ty;
		}

		// Lambda funkce pro vykresleni dvojite usecky
		auto draw_double_line = [&](int x1, int y1, int x2, int y2) {
			draw_line(bitmap, height, width, x1, y1, x2, y2);         // Hlavni usecka
			if (std::abs(x2 - x1) > std::abs(y2 - y1)) {              // usecka je spise vodorovna
				draw_line(bitmap, height, width, x1, y1 + 1, x2, y2 + 1); // druha usecka je dole
			}
			else {                                                 // spise svisla
				draw_line(bitmap, height, width, x1 + 1, y1, x2 + 1, y2); // druha usecka je vpravo
			}
			};

		// kresleni obdelniku
		draw_double_line(corners[0].first, corners[0].second, corners[1].first, corners[1].second);
		draw_double_line(corners[1].first, corners[1].second, corners[2].first, corners[2].second);
		draw_double_line(corners[2].first, corners[2].second, corners[3].first, corners[3].second);
		draw_double_line(corners[3].first, corners[3].second, corners[0].first, corners[0].second);

		return 1;
	}

	// Psani SVG tagu obdelniku
	std::string write_svg() const override {
		if (!ok) return "";
		std::string tag = "<rect x=\"" + std::to_string(x) + "\" y=\"" + std::to_string(y) + "\" width=\"" + std::to_string(width) + "\" height=\"" + std::to_string(height) + "\" stroke=\"black\" fill=\"none\" stroke-width=\"2\" />\n";
		return tag;
	}

private:
	// Pomocna funkce pro vykresleni usecky (Bresenhamuv algoritmus)
	void draw_line(std::vector<int>& bitmap, int height, int width, double x1, double y1, double x2, double y2) const {
		int ix1 = static_cast<int>(std::round(x1));
		int iy1 = static_cast<int>(std::round(y1));
		int ix2 = static_cast<int>(std::round(x2));
		int iy2 = static_cast<int>(std::round(y2));

		auto clamp = [&](int& val, int max) { val = std::max(0, std::min(val, max - 1)); };
		clamp(ix1, width);
		clamp(iy1, height);
		clamp(ix2, width);
		clamp(iy2, height);

		int dx = std::abs(ix2 - ix1), sx = ix1 < ix2 ? 1 : -1;
		int dy = -std::abs(iy2 - iy1), sy = iy1 < iy2 ? 1 : -1;
		int err = dx + dy, e2;

		while (true) {
			bitmap[iy1 * width + ix1] = 0;
			if (ix1 == ix2 && iy1 == iy2) break;
			e2 = 2 * err;
			if (e2 >= dy) { err += dy; ix1 += sx; }
			if (e2 <= dx) { err += dx; iy1 += sy; }
		}
	}
};