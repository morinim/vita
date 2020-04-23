/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>

#include "utility/csv_parser.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("CSV_PARSER")
{

TEST_CASE("Header")
{
  using namespace vita;

  std::istringstream abalone_h(R"(
    sex,length,diameter,height,whole weight,shucked weight,viscera weight,shell weight,rings
    M,0.455,0.365,0.095,0.514,0.2245,0.101,0.15,15
    M,0.35,0.265,0.09,0.2255,0.0995,0.0485,0.07,7
    F,0.53,0.42,0.135,0.677,0.2565,0.1415,0.21,9
    M,0.44,0.365,0.125,0.516,0.2155,0.114,0.155,10
    I,0.33,0.255,0.08,0.205,0.0895,0.0395,0.055,7
    I,0.425,0.3,0.095,0.3515,0.141,0.0775,0.12,8
    F,0.53,0.415,0.15,0.7775,0.237,0.1415,0.33,20
    F,0.545,0.425,0.125,0.768,0.294,0.1495,0.26,16
    M,0.475,0.37,0.125,0.5095,0.2165,0.1125,0.165,9
    F,0.55,0.44,0.15,0.8945,0.3145,0.151,0.32,19)");

  std::istringstream abalone_nh(R"(
    M,0.455,0.365,0.095,0.514,0.2245,0.101,0.15,15
    M,0.35,0.265,0.09,0.2255,0.0995,0.0485,0.07,7
    F,0.53,0.42,0.135,0.677,0.2565,0.1415,0.21,9
    M,0.44,0.365,0.125,0.516,0.2155,0.114,0.155,10
    I,0.33,0.255,0.08,0.205,0.0895,0.0395,0.055,7
    I,0.425,0.3,0.095,0.3515,0.141,0.0775,0.12,8
    F,0.53,0.415,0.15,0.7775,0.237,0.1415,0.33,20
    F,0.545,0.425,0.125,0.768,0.294,0.1495,0.26,16
    M,0.475,0.37,0.125,0.5095,0.2165,0.1125,0.165,9
    F,0.55,0.44,0.15,0.8945,0.3145,0.151,0.32,19)");

  std::istringstream iris_h(R"(
    sepal length,sepal width,petal length,petal width,class
    5.1,3.5,1.4,0.2,Iris-setosa
    4.9,3,1.4,0.2,Iris-setosa
    4.7,3.2,1.3,0.2,Iris-setosa
    7,3.2,4.7,1.4,Iris-versicolor
    6.4,3.2,4.5,1.5,Iris-versicolor
    6.9,3.1,4.9,1.5,Iris-versicolor
    6.3,2.5,5,1.9,Iris-virginica
    6.5,3,5.2,2,Iris-virginica
    6.2,3.4,5.4,2.3,Iris-virginica
    5.9,3,5.1,1.8,Iris-virginica)");

  std::istringstream iris_nh(R"(
    5.1,3.5,1.4,0.2,Iris-setosa
    4.9,3,1.4,0.2,Iris-setosa
    4.7,3.2,1.3,0.2,Iris-setosa
    7,3.2,4.7,1.4,Iris-versicolor
    6.4,3.2,4.5,1.5,Iris-versicolor
    6.9,3.1,4.9,1.5,Iris-versicolor
    6.3,2.5,5,1.9,Iris-virginica
    6.5,3,5.2,2,Iris-virginica
    6.2,3.4,5.4,2.3,Iris-virginica
    5.9,3,5.1,1.8,Iris-virginica)");

  csv_parser csv1(abalone_h);
  CHECK(csv1.dialect().has_header == true);

  csv_parser csv2(abalone_nh);
  CHECK(csv2.dialect().has_header == false);

  csv_parser csv3(iris_h);
  CHECK(csv3.dialect().has_header == true);

  csv_parser csv4(iris_nh);
  CHECK(csv4.dialect().has_header == false);
}

}  // TEST_SUITE("CSV_PARSER")
