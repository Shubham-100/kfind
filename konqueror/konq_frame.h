/*  This file is part of the KDE project
    Copyright (C) 1998, 1999 Michael Reiher <michael.reiher@gmx.de>

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

#ifndef __konq_frame_h__
#define __konq_frame_h__

#include "konq_factory.h"

#include <qguardedptr.h>
#include <qcolor.h>
#include <qwidget.h>
#include <qsplitter.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qtabbar.h>

#include <kpixmap.h>
#include <kpixmapeffect.h>

class QPixmap;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QToolButton;

class KonqView;
class KonqViewManager;
class KonqFrameBase;
class KonqFrame;
class KonqFrameContainerBase;
class KonqFrameContainer;
class KonqFrameTabs;
class KConfig;
class KSeparator;
class KProgress;
class KAction;

namespace KParts
{
  class ReadOnlyPart;
};

/**
 * A CheckBox with a special paintEvent(). It looks like the
 * unchecked radiobutton in b2k style if unchecked and contains a little
 * anchor if checked.
 */
class KonqCheckBox : public QCheckBox
{
    Q_OBJECT // for classname
public:
    KonqCheckBox(QWidget *parent=0, const char *name=0)
      :QCheckBox( parent, name ) {}
protected:
    virtual void paintEvent(QPaintEvent *ev);
};



/**
 * The KonqFrameStatusBar indicates wether a view is active or not.
 */
class KonqFrameStatusBar : public QWidget
{
  Q_OBJECT

   public:
      KonqFrameStatusBar( KonqFrame *_parent = 0L, const char *_name = 0L );
      virtual ~KonqFrameStatusBar();

      void setLinkedView( bool b );
      /**
       * Shows/hides the active-view indicator
       */
      void showActiveViewIndicator( bool b );
      /**
       * Shows/hides the linked-view indicator
       */
      void showLinkedViewIndicator( bool b );

   public slots:
      void message( const QString &msg );
      void slotConnectToNewView(KonqView *, KParts::ReadOnlyPart *oldOne,KParts::ReadOnlyPart *newOne);
      void slotLoadingProgress( int percent );
      void slotSpeedProgress( int bytesPerSecond );
      void slotDisplayStatusText(const QString& text);
      void slotClear();

   signals:
      /**
       * This signal is emitted when the user clicked the bar.
       */
      void clicked();

      /**
       * The "linked view" checkbox was clicked
       */
      void linkedViewClicked( bool mode );

   protected:
      virtual bool eventFilter(QObject*,QEvent *);
      virtual void resizeEvent( QResizeEvent* );
      virtual void mousePressEvent( QMouseEvent* );
      /**
       * Brings up the context menu for this frame
       */
      virtual void splitFrameMenu();

      virtual void paintEvent(QPaintEvent *e);
      KonqFrame* m_pParentKonqFrame;
      QCheckBox *m_pLinkedViewCheckBox;
      KProgress *m_progressBar;
      QLabel *m_pStatusLabel;
      int m_yOffset;
      bool m_showLed;
//      QTimer *m_msgTimer;
      QString m_savedMessage;
};


/**
 * The KonqFrameHeader provides a title and button to remove the view.
 */

class KonqFrameHeader : public QWidget
{
  Q_OBJECT

   public:
      KonqFrameHeader( KonqFrame *_parent = 0L, const char *_name = 0L );
      virtual ~KonqFrameHeader();
      /**
       * Connects an action to the button.
       * @param inAction the KAction to connect.
       */
      virtual void setAction( KAction *inAction );
      /**
       * Sets the text on the header.
       * @param text the text to display.
       */
      virtual void setText(const QString& text);
    /**
     * Used to pop up close menu on RMB click.
     */
      virtual bool eventFilter(QObject *obj, QEvent *ev);
   protected:
      virtual void showCloseMenu();
      KonqFrame* m_pParentKonqFrame;
      QLabel *m_pHeaderLabel;
      QToolButton *m_pCloseButton;
      QHBoxLayout *m_pLayout;
};

typedef QPtrList<KonqView> ChildViewList;

class KonqFrameBase
{
 public:
  virtual ~KonqFrameBase() {}

  virtual void saveConfig( KConfig* config, const QString &prefix, bool saveURLs, KonqFrameBase* docContainer, int id = 0, int depth = 0) = 0;

  virtual void copyHistory( KonqFrameBase *other ) = 0;

  virtual void printFrameInfo( QString spaces );

  virtual void reparentFrame( QWidget* parent,
                              const QPoint & p, bool showIt=FALSE ) = 0;

  virtual KonqFrameContainerBase* parentContainer() { return m_pParentContainer; }
  virtual void setParentContainer(KonqFrameContainerBase* parent) { m_pParentContainer = parent; }

