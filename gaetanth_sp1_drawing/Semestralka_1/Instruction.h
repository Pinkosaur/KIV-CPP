#pragma once
#include<string>
#include<iostream>
#include<stdexcept>
#include<sstream>
#include<vector>
#define _USE_MATH_DEFINES
#include<math.h>
#include<cmath>

// Matice pro transformace souradneho systemu
struct Matrix3x3 {
	double m[3][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };

	Matrix3x3 operator*(const Matrix3x3& other) const {
		Matrix3x3 result{};
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				result.m[i][j] = 0;
				for (int k = 0; k < 3; ++k) {
					result.m[i][j] += m[i][k] * other.m[k][j];
				}
			}
		}
		return result;
	}
};

// Interface pro instrukce
class Instruction {
public:
	virtual ~Instruction() = default;

	virtual bool is_ok() const = 0;

	virtual int write_pgm(std::vector<int>& bitmap, const int height, const int width, const Matrix3x3& transform) const = 0;
	virtual std::string write_svg() const = 0;
};
