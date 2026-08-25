# ifndef OPTIONAL_STD_HPP
# define OPTIONAL_STD_HPP

# include <optional>

namespace datastax { namespace internal {

template <typename T>
using CassOptional = std::optional<T>;
constexpr auto CassNullopt = std::nullopt;

}} // namespace datastax::internal

# endif /* OPTIONAL_STD_HPP */
