// Copyright David Abrahams 2002. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef OPERATORS2_DWA2002530_HPP
# define OPERATORS2_DWA2002530_HPP

# include <boost/python/detail/wrap_python.hpp>
# include <boost/python/converter/callback.hpp>
# include <boost/python/detail/operator_id.hpp>
# include <boost/python/detail/not_specified.hpp>
# include <boost/mpl/select_type.hpp>
# include <boost/python/self.hpp>
# include <boost/python/other.hpp>

namespace boost { namespace python { 

namespace detail
{
  // This is essentially the old v1 to_python(). It will be eliminated
  // once the public interface for to_python is settled on.
  template <class T>
  PyObject* convert_result(T const& x)
  {
      return converter::callback_to_python<T>(x).get_incref();
  }

  // Operator implementation template declarations. The nested apply
  // declaration here keeps MSVC6 happy.
  template <operator_id> struct operator_l
  {
      template <class L, class R> struct apply;
  };
  
  template <operator_id> struct operator_r
  {
      template <class L, class R> struct apply;
  };

  template <operator_id> struct operator_1
  {
      template <class T> struct apply;
  };

  // MSVC6 doesn't want us to do this sort of inheritance on a nested
  // class template, so we use this layer of indirection to avoid
  // ::template<...> on the nested apply functions below
  template <operator_id id, class L, class R>
  struct operator_l_inner
      : operator_l<id>::template apply<L,R>
  {};
      
  template <operator_id id, class L, class R>
  struct operator_r_inner
      : operator_r<id>::template apply<L,R>
  {};

  template <operator_id id, class T>
  struct operator_1_inner
      : operator_1<id>::template apply<T>
  {};
      
  // Define three different binary_op templates which take care of
  // these cases:
  //    self op self
  //    self op R
  //    L op self
  // 
  // The inner apply metafunction is used to adjust the operator to
  // the class type being defined. Inheritance of the outer class is
  // simply used to provide convenient access to the operation's
  // name().

  // self op self
  template <operator_id id>
  struct binary_op : operator_l<id>
  {
      template <class T>
      struct apply : operator_l_inner<id,T,T>
      {
      };
  };

  // self op R
  template <operator_id id, class R>
  struct binary_op_l : operator_l<id>
  {
      template <class T>
      struct apply : operator_l_inner<id,T,R>
      {
      };
  };

  // L op self
  template <operator_id id, class L>
  struct binary_op_r : operator_r<id>
  {
      template <class T>
      struct apply : operator_r_inner<id,L,T>
      {
      };
  };

  template <operator_id id>
  struct unary_op : operator_1<id>
  {
      template <class T>
      struct apply : operator_1_inner<id,T>
      {
      };
  };

