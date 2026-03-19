#ifndef ARRAY_HPP
#define ARRAY_HPP

// Absolute Includes:
#include "stdacris/core/types.h"

namespace stdlibacris::internal {
// Compiler is hardcoded to search for this in a -nostdlib environment.
template<class T>
class InitList {
  private:
  const T* m_series;
  unsigned long m_size;

  public:
  // The compiler expects this specific private constructor
  // to link the internal array to the object.
  InitList(const T* t_series, usize_t t_size)
    : m_series{t_series}, m_size{t_size}
  {}

  public:
  InitList(): m_series{nullptr}, m_size{0}
  {}

  usize_t size() const
  {
    return m_size;
  }

  const T* begin() const
  {
    return m_series;
  }

  const T* end() const
  {
    return (m_series + m_size);
  }
};

/*!
 * Lazy helper struct for dealing with arrays.
 */
template<typename T, usize_t N>
struct Array {
  T m_data[N];
  static constexpr usize_t m_len = N;

  Array() = default;

  Array(InitList<T> t_list)
  {
    // Deal with InitList.
    auto* iter{t_list.begin()};
    if(iter == nullptr) {
      return;
    }

    usize_t index{0};
    for(; iter != t_list.end(); iter++) {
      m_data[index] = *iter;

      index++;
    }
  }

  inline constexpr auto operator[](const usize_t t_index) -> T&
  {
    return m_data[t_index];
  }

  // virtual ~Array() = default;
};
} // namespace stdlibacris::internal

#endif // ARRAY_HPP
