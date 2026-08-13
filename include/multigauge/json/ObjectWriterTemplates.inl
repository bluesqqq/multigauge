namespace mg::json {

template <typename Fn>
bool ObjectWriter::writeValue(std::string_view key, Fn&& fn) {
    return writer_.key(key) && std::forward<Fn>(fn)(writer_);
}

template <typename Fn>
bool ObjectWriter::writeObject(std::string_view key, Fn&& fn) {
    return writer_.key(key) && writer_.writeObject(std::forward<Fn>(fn));
}

template <typename Fn>
bool ObjectWriter::writeArray(std::string_view key, Fn&& fn) {
    return writer_.key(key) && writer_.writeArray(std::forward<Fn>(fn));
}

} // namespace mg::json
