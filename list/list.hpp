#ifndef CAMP_LIST_LIST_HPP
#define CAMP_LIST_LIST_HPP

namespace camp
{
// TODO: document

template <typename... Ts>
struct list {
  using type = list;
  static constexpr size_t size = sizeof...(Ts);
};
}

#endif /* CAMP_LIST_LIST_HPP */
