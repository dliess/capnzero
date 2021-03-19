/*
This is an c++17 implementation of C++20's std::span
and is derived from:
https://github.com/tcbrindle/span
https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/span
*/

#ifndef UTIL_SPAN_H
#define UTIL_SPAN_H

//******************************************************************************
// Header

#include <type_traits>
#include <stdint.h>
#include <limits>
#include <bits/stl_iterator.h>

#if defined(UTILS_SPAN_THROW_ON_VIOLATION)
   #include <error_msg.hpp> //buildErrorMessage
   #include <stdexcept>
#elif defined(UTILS_SPAN_TERMINATE_ON_VIOLATION)
   #include <exception>
#endif


#if defined (ET_SPAN_CHECK_VIOLATION)

   #define ET_SPAN_EXPECT(cond)  \
   do {                          \
      cond ? (void)0 : utils::condition_violation("Expected " #cond);\
   } while(0)

#else
   #define ET_SPAN_EXPECT(cond)  \
   do{ }while(0)
#endif



namespace utils
{

#if defined(UTILS_SPAN_THROW_ON_VIOLATION)
   inline void conditon_violation(const char* msg)
   {
      throw std::logic_error(utils::buildErrorMessage(__func__,msg));
   }
#elif defined(UTILS_SPAN_TERMINATE_ON_VIOLATION)
   [[noreturn]] inline void conditon_violation(const char* msg)
   {
      (void)msg;
      std::terminate();
   }
#endif


inline constexpr std::size_t dynamic_extent = SIZE_MAX;

namespace detail
{

// type trait to check if element are compatible
template <typename, typename,
          typename = void>
struct is_element_type_compatible : std::false_type{};


template <typename T, typename E>
struct is_element_type_compatible<T, E,
      typename std::enable_if_t<
         !std::is_same<
            typename std::remove_cv_t<decltype(std::data(std::declval<T>()))>,void>::value>>
         : std::is_convertible<
            std::remove_pointer_t<decltype(std::data(std::declval<T>()))> (*)[], E (*)[]> {};


//type trait to check if forward declaration
template <typename, typename = size_t>
struct is_complete : std::false_type {};

template <typename T>
struct is_complete<T, decltype(sizeof(T))> : std::true_type {};

template <typename _Tp>
  inline constexpr bool is_complete_v = is_complete<_Tp>::value;

//*****************************************************************************
//! \brief SSpanStorage
//!
template<typename T, std::size_t S>
struct SSpanStorage
{
   constexpr SSpanStorage() noexcept = default;
   constexpr SSpanStorage(T* ptr, std::size_t) noexcept :
      ptr(ptr)
   { }
   T* ptr = nullptr;
   constexpr static std::size_t size = S;
};

// template specialization of SSpanStorage
template<typename T>
struct SSpanStorage<T, dynamic_extent>
{
   constexpr SSpanStorage() noexcept = default;
   constexpr SSpanStorage(T* ptr, std::size_t size) noexcept :
      ptr(ptr),
      size(size)
   { }
   T* ptr = nullptr;
   std::size_t size = 0;
};

} // namespace detail


//*****************************************************************************
//! \brief SSpanStorage
//!
template<typename ElementType, std::size_t Extent = utils::dynamic_extent>
class span
{
   static_assert(std::is_object_v<ElementType>, "Span ElementType have to be an object no reference or pointer");
   static_assert(detail::is_complete_v<ElementType>, "No forward declared types are allowed");

public:
   using element_type = ElementType;
   using value_type = std::remove_cv_t<ElementType>;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using pointer = element_type*;
   using const_pointer = const element_type*;
   using reference = element_type&;
   using const_reference = const element_type&;
   using iterator =  __gnu_cxx::__normal_iterator<pointer, span>;
   using const_iterator = __gnu_cxx::__normal_iterator<const_pointer, span>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   static inline constexpr size_type extent = Extent;

   using storage_type = detail::SSpanStorage<ElementType, Extent>;

   constexpr span() noexcept = default;
   ~span() noexcept = default;

   constexpr span(const span&) noexcept = default;
   constexpr span(span&&) noexcept = default;

   span& operator=(const span&) noexcept = default;
   span& operator=(span&&) noexcept = default;

   constexpr span(pointer first_elem, size_type count) :
      m_storage(first_elem, count)
   {
      constexpr std::size_t declaredSize = static_cast<std::ptrdiff_t>(extent);
      std::size_t requiredSize = count;
      ET_SPAN_EXPECT(declaredSize < requiredSize);
   }

   constexpr span(pointer first_elem, pointer last_elem) :
      m_storage(first_elem, last_elem - first_elem)
   {
      constexpr std::size_t declaredSize = static_cast<std::ptrdiff_t>(extent);
      std::size_t requiredSize = (last_elem - first_elem) + 1;
      ET_SPAN_EXPECT(declaredSize < requiredSize);
   }

   template<size_t N = 1, std::size_t E = Extent,
      std::enable_if_t<std::is_same_v<std::remove_pointer_t<element_type>,ElementType>,int> = 0 >
   constexpr span(element_type &arr) noexcept :
      m_storage(&arr, N)
   { }

   template<size_t N, std::size_t E = Extent,
      std::enable_if_t<
         (E == dynamic_extent || N == E) &&
         detail::is_element_type_compatible<element_type(&)[N],ElementType>::value,int> = 0
      >
   constexpr span(element_type (&arr)[N]) noexcept :
      m_storage(arr, N)
   { }

   template<size_t N, std::size_t E = Extent,
      std::enable_if_t<
         (E == dynamic_extent || N == E) &&
         detail::is_element_type_compatible<std::array<value_type, N>&,ElementType>::value,int> = 0
      >
   constexpr span(std::array<value_type, N>& arr) noexcept :
      m_storage(arr.data(),N)
   { }

   template<size_t N, std::size_t E = Extent,
      std::enable_if_t<
         (E == dynamic_extent || N == E) &&
         detail::is_element_type_compatible<const std::array<value_type, N>&,ElementType>::value,int> = 0
      >
   constexpr span(const std::array<value_type, N>& arr) noexcept :
      m_storage(arr.data(),N)
   { }

   template<typename Container, std::size_t E = Extent,
      std::enable_if_t<
         (E == dynamic_extent) &&
         detail::is_element_type_compatible<Container&,ElementType>::value,int> = 0
      >
   constexpr span(Container& cont) noexcept :
      m_storage(std::data(cont), std::size(cont))
   { }

   template<typename Container, std::size_t E = Extent,
      std::enable_if_t<
         (E == dynamic_extent) &&
         detail::is_element_type_compatible<const Container&,ElementType>::value,int> = 0
      >
   constexpr span(const Container& cont) noexcept :
      m_storage(std::data(cont), std::size(cont))
   { }

   template <typename OtherElementType, std::size_t OtherExtent ,
      std::enable_if_t<
         ((OtherExtent == Extent) || (Extent == dynamic_extent)) &&
         std::is_convertible_v<std::decay_t<ElementType>, std::decay_t<OtherElementType>>, int> = 0
      >
   constexpr span(const span<OtherElementType, OtherExtent>& rhs) noexcept :
      m_storage(static_cast<pointer>(rhs.data()), rhs.size())
   { }

   // observers
   constexpr size_type size() const noexcept
   {
      return m_storage.size;
   }

   constexpr size_type size_bytes() const noexcept
   {
      return m_storage.size * sizeof(element_type);
   }

   constexpr bool empty() const noexcept
   {
      return size() == 0;
   }

   // element access
   constexpr reference front() const
   {
      static_assert(Extent != 0);
      ET_SPAN_EXPECT(!empty());
      return *m_storage.ptr;
   }

   constexpr reference back() const
   {
	   static_assert(Extent != 0);
	   ET_SPAN_EXPECT(!empty());
	   return *(m_storage.ptr + (size() - 1));
   }

   constexpr reference operator[](size_type _idx) const
   {
	   static_assert(Extent != 0);
      ET_SPAN_EXPECT(_idx < size());
	   return *(m_storage.ptr + _idx);
   }

   constexpr pointer data() const noexcept
   {
      return m_storage.ptr;
   }

   constexpr auto as_byte() const noexcept
   {
      using byteType = typename std::conditional_t<std::is_const_v<element_type>, std::add_const_t<uint8_t>, uint8_t>;
      return span<byteType>(reinterpret_cast<byteType *>(data()),size_bytes());
   }

   // iterator support
   constexpr iterator begin() const noexcept
   {
      return iterator(m_storage.ptr);
   }

   constexpr const_iterator cbegin() const noexcept
   {
      return const_iterator(m_storage.ptr);
   }

   constexpr iterator end() const noexcept
   {
      return iterator(m_storage.ptr + size());
   }

   constexpr const_iterator cend() const noexcept
   {
      return const_iterator(m_storage.ptr + size());
   }

   constexpr reverse_iterator rbegin() const noexcept
   {
      return reverse_iterator(end());
   }

   constexpr const_reverse_iterator crbegin() const noexcept
   {
      return const_reverse_iterator(cend());
   }

   constexpr reverse_iterator rend() const noexcept
   {
      return reverse_iterator(begin());
   }

   constexpr const_reverse_iterator crend() const noexcept
   {
      return const_reverse_iterator(this->cbegin());
   }

   private:
   storage_type m_storage {};
};

/* Deduction Guides */
template <class T, size_t N>
span(T (&)[N])->span<T, N>;

template <class T, size_t N>
span(std::array<T, N>&)->span<T, N>;

template <class T, size_t N>
span(const std::array<T, N>&)->span<const T, N>;

template <class Container>
span(Container&)->span<typename Container::value_type>;

template <class Container>
span(const Container&)->span<const typename Container::value_type>;

/* utils::make_span */
template <typename ElementType, std::size_t Extent>
constexpr span<ElementType, Extent> make_span(span<ElementType, Extent> s) noexcept
{
    return s;
}

template <typename T, std::size_t N = 1>
constexpr span<T, N> make_span(T &arr) noexcept
{
    return span<T, N> {arr};
}

template <typename T, std::size_t N>
constexpr span<T, N> make_span(T (&arr)[N]) noexcept
{
    return span<T, N> {arr};
}

template <typename T, std::size_t N>
constexpr span<T, N> make_span(std::array<T, N>& arr) noexcept
{
    return span<T, N> {arr};
}

template <typename T, std::size_t N>
constexpr span<const T, N>
make_span(const std::array<T, N>& arr) noexcept
{
    return span<T, N> {arr};
}

template <typename Container>
constexpr span<typename Container::value_type> make_span(Container& cont)
{
    return span<Container> {cont};
}

template <typename Container>
constexpr span<const typename Container::value_type>
make_span(const Container& cont)
{
    return span<Container> {cont};
}



} // utils

#endif