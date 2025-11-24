#include <iostream>
#include <string>
#include <stdexcept>
#include "MpInt.h"
#include "MpTerm.h"

// Hlavni vstupni bod programu
int main(int argc, char** argv) {
    std::cout << "Vitejte" << std::endl;

    try {
        // Overeni poctu argumentu
        if (argc != 2) {
            throw std::invalid_argument("Program vyzaduje presne 1 argument: 1, 2 nebo 3.");
        }

        // Prevod argumentu na cislo
        const int mode = std::stoi(argv[1]);
        std::cout << "Rezim: " << mode << std::endl;

        // Overeni platnosti rezimu
        if (mode < 1 || mode > 3) {
            throw std::invalid_argument("Neplatny rezim. Povolene hodnoty jsou 1, 2 nebo 3.");
        }

        // Spusteni vybraneho rezimu
        runMode(mode);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Chyba v argumentech: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Neocekavana chyba: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Koncim." << std::endl;
    return 0;
}