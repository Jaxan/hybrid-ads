#pragma once

#include "phantom.hpp"
#include "mealy.hpp"

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <iostream>
#include <fstream>

namespace boost {
namespace serialization {

template<class Archive, typename B, typename T>
void serialize(Archive & ar, phantom<B, T> & value, const unsigned int /*version*/){
	ar & value.x;
}

} // namespace serialization
} // namespace boost
