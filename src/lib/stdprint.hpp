#ifndef ACRIS_LIB_STDPRINT_HPP
#define ACRIS_LIB_STDPRINT_HPP

/*!
 * @file Utilities for printing standard library containers.
 * Every output function is in its own
 */

// STL Includes:
#include <array>
#include <concepts>
#include <format>
#include <list>
#include <map>
#include <memory>
#include <string_view>
#include <vector>

// Absolute Includes:
#include "lib/iomanip/iomanip.hpp"

// TODO: Relocate, to stdprint directory.
namespace lib::stdprint {
//! Internal helper utilities do not use directly.
namespace detail {
template<typename T>
concept Container = requires(T t) {
  typename T::value_type;
  typename T::iterator;
  typename T::const_iterator;
  typename T::size_type;
  { t.begin() } -> std::input_iterator;
  { t.end() } -> std::input_iterator;
  { t.size() } -> std::same_as<typename T::size_type>;
};

template<typename T>
concept SmartPointer = requires(T ptr) {
  typename T::element_type;
  { ptr.get() } -> std::convertible_to<typename T::element_type*>;
  { *ptr } -> std::same_as<typename T::element_type&>;
};

template<typename T>
  requires Container<T>
inline auto print_map(std::ostream& t_os, const T& t_container) -> std::ostream&
{
  using namespace std::literals;

  t_os << '{';
  auto sep{""sv};
  for(auto&& [key, val] : t_container) {
    t_os << sep << key << "=" << val;

    sep = ", "sv;
  }
  t_os << '}';

  return t_os;
}

//! Helper function which prints all sequential containers.
template<typename T>
  requires Container<T>
inline auto print_seq(std::ostream& t_os, const T& t_container) -> std::ostream&
{
  using namespace std::literals;

  t_os << '[';
  auto sep{""sv};
  for(auto&& elem : t_container) {
    t_os << sep << elem;

    sep = ", "sv;
  }
  t_os << ']';

  return t_os;
}

template<typename T>
  requires SmartPointer<T>
inline auto print_smart_ptr(std::ostream& t_os, const T& t_ptr) -> std::ostream&
{
  // TODO: For GCC and Clang demangle the PtrElementType and include it.
  // In the information.
  using PtrElementType [[maybe_unused]] = T::element_type;

  if(t_ptr) {
    t_os << *t_ptr;
  } else {
    t_os << "<*nil>";
  }

  return t_os;
}

template<typename T>
inline auto print_weak_ptr(std::ostream& t_os, const std::weak_ptr<T>& t_wk_ptr)
  -> std::ostream&
{
  // Guard clause.
  if(t_wk_ptr.expired()) {
    t_os << "<*expired>";

    return t_os;
  }

  const auto ptr{t_wk_ptr.lock()};
  if(ptr) {
    t_os << *ptr;
  } else {
    t_os << "<*nil>";
  }

  return t_os;
}

} // namespace detail

namespace map {
//! For printing vectors.
template<typename Key, typename Val>
inline auto operator<<(std::ostream& t_os, const std::map<Key, Val>& t_map)
  -> std::ostream&
{
  return detail::print_map(t_os, t_map);
}
} // namespace map

namespace vector {
//! For printing vectors.
template<typename T>
inline auto operator<<(std::ostream& t_os, const std::vector<T>& t_vector)
  -> std::ostream&
{
  return detail::print_seq(t_os, t_vector);
}
} // namespace vector

namespace array {
//
template<typename T, size_t S>
inline auto operator<<(std::ostream& t_os, const std::array<T, S>& t_array)
  -> std::ostream&
{
  return detail::print_seq(t_os, t_array);
}
} // namespace array

namespace list {
template<typename T>
inline auto operator<<(std::ostream& t_os, const std::list<T>& t_list)
  -> std::ostream&
{
  return detail::print_seq(t_os, t_list);
}
} // namespace list

namespace smart_ptr {
template<typename T>
inline auto operator<<(std::ostream& t_os, const std::unique_ptr<T>& t_ptr)
  -> std::ostream&
{
  return detail::print_smart_ptr(t_os, t_ptr);
}

template<typename T>
inline auto operator<<(std::ostream& t_os, const std::shared_ptr<T>& t_ptr)
  -> std::ostream&
{
  return detail::print_smart_ptr(t_os, t_ptr);
}

template<typename T>
inline auto operator<<(std::ostream& t_os, const std::weak_ptr<T>& t_ptr)
  -> std::ostream&
{
  return detail::print_smart_ptr(t_os, t_ptr);
}
} // namespace smart_ptr

namespace optional {
// TODO: Implement.
}

namespace variant {
// TODO: Implement.
}

//! Namespace that when included allows you to use all the ostream functions.
namespace all {
using namespace lib::stdprint::map;
using namespace lib::stdprint::vector;
using namespace lib::stdprint::list;
using namespace lib::stdprint::array;
using namespace lib::stdprint::smart_ptr;
using namespace lib::stdprint::optional;
using namespace lib::stdprint::variant;
} // namespace all
} // namespace lib::stdprint

#endif // ACRIS_LIB_STDPRINT_HPP
