namespace mg::json {

template <typename Fn>
bool ArrayWriter::writeObject(Fn&& fn) {
    return writer_.writeObject(std::forward<Fn>(fn));
}

template <typename Fn>
bool ArrayWriter::writeArray(Fn&& fn) {
    return writer_.writeArray(std::forward<Fn>(fn));
}

} // namespace mg::json
