/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __favicons_h
#define __favicons_h

#include <time.h>
#include <kmainwindow.h>
#include <kbookmark.h>
#include <qlistview.h>
#include <klistview.h>
#include <kcommand.h>
#include <dcopobject.h>
#include <commands.h>
#include <konq_faviconmgr.h>

#include <kparts/browserinterface.h>
#include <kparts/browserextension.h>
#include <khtml_part.h>

class FavIconWebGrabber : public QObject
{

    Q_OBJECT
public:
    FavIconWebGrabber( KHTMLPart * part, const KURL & url );
    ~FavIconWebGrabber() {}

protected slots:
    void slotMimetype( KIO::Job *job, const QString &_type );
    void slotFinished( KIO::Job * job );

private:
    KHTMLPart * m_part;
    KURL m_url;
};

class FavIconUpdater ;

class FavIconBrowserInterface : public KParts::BrowserInterface
{
   Q_OBJECT
   public:
      FavIconBrowserInterface( FavIconUpdater *view, const char *name );
   private:
      FavIconUpdater *m_view;
};

class FavIconUpdater : public KonqFavIconMgr {

   Q_OBJECT

public:   
   static FavIconUpdater * self();
   FavIconUpdater( QObject *parent, const char *name );
   ~FavIconUpdater();
   void queueIcon(const KBookmark &bk);
   void downloadIcon(const KBookmark &bk);
   void downloadIconComplex(const KBookmark &bk);

   // from KonqFavIconMgr
   virtual void notifyChange( bool isHost, QString hostOrURL, QString iconName );

private slots:
   void setIconURL( const KURL & iconURL );
   void slotCompleted();

private:
   static FavIconUpdater * s_self;
   KHTMLPart *m_part;
   FavIconBrowserInterface *m_browserIface;
   KBookmark m_bk;
};

#endif

