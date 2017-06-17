/**
 *  \file
 *  \remark This file is part of VITA.
 *
 *  \copyright Copyright (C) 2017 EOS di Manlio Morini.
 *
 *  \license
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 */

#include "kernel/individual.h"

namespace vita
{

// **********************
// * PRINTING SUBSYSTEM *
// **********************
namespace out
{
const long print_format_flag = std::ios_base::xalloc();
const long long_form_flag = std::ios_base::xalloc();

///
/// Used to print the complete content of an individual.
///
/// \note Mostly used during debugging.
/// \remark Sticky manipulator.
///
std::ostream &dump(std::ostream &o)
{
  o.iword(print_format_flag) = dump_f;
  return o;
}

///
/// Used to print a graph, in dot language, representing the individual.
///
/// \see http://www.graphviz.org/
/// \remark Sticky manipulator.
///
std::ostream &graphviz(std::ostream &o)
{
  o.iword(print_format_flag) = graphviz_f;
  return o;
}

///
/// Used to print the individual on a single line.
///
/// Not at all human readable, but a compact representation for import/export.
///
/// \remark Sticky manipulator.
///
std::ostream &in_line(std::ostream &o)
{
  o.iword(print_format_flag) = in_line_f;
  return o;
}

///
/// Used to print a human readable representation of the individual.
///
/// Do you remember C=64's `LIST`? :-)
///
///     10 PRINT "HOME"
///     20 PRINT "SWEET"
///     30 GOTO 10
///
/// \remark Sticky manipulator.
///
std::ostream &list(std::ostream &o)
{
  o.iword(print_format_flag) = list_f;
  return o;
}

///
/// Used to print the individual as a tree structure.
///
/// \remark Sticky manipulator.
///
std::ostream &tree(std::ostream &o)
{
  o.iword(print_format_flag) = tree_f;
  return o;
}

///
/// Enable printing of every detail of the individual.
///
/// \remark Sticky manipulator.
///
std::ostream &long_form(std::ostream &o)
{
  o.iword(long_form_flag) = true;
  return o;
}

///
/// Hide secondary details of the individual.
///
/// \remark Sticky manipulator.
///
std::ostream &short_form(std::ostream &o)
{
  o.iword(long_form_flag) = false;
  return o;
}

}  // namespace out

}  // namespace vita