  virtual void setTitle( QString title , QWidget* sender) = 0;
  virtual void setTabIcon( QString url, QWidget* sender ) = 0;

  virtual QWidget* widget() = 0;

  virtual void listViews( ChildViewList *viewList ) = 0;
  virtual QCString frameType() = 0;

  virtual void activateChild() = 0;

  virtual KonqView* activeChildView() = 0;

protected:
  KonqFrameBase() {}

  KonqFrameContainerBase* m_pParentContainer;
};

/**
 * The KonqFrame is the actual container for the views. It takes care of the
 * widget handling i.e. it attaches/detaches the view widget and activates
 * them on click at the statusbar.
 *
 * KonqFrame makes the difference between built-in views and remote ones.
 * We create a layout in it (with the KonqFrameStatusBar as top item in the layout)
 * For builtin views we have the view as direct child widget of the layout
 * For remote views we have an OPFrame, having the view attached, as child
 * widget of the layout
 */

class KonqFrame : public QWidget, public KonqFrameBase
{
  Q_OBJECT

public:
  KonqFrame( QWidget* parent, KonqFrameContainerBase *parentContainer = 0L,
             const char *name = 0L );
  virtual ~KonqFrame();

  /**
   * Attach a view to the KonqFrame.
   * @param viewFactory the view to attach (instead of the current one, if any)
   */
  KParts::ReadOnlyPart *attach( const KonqViewFactory &viewFactory );

  /**
   * Filters the CTRL+Tab event from the views and emits ctrlTabPressed to
   make KonqMainWindow switch to the next view
   */
  virtual bool eventFilter(QObject*obj,QEvent *ev);

  /**
   * Inserts the part's widget and the statusbar into the layout
   */
  void attachInternal();

  /**
   * Inserts a widget at the top of the part's widget, in the layout
   * (used for the find functionality)
   */
  void insertTopWidget( QWidget * widget );

  /**
   * Returns the part that is currently connected to the Frame.
   */
  KParts::ReadOnlyPart *part() { return m_pPart; }
  /**
   * Returns the view that is currently connected to the Frame.
   */
  KonqView* childView() { return m_pView; }

  bool isActivePart();

  void setView( KonqView* child );
  virtual void listViews( ChildViewList *viewList );

  virtual void saveConfig( KConfig* config, const QString &prefix, bool saveURLs, KonqFrameBase* docContainer, int id = 0, int depth = 0 );
  virtual void copyHistory( KonqFrameBase *other );

  virtual void printFrameInfo( QString spaces );

  virtual void setTitle( QString title, QWidget* sender );
  virtual void setTabIcon( QString url, QWidget* sender );

  virtual void reparentFrame(QWidget * parent,
                     const QPoint & p, bool showIt=FALSE );

  //virtual KonqFrameContainerBase* parentContainer();
  virtual QWidget* widget() { return this; }
  virtual QCString frameType() { return QCString("View"); }

  QVBoxLayout *layout() { return m_pLayout; }

  KonqFrameStatusBar *statusbar() const { return m_pStatusBar; }
  KonqFrameHeader    *header() const { return m_pHeader; }

  virtual void activateChild();
  
  KonqView* activeChildView() { return m_pView; }

public slots:

  /**
   * Is called when the frame statusbar has been clicked
   */
  void slotStatusBarClicked();

  void slotLinkedViewClicked( bool mode );

  /**
   * Is called when 'Remove View' is called from the popup menu
   */
  void slotRemoveView();

protected:
  virtual void paintEvent( QPaintEvent* );

  QVBoxLayout *m_pLayout;
  QGuardedPtr<KonqView> m_pView;

  QGuardedPtr<KParts::ReadOnlyPart> m_pPart;

  KonqViewManager* m_pViewManager;

  KSeparator *m_separator;
  KonqFrameStatusBar* m_pStatusBar;
  KonqFrameHeader *m_pHeader;
};

class KonqFrameContainerBase : public KonqFrameBase
{
public:
  virtual ~KonqFrameContainerBase() {}

  /**
   * Call this after inserting a new frame into the splitter.
   */
  virtual void insertChildFrame( KonqFrameBase * frame, int index = -1 ) = 0;
  /**
   * Call this before deleting one of our children.
   */
  virtual void removeChildFrame( KonqFrameBase * frame ) = 0;

  //inherited
  virtual void printFrameInfo( QString spaces );

  virtual QCString frameType() { return QCString("ContainerBase"); }

  virtual void reparentFrame(QWidget * parent,
                             const QPoint & p, bool showIt=FALSE ) = 0;

  virtual KonqFrameBase* activeChild() { return m_pActiveChild; }

  virtual void setActiveChild( KonqFrameBase* activeChild ) { m_pActiveChild = activeChild;
                                                              m_pParentContainer->setActiveChild( this ); }

