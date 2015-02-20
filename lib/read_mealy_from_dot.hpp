#pragma once

#include <iosfwd>

struct Mealy;
Mealy read_mealy_from_dot(const std::string & filename);
Mealy read_mealy_from_dot(std::istream & input);
