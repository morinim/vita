/**
 *
 *  \file data.cc
 *
 *  Copyright 2011 EOS di Manlio Morini.
 *
 *  This file is part of VITA.
 *
 *  VITA is free software: you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation, either version 3 of the License, or (at your option) any later
 *  version.
 *
 *  VITA is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with VITA. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kernel/data.h"
#include "kernel/random.h"

namespace vita
{
  ///
  /// \param[in] n number of distinct datasets. E.g.:
  ///              (0) auto choose;
  ///              (1) not partitioned;
  ///              (2) one training set and one validation set;
  ///              (3) two training sets and one validation set...
  /// \see clear
  ///
  /// New empty data instance (partitioned in \a n training sets).
  ///
  data::data(unsigned n)
  {
    assert(n);

    clear(n);

    assert(check());
  }

  ///
  /// \param[in] filename nome of the file containing the learning collection.
  /// \param[in] n number of distinct datasets.
  ///
  /// New \a data instance containing the learning collection from \a filename
  /// and partitioned in \a n datasets.
  ///
  data::data(const std::string &filename, unsigned n)
  {
    assert(filename != "");
    assert(n);

    clear(n);
    open(filename);

    assert(check());
  }

  ///
  /// \param[in] n number of distinct datasets.
  ///
  /// Resets the object.
  ///
  void data::clear(unsigned n)
  {
    assert(n);

    labels_.clear();

    datasets_.clear();
    datasets_.resize(n ? n : 1);

    // This is the active data partition.
    active_ = 0;

    assert(check());
  }

  ///
  /// \return constant reference to the first element of the active dataset.
  ///
  data::const_iterator data::begin() const
  {
    assert(active_ < datasets_.size());
    return datasets_[active_].begin();
  }

  ///
  /// \return constant reference to the last+1 (sentry) element of the active
  ///         dataset.
  ///
  data::const_iterator data::end() const
  {
    assert(active_ < datasets_.size());
    return datasets_[active_].end();
  }

  ///
  /// \return the size of the active dataset.
  ///
  unsigned data::size() const
  {
    return datasets_[active_].size();
  }

  ///
  /// \return input vector dimension.
  ///
  unsigned data::variables() const
  {
    return datasets_.empty() || datasets_[0].empty()
      ? 0 : begin()->input.size();
  }

  ///
  /// \return number of classes of the classification problem (1 for a symbolic
  ///         regression problem).
  ///
  unsigned data::classes() const
  {
    return labels_.size();
  }

  ///
  /// \param[in] label name of a class of the learing collection.
  /// \return a positive integer used as primary key for the class \a label.
  ///
  unsigned data::encode(const std::string &label)
  {
    if (labels_.find(label) == labels_.end())
    {
      const unsigned n(labels_.size());
      labels_[label] = n;
    }

    return labels_[label];
  }

  ///
  /// \param[in] filename name of the file containing the learning collection.
  /// \return number of lines read (0 in case of error).
  ///
  unsigned data::open(const std::string &filename)
  {
    std::ifstream from(filename.c_str());
    if (!from)
      return 0;

    std::string line;

    // First line is special, it codes type and mean of each field.
    if (!std::getline(from, line))
      return 0;

    // format will contain the data file line format.
    // IN IN IN OL IN means: first three fields are input fields (I) and their
    // type is numeric (N). Fourth field is an output field and it is a
    // label (L). The last field is a numerical input.
    // If output field is numeric this is a symbolic regression problem,
    // otherwise it's a classification problem.
    // No more than one output field, please!
    std::vector<std::string> format;

    std::istringstream ifs(line);
    std::string f;
    while (ifs >> f)
      format.push_back(f);

    // We know the format, let's read the data.
    unsigned ln(1);
    for (; std::getline(from, line); ++ln)
    {
      std::istringstream ist(line);

      value_type v;

      for (unsigned field(0); field < format.size(); ++field)
        if (format[field][0] == 'I')
        {
          const char intype(std::toupper(format[field][1]));

          switch (intype)
          {
          case 'N':
            {
              double d;
              if (ist >> d)
                v.input.push_back(d);
              else
                return ln;
            }
            break;
          }
        }
        else  // output field
          if (format[field][1] == 'L')  // symbolic output field
          {
            std::string label;
            if (ist >> label)
              v.output = encode(label);
            else
              return 1;
          }
          else  // numerical output field
          {
            double output;
            if (ist >> output)
              v.output = output;
            else
              return 1;
          }

      const unsigned set(vita::random::between<unsigned>(0, datasets_.size()));
      datasets_[set].push_back(v);
    }

    check();
    return ln;
  }

  ///
  /// \return \c true if the learning collection is empty.
  ///
  bool data::operator!() const
  {
    bool found(false);

    for (auto i(datasets_.begin()); i != datasets_.end() && !found; ++i)
      found = !i->empty();

    return !found;
  }

  ///
  /// \return \c true if the object passes the internal consistency check.
  ///
  bool data::check() const
  {
    if (datasets_.empty())
      return false;

    const unsigned cl_size(classes());
    // If this is a classification problem, there should be at least two
    //  classes.
    if (cl_size == 1)
      return false;

    const unsigned in_size(datasets_[0].begin()->input.size());

    for (auto l(datasets_.begin()); l != datasets_.end(); ++l)
      for (auto i(l->begin()); i != l->end(); ++i)
      {
        if (i->input.size() != in_size)
          return false;

        if (cl_size && i->label() >= cl_size)
          return false;
      }

    return active_ < datasets_.size();
  }
}  // Namespace vita
