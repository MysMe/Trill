#pragma once
#include <string>
#include <cstdint>
#include <span>
#include <cstddef>
#include <vector>

namespace settings
{
	struct settings
	{
		int32_t window_x = 0;
		int32_t window_y = 0;
		int16_t window_width = 800;
		int16_t window_height = 600;
		float dp_ratio = 1.0f;
		float opacity = 1.0f;

		bool operator==(const settings&) const = default;

		static settings fromBytes(std::span<std::byte> bytes);
		std::vector<std::byte> toBytes() const;
	};

	settings getSettings();

	void saveSettings(const settings& settings);
}