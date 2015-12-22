#pragma once

#include <iostream>
#include <utility>
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace json {
template <template <class...> class T, template <class...> class U>
struct is_same_template : std::false_type {};

template <template <class...> class T>
struct is_same_template<T, T> : std::true_type {};

template <template <class...> class T>
struct is_array_template {
  static constexpr bool value =
    is_same_template<T, std::vector>::value ||
    is_same_template<T, std::list>::value   ||
    is_same_template<T, std::deque>::value  ||
    is_same_template<T, std::set>::value    ||
    is_same_template<T, std::unordered_set>::value;
};

template <template <class...> class T>
struct is_object_template {
  static constexpr bool value =
    is_same_template<T, std::map>::value ||
    is_same_template<T, std::unordered_map>::value;
};

template <template <class...> class T>
struct is_container_template {
  static constexpr bool value =
    is_array_template<T>::value || is_object_template<T>::value;
};

template <class E>
struct is_container {
  static constexpr bool value = false;
};

template <template <class...> class T, class... Args>
struct is_container<T<Args...>> : is_container_template<T> {};

class JsonConverter {
 private:
  struct CurlyBrackets {
    static constexpr char left  = '{';
    static constexpr char right = '}';
  };

  struct SquareBrackets {
    static constexpr char left  = '[';
    static constexpr char right = ']';
  };

  template <
   template <class...> class T,
   class U = typename std::conditional<
     is_object_template<T>::value,
     CurlyBrackets,
     SquareBrackets>::type>
  static inline auto outputBracket_(char c, std::ostream& os = std::cout)
    -> void const {
    assert(c == 'l' || c == 'r');
    if     (c == 'l') os << U::left;
    else if(c == 'r') os << U::right;
  }

  template <size_t width = 2>
  static inline auto outputIndent(unsigned int nest = 0,
                                  std::ostream& os = std::cout)
    -> void const {
    os << std::string(width * nest, ' ');
  }

  template <class E>
  static inline auto output_(const E& e,
                             std::ostream& os,
                             unsigned int nest,
                             bool indent,
                             bool cancel_indent = false)
    -> void const {
    os << e;
  }

  template <class E>
  static inline auto output_(const std::basic_string<E>& e,
                             std::ostream& os,
                             unsigned int nest,
                             bool indent,
                             bool cancel_indent = false)
    -> void const {
    os << '"' << e << '"';
  }

  template <class K, class V>
  static auto output_(const std::pair<K, V>& e,
                      std::ostream& os,
                      unsigned int nest,
                      bool indent,
                      bool cancel_indent = false)
    -> void const {
    // この assert はうまく機能しない
    assert(!(indent && is_container<K>::value));
    if(indent) outputIndent(nest, os);
    output_(e.first, os, nest, indent);
    os << ": ";
    output_(e.second, os, nest, indent, true);
  }

  template <template <class...> class T, class... Args>
  static auto output_(const T<Args...>& c,
                      std::ostream& os,
                      unsigned int nest,
                      bool indent,
                      bool cancel_indent = false)
    -> void const {
    constexpr bool is_nest_or_object =
      is_container<typename T<Args...>::value_type>::value ||
      is_object_template<T>::value;

    auto do_indent = [&](bool cond) {
      if(indent && cond) outputIndent(nest, os);
    };
    auto do_line_break = [&] {
      if(is_nest_or_object && indent) os << std::endl;
    };

    do_indent(!cancel_indent);
    outputBracket_<T>('l', os);
    do_line_break();

    auto it = c.cbegin();
    output_(*it, os, nest + 1, indent);
    for(it++; it != c.cend(); it++) {
      os << ", ";
      do_line_break();
      output_(*it, os, nest + 1, indent);
      do_line_break();
    }

    do_indent(is_nest_or_object);
    outputBracket_<T>('r', os);
  }

 public:
  template <template <class...> class T, class... E>
  static auto output(const T<E...>& c,
                     std::ostream& os = std::cout,
                     bool indent = false)
    -> void const {
    output_(c, os, 0, indent);
    os << std::endl;
  }

  template <template <class...> class T, class... E>
  static auto output(const T<E...>& c, bool indent)
    -> void const {
    output(c, std::cout, indent);
  }
};
}
