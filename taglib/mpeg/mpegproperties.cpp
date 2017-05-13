/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org
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

#include <tdebug.h>
#include <tstring.h>

#include "mpegproperties.h"
#include "mpegfile.h"
#include "xingheader.h"
#include "vbriHeader.h"
#include "id3v2tag.h"
#include "id3v2header.h"
#include "apetag.h"
#include "apefooter.h"

using namespace TagLib;

class MPEG::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate() :
    xingHeader(0),
    vbriHeader(0),
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    layer(0),
    version(Header::Version1),
    channelMode(Header::Stereo),
    protectionEnabled(false),
    isCopyrighted(false),
    isOriginal(false) {}

  ~PropertiesPrivate()
  {
    delete xingHeader;
    delete vbriHeader;
  }

  XingHeader *xingHeader;
  VbriHeader *vbriHeader;
  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int layer;
  Header::Version version;
  Header::ChannelMode channelMode;
  bool protectionEnabled;
  bool isCopyrighted;
  bool isOriginal;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

MPEG::Properties::Properties(File *file, ReadStyle style) :
  AudioProperties(style),
  d(new PropertiesPrivate())
{
  read(file);
}

MPEG::Properties::~Properties()
{
  delete d;
}

int MPEG::Properties::length() const
{
  return lengthInSeconds();
}

int MPEG::Properties::lengthInSeconds() const
{
  return d->length / 1000;
}

int MPEG::Properties::lengthInMilliseconds() const
{
  return d->length;
}

int MPEG::Properties::bitrate() const
{
  return d->bitrate;
}

int MPEG::Properties::sampleRate() const
{
  return d->sampleRate;
}

int MPEG::Properties::channels() const
{
  return d->channels;
}

const MPEG::XingHeader *MPEG::Properties::xingHeader() const
{
  return d->xingHeader;
}

MPEG::Header::Version MPEG::Properties::version() const
{
  return d->version;
}

int MPEG::Properties::layer() const
{
  return d->layer;
}

bool MPEG::Properties::protectionEnabled() const
{
  return d->protectionEnabled;
}

MPEG::Header::ChannelMode MPEG::Properties::channelMode() const
{
  return d->channelMode;
}

bool MPEG::Properties::isCopyrighted() const
{
  return d->isCopyrighted;
}

bool MPEG::Properties::isOriginal() const
{
  return d->isOriginal;
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void MPEG::Properties::read(File *file)
{
  // Only the first frame is required if we have a VBR header.

  const long firstHeaderOffset = file->firstFrameOffset();
  if(firstHeaderOffset < 0) {
    debug("MPEG::Properties::read() -- Could not find a valid first MPEG frame in the stream.");
    return;
  }

  Header firstHeader(file, firstHeaderOffset, false);

  if(!firstHeader.isValid()) {
    debug("MPEG::Properties::read() -- The first page header is invalid.");
    return;
  }

  // Check for a VBR header that will help us in gathering information about a
  // VBR stream.

  file->seek(firstHeaderOffset + 4);
  d->xingHeader = new XingHeader(file->readBlock(firstHeader.frameLength() - 4));

  Header validHeader = firstHeader;
  
  // Read the length and the bitrate from the Xing or Vbri header.
  if(d->xingHeader->isValid() &&
     firstHeader.sampleRate() > 0 &&
     d->xingHeader->totalFrames() > 0)
  {    
    const double timePerFrame = firstHeader.samplesPerFrame() * 1000.0 / firstHeader.sampleRate();
    const double length = timePerFrame * d->xingHeader->totalFrames();
    
    d->length = static_cast<int>(length + 0.5);
    d->bitrate = static_cast<int>(d->xingHeader->totalSize() * 8.0 / length + 0.5);
  }
  else
  {
    delete d->xingHeader;
    d->xingHeader = 0;
    
    file->seek(firstHeaderOffset + 4 + 32);
    d->vbriHeader = new VbriHeader(file->readBlock(24));
    
    if (d->vbriHeader->isValid() &&
        d->vbriHeader->totalFrames())
    {
      const double timePerFrame = firstHeader.samplesPerFrame() * 1000.0 / firstHeader.sampleRate();
      const double length = timePerFrame * d->vbriHeader->totalFrames();
      
      d->length = static_cast<int>(length + 0.5);
      d->bitrate = static_cast<int>(d->vbriHeader->totalSize() * 8.0 / length + 0.5);
    }
    else
    {
      // Since there was no valid Xing or Vbri header found, we hope that we're in a constant
      // bitrate file.
      // TODO: Make this more robust with audio property detection for VBR without a
      // Xing header.
      
      delete d->vbriHeader;
      d->vbriHeader = 0;
      
      const long last = file->lastFrameOffset();
      Header lastHeader(file, last, false);
      
      // To be sure the header is valid, search until two headers match
      
      // use the first header if we don't find any matching ones
      validHeader = firstHeader;
      
      if (!(firstHeader.version() == lastHeader.version()
            && firstHeader.layer() == lastHeader.layer()
            && firstHeader.sampleRate() == lastHeader.sampleRate()
            && firstHeader.channelMode() == lastHeader.channelMode()
            && firstHeader.isCopyrighted() == lastHeader.isCopyrighted()
            && firstHeader.isOriginal() == lastHeader.isOriginal()))
      {
        // First and last headers didn't match, try to find some matching ones
        long headerOffset = file->nextFrameOffset(firstHeaderOffset + 2);
        while(headerOffset < last && headerOffset >= 0)
        {
          Header nextHeader(file, headerOffset, false);
          if (nextHeader.isValid() && nextHeader.frameLength() > 0 && nextHeader.bitrate() > 0)
          {
            if (firstHeader.version() == nextHeader.version()
                && firstHeader.layer() == nextHeader.layer()
                && firstHeader.sampleRate() == nextHeader.sampleRate()
                && firstHeader.channelMode() == nextHeader.channelMode()
                && firstHeader.isCopyrighted() == nextHeader.isCopyrighted()
                && firstHeader.isOriginal() == nextHeader.isOriginal())
            {
              // headers match!
              validHeader = firstHeader;
              break;
            }
            
            firstHeader = nextHeader;
          }
          
          headerOffset = file->nextFrameOffset(headerOffset + 2);
        }
      }
      
      if(validHeader.frameLength() > 0 && validHeader.bitrate() > 0) {
        
        d->bitrate = validHeader.bitrate();
        
        long long streamLength = file->length();
        
        if(file->hasID3v1Tag())
          streamLength -= 128;
        
        if(file->hasID3v2Tag())
          streamLength -= file->ID3v2Tag()->header()->completeTagSize();
        
        if(file->hasAPETag())
          streamLength -= file->APETag()->footer()->completeTagSize();
        
        if(streamLength > 0)
          d->length = static_cast<int>(streamLength * 8.0 / d->bitrate + 0.5);
      }
    }
  }
    
  d->sampleRate = validHeader.sampleRate();
  d->channels = validHeader.channelMode() == Header::SingleChannel ? 1 : 2;
  d->version = validHeader.version();
  d->layer = validHeader.layer();
  d->protectionEnabled = validHeader.protectionEnabled();
  d->channelMode = validHeader.channelMode();
  d->isCopyrighted = validHeader.isCopyrighted();
  d->isOriginal = validHeader.isOriginal();
}
