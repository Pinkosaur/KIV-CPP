#pragma once
#include "Instruction.h"
#include<algorithm>

class Line : public Instruction {
private:
    int x1;
    int y1;
    int x2;
    int y2;
    bool ok;
public:
    Line(std::string s) {
        // ok - true pokud je vse v poradku, v pripade chyby se nastavi na false
        ok = true;
        try {
            // parsovani vstupu
            std::istringstream iss(s);
            if (!(iss >> x1 >> y1 >> x2 >> y2)) {
                std::cerr << "Neplatny vstup pro caru" << std::endl;
                ok = false;
            }
            if (x1 == x2 && y1 == y2) {
                std::cerr << "Nelze kreslit caru mezi body stejnych souradnic" << std::endl;
                ok = false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Doslo k chybe pri parsovani instrukce 'line'. Uziti: line <x1> <y1> <x2> <y2> (cela cisla)" << std::endl;
            ok = false;
        }
    }

    // indikace spravnosti instrukce
    bool is_ok() const override {
        return ok;
    }

	// Vykresleni caru do bitmapy pro PGM
    int write_pgm(std::vector<int>& bitmap, const int height, const int width, const Matrix3x3& transform) const override {
        if (!ok) return 0;
		// Transformace souradnic podle matice transformace
        auto transform_point = [&transform](int x, int y) -> std::pair<int, int> {
            float new_x = transform.m[0][0] * x + transform.m[0][1] * y + transform.m[0][2];
            float new_y = transform.m[1][0] * x + transform.m[1][1] * y + transform.m[1][2];
            return { static_cast<int>(std::round(new_x)), static_cast<int>(std::round(new_y)) };
            };

        auto [tx1, ty1] = transform_point(x1, y1);
        auto [tx2, ty2] = transform_point(x2, y2);

		// Clamp transformovane souradnice do rozsahu bitmapy
        tx1 = std::clamp(tx1, 0, width - 1);
        ty1 = std::clamp(ty1, 0, height - 1);
        tx2 = std::clamp(tx2, 0, width - 1);
        ty2 = std::clamp(ty2, 0, height - 1);


		// Bresenhamuv algoritmus pro vykresleni usecky
        int dx = std::abs(tx2 - tx1), sx = tx1 < tx2 ? 1 : -1;
        int dy = -std::abs(ty2 - ty1), sy = ty1 < ty2 ? 1 : -1;
        int err = dx + dy, e2;

        while (true) {
			// kresleni hlavniho pixelu
            if (tx1 >= 0 && tx1 < width && ty1 >= 0 && ty1 < height) {
				bitmap[ty1 * width + tx1] = 0; // hodnota pixelu na 0 (cerna)
            }

			// kresleni vedlejsich pixelu pro tloustku 2px
			if (std::abs(dx) > std::abs(dy)) { // Cara je spise vodorovna
				int ny = ty1 + 1; // Kresleni pixelu nad nebo pod hlavnim pixelem
                if (ny >= 0 && ny < height) {
                    bitmap[ny * width + tx1] = 0;
                }
            }
			else { // Cara je spise svisla
				int nx = tx1 + 1; // Kresleni pixelu vedle hlavniho pixelu
                if (nx >= 0 && nx < width) {
                    bitmap[ty1 * width + nx] = 0;
                }
            }

			// Podminky pro ukonceni algoritmu
            if (tx1 == tx2 && ty1 == ty2) break;

            e2 = 2 * err;
            if (e2 >= dy) { err += dy; tx1 += sx; }
            if (e2 <= dx) { err += dx; ty1 += sy; }
        }
        return 1;
    }

	// Psani SVG tagu pro caru
    std::string write_svg() const override {
        if (!ok) return "";
        std::string tag = "<line x1=\"" + std::to_string(x1) + "\" y1=\"" + std::to_string(y1) + "\" x2=\"" + std::to_string(x2) + "\" y2=\"" + std::to_string(y2) + "\" stroke=\"black\" stroke-width=\"2\" />\n";
        return tag;
    }
};