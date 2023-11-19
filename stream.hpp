#ifndef STREAM_HPP
#define STREAM_HPP

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

using usize = std::size_t;

template <typename D, typename B>
concept Derives = std::is_base_of_v<B, D>;

template <typename T>
concept Cloneable = std::is_copy_constructible_v<T>;

template <typename T>
concept Destructible = std::is_trivially_destructible_v<T>;

template <typename TCollection>
concept Iterable = requires(TCollection iterable) {
    { iterable.begin() } -> std::template same_as<decltype(std::begin(iterable))>;
    { iterable.end() } -> std::template same_as<decltype(std::end(iterable))>;
};

template <typename T>
concept PointerCloneable = requires(T obj) {
    { obj.clone() } -> std::same_as<T*>;
};

template <typename T>
concept Default = std::is_default_constructible_v<T>;

template <typename F, typename T>
concept Consumer = requires(F consumer, T value) {
    { consumer(value) } -> std::same_as<void>;
};

template <typename F, typename Key, typename Value>
concept KeyValueConsumer = requires(F consumer, Key key, Value value) {
    { consumer(key, value) } -> std::same_as<void>;
};

template <typename F, typename R>
concept Producer = requires(F producer) {
    { producer() } -> std::same_as<R>;
};

template <typename F, typename T>
concept Predicate = requires(F predicate, T value) {
    { predicate(value) } -> std::same_as<bool>;
};

template <typename F, typename K, typename V>
concept KeyValuePredicate = requires(F predicate, K key, V value) {
    { predicate(key, value) } -> std::same_as<bool>;
};

template <typename F, typename T, typename R>
concept Mapper = requires(F mapper, T t) {
    { mapper(t) } -> std::same_as<R>;
};

template <typename  F, typename T, typename R>
concept Reducer = requires(F reducer, T value, R acc) {
    { reducer(acc, value) } -> std::same_as<R>;
};

template <typename F, typename T1, typename T2>
concept Matcher = requires(F matcher, T1 t1, T2 t2) {
    { matcher(t1, t2) } -> std::same_as<bool>;
};

template <typename T>
concept Comparable = requires(T a, T b) {
    { a == b } -> std::same_as<bool>;
};

template <typename F, typename T>
concept Comparator = requires(F cmp, const T& a, const T& b) {
    { cmp(a, b) } -> std::same_as<bool>;
};

template <Iterable TCollection>
struct Collection
{
    using Value = typename TCollection::value_type;
    template <typename R>
    using WithValueType = std::vector<R>;

    static void insert(std::vector<Value>& collection, auto value) {
        collection.emplace_back(value);
    }
};

template <typename T>
struct Collection<std::unordered_set<T>>
{
    using Value = typename std::unordered_set<T>::value_type;
    template <typename R>
    using WithValueType = std::unordered_set<R>;

    static void insert(std::unordered_set<Value>& collection, auto value) {
        collection.insert(value);
    }
};

template <typename T>
struct Collection<std::set<T>>
{
    using Value = typename std::set<T>::value_type;
    template <typename R>
    using WithValueType = std::set<R>;

    static void insert(std::set<Value>& collection, auto value) {
        collection.insert(value);
    }
};

template <Iterable TCollection>
class Stream;

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    typename R, Mapper<typename TStream::Value, R> FMapper>
class Map;

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
class Filter;

template <Iterable TCollection>
struct Take;

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
struct TakeWhile;

template <Iterable TCollection>
struct Skip;

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
struct SkipWhile;

template <Iterable TCollection>
class Stream
{
  protected:

    using Iterator = typename TCollection::const_iterator;

    Iterator begin;
    Iterator end;

    Stream() = default;

  public:

    using Value = typename TCollection::value_type;

    explicit Stream(TCollection& collection) : begin(collection.begin()), end(collection.end()) {}

    template <typename R, Mapper<Value, R> FMapper>
    auto map(FMapper mapper) -> Map<TCollection, Stream, R, FMapper> {
        return Map<TCollection, Stream, R, FMapper>(mapper, begin, end);
    }

    template <Predicate<Value> FPredicate>
    auto filter(FPredicate predicate) -> Filter<TCollection, Stream, FPredicate> {
        return Filter<TCollection, Stream, FPredicate>(predicate, begin, end);
    }

    auto take(usize count) -> Take<TCollection> {
        return Take<TCollection>(count, begin, end);
    }

    template <Predicate<Value> FPredicate>
    auto takeWhile(FPredicate predicate) -> TakeWhile<TCollection, Stream, FPredicate> {
        return TakeWhile<TCollection, Stream, FPredicate>(predicate, begin, end);
    }

    auto skip(usize count) -> Skip<TCollection> {
        return Skip<TCollection>(count, begin, end);
    }

    template <Predicate<Value> FPredicate>
    auto skipWhile(FPredicate predicate) -> SkipWhile<TCollection, Stream, FPredicate> {
        return SkipWhile<TCollection, Stream, FPredicate>(predicate, begin, end);
    }

