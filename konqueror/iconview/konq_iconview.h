/*  This file is part of the KDE project
    Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __konq_iconview_h__
#define __konq_iconview_h__

#include <kparts/browserextension.h>
#include <konqiconviewwidget.h>
#include <konqoperations.h>

class KonqPropsView;
class KDirLister;
class KonqFileItem;
class KonqFMSettings;
class KFileIVI;
class KAction;
class KToggleAction;
class KActionMenu;
class QIconViewItem;
class IconViewBrowserExtension;

/**
 * The Icon View for konqueror.
 * The "Kfm" in the name stands for file management since it shows files :)
 */
class KonqKfmIconView : public KParts::ReadOnlyPart
{
  friend class IconViewBrowserExtension; // to access m_pProps
  Q_OBJECT
public:

  enum SortCriterion { NameCaseSensitive, NameCaseInsensitive, Size };

  KonqKfmIconView( QWidget *parentWidget, QObject *parent, const char *name );
  virtual ~KonqKfmIconView();

  virtual bool openURL( const KURL &_url );
  virtual bool closeURL();

  virtual bool openFile() { return true; }

  KonqIconViewWidget *iconViewWidget() const { return m_pIconView; }

  virtual void saveState( QDataStream &stream );
  virtual void restoreState( QDataStream &stream );

public slots:
  void slotImagePreview( bool toggle );
  void slotShowDot();
  void slotSelect();
  void slotUnselect();
  void slotSelectAll();
  void slotUnselectAll();
  void slotInvertSelection();

  void slotSortByNameCaseSensitive( bool toggle );
  void slotSortByNameCaseInsensitive( bool toggle );
  void slotSortBySize( bool toggle );
  void slotSortDescending();
  void slotSortDirsFirst();

  void slotKofficeMode( bool b );
  void slotViewLarge( bool b );
  void slotViewMedium( bool b );
  void slotViewSmall( bool b );
  void slotViewNone( bool b );

  void slotTextBottom( bool b );
  void slotTextRight( bool b );

  void slotBackgroundColor();
  void slotBackgroundImage();

protected slots:
  // slots connected to QIconView
  void slotReturnPressed( QIconViewItem *item );
  void slotMouseButtonPressed(int, QIconViewItem*, const QPoint&);
  void slotViewportRightClicked( QIconViewItem * );
  void slotOnItem( QIconViewItem *item );
  void slotOnViewport();

  // slots connected to the directory lister
  void slotStarted( const QString & );
  void slotCanceled();
  void slotCompleted();
  void slotNewItems( const KonqFileItemList& );
  void slotDeleteItem( KonqFileItem * );

  void slotClear();

  void slotTotalFiles( int, unsigned long files );

  void slotDisplayFileSelectionInfo();

  void slotProcessMimeIcons();

  void slotOpenURLRequest();

protected:

  virtual void guiActivateEvent( KParts::GUIActivateEvent *event );

  void setupSorting( SortCriterion criterion );

  /** */
  void setupSortKeys();

  QString makeSizeKey( KFileIVI *item );

  /** The directory lister for this URL */
  KDirLister* m_dirLister;

  /** Konqueror settings */
  KonqFMSettings * m_pSettings;

  /** View properties */
  KonqPropsView * m_pProps;

  /**
   * Set to true while the constructor is running.
   * @ref #initConfig needs to know about that.
   */
  bool m_bInit;

  bool m_bLoading;
  /**
   * Set to true if slotCompleted needs to realign the icons
   */
  bool m_bNeedAlign;

  unsigned long m_ulTotalFiles;

  SortCriterion m_eSortCriterion;

  KToggleAction *m_paDotFiles;
  KToggleAction *m_paImagePreview;
  KActionMenu *m_pamSort;

  KToggleAction *m_paLargeIcons;
  KToggleAction *m_paMediumIcons;
  KToggleAction *m_paSmallIcons;
  KToggleAction *m_paNoIcons;

  KToggleAction *m_paBottomText;
  KToggleAction *m_paRightText;

  KToggleAction *m_paKOfficeMode;
  KAction *m_paSelect;
  KAction *m_paUnselect;
  KAction *m_paSelectAll;
  KAction *m_paUnselectAll;
  KAction *m_paInvertSelection;

  KToggleAction *m_paSortDirsFirst;

  long m_lDirSize;
  long m_lFileCount;
  long m_lDirCount;

  IconViewBrowserExtension *m_extension;

  // used by slotOpenURLRequest
  KonqFileItem * openURLRequestFileItem;

  KonqIconViewWidget *m_pIconView;

  QList<KFileIVI> m_lstPendingMimeIconItems;
};

class IconViewBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
    friend class KonqKfmIconView; // so that it can emit our signals
public:
  IconViewBrowserExtension( KonqKfmIconView *iconView );

  virtual int xOffset();
  virtual int yOffset();

  virtual void saveState( QDataStream &stream )
    {
      KParts::BrowserExtension::saveState( stream );
      m_iconView->saveState( stream );
    }

  virtual void restoreState( QDataStream &stream )
    {
      KParts::BrowserExtension::restoreState( stream );
      m_iconView->restoreState( stream );
    }

public slots:
  // Those slots are automatically connected by the shell
  void reparseConfiguration();
  void saveLocalProperties();
  void savePropertiesAsDefault();
  void refreshMimeTypes() { m_iconView->iconViewWidget()->refreshMimeTypes(); }

  void cut() { m_iconView->iconViewWidget()->cutSelection(); }
  void copy() { m_iconView->iconViewWidget()->copySelection(); }
  void pastecopy() { m_iconView->iconViewWidget()->pasteSelection(false); }
  void pastecut() { m_iconView->iconViewWidget()->pasteSelection(true); }

  void trash() { KonqOperations::del(KonqOperations::TRASH,
                                     m_iconView->iconViewWidget()->selectedUrls()); }
  void del() { KonqOperations::del(KonqOperations::DEL,
                                   m_iconView->iconViewWidget()->selectedUrls()); }
  void shred() { KonqOperations::del(KonqOperations::SHRED,
                                     m_iconView->iconViewWidget()->selectedUrls()); }
  // void print();

private:
  KonqKfmIconView *m_iconView;
};

#endif
