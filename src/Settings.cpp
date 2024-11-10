#include "Settings.h"
#include <fstream>
#include <filesystem>
#include "ProjectDir.h"
#include <cstring>

namespace settings
{
	settings settings::fromBytes(std::span<std::byte> bytes)
	{
		static_assert(std::is_trivially_copyable_v<settings>, "Settings must be trivially copyable for memcpy.");
		settings s;
		if (bytes.size() != sizeof(settings))
		{
			//If the settings vary (e.g. new settings added), return a default settings object - ignore any existing settings
			return s;
		}
		std::memcpy(&s, bytes.data(), sizeof(settings));
		return s;
	}

	std::vector<std::byte> settings::toBytes() const
	{
		static_assert(std::is_trivially_copyable_v<settings>, "Settings must be trivially copyable for memcpy.");
		std::vector<std::byte> bytes(sizeof(settings));
		std::memcpy(bytes.data(), this, sizeof(settings));
		return bytes;
	}

	settings getSettings()
	{
		settings s;
		auto settingsPath = constants::project_path / "settings.bin";
		if (std::filesystem::exists(settingsPath))
		{
			std::ifstream file(settingsPath, std::ios::binary);
			if (file.is_open())
			{
				//Determine file size
				file.seekg(0, std::ios::end);
				auto size = file.tellg();
				//Read entire file into memory
				file.seekg(0, std::ios::beg);
				std::vector<std::byte> bytes(size);
				//Reinterpret cast to char* is well defined
				file.read(reinterpret_cast<char*>(bytes.data()), size);
				s = settings::fromBytes(bytes);
			}
		}
		return s;
	}
	void saveSettings(const settings& settings)
	{
		auto settingsPath = constants::project_path / "settings.bin";
		std::ofstream file(settingsPath, std::ios::binary);
		if (file.is_open())
		{
			auto bytes = settings.toBytes();
			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
		}
	}
}