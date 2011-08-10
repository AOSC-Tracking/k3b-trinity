/* 
 *
 * $Id: sourceheader,v 1.3 2005/01/19 13:03:46 trueg Exp $
 * Copyright (C) 2005 Sebastian Trueg <trueg@k3b.org>
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

#ifndef __KFILE_K3BPROJECTFILEPLUGIN_H__
#define __KFILE_K3BPROJECTFILEPLUGIN_H__

/**
 * Note: For further information look into <$KDEDIR/include/kfilemetainfo.h>
 */
#include <kfilemetainfo.h>

class TQStringList;

class K3bProjectFilePlugin: public KFilePlugin
{
  Q_OBJECT
  TQ_OBJECT
    
 public:
  K3bProjectFilePlugin( TQObject *parent, const char *name, const TQStringList& args );
  
  virtual bool readInfo( KFileMetaInfo& info, uint what);
};

#endif

