namespace bmhpal {
namespace algo {

// Remove a random 1/4 of the entries in the map
template <typename TMap>
void PruneCacheRandom(TMap& map) {
	typedef typename TMap::key_type KeyType;
	std::vector<KeyType>            keys;
	for (const auto& p : map)
		keys.push_back(p.first);
	std::random_shuffle(keys.begin(), keys.end());

	size_t nKeep = (keys.size() * 3) / 4;
	TMap   newMap;
	for (size_t i = 0; i < nKeep; i++)
		newMap.insert({keys[i], map.at(keys[i])});
	std::swap(map, newMap);
}

// Prunes the cache if it's size exceeds sizeLimit
// Returns true if a prune operation took place
template <typename TMap>
bool PruneCacheRandom(TMap& map, size_t sizeLimit) {
	if (map.size() > sizeLimit) {
		PruneCacheRandom(map);
		return true;
	}
	return false;
}

} // namespace algo
} // namespace bmhpal