  virtual void activateChild() { if (m_pActiveChild) m_pActiveChild->activateChild(); }
  
  virtual KonqView* activeChildView() { if (m_pActiveChild) return m_pActiveChild->activeChildView();
                                        else return 0L; }

protected:
  KonqFrameContainerBase() {}

  KonqFrameBase* m_pActiveChild;
};

/**
 * With KonqFrameContainers and @refKonqFrames we can create a flexible
 * storage structure for the views. The top most element is a
 * KonqFrameContainer. It's a direct child of the MainView. We can then
 * build up a binary tree of containers. KonqFrameContainers are the nodes.
 * That means that they always have two childs. Which are either again
 * KonqFrameContainers or, as leaves, KonqFrames.
 */

class KonqFrameContainer : public QSplitter, public KonqFrameContainerBase
{
  Q_OBJECT
  friend class KonqFrame; //for emitting ctrlTabPressed() only, aleXXX
public:
  KonqFrameContainer( Orientation o,
                      QWidget* parent,
                      KonqFrameContainerBase* parentContainer,
                      const char * name = 0);
  virtual ~KonqFrameContainer();

  virtual void listViews( ChildViewList *viewList );

  virtual void saveConfig( KConfig* config, const QString &prefix, bool saveURLs, KonqFrameBase* docContainer, int id = 0, int depth = 0 );
  virtual void copyHistory( KonqFrameBase *other );

  KonqFrameBase* firstChild() { return m_pFirstChild; }
  KonqFrameBase* secondChild() { return m_pSecondChild; }
  KonqFrameBase* otherChild( KonqFrameBase* child );

	virtual void printFrameInfo( QString spaces );

  void swapChildren();

  virtual void setTitle( QString title, QWidget* sender );
  virtual void setTabIcon( QString url, QWidget* sender );

  virtual QWidget* widget() { return this; }
  virtual QCString frameType() { return QCString("Container"); }

  /**
   * Call this after inserting a new frame into the splitter.
   */
  void insertChildFrame( KonqFrameBase * frame, int index = -1 );
  /**
   * Call this before deleting one of our children.
   */
  void removeChildFrame( KonqFrameBase * frame );

  //inherited
  virtual void reparentFrame(QWidget * parent,
                             const QPoint & p, bool showIt=FALSE );

  //make this one public
  int idAfter( QWidget* w ){ return QSplitter::idAfter( w ); }

signals:
  void ctrlTabPressed();

protected:
  KonqFrameBase* m_pFirstChild;
  KonqFrameBase* m_pSecondChild;
};

class KonqTabBar : public QTabBar
{
  Q_OBJECT

  public:
    KonqTabBar(KonqViewManager* viewManager, KonqFrameTabs *parent, const char *name = 0);

  protected:
    void mousePressEvent(QMouseEvent *e);

  private:
    KonqFrameTabs* m_pTabWidget;
    KonqViewManager* m_pViewManager;
    QPopupMenu* m_pPopupMenu;
};

class KonqFrameTabs : public QTabWidget, public KonqFrameContainerBase
{
  Q_OBJECT
  friend class KonqFrame; //for emitting ctrlTabPressed() only, aleXXX

public:
  KonqFrameTabs(QWidget* parent, KonqFrameContainerBase* parentContainer, KonqViewManager* viewManager, const char * name = 0);
  virtual ~KonqFrameTabs();

  virtual void listViews( ChildViewList *viewList );

  virtual void saveConfig( KConfig* config, const QString &prefix, bool saveURLs, KonqFrameBase* docContainer, int id = 0, int depth = 0 );
  virtual void copyHistory( KonqFrameBase *other );

  virtual void printFrameInfo( QString spaces );

  QPtrList<KonqFrameBase>* childFrameList() { return m_pChildFrameList; }

  virtual void setTitle( QString title, QWidget* sender );
  virtual void setTabIcon( QString url, QWidget* sender );

  virtual QWidget* widget() { return this; }
  virtual QCString frameType() { return QCString("Tabs"); }

  void activateChild();

  /**
   * Call this after inserting a new frame into the splitter.
   */
  void insertChildFrame( KonqFrameBase * frame, int index = -1);

  /**
   * Call this before deleting one of our children.
   */
  void removeChildFrame( KonqFrameBase * frame );

  //inherited
  virtual void reparentFrame(QWidget * parent,
                             const QPoint & p, bool showIt=FALSE );

public slots:
  void slotCurrentChanged( QWidget* newPage );

signals:
  void ctrlTabPressed();

protected:
  QPtrList<KonqFrameBase>* m_pChildFrameList;
  
private:
  KonqViewManager* m_pViewManager;
};

#endif
