#pragma once
#include "Includes.hpp"
#include <fstream>
#include <type_traits>

#if 0
template <typename KeyType, typename ValueType>
class MapSerializer {
public:
	static auto Serialize(const std::map<KeyType, ValueType> &map, const fs::path &file) -> void {
		std::ofstream ofs(file, std::ios::binary);
		if (!ofs) {
			throw std::runtime_error("Kann die Datei nicht öffnen " + file.string());
		}

		size_t size = map.size();
		ofs.write(reinterpret_cast<const char *>(&size), sizeof(size));
		for (const auto &entry : map) {
			SerializeValue(ofs, entry.first);
			SerializeValue(ofs, entry.second);
		}
		ofs.close();
	}

	static auto Deserialize(const fs::path &file) -> std::map<KeyType, ValueType> {
		std::ifstream ifs(file, std::ios::binary);
		if (!ifs) {
			throw std::runtime_error("Kann die Datei nicht öffnen " + file.string());
		}

		size_t size = 0;
		if (!ifs.read(reinterpret_cast<char *>(&size), sizeof(size))) {
			throw std::runtime_error("Fehler beim Lesen der Map-Größe");
		}

		std::map<KeyType, ValueType> map;

		for (size_t i = 0; i < size; ++i) {
			try {
				KeyType key = DeserializeValue<KeyType>(ifs);
				ValueType value = DeserializeValue<ValueType>(ifs);
				map.emplace(std::move(key), std::move(value));
			} catch (const std::exception &e) {
				throw std::runtime_error(
					"Fehler beim Deserialisieren von Element " + std::to_string(i) +
					": " + e.what());
			}
		}

		if (ifs.peek() != EOF) {
			throw std::runtime_error("Unerwartete Daten am Ende der Datei");
		}

		return map;
	}

private:
	template <typename T>
	static auto SerializeValue(std::ofstream &ofs, const T &value) -> void {
		if constexpr (std::is_same_v<T, fs::path>) {
			SerializeValue(ofs, value.string());
		} else {
			ofs.write(reinterpret_cast<const char *>(&value), sizeof(value));
		}
	}

	static auto SerializeValue(std::ofstream &ofs, const std::string &value) -> void {
		size_t size = value.size();
		ofs.write(reinterpret_cast<const char *>(&size), sizeof(size));
		ofs.write(value.c_str(), size);
	}

	template <typename T>
	static auto DeserializeValue(std::ifstream &ifs) -> T {
		if constexpr (std::is_same_v<T, fs::path>) {
			return fs::path(DeserializeValue<std::string>(ifs));
		} else {
			T value;
			ifs.read(reinterpret_cast<char *>(&value), sizeof(value));
			return value;
		}
	}

	static auto DeserializeValue(std::ifstream &ifs) -> std::string {
		size_t size;
		ifs.read(reinterpret_cast<char *>(&size), sizeof(size));
		std::string value(size, '\0');
		ifs.read(&value[0], size);
		return value;
	}
};

typedef MapSerializer<fs::path, i32> PathMapSerializer;
#else

class PathMapSerializer {
public:
	static void Serialize(const std::map<fs::path, i32> &map, const fs::path &filepath) {
		std::ofstream ofs(filepath, std::ios::binary | std::ios::trunc);
		if (!ofs) {
			throw std::runtime_error("Kann die Datei nicht öffnen: " + filepath.string());
		}

		size_t size = map.size();
		ofs.write(reinterpret_cast<const char *>(&size), sizeof(size));

		for (const auto &[path, count] : map) {
			const std::string pathStr = path.string();
			size_t pathLen = pathStr.length();
			ofs.write(reinterpret_cast<const char *>(&pathLen), sizeof(pathLen));
			ofs.write(pathStr.data(), pathLen);

			ofs.write(reinterpret_cast<const char *>(&count), sizeof(count));
		}
	}

	static std::map<fs::path, i32> Deserialize(const fs::path &filepath) {
		std::ifstream ifs(filepath, std::ios::binary);
		if (!ifs) {
			return std::map<fs::path, i32>();
		}

		std::map<fs::path, i32> result;

		size_t size;
		ifs.read(reinterpret_cast<char *>(&size), sizeof(size));

		for (size_t i = 0; i < size; ++i) {
			size_t pathLen;
			ifs.read(reinterpret_cast<char *>(&pathLen), sizeof(pathLen));
			std::string pathStr(pathLen, '\0');
			ifs.read(&pathStr[0], pathLen);

			i32 count;
			ifs.read(reinterpret_cast<char *>(&count), sizeof(count));

			result[fs::path(pathStr)] = count;
		}

		return result;
	}
};

#endif