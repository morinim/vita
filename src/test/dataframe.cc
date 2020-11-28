/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017-2020 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include <sstream>

#include "kernel/random.h"
#include "kernel/gp/src/dataframe.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "third_party/doctest/doctest.h"

std::istringstream iris_xrff(R"(
<dataset name="iris">
  <header>
    <attributes>
      <attribute class="yes" name="class" type="nominal">
        <labels>
          <label>Iris-setosa</label>
          <label>Iris-versicolor</label>
          <label>Iris-virginica</label>
        </labels>
      </attribute>
      <attribute name="sepallength" type="numeric" />
      <attribute name="sepalwidth" type="numeric" />
      <attribute name="petallength" type="numeric" />
      <attribute name="petalwidth" type="numeric" />
    </attributes>
  </header>
  <body>
    <instances>
      <instance><value>Iris-setosa</value><value>5.1</value><value>3.5</value><value>1.4</value><value>0.2</value></instance>
      <instance><value>Iris-setosa</value><value>4.9</value><value>3</value><value>1.4</value><value>0.2</value></instance>
      <instance><value>Iris-setosa</value><value>4.7</value><value>3.2</value><value>1.3</value><value>0.2</value></instance>
      <instance><value>Iris-versicolor</value><value>7</value><value>3.2</value><value>4.7</value><value>1.4</value></instance>
      <instance><value>Iris-versicolor</value><value>6.4</value><value>3.2</value><value>4.5</value><value>1.5</value></instance>
      <instance><value>Iris-versicolor</value><value>6.9</value><value>3.1</value><value>4.9</value><value>1.5</value></instance>
      <instance><value>Iris-virginica</value><value>6.3</value><value>3.3</value><value>6</value><value>2.5</value></instance>
      <instance><value>Iris-virginica</value><value>5.8</value><value>2.7</value><value>5.1</value><value>1.9</value></instance>
      <instance><value>Iris-virginica</value><value>7.1</value><value>3</value><value>5.9</value><value>2.1</value></instance>
      <instance><value>Iris-virginica</value><value>6.3</value><value>2.9</value><value>5.6</value><value>1.8</value></instance>
    </instances>
  </body>
</dataset>)");

TEST_SUITE("DATAFRAME")
{

TEST_CASE("load_csv filter")
{
  using namespace vita;

  dataframe d1, d2, d3;
  constexpr std::size_t n1(10), n2(150), n3(351);

  CHECK(d1.read("./test_resources/mep.csv") == n1);
  CHECK(d1.size() == n1);
  CHECK(d2.read("./test_resources/iris.csv") == n2);
  CHECK(d2.size() == n2);
  CHECK(d3.read("./test_resources/ionosphere.csv") == n3);
  CHECK(d3.size() == n3);

  dataframe::params p;
  p.filter = [](dataframe::record_t &)
  {
    return random::boolean();
  };

  std::size_t n(0), sup(10);
  for (unsigned i(0); i < sup; ++i)
  {
    dataframe d3f;

    d3f.read("./test_resources/ionosphere.csv", p);
    n += d3f.size();
  }

  const auto half(n3 * sup / 2);
  CHECK(9 * half <= 10 * n);
  CHECK(10 * n <= 11 * half);
}

TEST_CASE("load_csv headers")
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

constexpr std::size_t ncol(12);

  dataframe d;
  dataframe::params p;

  CHECK(d.columns.size() == 0);
  CHECK(d.columns.empty());

  CHECK(d.read_csv(wine, p) == 10);
  CHECK(d.is_valid());

  CHECK(d.columns.size() == ncol);
  CHECK(!d.columns.empty());

  CHECK(d.columns[ 0].name ==        "fixed acidity");
  CHECK(d.columns[ 1].name ==     "volatile acidity");
  CHECK(d.columns[ 2].name ==          "citric acid");
  CHECK(d.columns[ 3].name ==       "residual sugar");
  CHECK(d.columns[ 4].name ==            "chlorides");
  CHECK(d.columns[ 5].name ==  "free sulfur dioxide");
  CHECK(d.columns[ 6].name == "total sulfur dioxide");
  CHECK(d.columns[ 7].name ==              "density");
  CHECK(d.columns[ 8].name ==                   "pH");
  CHECK(d.columns[ 9].name ==            "sulphates");
  CHECK(d.columns[10].name ==              "alcohol");
  CHECK(d.columns[11].name ==              "quality");

  CHECK(d.columns.begin()->name == d.columns[       0].name);
  CHECK(d.columns.begin()->name ==   d.columns.front().name);
  CHECK(d.columns.back().name   == d.columns[ncol - 1].name);

  std::size_t count(0);
  for (const auto &c: d.columns)
  {
    ++count;
    CHECK(c.domain == d_double);
  }
  CHECK(count == ncol);

  CHECK(d.classes() == 0);
  CHECK(d.front().input.size() == ncol - 1);

  for (const auto &e : d)
  {
    CHECK(std::holds_alternative<D_DOUBLE>(e.output));

    for (const auto &i : e.input)
      CHECK(std::holds_alternative<D_DOUBLE>(i));
  }
}

