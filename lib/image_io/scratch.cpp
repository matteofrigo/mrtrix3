/*
    Copyright 2009 Brain Research Institute, Melbourne, Australia

    Written by J-Donald Tournier, 22/08/09.

    This file is part of MRtrix.

    MRtrix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MRtrix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MRtrix.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <memory>
#include <vector>

#include "image_io/scratch.h"
#include "header.h"

namespace MR
{
  namespace ImageIO
  {

    bool Scratch::is_file_backed () const { return false; }

    void Scratch::load (const Header& header, size_t buffer_size)
    {
      assert (buffer_size);
      DEBUG ("allocating scratch buffer for image \"" + header.name() + "\"...");
      addresses.push_back (std::unique_ptr<uint8_t[]> (new uint8_t [buffer_size]));
      memset (addresses[0].get(), 0, buffer_size);
    }


    void Scratch::unload (const Header& header)
    {
      if (addresses.size()) {
        DEBUG ("deleting scratch buffer for image \"" + header.name() + "\"...");
        addresses[0].reset();
      }
    }

  }
}


