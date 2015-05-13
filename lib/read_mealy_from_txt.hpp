#pragma once

#include "types.hpp"
#include "read_mealy_from_dot.hpp"

#include <iosfwd>

struct mealy;

/// \brief reads a mealy machine from plain txt file (as provided by A. T. Endo)
/// States, inputs and outputs in these files are already integral (and contiguous)
mealy read_mealy_from_txt(std::istream & in);
mealy read_mealy_from_txt(std::string const & filename);

translation create_translation_for_mealy(mealy const & m);
