/***************************************************************************
    copyright            : (C) 2013 James Burton
    email                : 
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tbytevector.h>
#include <tstring.h>
#include <tdebug.h>

#include "vbriheader.h"

using namespace TagLib;

class MPEG::VbriHeader::VbriHeaderPrivate
{
public:
  VbriHeaderPrivate() :
    frames(0),
    size(0),
    valid(false)
    {}

  uint frames;
  uint size;
  bool valid;
};

MPEG::VbriHeader::VbriHeader(const ByteVector &data)
{
  d = new VbriHeaderPrivate;
  parse(data);
}

MPEG::VbriHeader::~VbriHeader()
{
  delete d;
}

bool MPEG::VbriHeader::isValid() const
{
  return d->valid;
}

TagLib::uint MPEG::VbriHeader::totalFrames() const
{
  return d->frames;
}

TagLib::uint MPEG::VbriHeader::totalSize() const
{
  return d->size;
}

void MPEG::VbriHeader::parse(const ByteVector &data)
{
  // Check to see if a valid VBRI header is available.

  if(!data.startsWith("VBRI"))
    return;

  d->size   = data.toUInt(10, true);
  d->frames = data.toUInt(14, true);

  d->valid = true;
}
