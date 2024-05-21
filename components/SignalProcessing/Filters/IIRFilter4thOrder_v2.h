/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IIR Filter
//
// Rob Dobson 2024
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <iostream>
#include <vector>

using namespace std;

class IIRFilter4thOrder {
public:
  // Constructor
  IIRFilter4thOrder(const double* a_coeffs, const double* b_coeffs, const double* zi_initial) {
    // Copy the coefficients and initial values into the class variables
    for (int i = 0; i < 5; i++) {
      a_coeffs_[i] = a_coeffs[i];
      b_coeffs_[i] = b_coeffs[i];
    }
    for (int i = 0; i < 4; i++) {
      zi_[i] = zi_initial[i];
    }
  }

  // Process one new sample through the filter
  double process(double x1) {
    // Calculate the output of the filter
    double y0 = b_coeffs_[0] * x1 + zi_[0];
    double y1 = b_coeffs_[1] * x1 + b_coeffs_[0] * zi_[0] - a_coeffs_[1] * zi_[1];
    double y2 = b_coeffs_[2] * x1 + b_coeffs_[1] * zi_[0] + b_coeffs_[0] * zi_[1] - a_coeffs_[2] * zi_[2];
    double y3 = b_coeffs_[3] * x1 + b_coeffs_[2] * zi_[0] + b_coeffs_[1] * zi_[1] + b_coeffs_[0] * zi_[2] - a_coeffs_[3] * zi_[3];
    double y4 = b_coeffs_[4] * x1 + b_coeffs_[3] * zi_[0] + b_coeffs_[2] * zi_[1] + b_coeffs_[1] * zi_[2] + b_coeffs_[0] * zi_[3] - a_coeffs_[4] * zi_[4];

    // Update the initial values
    zi_[0] = y0;
    zi_[1] = y1;
    zi_[2] = y2;
    zi_[3] = y3;

    // Return the output of the filter
    return y4;
  }

private:
  // Coefficients of the filter
  double a_coeffs_[5];
  double b_coeffs_[5];

  // Initial values of the filter
  double zi_[4];
};
