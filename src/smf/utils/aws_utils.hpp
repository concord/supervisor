#pragma once

#include <string>

namespace Concord {
bool sendLogsToS3(const std::string &stderr_file, const std::string &stdout_file);
}
