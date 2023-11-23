#pragma once

#include <bitset>
#include <immintrin.h>
#include <iostream>
#include <optional>
#include <tuple>
#include <vector>
#include <cstring>

namespace ecs {
using entity_id = std::size_t;

template <typename... Components> class context {
public:
  entity_id new_entity() noexcept {
    entity_id new_id = _component_masks.size();
    _component_masks.push_back(_component_bitset());
    return new_id;
  }

  template <typename Component, typename... Args>
  bool add_component(entity_id entity,
                     Args &&...args) noexcept {
    if (entity >= _component_masks.size())
      return false;

    auto &component_vector =
        std::get<std::vector<Component>>(_components);
    if (component_vector.size() <= entity) {
      component_vector.resize(entity + 1);
    }
    
    component_vector[entity] =
        Component(std::forward<Args>(args)...);
    _component_masks[entity].set(
        _get_bitset_index<Component>());
    return true;
  }

  template <typename Component>
  bool remove_component(entity_id entity) noexcept {
    if (entity >= _component_masks.size())
      return false;

    auto &component_vector =
        std::get<std::vector<Component>>(_components);
    if (entity >= component_vector.size())
      return false;

    _component_masks[entity].reset(
        _get_bitset_index<Component>());

    return true;
  }

  template <typename Component>
  bool has_component(entity_id id) const noexcept {
    if (id >= _component_masks.size())
      return false;

    constexpr auto bitset_idx =
        _get_bitset_index<Component>();
    return _component_masks[id].test(bitset_idx);
  }

  template <typename... QueryComponents>
  bool has_components(entity_id entity) const noexcept {
    if (entity >= _component_masks.size())
      return false;
    constexpr auto query_bitset =
        _get_query_bitset<QueryComponents...>();
    return (_component_masks[entity] & query_bitset) ==
           query_bitset;
  }

  template <typename... QueryComponents>
  std::optional<std::tuple<QueryComponents &...>>
  try_get_components(entity_id id) noexcept {
    if (!has_components<QueryComponents...>(id))
      return std::nullopt;

    return std::tuple<QueryComponents &...>(
        _get_component_unchecked<QueryComponents>(id)...);
  }

  template <typename Component>
  std::optional<std::reference_wrapper<Component>>
  try_get_component(entity_id id) noexcept {
    if (id >= _component_masks.size())
      return std::nullopt;

    constexpr auto bitset_idx =
        _get_bitset_index<Component>();

    if (!_component_masks[id].test(bitset_idx))
      return std::nullopt;

    return _get_component_unchecked<Component>(id);
  }

  template <typename... QueryComponents>
  void for_each_entity(auto &&fn) noexcept {
    using fn_t = std::decay_t<decltype(fn)>;

    static_assert(
        std::is_invocable_v<fn_t, entity_id,
                            QueryComponents &...> ||
            std::is_invocable_v<fn_t, QueryComponents &...>,
        "fn must be invocable with either (entity_id, "
        "QueryComponents &...) or (QueryComponents &...) "
        "arguments");

    constexpr auto query_bitset =
        _get_query_bitset<QueryComponents...>();

    for (entity_id i = 0; i < _component_masks.size();
         ++i) {
      if ((_component_masks[i] & query_bitset) ==
          query_bitset) {
        if constexpr (std::is_invocable_v<
                          fn_t, entity_id,
                          QueryComponents &...>) {
          fn(i, _get_component_unchecked<QueryComponents>(
                    i)...);
        } else if constexpr (std::is_invocable_v<
                                 fn_t,
                                 QueryComponents &...>) {
          fn(_get_component_unchecked<QueryComponents>(
              i)...);
        }
      }
    }
  }

private:
  using _component_bitset =
      std::bitset<sizeof...(Components)>;

  std::vector<_component_bitset> _component_masks;

  std::tuple<std::vector<Components>...> _components;

  template <typename Component, std::size_t I = 0>
  static constexpr std::size_t _get_bitset_index() {
    using component_tuple = std::tuple<Components...>;

    if constexpr (I == sizeof...(Components)) {
      return I;
    } else if constexpr (std::is_same_v<
                             Component,
                             std::tuple_element_t<
                                 I, component_tuple>>) {
      return I;
    } else {
      return _get_bitset_index<Component, I + 1>();
    }
  }

  template <typename... QueryComponents>
  static constexpr _component_bitset _get_query_bitset() {
    return (... |
            (1ULL << _get_bitset_index<QueryComponents>()));
  }

  template <typename Component>
  Component &
  _get_component_unchecked(entity_id id) noexcept {
    return std::get<std::vector<Component>>(
        _components)[id];
  }
};
} // namespace ecs