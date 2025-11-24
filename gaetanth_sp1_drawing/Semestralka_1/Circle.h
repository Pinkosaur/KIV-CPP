#pragma once
#include "Instruction.h"

class Circle : public Instruction {
private:
	int center_x;
	int center_y;
	int radius;
    bool ok;
public:
	Circle(std::string s) {
		// ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
        ok = true;
		try {
            // parsovani vstupu
            std::istringstream iss(s);
            if (!(iss >> center_x >> center_y >> radius)) {
                std::cerr << "Neplatny vstup pro kruh" << std::endl;
                ok = false;
            }
            // nevalidni polomer
            if (radius < 0) {
                std::cerr << "Polomer musi byt kladny" << std::endl;
				ok = false;
            }
		}
		catch (const std::exception& e) {
			std::cerr << "Doslo k chybe pri parsovani instrukce 'circle'. Uziti: circle <x> <y> <polomer> (cela cisla)" << std::endl;
			ok = false;
		}
	}

	// indikace spravnosti instrukce
	bool is_ok() const override {
		return ok;
	}

	// Vykresleni kruhu do bitmapy pro PGM
    int write_pgm(std::vector<int>& bitmap, int height, int width, const Matrix3x3& transform) const override {
        if (!ok) return 0;
		// Transcormace souradnic stredoveho bodu
        double transformed_x = transform.m[0][0] * center_x + transform.m[0][1] * center_y + transform.m[0][2];
        double transformed_y = transform.m[1][0] * center_x + transform.m[1][1] * center_y + transform.m[1][2];

		// rasterizace kruhu
        int xm = static_cast<int>(std::round(transformed_x));
        int ym = static_cast<int>(std::round(transformed_y));
        int r = radius;

		// Lambda funkce pro vykresleni kruhu
        auto draw_circle = [&](int radius) {
            int x = -radius, y = 0, err = 2 - 2 * radius;
            auto set_pixel = [&](int px, int py) {
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    bitmap[py * width + px] = 0;
                }
                };

            do {
                set_pixel(xm - x, ym + y);
                set_pixel(xm - y, ym - x);
                set_pixel(xm + x, ym - y);
                set_pixel(xm + y, ym + x);
                int r_err = err;
                if (r_err <= y) err += ++y * 2 + 1;
                if (r_err > x || err > y) err += ++x * 2 + 1;
            } while (x < 0);
            };

        draw_circle(r);

        // Pro tloustku 2px
        draw_circle(r + 1);
		
        return 1;
    }

	// Psani SVG tagu kruhu
	std::string write_svg() const override {
        if (!ok) return "";
		std::string tag = "<circle r=\"" + std::to_string(radius) + "\" cx=\"" + std::to_string(center_x) + "\" cy=\"" + std::to_string(center_y) + "\" stroke=\"black\" fill=\"none\" stroke-width=\"2\" />\n";
		return tag;
	}
};