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

#ifndef TAGLIB_VBRIHEADER_H
#define TAGLIB_VBRIHEADER_H

#include "mpegheader.h"
#include "taglib_export.h"

namespace TagLib {

  class ByteVector;

  namespace MPEG {

    //! An implementation of Fraunhofer VBRI headers

    class TAGLIB_EXPORT VbriHeader
    {
    public:
      /*!
       * Parses a Vbri header based on \a data.  The data must be at least 18
       * bytes long.
       */
      VbriHeader(const ByteVector &data);

      /*!
       * Destroy this VbriHeader instance.
       */
      virtual ~VbriHeader();

      /*!
       * Returns true if the data was parsed properly and if there is a valid
       * Vbri header present.
       */
      bool isValid() const;

      /*!
       * Returns the total number of frames.
       */
      uint totalFrames() const;

      /*!
       * Returns the total size of stream in bytes.
       */
      uint totalSize() const;

    private:
      VbriHeader(const VbriHeader &);
      VbriHeader &operator=(const VbriHeader &);

      void parse(const ByteVector &data);

      class VbriHeaderPrivate;
      VbriHeaderPrivate *d;
    };
  }
}

#endif
