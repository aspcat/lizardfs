#pragma once

#include <sys/types.h>

#include <iostream>
#include <string>

#include "common/exception.h"

LIZARDFS_CREATE_EXCEPTION_CLASS(GetIoLimitGroupIdException, Exception);

typedef std::string IoLimitGroupId;

IoLimitGroupId getIoLimitGroupId(std::istream& is, const std::string& subsystem);
IoLimitGroupId getIoLimitGroupId(const pid_t pid, const std::string& subsystem);
