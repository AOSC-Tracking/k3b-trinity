/* 
 *
 * $Id: k3bimagefilereader.cpp 619556 2007-01-03 17:38:12Z trueg $
 * Copyright (C) 2003 Sebastian Trueg <trueg@k3b.org>
 *
 * This file is part of the K3b project.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING" for the exact licensing terms.
 */

#include "k3bimagefilereader.h"

#include <tqfile.h>
#include <tqtextstream.h>

#include <kdebug.h>



class K3bImageFileReader::Private
{
public:
  Private()
    : isValid(false) {
  }

  TQString filename;
  TQString imageFilename;
  bool isValid;
};


K3bImageFileReader::K3bImageFileReader()
{
  d = new Private();
}


K3bImageFileReader::~K3bImageFileReader()
{
  delete d;
}


void K3bImageFileReader::openFile( const TQString& filename )
{
  d->filename = filename;
  d->imageFilename = TQString();
  setValid(false);

  if( !filename.isEmpty() )
    readFile();
}


void K3bImageFileReader::setValid( bool b )
{
  d->isValid = b;
}


void K3bImageFileReader::setImageFilename( const TQString& filename )
{
  d->imageFilename = filename;
}


bool K3bImageFileReader::isValid() const
{
  return d->isValid;
}


const TQString& K3bImageFileReader::filename() const
{
  return d->filename;
}


const TQString& K3bImageFileReader::imageFilename() const
{
  return d->imageFilename;
}
