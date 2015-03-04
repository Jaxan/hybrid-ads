#pragma once

#include <iosfwd>

struct mealy;
mealy read_mealy_from_dot(const std::string & filename);
mealy read_mealy_from_dot(std::istream & input);
