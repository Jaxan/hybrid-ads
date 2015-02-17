#pragma once

#include <iosfwd>

struct splijtboom;

void write_splitting_tree_to_dot(const splijtboom & root, std::ostream & out);
void write_splitting_tree_to_dot(const splijtboom & root, std::string const & filename);
