#pragma once
#include <print>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <vulkan/vk_enum_string_helper.h>

std::string readTextFile(const std::string_view filepath);