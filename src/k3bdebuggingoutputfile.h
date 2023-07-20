/* 
 *
 * $Id: k3bdebuggingoutputfile.h 619556 2007-01-03 17:38:12Z trueg $
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

#ifndef _K3B_DEBUGGING_OUTPUT_FILE_H_
#define _K3B_DEBUGGING_OUTPUT_FILE_H_

#include <tqfile.h>
#include <tqobject.h>

#ifdef Q_MOC_RUN
#define USE_QT4
#endif // Q_MOC_RUN

// MOC_SKIP_BEGIN
#ifdef USE_QT4
class K3bDebuggingOutputFile : public TQFile
#else // USE_QT4
// MOC_SKIP_END
class K3bDebuggingOutputFile : public TQObject, public TQFile
// MOC_SKIP_BEGIN
#endif // USE_QT4
// MOC_SKIP_END
{
  TQ_OBJECT
  

 public:
  K3bDebuggingOutputFile();

  /**
   * Open the default output file and write some system information.
   */
  bool open();

 public slots:
  void addOutput( const TQString&, const TQString& );
};


#endif
