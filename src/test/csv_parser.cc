/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2020-2022 EOS di Manlio Morini.
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

const std::string s_abalone_nh(R"(
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

const std::string s_abalone_h(R"(
sex,length,diameter,height,whole weight,shucked weight,viscera weight,shell weight,rings
)" + s_abalone_nh);

const std::string s_iris_nh(R"(
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

const std::string s_iris_h(R"(
sepal length,sepal width,petal length,petal width,class
)" + s_iris_nh);

const std::string s_car_speed_nh(R"(
Blue,32,NewMexico
Red,45,Arizona
Blue,-9999,Colorado
White,34,Arizona
Red,25,Arizona
Blue,34,NewMexico
Black,,Colorado
White,31,Arizona
Red,26,Colorado
White,31,Arizona)");

const std::string s_car_speed_h(R"(
Color,Speed,State
)" + s_car_speed_nh);

const std::string s_colors_nh(R"(
Blue
Red
Blue
White
Red
Blue
Black
White
Red
White)");

const std::string s_colors_h(R"(
COLOR
)" + s_colors_nh);

const std::string s_addresses(R"(
John,Doe,120 jefferson st.,Riverside, NJ, 08075
Jack,McGinnis,220 hobo Av.,Phila, PA,09119
"John ""Da Man""",Repici,120 Jefferson St.,Riverside, NJ,08075
Stephen,Tyler,"7452 Terrace ""At the Plaza"" road",SomeTown,SD, 91234
,Blankman,,SomeTown, SD, 00298
"Joan ""the bone"", Anne",Jet,"9th, at Terrace plc",Desert City,CO,00123)");

const std::string s_air_travel(R"(
"Month", "1958", "1959", "1960"
"JAN",  340,  360,  417
"FEB",  318,  342,  391
"MAR",  362,  406,  419
"APR",  348,  396,  461
"MAY",  363,  420,  472
"JUN",  435,  472,  535
"JUL",  491,  548,  622
"AUG",  505,  559,  606
"SEP",  404,  463,  508
"OCT",  359,  407,  461
"NOV",  310,  362,  390
"DEC",  337,  405,  432)");

const std::string s_numbers_nh(R"(
123.11
234.22
345.33
456.44
567.55)");

const std::string s_numbers_h(R"(
VALUE
)" + s_numbers_nh);

TEST_SUITE("CSV_PARSER")
{

TEST_CASE("Reading")
{
  using namespace vita;

  // --------------------------------------------------------------
  std::istringstream abalone(s_abalone_h);
  csv_parser csv_abalone(abalone);
  csv_abalone.trim_ws(true);

  auto header_abalone(*csv_abalone.begin());
  CHECK(header_abalone.size() == 9);
  CHECK(header_abalone[0] == "sex");
  CHECK(header_abalone[1] == "length");
  CHECK(header_abalone[2] == "diameter");
  CHECK(header_abalone[3] == "height");
  CHECK(header_abalone[4] == "whole weight");
  CHECK(header_abalone[5] == "shucked weight");
  CHECK(header_abalone[6] == "viscera weight");
  CHECK(header_abalone[7] == "shell weight");
  CHECK(header_abalone[8] == "rings");

  auto row1_abalone(*std::next(csv_abalone.begin()));
  CHECK(row1_abalone.size() == 9);
  CHECK(row1_abalone[0] == "M");
  CHECK(row1_abalone[1] == "0.455");
  CHECK(row1_abalone[2] == "0.365");
  CHECK(row1_abalone[3] == "0.095");
  CHECK(row1_abalone[4] == "0.514");
  CHECK(row1_abalone[5] == "0.2245");
  CHECK(row1_abalone[6] == "0.101");
  CHECK(row1_abalone[7] == "0.15");
  CHECK(row1_abalone[8] == "15");

  // --------------------------------------------------------------
  std::istringstream iris(s_iris_h);
  csv_parser csv_iris(iris);
  csv_iris.trim_ws(true);

  auto header_iris(*csv_iris.begin());
  CHECK(header_iris.size() == 5);
  CHECK(header_iris[0] == "sepal length");
  CHECK(header_iris[1] == "sepal width");
  CHECK(header_iris[2] == "petal length");
  CHECK(header_iris[3] == "petal width");
  CHECK(header_iris[4] == "class");

  auto row1_iris(*std::next(csv_iris.begin()));
  CHECK(row1_iris.size() == 5);
  CHECK(row1_iris[0] == "5.1");
  CHECK(row1_iris[1] == "3.5");
  CHECK(row1_iris[2] == "1.4");
  CHECK(row1_iris[3] == "0.2");
  CHECK(row1_iris[4] == "Iris-setosa");

  // --------------------------------------------------------------
  std::istringstream car_speed(s_car_speed_h);
  csv_parser csv_car_speed(car_speed);

  auto header_car_speed(*csv_car_speed.begin());
  CHECK(header_car_speed.size() == 3);
  CHECK(header_car_speed[0] == "Color");
  CHECK(header_car_speed[1] == "Speed");
  CHECK(header_car_speed[2] == "State");

  auto row1_car_speed(*std::next(csv_car_speed.begin()));
  CHECK(row1_car_speed.size() == 3);
  CHECK(row1_car_speed[0] == "Blue");
  CHECK(row1_car_speed[1] == "32");
  CHECK(row1_car_speed[2] == "NewMexico");

  // --------------------------------------------------------------
  std::istringstream colors(s_colors_h);
  csv_parser csv_colors(colors);

  auto header_colors(csv_colors.begin());
  CHECK(header_colors->size() == 1);
  CHECK(header_colors->front() == "COLOR");

  auto row1_colors(std::next(header_colors));
  CHECK(row1_colors->size() == 1);
  CHECK(row1_colors->front() == "Blue");

  auto row2_colors(std::next(row1_colors));
  CHECK(row2_colors->size() == 1);
  CHECK(row2_colors->front() == "Red");

  // --------------------------------------------------------------
  std::istringstream addresses(s_addresses);
  csv_parser csv_addresses(addresses);

  auto row1_addresses(*csv_addresses.begin());
  CHECK(row1_addresses.size() == 6);
  CHECK(row1_addresses[0] == "John");
  CHECK(row1_addresses[1] == "Doe");
  CHECK(row1_addresses[2] == "120 jefferson st.");
  CHECK(row1_addresses[3] == "Riverside");
  CHECK(row1_addresses[4] == " NJ");
  CHECK(row1_addresses[5] == " 08075");

  auto row2_addresses(*std::next(csv_addresses.begin()));
  CHECK(row2_addresses.size() == 6);
  CHECK(row2_addresses[0] == "Jack");
  CHECK(row2_addresses[1] == "McGinnis");
  CHECK(row2_addresses[2] == "220 hobo Av.");
  CHECK(row2_addresses[3] == "Phila");
  CHECK(row2_addresses[4] == " PA");
  CHECK(row2_addresses[5] == "09119");

  // --------------------------------------------------------------
  std::istringstream air_travel(s_air_travel);
  csv_parser csv_air_travel(air_travel);
  csv_air_travel.trim_ws(true);

  auto header_air_travel(*csv_air_travel.begin());
  CHECK(header_air_travel.size() == 4);
  CHECK(header_air_travel[0] == "Month");
  CHECK(header_air_travel[1] == "1958");
  CHECK(header_air_travel[2] == "1959");
  CHECK(header_air_travel[3] == "1960");

  auto row1_air_travel(*std::next(csv_air_travel.begin()));
  CHECK(row1_air_travel.size() == 4);
  CHECK(row1_air_travel[0] == "JAN");
  CHECK(row1_air_travel[1] == "340");
  CHECK(row1_air_travel[2] == "360");
  CHECK(row1_air_travel[3] == "417");
}

TEST_CASE("Sniffer")
{
  using namespace vita;

  const std::vector delimiters = {',', ';', ':', '\t'};

  // A dataset of string / has_header pairs.
  const std::vector<std::pair<std::string, bool>> ts =
  {
    {s_abalone_h, true},
    {s_abalone_nh, false},
    {s_iris_h, true},
    {s_iris_nh, false},
    {s_car_speed_h, true},
    {s_car_speed_nh, false},
    {s_addresses, false},
    {s_air_travel, true}
  };

  const auto controlla([](const std::string &s, bool h, char d)
                       {
                         std::istringstream is(s);
                         csv_parser csvp(is);

                         CHECK(csvp.dialect().has_header == h);
                         CHECK(csvp.dialect().delimiter == d);
                       });

  for (const auto &t : ts)
    for (auto c : delimiters)
    {
      std::string s;
      if (c == ',')
        s = t.first;
      else
        std::replace_copy(t.first.begin(), t.first.end(),
                          std::back_inserter(s), ',', c);

      controlla(s, t.second, c);
    }

  controlla(s_colors_h, true, '\n');
  controlla(s_colors_nh, false, '\n');
  controlla(s_numbers_h, true, '\n');
  controlla(s_numbers_nh, false, '\n');
}

}  // TEST_SUITE("CSV_PARSER")
