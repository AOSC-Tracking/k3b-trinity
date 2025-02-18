/* 
 *
 * $Id: k3bexternalbinwidget.h 619556 2007-01-03 17:38:12Z trueg $
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


#ifndef K3B_EXTERNAL_BIN_WIDGET_H
#define K3B_EXTERNAL_BIN_WIDGET_H


#include <tqwidget.h>
#include <tqptrlist.h>

#include <k3blistview.h>


class K3bExternalBinManager;
class TQPushButton;
class TQTabWidget;
class KEditListBox;
class K3bExternalProgram;
class K3bExternalBin;


class K3bExternalBinWidget : public TQWidget
{
  TQ_OBJECT
  

 public:
  K3bExternalBinWidget( K3bExternalBinManager*, TQWidget* parent = 0, const char* name = 0 );
  ~K3bExternalBinWidget();

  class K3bExternalBinViewItem;
  class K3bExternalProgramViewItem;

 public slots:
  void rescan();
  void load();
  void save();

 private slots:
  void slotSetDefaultButtonClicked();
  void slotProgramSelectionChanged( TQListViewItem* );
  void saveSearchPath();

 private:
  K3bExternalBinManager* m_manager;

  TQTabWidget* m_mainTabWidget;
  K3bListView* m_programView;
  K3bListView* m_parameterView;
  KEditListBox* m_searchPathBox;

  TQPushButton* m_defaultButton;
  TQPushButton* m_rescanButton;

  TQPtrList<K3bExternalProgramViewItem> m_programRootItems;
};


class K3bExternalBinWidget::K3bExternalProgramViewItem : public K3bListViewItem
{
 public:
  K3bExternalProgramViewItem( K3bExternalProgram* p, TQListView* parent );
  
  K3bExternalProgram* program() const { return m_program; }
  
 private:
  K3bExternalProgram* m_program;
};



class K3bExternalBinWidget::K3bExternalBinViewItem : public K3bListViewItem
{
 public:
  K3bExternalBinViewItem( K3bExternalBin* bin, K3bExternalProgramViewItem* parent );

  K3bExternalBin* bin() const { return m_bin; }
  K3bExternalProgramViewItem* parentProgramItem() const { return m_parent; }

  bool isDefault() const { return m_default; }
  void setDefault( bool b );

 private:
  K3bExternalBin* m_bin;
  K3bExternalProgramViewItem* m_parent;

  bool m_default;
};


#endif
