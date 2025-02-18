/* 
 *
 * $Id: k3bdvdview.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef _K3B_DVDVIEW_H_
#define _K3B_DVDVIEW_H_

#include <k3bdataview.h>

class K3bDvdDoc;


class K3bDvdView : public K3bDataView
{
  TQ_OBJECT
  

 public:
  K3bDvdView( K3bDvdDoc* doc, TQWidget *parent = 0, const char *name = 0 );
  ~K3bDvdView();

 protected:
  virtual K3bProjectBurnDialog* newBurnDialog( TQWidget* parent = 0, const char* name = 0 );

 private:
  K3bDvdDoc* m_doc;
};

#endif
