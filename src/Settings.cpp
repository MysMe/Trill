#include "Settings.h"
#include <fstream>
#include <filesystem>
#include "ProjectDir.h"

namespace settings
{
	settings settings::fromBytes(std::span<std::byte> bytes)
	{
		settings s;
		if (bytes.size() != sizeof(settings))
		{
			return s;
		}
		std::memcpy(&s, bytes.data(), sizeof(settings));
		return s;
	}
	std::vector<std::byte> settings::toBytes() const
	{
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
				file.seekg(0, std::ios::end);
				auto size = file.tellg();
				file.seekg(0, std::ios::beg);
				std::vector<std::byte> bytes(size);
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