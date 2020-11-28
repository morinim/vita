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

#include "kernel/gp/src/category_set.h"
#include "kernel/gp/src/dataframe.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

TEST_SUITE("CATEGORY_SET")
{

TEST_CASE("wine categories weak")
{
  using namespace vita;

  std::istringstream wine(R"(
fixed acidity,volatile acidity,citric acid,residual sugar,chlorides,free sulfur dioxide,total sulfur dioxide,density,pH,sulphates,alcohol,quality
    7.4,0.7, 0,   1.9,0.076,11, 34,0.9978,3.51,0.56, 9.4,5
    7.8,0.88,0,   2.6,0.098,25, 67,0.9968,3.2, 0.68, 9.8,5
    7.8,0.76,0.04,2.3,0.092,15, 54,0.997, 3.26,0.65, 9.8,5
   11.2,0.28,0.56,1.9,0.075,17, 60,0.998, 3.16,0.58, 9.8,6
    7.4,0.7, 0,   1.9,0.076,11, 34,0.9978,3.51,0.56, 9.4,5
    7.4,0.66,0,   1.8,0.075,13, 40,0.9978,3.51,0.56, 9.4,5
    7.9,0.6, 0.06,1.6,0.069,15, 59,0.9964,3.3, 0.46, 9.4,5
    7.3,0.65,0,   1.2,0.065,15, 21,0.9946,3.39,0.47,10,  7
    7.8,0.58,0.02,2,  0.073, 9, 18,0.9968,3.36,0.57, 9.5,7
    7.5,0.5, 0.36,6.1,0.071,17,102,0.9978,3.35,0.8, 10.5,5)");

  dataframe d;
  dataframe::params p;

  CHECK(d.read_csv(wine, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns);
  CHECK(cs.is_valid());

  CHECK(cs.column( 0) == category_info{0, d_double,        "fixed acidity"});
  CHECK(cs.column( 1) == category_info{0, d_double,     "volatile acidity"});
  CHECK(cs.column( 2) == category_info{0, d_double,          "citric acid"});
  CHECK(cs.column( 3) == category_info{0, d_double,       "residual sugar"});
  CHECK(cs.column( 4) == category_info{0, d_double,            "chlorides"});
  CHECK(cs.column( 5) == category_info{0, d_double,  "free sulfur dioxide"});
  CHECK(cs.column( 6) == category_info{0, d_double, "total sulfur dioxide"});
  CHECK(cs.column( 7) == category_info{0, d_double,              "density"});
  CHECK(cs.column( 8) == category_info{0, d_double,                   "pH"});
  CHECK(cs.column( 9) == category_info{0, d_double,            "sulphates"});
  CHECK(cs.column(10) == category_info{0, d_double,              "alcohol"});
  CHECK(cs.column(11) == category_info{0, d_double,              "quality"});

  CHECK(cs.column(       "fixed acidity") == cs.column( 0));
  CHECK(cs.column(    "volatile acidity") == cs.column( 1));
  CHECK(cs.column(         "citric acid") == cs.column( 2));
  CHECK(cs.column(      "residual sugar") == cs.column( 3));
  CHECK(cs.column(           "chlorides") == cs.column( 4));
  CHECK(cs.column( "free sulfur dioxide") == cs.column( 5));
  CHECK(cs.column("total sulfur dioxide") == cs.column( 6));
  CHECK(cs.column(             "density") == cs.column( 7));
  CHECK(cs.column(                  "pH") == cs.column( 8));
  CHECK(cs.column(           "sulphates") == cs.column( 9));
  CHECK(cs.column(             "alcohol") == cs.column(10));
  CHECK(cs.column(             "quality") == cs.column(11));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0});
  CHECK(cs.category( 0).domain == d_double);

  for (const auto &c: cs)
  {
    CHECK(c.domain == d_double);
    CHECK(c.category == 0);
  }

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("wine categories strong")
{
  using namespace vita;

  std::istringstream wine(R"(
fixed acidity,volatile acidity,citric acid,residual sugar,chlorides,free sulfur dioxide,total sulfur dioxide,density,pH,sulphates,alcohol,quality
    7.4,0.7, 0,   1.9,0.076,11, 34,0.9978,3.51,0.56, 9.4,5
    7.8,0.88,0,   2.6,0.098,25, 67,0.9968,3.2, 0.68, 9.8,5
    7.8,0.76,0.04,2.3,0.092,15, 54,0.997, 3.26,0.65, 9.8,5
   11.2,0.28,0.56,1.9,0.075,17, 60,0.998, 3.16,0.58, 9.8,6
    7.4,0.7, 0,   1.9,0.076,11, 34,0.9978,3.51,0.56, 9.4,5
    7.4,0.66,0,   1.8,0.075,13, 40,0.9978,3.51,0.56, 9.4,5
    7.9,0.6, 0.06,1.6,0.069,15, 59,0.9964,3.3, 0.46, 9.4,5
    7.3,0.65,0,   1.2,0.065,15, 21,0.9946,3.39,0.47,10,  7
    7.8,0.58,0.02,2,  0.073, 9, 18,0.9968,3.36,0.57, 9.5,7
    7.5,0.5, 0.36,6.1,0.071,17,102,0.9978,3.35,0.8, 10.5,5)");

  dataframe d;
  dataframe::params p;

  CHECK(d.read_csv(wine, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns, typing::strong);

  CHECK(cs.column( 0) == category_info{ 0, d_double,        "fixed acidity"});
  CHECK(cs.column( 1) == category_info{ 1, d_double,     "volatile acidity"});
  CHECK(cs.column( 2) == category_info{ 2, d_double,          "citric acid"});
  CHECK(cs.column( 3) == category_info{ 3, d_double,       "residual sugar"});
  CHECK(cs.column( 4) == category_info{ 4, d_double,            "chlorides"});
  CHECK(cs.column( 5) == category_info{ 5, d_double,  "free sulfur dioxide"});
  CHECK(cs.column( 6) == category_info{ 6, d_double, "total sulfur dioxide"});
  CHECK(cs.column( 7) == category_info{ 7, d_double,              "density"});
  CHECK(cs.column( 8) == category_info{ 8, d_double,                   "pH"});
  CHECK(cs.column( 9) == category_info{ 9, d_double,            "sulphates"});
  CHECK(cs.column(10) == category_info{10, d_double,              "alcohol"});
  CHECK(cs.column(11) == category_info{11, d_double,              "quality"});

  CHECK(cs.column(       "fixed acidity") == cs.column( 0));
  CHECK(cs.column(    "volatile acidity") == cs.column( 1));
  CHECK(cs.column(         "citric acid") == cs.column( 2));
  CHECK(cs.column(      "residual sugar") == cs.column( 3));
  CHECK(cs.column(           "chlorides") == cs.column( 4));
  CHECK(cs.column( "free sulfur dioxide") == cs.column( 5));
  CHECK(cs.column("total sulfur dioxide") == cs.column( 6));
  CHECK(cs.column(             "density") == cs.column( 7));
  CHECK(cs.column(                  "pH") == cs.column( 8));
  CHECK(cs.column(           "sulphates") == cs.column( 9));
  CHECK(cs.column(             "alcohol") == cs.column(10));
  CHECK(cs.column(             "quality") == cs.column(11));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories()
        == std::set<category_t>{0,1,2,3,4,5,6,7,8,9,10,11});
  CHECK(cs.category( 0).domain == d_double);
  CHECK(cs.category( 1).domain == d_double);
  CHECK(cs.category( 2).domain == d_double);
  CHECK(cs.category( 3).domain == d_double);
  CHECK(cs.category( 4).domain == d_double);
  CHECK(cs.category( 5).domain == d_double);
  CHECK(cs.category( 6).domain == d_double);
  CHECK(cs.category( 7).domain == d_double);
  CHECK(cs.category( 8).domain == d_double);
  CHECK(cs.category( 9).domain == d_double);
  CHECK(cs.category(10).domain == d_double);
  CHECK(cs.category(11).domain == d_double);

  category_t i(0);
  for (const auto &c: cs)
  {
    CHECK(c.domain == d_double);
    CHECK(c.category == i);
    CHECK(c.category == cs.category(i).category);

    ++i;
  }

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("abalone categories weak")
{
  using namespace vita;

  std::istringstream abalone(R"(
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

  dataframe d;
  dataframe::params p;
  p.output_index = 8;

  CHECK(d.read_csv(abalone, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{0, d_double,         "rings"});
  CHECK(cs.column(1) == category_info{1, d_string,           "sex"});
  CHECK(cs.column(2) == category_info{0, d_double,        "length"});
  CHECK(cs.column(3) == category_info{0, d_double,      "diameter"});
  CHECK(cs.column(4) == category_info{0, d_double,        "height"});
  CHECK(cs.column(5) == category_info{0, d_double,  "whole weight"});
  CHECK(cs.column(6) == category_info{0, d_double,"shucked weight"});
  CHECK(cs.column(7) == category_info{0, d_double,"viscera weight"});
  CHECK(cs.column(8) == category_info{0, d_double,  "shell weight"});

  CHECK(cs.column(         "rings") == cs.column(0));
  CHECK(cs.column(           "sex") == cs.column(1));
  CHECK(cs.column(        "length") == cs.column(2));
  CHECK(cs.column(      "diameter") == cs.column(3));
  CHECK(cs.column(        "height") == cs.column(4));
  CHECK(cs.column(  "whole weight") == cs.column(5));
  CHECK(cs.column("shucked weight") == cs.column(6));
  CHECK(cs.column("viscera weight") == cs.column(7));
  CHECK(cs.column(  "shell weight") == cs.column(8));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0,1});

  CHECK(cs.category(0).domain == d_double);
  CHECK(cs.category(1).domain == d_string);

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("abalone categories strong")
{
  using namespace vita;

  std::istringstream abalone(R"(
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

  dataframe d;
  dataframe::params p;
  p.output_index = 8;

  CHECK(d.read_csv(abalone, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns, typing::strong);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{0, d_double,         "rings"});
  CHECK(cs.column(1) == category_info{1, d_string,           "sex"});
  CHECK(cs.column(2) == category_info{2, d_double,        "length"});
  CHECK(cs.column(3) == category_info{3, d_double,      "diameter"});
  CHECK(cs.column(4) == category_info{4, d_double,        "height"});
  CHECK(cs.column(5) == category_info{5, d_double,  "whole weight"});
  CHECK(cs.column(6) == category_info{6, d_double,"shucked weight"});
  CHECK(cs.column(7) == category_info{7, d_double,"viscera weight"});
  CHECK(cs.column(8) == category_info{8, d_double,  "shell weight"});

  CHECK(cs.column(         "rings") == cs.column(0));
  CHECK(cs.column(           "sex") == cs.column(1));
  CHECK(cs.column(        "length") == cs.column(2));
  CHECK(cs.column(      "diameter") == cs.column(3));
  CHECK(cs.column(        "height") == cs.column(4));
  CHECK(cs.column(  "whole weight") == cs.column(5));
  CHECK(cs.column("shucked weight") == cs.column(6));
  CHECK(cs.column("viscera weight") == cs.column(7));
  CHECK(cs.column(  "shell weight") == cs.column(8));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0,1,2,3,4,5,6,7,8});

  CHECK(cs.category(0) == cs.column(0));
  CHECK(cs.category(1) == cs.column(1));
  CHECK(cs.category(2) == cs.column(2));
  CHECK(cs.category(3) == cs.column(3));
  CHECK(cs.category(4) == cs.column(4));
  CHECK(cs.category(5) == cs.column(5));
  CHECK(cs.category(6) == cs.column(6));
  CHECK(cs.category(7) == cs.column(7));
  CHECK(cs.category(8) == cs.column(8));

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("ecoli categories")
{
  using namespace vita;

  std::istringstream ecoli(R"(
    sequence name, mcg,  gvh,  lip,  chg,  aac, alm1, alm2, localization
    AAT_ECOLI,    0.49, 0.29, 0.48, 0.50, 0.56, 0.24, 0.35, cp
    ACEA_ECOLI,   0.07, 0.40, 0.48, 0.50, 0.54, 0.35, 0.44, cp
    ACEK_ECOLI,   0.56, 0.40, 0.48, 0.50, 0.49, 0.37, 0.46, cp
    ACKA_ECOLI,   0.59, 0.49, 0.48, 0.50, 0.52, 0.45, 0.36, cp
    ADI_ECOLI,    0.23, 0.32, 0.48, 0.50, 0.55, 0.25, 0.35, cp
    ALKH_ECOLI,   0.67, 0.39, 0.48, 0.50, 0.36, 0.38, 0.46, cp
    AMPD_ECOLI,   0.29, 0.28, 0.48, 0.50, 0.44, 0.23, 0.34, cp
    AMY2_ECOLI,   0.21, 0.34, 0.48, 0.50, 0.51, 0.28, 0.39, cp
    APT_ECOLI,    0.20, 0.44, 0.48, 0.50, 0.46, 0.51, 0.57, cp
    ARAC_ECOLI,   0.42, 0.40, 0.48, 0.50, 0.56, 0.18, 0.30, cp)");

  dataframe d;
  dataframe::params p;
  p.output_index = std::nullopt;

  CHECK(d.read_csv(ecoli, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{undefined_category, d_void, ""});
  CHECK(cs.column(1) == category_info{0, d_string,"sequence name"});
  CHECK(cs.column(2) == category_info{1, d_double,          "mcg"});
  CHECK(cs.column(3) == category_info{1, d_double,          "gvh"});
  CHECK(cs.column(4) == category_info{1, d_double,          "lip"});
  CHECK(cs.column(5) == category_info{1, d_double,          "chg"});
  CHECK(cs.column(6) == category_info{1, d_double,          "aac"});
  CHECK(cs.column(7) == category_info{1, d_double,         "alm1"});
  CHECK(cs.column(8) == category_info{1, d_double,         "alm2"});
  CHECK(cs.column(9) == category_info{2, d_string, "localization"});

  CHECK(cs.column(             "") == cs.column(0));
  CHECK(cs.column("sequence name") == cs.column(1));
  CHECK(cs.column(          "mcg") == cs.column(2));
  CHECK(cs.column(          "gvh") == cs.column(3));
  CHECK(cs.column(          "lip") == cs.column(4));
  CHECK(cs.column(          "chg") == cs.column(5));
  CHECK(cs.column(          "aac") == cs.column(6));
  CHECK(cs.column(         "alm1") == cs.column(7));
  CHECK(cs.column(         "alm2") == cs.column(8));
  CHECK(cs.column( "localization") == cs.column(9));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0,1,2,undefined_category});

  CHECK(cs.category(undefined_category) == cs.column(0));
  CHECK(cs.category(0) == cs.column(1));
  CHECK(cs.category(1).domain == d_double);
  CHECK(cs.category(2) == cs.column(9));

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("ecoli categories strong")
{
  using namespace vita;

  std::istringstream ecoli(R"(
    sequence name, mcg,  gvh,  lip,  chg,  aac, alm1, alm2, localization
    AAT_ECOLI,    0.49, 0.29, 0.48, 0.50, 0.56, 0.24, 0.35, cp
    ACEA_ECOLI,   0.07, 0.40, 0.48, 0.50, 0.54, 0.35, 0.44, cp
    ACEK_ECOLI,   0.56, 0.40, 0.48, 0.50, 0.49, 0.37, 0.46, cp
    ACKA_ECOLI,   0.59, 0.49, 0.48, 0.50, 0.52, 0.45, 0.36, cp
    ADI_ECOLI,    0.23, 0.32, 0.48, 0.50, 0.55, 0.25, 0.35, cp
    ALKH_ECOLI,   0.67, 0.39, 0.48, 0.50, 0.36, 0.38, 0.46, cp
    AMPD_ECOLI,   0.29, 0.28, 0.48, 0.50, 0.44, 0.23, 0.34, cp
    AMY2_ECOLI,   0.21, 0.34, 0.48, 0.50, 0.51, 0.28, 0.39, cp
    APT_ECOLI,    0.20, 0.44, 0.48, 0.50, 0.46, 0.51, 0.57, cp
    ARAC_ECOLI,   0.42, 0.40, 0.48, 0.50, 0.56, 0.18, 0.30, cp)");

  dataframe d;
  dataframe::params p;
  p.output_index = std::nullopt;

  CHECK(d.read_csv(ecoli, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns, typing::strong);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{undefined_category, d_void, ""});
  CHECK(cs.column(1) == category_info{0, d_string,"sequence name"});
  CHECK(cs.column(2) == category_info{1, d_double,          "mcg"});
  CHECK(cs.column(3) == category_info{2, d_double,          "gvh"});
  CHECK(cs.column(4) == category_info{3, d_double,          "lip"});
  CHECK(cs.column(5) == category_info{4, d_double,          "chg"});
  CHECK(cs.column(6) == category_info{5, d_double,          "aac"});
  CHECK(cs.column(7) == category_info{6, d_double,         "alm1"});
  CHECK(cs.column(8) == category_info{7, d_double,         "alm2"});
  CHECK(cs.column(9) == category_info{8, d_string, "localization"});

  CHECK(cs.column(             "") == cs.column(0));
  CHECK(cs.column("sequence name") == cs.column(1));
  CHECK(cs.column(          "mcg") == cs.column(2));
  CHECK(cs.column(          "gvh") == cs.column(3));
  CHECK(cs.column(          "lip") == cs.column(4));
  CHECK(cs.column(          "chg") == cs.column(5));
  CHECK(cs.column(          "aac") == cs.column(6));
  CHECK(cs.column(         "alm1") == cs.column(7));
  CHECK(cs.column(         "alm2") == cs.column(8));
  CHECK(cs.column( "localization") == cs.column(9));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0,1,2,3,4,5,6,7,8,
                                                     undefined_category});

  CHECK(cs.category(undefined_category) == cs.column(0));
  CHECK(cs.category(0) == cs.column(1));
  CHECK(cs.category(1) == cs.column(2));
  CHECK(cs.category(2) == cs.column(3));
  CHECK(cs.category(3) == cs.column(4));
  CHECK(cs.category(4) == cs.column(5));
  CHECK(cs.category(5) == cs.column(6));
  CHECK(cs.category(6) == cs.column(7));
  CHECK(cs.category(7) == cs.column(8));
  CHECK(cs.category(8) == cs.column(9));

  CHECK(cs.category(123) == category_info::null);
}

TEST_CASE("load_csv classification")
{
  using namespace vita;

  std::istringstream iris(R"(
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

  dataframe d;
  dataframe::params p;
  p.output_index = 4;

  CHECK(d.read_csv(iris, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{0, d_double,        "class"});
  CHECK(cs.column(1) == category_info{0, d_double, "sepal length"});
  CHECK(cs.column(2) == category_info{0, d_double,  "sepal width"});
  CHECK(cs.column(3) == category_info{0, d_double, "petal length"});
  CHECK(cs.column(4) == category_info{0, d_double,  "petal width"});

  CHECK(cs.column(       "class") == cs.column(0));
  CHECK(cs.column("sepal length") == cs.column(1));
  CHECK(cs.column( "sepal width") == cs.column(2));
  CHECK(cs.column("petal length") == cs.column(3));
  CHECK(cs.column( "petal width") == cs.column(4));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0});

  CHECK(cs.category(0).domain == d_double);

  CHECK(cs.category(123) == category_info::null);

  for (const auto &ci: cs)
  {
    CHECK(ci.domain == d_double);
    CHECK(ci.category == 0);
  }
}

TEST_CASE("load_csv classification strong")
{
  using namespace vita;

  std::istringstream iris(R"(
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

  dataframe d;
  dataframe::params p;
  p.output_index = 4;

  CHECK(d.read_csv(iris, p) == 10);
  CHECK(d.is_valid());

  category_set cs(d.columns, typing::strong);
  CHECK(cs.is_valid());

  CHECK(cs.column(0) == category_info{0, d_double,        "class"});
  CHECK(cs.column(1) == category_info{1, d_double, "sepal length"});
  CHECK(cs.column(2) == category_info{2, d_double,  "sepal width"});
  CHECK(cs.column(3) == category_info{3, d_double, "petal length"});
  CHECK(cs.column(4) == category_info{4, d_double,  "petal width"});

  CHECK(cs.column(       "class") == cs.column(0));
  CHECK(cs.column("sepal length") == cs.column(1));
  CHECK(cs.column( "sepal width") == cs.column(2));
  CHECK(cs.column("petal length") == cs.column(3));
  CHECK(cs.column( "petal width") == cs.column(4));

  CHECK(cs.column("XXXXXXXXXXXXXXXXXXXX") == category_info::null);

  CHECK(cs.used_categories() == std::set<category_t>{0,1,2,3,4});

  CHECK(cs.category(0) == cs.column(0));
  CHECK(cs.category(1) == cs.column(1));
  CHECK(cs.category(2) == cs.column(2));
  CHECK(cs.category(3) == cs.column(3));
  CHECK(cs.category(4) == cs.column(4));

  CHECK(cs.category(123) == category_info::null);

  for (const auto &ci: cs)
    CHECK(ci.domain == d_double);
}

}  // TEST_SUITE("CATEGORY_SET")
