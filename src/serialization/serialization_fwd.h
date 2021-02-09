#pragma once

#include <functional>
#include <type_traits>

namespace serialization
{
template<typename T>
struct TypeId;

template<typename TContext>
class Serializer;

template<typename TContext>
using LazyCallback = std::function<void(const Serializer<TContext>&)>;

#define S_NV(name, obj) name, obj

template<typename TContext>
inline void serialize(std::string& data, const Serializer<TContext>& ser);
template<typename T, typename TContext>
inline std::enable_if_t<std::is_arithmetic_v<T>, void> serialize(T& data, const Serializer<TContext>& ser);
template<typename T, typename TContext>
inline std::enable_if_t<std::is_enum_v<T>, void> serialize(T& data, const Serializer<TContext>& ser);
template<typename TContext>
inline void serialize(bool& data, const Serializer<TContext>& ser);
template<typename T, typename TContext>
inline std::enable_if_t<std::is_arithmetic_v<T>, T> create(const TypeId<T>& tid, const Serializer<TContext>& ser);
template<typename T, typename TContext>
inline std::enable_if_t<std::is_enum_v<T>, T> create(const TypeId<T>&, const Serializer<TContext>& ser);
template<typename TContext>
inline bool create(const TypeId<bool>& tid, const Serializer<TContext>& ser);
template<typename TContext>
inline std::string create(const TypeId<std::string>& tid, const Serializer<TContext>& ser);

namespace access
{
template<typename T, typename TContext>
inline auto callCreate(const TypeId<T>&, const Serializer<TContext>& ser) -> decltype(T::create(ser));

template<typename T, typename TContext>
inline auto callCreate(const TypeId<T>& tid, const Serializer<TContext>& ser) -> decltype(create(tid, ser));

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(data.load(ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrLoad(T& data, const Serializer<TContext>& ser) -> decltype(load(data, ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.serialize(ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(serialize(data, ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(data.save(ser), void());

template<typename T, typename TContext>
inline auto callSerializeOrSave(T& data, const Serializer<TContext>& ser) -> decltype(save(data, ser), void());

template<typename T, typename TContext>
inline void callSerialize(T& data, const Serializer<TContext>&);
} // namespace access
} // namespace serialization
