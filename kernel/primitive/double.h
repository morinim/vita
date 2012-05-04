/**
 *
 *  \file double.h
 *  \remark This file is part of VITA.
 *
 *  Copyright (C) 2011 EOS di Manlio Morini.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this file,
 *  You can obtain one at http://mozilla.org/MPL/2.0/
 *
 */

#if !defined(DOUBLE_PRIMITIVE_H)
#define      DOUBLE_PRIMITIVE_H

#include <boost/any.hpp>

#include <string>

#include "kernel/function.h"
#include "kernel/interpreter.h"
#include "kernel/random.h"
#include "kernel/terminal.h"

namespace vita
{
  /// We assume that errors during floating-point operations aren't terminal
  /// error. So we dont't try to prevent domain errors (e.g. square root of a
  /// negative number) or range error (e.g. pow(10.0, 1e6)) checking arguments
  /// beforehand (domain errors could be prevented by carefully bounds checking
  /// the arguments before calling functions and taking alternative action if
  /// the bounds are violated; range errors usually can not be prevented, as
  /// they are dependent on the implementation of floating-point numbers, as
  /// well as the function being applied).
  /// Instead we detect them and take alternative action (usually returning
  /// an empty boost::any()).
  namespace dbl
  {
    typedef double base_t;

    ///
    /// \param[in] v the value that must be casted to base type.
    ///
    /// Just a simple shortcut.
    inline
    base_t cast(const boost::any &v) { return boost::any_cast<base_t>(v); }

    ///
    /// It is assumed that the creation of floating-point constants is
    /// necessary to do symbolic regression in evolutionary computation.
    /// Genetic programming solves the problem of constant creation by using a
    /// special terminal named "ephemeral random constant" (Koza 1992). For
    /// each ephemeral random constant used in the initial population, a random
    /// number of a special data type in a specified range is generated. Then
    /// these random constants are moved around from tree to tree by the
    /// crossover operator.
    ///
    class number : public terminal
    {
    public:
      explicit number(category_t t, int m = -128, int u = 127)
        : terminal("REAL", t, false, true, default_weight*2), min(m), upp(u)
      { assert(m < u); }

      int init() const { return random::between<int>(min, upp); }

      std::string display(int v) const
      { return boost::lexical_cast<std::string>(v); }

      boost::any eval(interpreter *i) const
      { return static_cast<base_t>(boost::any_cast<int>(i->eval())); }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      {
        ar & boost::serialization::base_object<terminal>(*this);
        ar & min;
        ar & upp;
      }

    private: // Private data members.
      const int min, upp;
    };

    class abs : public function
    {
    public:
      explicit abs(category_t t) : function("FABS", t, {t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return std::fabs(dbl::cast(ev));
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class add : public function
    {
    public:
      explicit add(category_t t)
        : function("FADD", t, {t, t}, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) + dbl::cast(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class div : public function
    {
    public:
      explicit div(category_t t) : function("FDIV", t, {t, t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) / dbl::cast(ev1));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class idiv : public function
    {
    public:
      explicit idiv(category_t t) : function("FIDIV", t, {t, t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(std::floor(dbl::cast(ev0) / dbl::cast(ev1)));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class ife : public function
    {
    public:
      ife(category_t t1, category_t t2)
        : function("FIFE", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t cmp(std::fabs(dbl::cast(ev0) - dbl::cast(ev1)));

        if (cmp < float_epsilon)
          return i->eval(2);
        else
          return i->eval(3);
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class ifl : public function
    {
    public:
      ifl(category_t t1, category_t t2)
        : function("FIFL", t2, {t1, t1, t2, t2}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        if (dbl::cast(ev0) < dbl::cast(ev1))
          return i->eval(2);
        else
          return i->eval(3);
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class ifz : public function
    {
    public:
      explicit ifz(category_t t) : function("FIFZ", t, {t, t, t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        if (std::fabs(dbl::cast(ev0)) < float_epsilon)
          return i->eval(1);
        else
          return i->eval(2);
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class length : public function
    {
    public:
      explicit length(category_t t1, category_t t2)
        : function("FLENGTH", t2, {t1}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return static_cast<base_t>(boost::any_cast<std::string>(ev).size());
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class ln : public function
    {
    public:
      explicit ln(category_t t)
        : function("FLN", t, {t}, function::default_weight/2) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const base_t ret(std::log(dbl::cast(i->eval(0))));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class mod : public function
    {
    public:
      explicit mod(category_t t) : function("FMOD", t, {t, t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(std::fmod(dbl::cast(ev0), dbl::cast(ev1)));
        if (!std::isfinite(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class mul : public function
    {
    public:
      explicit mul(category_t t)
        : function("FMUL", t, {t, t}, function::default_weight, true) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) * dbl::cast(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class sin : public function
    {
    public:
      explicit sin(category_t t) : function("FSIN", t, {t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev(i->eval(0));
        if (ev.empty())  return ev;

        return std::sin(dbl::cast(ev));
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };

    class sub : public function
    {
    public:
      explicit sub(category_t t) : function("FSUB", t, {t, t}) {}

      boost::any eval(interpreter *i) const
      {
        const boost::any ev0(i->eval(0));
        if (ev0.empty())  return ev0;

        const boost::any ev1(i->eval(1));
        if (ev1.empty())  return ev1;

        const base_t ret(dbl::cast(ev0) - dbl::cast(ev1));
        if (isinf(ret))  return boost::any();

        return ret;
      }

    private: // Serialization.
      friend class boost::serialization::access;

      /// \see \c boost::serialization
      template<class Archive> void serialize(Archive &ar, unsigned)
      { ar & boost::serialization::base_object<function>(*this); }
    };
  }  // namespace dbl
}  // namespace vita

#endif  // DOUBLE_PRIMITIVE_H
