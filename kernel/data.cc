/**
 *
 *  \file data.cc
 *
 *  \author Manlio Morini
 *  \date 2010/12/29
 *
 *  This file is part of VITA
 *
 */

#include "data.h"
#include "random.h"

namespace vita
{

  ///
  /// \param n[in] number of distinct training sets.
  /// \see clear
  ///
  /// New empty data instance (partitioned in \a n training sets).
  ///
  data::data(unsigned n)
  {
    clear(n);
    check();
  }

  ///
  /// \param filename[in] nome of the file containing the learning collection.
  /// \param n[in] number of distinct training sets.
  ///
  /// New data instance containing the learning collection from \a filename and
  /// partitioned in \a n training sets.
  /// 
  data::data(const std::string &filename, unsigned n)
  {
    clear(n);
    open(filename);
    check();
  }

  ///
  /// \param n[in] number of distinct training sets (if 1 the learning 
  ///              collection won't be partitioned).
  ///
  void
  data::clear(unsigned n)
  {
    _labels.clear();

    _training.clear(); 
    _training.resize(n ? n : 1);

    // This is the active data partition.
    _active = 0;
  }

  ///
  /// \return input vector dimension.
  ///
  unsigned
  data::variables() const
  {
    return _training.empty() || _training[0].empty() 
      ? 0 : _training[0].begin()->input.size(); 
  }

  ///
  /// \return number of classes of the classification problem (1 for a symbolic
  ///         regression problem).
  ///
  unsigned
  data::classes() const
  {
    return _labels.size();
  }
 
  ///
  /// \param label[in]
  ///
  unsigned
  data::encode(const std::string &label)
  {
    if (_labels.find(label) == _labels.end())
    {
      const unsigned n(_labels.size());
      _labels[label] = n;
    }

    return _labels[label];
  }

  ///
  /// \param filename[in]
  /// \return number of lines read (0 in case of error).
  ///
  unsigned
  data::open(const std::string &filename)
  {
    std::ifstream from(filename.c_str());
    if (!from)
      return 0;

    std::string line;
    
    // First line is special, it codes type and mean of each field.
    if (!std::getline(from,line))
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
    for (; std::getline(from,line); ++ln)
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
	else // output field
	  if (format[field][1] == 'L')  // symbolic output field
	  {
	    std::string label;
	    if (ist >> label)
	    {
	      v.label  = encode(label);
	      v.output = 0.0;
	    }
	    else
	      return 1;
	  }
	  else  // numerical output field
          {
	    double output;
            if (ist >> output)
            {
              v.output = output;
	      v.label = 0;
            }
	    else
	      return 1;
          }

      const unsigned set(vita::random::between<unsigned>(0,_training.size()));
      _training[set].push_back(v);
    }

    check();
    return ln;
  }

  ///
  /// \return true if the learning collection is empty.
  ///
  bool
  data::operator!() const
  {
    bool found(false);

    for (std::vector<std::list<value_type> >::const_iterator 
           i(_training.begin());
         i != _training.end() && !found;
         ++i)
      found = !i->empty();
    
    return !found;
  }

  ///
  /// \return true if the individual passes the internal consistency check.
  ///
  bool
  data::check() const
  {
    if (_training.empty())
      return false;

    const unsigned cl_size(classes());
    // If this is a classification problem, there should be at least two
    //  classes.
    if (cl_size==1)
      return false;

    const unsigned in_size(_training[0].begin()->input.size());

    for (std::vector<std::list<value_type> >::const_iterator
           l(_training.begin());
	 l != _training.end();
	 ++l)
      for (const_iterator i(l->begin()); i != l->end(); ++i)
      {
	if (i->input.size() != in_size)
	  return false;

	if (cl_size && i->label >= cl_size)
	  return false;
      }

    return _active < _training.size();
  }

}  // Namespace vita