    template <Consumer<const Value&> FConsumer>
    void forEach(FConsumer consumer) {
        for (auto iter = begin; iter != end; ++iter) {
            consumer(*iter);
        }
    }

    template <KeyValueConsumer<usize, const Value&> FConsumer>
    void forEachIndexed(FConsumer consumer) {
        usize index = 0;
        for (auto iter = begin; iter != end; ++iter) {
            consumer(index++, *iter);
        }
    }

    template <Reducer<Value, Value> FReducer>
    Value reduce(FReducer reducer) {
        auto iter = begin;
        Value acc = *iter++;
        for (; iter != end; ++iter) {
            acc = reducer(acc, *iter);
        }
        return acc;
    }

    template <typename R, Reducer<Value, R> FReducer>
    R reduce(R init, FReducer reducer) {
        R result = init;
        for (auto iter = begin; iter != end; ++iter) {
            result = reducer(result, *iter);
        }
        return result;
    }

    template <Predicate<Value> FPredicate>
    bool any(FPredicate predicate) {
        for (auto iter = begin; iter != end; ++iter) {
            if (predicate(*iter)) { return true; }
        }
        return false;
    }

    template <Predicate<Value> FPredicate>
    bool all(FPredicate predicate) {
        for (auto iter = begin; iter != end; ++iter) {
            if (!predicate(*iter)) { return false; }
        }
        return true;
    }

    template <typename RCollection>
    RCollection collect() {
        RCollection result;
        for (auto iter = begin; iter != end; ++iter) {
            Collection<RCollection>::insert(result, *iter);
        }
        return result;
    }
};

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    typename R, Mapper<typename TStream::Value, R> FMapper>
class Map final : public Stream<typename Collection<TCollection>::template WithValueType<R>>
{
  private:

    using RCollection = typename Collection<TCollection>::template WithValueType<R>;

    RCollection mapped;

    using TIterator = typename TCollection::const_iterator;

    static RCollection map(FMapper mapper, const TIterator& begin, const TIterator& end) {
        RCollection mapped;
        for (auto iter = begin; iter != end; ++iter) {
            Collection<RCollection>::insert(mapped, mapper(*iter));
        }
        return mapped;
    }

  public:

    explicit Map(
        FMapper mapper, const TIterator& begin, const TIterator& end
    ) : Stream<RCollection>(), mapped(map(mapper, begin, end)) {
        this->begin = mapped.begin();
        this->end = mapped.end();
    }
};

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
class Filter final : public Stream<TCollection>
{
  private:

    TCollection filtered;

    static TCollection filter(
        FPredicate predicate, const typename Filter::Iterator& begin, const typename Filter::Iterator& end
    ) {
        TCollection filtered;
        for (auto iter = begin; iter != end; ++iter) {
            if (predicate(*iter)) { Collection<TCollection>::insert(filtered, *iter); }
        }
        return filtered;
    }

  public:

    explicit Filter(
        FPredicate predicate, const typename Filter::Iterator& begin, const typename Filter::Iterator& end
    ) : Stream<TCollection>(), filtered(filter(predicate, begin, end)) {
        this->begin = filtered.begin();
        this->end = filtered.end();
    }
};

template <Iterable TCollection>
struct Take final : Stream<TCollection>
{
    explicit Take(
        usize count, const typename Take::Iterator& begin, const typename Take::Iterator& end
    ) : Stream<TCollection>() {
        this->begin = begin;
        auto iter = begin;
        for (usize i = 0; i < count && iter != end; ++i) { ++iter; }
        this->end = iter;
    }
};

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
struct TakeWhile final : Stream<TCollection>
{
    explicit TakeWhile(
        FPredicate predicate, const typename TakeWhile::Iterator& begin, const typename TakeWhile::Iterator& end
    ) : Stream<TCollection>() {
        this->begin = begin;
        for (auto iter = begin; iter != end; ++iter) {
            if (!predicate(*iter)) {
                this->end = iter;
                return;
            }
        }
        this->end = end;
    }
};

template <Iterable TCollection>
struct Skip final : Stream<TCollection>
{
    explicit Skip(
        usize count, const typename Skip::Iterator& begin, const typename Skip::Iterator& end
    ) : Stream<TCollection>() {
        this->end = end;
        auto iter = begin;
        for (usize i = 0; i < count && iter != end; ++i) { ++iter; }
        this->begin = iter;
    }
};

template <
    Iterable TCollection, Derives<Stream<TCollection>> TStream,
    Predicate<typename TStream::Value> FPredicate>
struct SkipWhile final : Stream<TCollection>
{
    explicit SkipWhile(
        FPredicate predicate, const typename SkipWhile::Iterator& begin, const typename SkipWhile::Iterator& end
    ) : Stream<TCollection>() {
        this->end = end;
        for (auto iter = begin; iter != end; ++iter) {
            if (!predicate(*iter)) {
                this->begin = iter;
                return;
            }
        }
        this->begin = end;
    }
};

#endif // STREAM_HPP