TEST_CASE("load_csv output_index")
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

  constexpr std::size_t ncol(9);

  dataframe d;
  dataframe::params p;
  p.output_index = 8;

  CHECK(d.columns.size() == 0);
  CHECK(d.columns.empty());

  CHECK(d.read_csv(abalone, p) == 10);
  CHECK(d.is_valid());

  CHECK(d.columns.size() == ncol);
  CHECK(!d.columns.empty());

  CHECK(d.columns[ 0].name ==          "rings");
  CHECK(d.columns[ 1].name ==            "sex");
  CHECK(d.columns[ 2].name ==         "length");
  CHECK(d.columns[ 3].name ==       "diameter");
  CHECK(d.columns[ 4].name ==         "height");
  CHECK(d.columns[ 5].name ==   "whole weight");
  CHECK(d.columns[ 6].name == "shucked weight");
  CHECK(d.columns[ 7].name == "viscera weight");
  CHECK(d.columns[ 8].name ==   "shell weight");

  CHECK(d.columns.begin()->name == d.columns[       0].name);
  CHECK(d.columns.begin()->name ==   d.columns.front().name);
  CHECK(d.columns.back().name   == d.columns[ncol - 1].name);

  CHECK(d.columns[0].domain == d_double);
  CHECK(d.columns[1].domain == d_string);

  CHECK(d.classes() == 0);
  CHECK(d.front().input.size() == ncol - 1);

  CHECK(std::holds_alternative<D_DOUBLE>(d.front().output));
  CHECK(std::holds_alternative<D_STRING>(d.front().input[0]));
  CHECK(std::holds_alternative<D_DOUBLE>(d.front().input[1]));
}

TEST_CASE("load_csv_no_output_index")
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

  constexpr std::size_t ncol(9);

  dataframe d;
  dataframe::params p;
  p.output_index = std::nullopt;

  CHECK(d.columns.size() == 0);
  CHECK(d.columns.empty());

  CHECK(d.read_csv(ecoli, p) == 10);
  CHECK(d.is_valid());

  CHECK(d.columns.size() == ncol + 1);
  CHECK(!d.columns.empty());

  CHECK(d.columns[ 0].name ==              "");
  CHECK(d.columns[ 1].name == "sequence name");
  CHECK(d.columns[ 2].name ==           "mcg");
  CHECK(d.columns[ 3].name ==           "gvh");
  CHECK(d.columns[ 4].name ==           "lip");
  CHECK(d.columns[ 5].name ==           "chg");
  CHECK(d.columns[ 6].name ==           "aac");
  CHECK(d.columns[ 7].name ==          "alm1");
  CHECK(d.columns[ 8].name ==          "alm2");
  CHECK(d.columns[ 9].name ==  "localization");

  CHECK(d.columns.begin()->name == d.columns[       0].name);
  CHECK(d.columns.begin()->name ==   d.columns.front().name);
  CHECK(d.columns.back().name   ==     d.columns[ncol].name);

  CHECK(d.columns[1].domain == d_string);
  CHECK(d.columns[2].domain == d_double);
  CHECK(d.columns[3].domain == d_double);
  CHECK(d.columns[4].domain == d_double);
  CHECK(d.columns[5].domain == d_double);
  CHECK(d.columns[6].domain == d_double);
  CHECK(d.columns[7].domain == d_double);
  CHECK(d.columns[8].domain == d_double);
  CHECK(d.columns[9].domain == d_string);

  CHECK(d.classes() == 0);

  for (const auto &e : d)
  {
    CHECK(e.input.size() == ncol);
    CHECK(!has_value(e.output));
  }
}

TEST_CASE("load_csv_classification")
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

constexpr std::size_t ncol(5);

  dataframe d;
  dataframe::params p;
  p.output_index = 4;

  CHECK(d.columns.size() == 0);
  CHECK(d.columns.empty());

  CHECK(d.read_csv(iris, p) == 10);
  CHECK(d.is_valid());

  CHECK(d.columns.size() == ncol);
  CHECK(!d.columns.empty());

  CHECK(d.columns[ 0].name ==        "class");
  CHECK(d.columns[ 1].name == "sepal length");
  CHECK(d.columns[ 2].name ==  "sepal width");
  CHECK(d.columns[ 3].name == "petal length");
  CHECK(d.columns[ 4].name ==  "petal width");

  CHECK(d.columns.begin()->name == d.columns[       0].name);
  CHECK(d.columns.begin()->name ==   d.columns.front().name);
  CHECK(d.columns.back().name   == d.columns[ncol - 1].name);

  std::size_t count(0);
  for (const auto &c: d.columns)
  {
    ++count;
    CHECK(c.domain == d_double);
  }
  CHECK(count == ncol);

  CHECK(d.classes() == 3);
  CHECK(d.front().input.size() == ncol - 1);

  CHECK(d.class_name(0) == "Iris-setosa");
  CHECK(d.class_name(1) == "Iris-versicolor");
  CHECK(d.class_name(2) == "Iris-virginica");
}

TEST_CASE("load_xrff_classification")
{
  using namespace vita;

  constexpr std::size_t ncol(5);

  dataframe d;

  CHECK(d.columns.size() == 0);
  CHECK(d.columns.empty());

  CHECK(d.read_xrff(iris_xrff) == 10);
  CHECK(d.is_valid());

  CHECK(d.columns.size() == ncol);
  CHECK(!d.columns.empty());

  CHECK(d.columns[0].name ==       "class");
  CHECK(d.columns[1].name == "sepallength");
  CHECK(d.columns[2].name ==  "sepalwidth");
  CHECK(d.columns[3].name == "petallength");
  CHECK(d.columns[4].name ==  "petalwidth");

  CHECK(d.columns.begin()->name == d.columns[       0].name);
  CHECK(d.columns.begin()->name ==   d.columns.front().name);
  CHECK(d.columns.back().name   == d.columns[ncol - 1].name);

  std::size_t count(0);
  for (const auto &c: d.columns)
  {
    ++count;
    CHECK(c.domain == d_double);
  }
  CHECK(count == ncol);

  CHECK(d.classes() == 3);
  CHECK(d.front().input.size() == ncol - 1);

  CHECK(d.class_name(0) == "Iris-setosa");
  CHECK(d.class_name(1) == "Iris-versicolor");
  CHECK(d.class_name(2) == "Iris-virginica");
}

}  // TEST_SUITE("DATAFRAME")