  // This type is what actually gets returned from operators used on
  // self_t
  template <operator_id id, class L = not_specified, class R = not_specified>
  struct operator_
      : mpl::select_type<
             (is_same<L,self_t>::value)
            , typename mpl::select_type<
                 (is_same<R,self_t>::value)
                , binary_op<id>
                , binary_op_l<id,typename unwrap_other<R>::type>
              >::type
            , typename mpl::select_type<
                 (is_same<L,not_specified>::value)
                , unary_op<id>
                , binary_op_r<id,typename unwrap_other<L>::type>
                 >::type
          >::type
  {
  };
}

# define BOOST_PYTHON_BINARY_OPERATOR(id, rid, op)                      \
namespace detail                                                        \
{                                                                       \
  template <>                                                           \
  struct operator_l<op_##id>                                            \
  {                                                                     \
      template <class L, class R>                                       \
      struct apply                                                      \
      {                                                                 \
          static inline PyObject* execute(L const& l, R const& r)       \
          {                                                             \
              return detail::convert_result(l op r);                    \
          }                                                             \
      };                                                                \
      static char const* name() { return "__" #id "__"; }               \
  };                                                                    \
                                                                        \
  template <>                                                           \
  struct operator_r<op_##id>                                            \
  {                                                                     \
      template <class L, class R>                                       \
      struct apply                                                      \
      {                                                                 \
          static inline PyObject* execute(R const& r, L const& l)       \
          {                                                             \
              return detail::convert_result(l op r);                    \
          }                                                             \
      };                                                                \
      static char const* name() { return "__" #rid "__"; }              \
  };                                                                    \
}                                                                       \
namespace self_ns                                                       \
{                                                                       \
  template <class L, class R>                                           \
  inline detail::operator_<detail::op_##id,L,R>                         \
  operator##op(L const&, R const&)                                      \
  {                                                                     \
      return detail::operator_<detail::op_##id,L,R>();                  \
  }                                                                     \
}
  
BOOST_PYTHON_BINARY_OPERATOR(add, radd, +)
BOOST_PYTHON_BINARY_OPERATOR(sub, rsub, -)
BOOST_PYTHON_BINARY_OPERATOR(mul, rmul, *)
BOOST_PYTHON_BINARY_OPERATOR(div, rdiv, /)
BOOST_PYTHON_BINARY_OPERATOR(mod, rmod, %)
BOOST_PYTHON_BINARY_OPERATOR(lshift, rlshift, <<)
BOOST_PYTHON_BINARY_OPERATOR(rshift, rrshift, >>)
BOOST_PYTHON_BINARY_OPERATOR(and, rand, &)
BOOST_PYTHON_BINARY_OPERATOR(xor, rxor, ^)
BOOST_PYTHON_BINARY_OPERATOR(or, ror, |)
BOOST_PYTHON_BINARY_OPERATOR(gt, lt, >)
BOOST_PYTHON_BINARY_OPERATOR(ge, le, >=)
BOOST_PYTHON_BINARY_OPERATOR(lt, gt, <)
BOOST_PYTHON_BINARY_OPERATOR(le, ge, <=)
BOOST_PYTHON_BINARY_OPERATOR(eq, eq, ==)
BOOST_PYTHON_BINARY_OPERATOR(ne, ne, !=)

# define BOOST_PYTHON_INPLACE_OPERATOR(id, op)                  \
namespace detail                                                \
{                                                               \
  template <>                                                   \
  struct operator_l<op_##id>                                    \
  {                                                             \
      template <class L, class R>                               \
      struct apply                                              \
      {                                                         \
          static inline PyObject*                               \
          execute(back_reference<L&> l, R const& r)             \
          {                                                     \
              l.get() op r;                                     \
              return l.reference().release();                   \
          }                                                     \
      };                                                        \
      static char const* name() { return "__" #id "__"; }       \
  };                                                            \
}                                                               \
namespace self_ns                                               \
{                                                               \
  template <class R>                                            \
  inline detail::operator_<detail::op_##id,self_t,R>            \
  operator##op(self_t const&, R const&)                         \
  {                                                             \
      return detail::operator_<detail::op_##id,self_t,R>();     \
  }                                                             \
}

BOOST_PYTHON_INPLACE_OPERATOR(iadd,+=)
BOOST_PYTHON_INPLACE_OPERATOR(isub,-=)
BOOST_PYTHON_INPLACE_OPERATOR(imul,*=)
BOOST_PYTHON_INPLACE_OPERATOR(idiv,/=)
BOOST_PYTHON_INPLACE_OPERATOR(imod,%=)
BOOST_PYTHON_INPLACE_OPERATOR(ilshift,<<=)
BOOST_PYTHON_INPLACE_OPERATOR(irshift,>>=)
BOOST_PYTHON_INPLACE_OPERATOR(iand,&=)
BOOST_PYTHON_INPLACE_OPERATOR(ixor,^=)
BOOST_PYTHON_INPLACE_OPERATOR(ior,|=)
    
# define BOOST_PYTHON_UNARY_OPERATOR(id, op, func_name)         \
namespace detail                                                \
{                                                               \
  template <>                                                   \
  struct operator_1<op_##id>                                    \
  {                                                             \
      template <class T>                                        \
      struct apply                                              \
      {                                                         \
          static PyObject* execute(T const& x)                  \
          {                                                     \
              return detail::convert_result(op(x));             \
          }                                                     \
      };                                                        \
      static char const* name() { return "__" #id "__"; }       \
  };                                                            \
}                                                               \
namespace self_ns                                               \
{                                                               \
  inline detail::operator_<detail::op_##id>                     \
  func_name(self_t const&)                                      \
  {                                                             \
      return detail::operator_<detail::op_##id>();              \
  }                                                             \
}

BOOST_PYTHON_UNARY_OPERATOR(neg, -, operator-)
BOOST_PYTHON_UNARY_OPERATOR(pos, +, operator+)
BOOST_PYTHON_UNARY_OPERATOR(abs, abs, abs)
BOOST_PYTHON_UNARY_OPERATOR(invert, ~, operator~)
BOOST_PYTHON_UNARY_OPERATOR(int, long, int_)
BOOST_PYTHON_UNARY_OPERATOR(long, PyLong_FromLong, long_)
BOOST_PYTHON_UNARY_OPERATOR(float, double, float_)
    
}} // namespace boost::python

# ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
using boost::python::self_ns::abs;
using boost::python::self_ns::int_;
using boost::python::self_ns::long_;
using boost::python::self_ns::float_;
# endif

#endif // OPERATORS2_DWA2002530_HPP







