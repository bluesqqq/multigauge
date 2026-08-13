namespace mg::json {

template <typename Fn>
bool Writer::writeObject(Fn&& fn) {
    if (!beginObject()) return false;
    ObjectWriter object(*this);
    const bool result = std::forward<Fn>(fn)(object);
    return endObject() && result;
}

template <typename Fn>
bool Writer::writeArray(Fn&& fn) {
    if (!beginArray()) return false;
    ArrayWriter array(*this);
    const bool result = std::forward<Fn>(fn)(array);
    return endArray() && result;
}

} // namespace mg::json
