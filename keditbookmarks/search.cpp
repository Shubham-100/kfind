/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#include <qregexp.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kfind.h>

#include <kbookmarkmanager.h>

#include "toplevel.h"
#include "listview.h"
#include "search.h"

class KBookmarkTextMap : private KBookmarkGroupTraverser {
public:
   KBookmarkTextMap(KBookmarkManager *);
   void update();
   QValueList<KBookmark> find(const QString &text) const;
private:
   virtual void visit(const KBookmark &);
   virtual void visitEnter(const KBookmarkGroup &);
   virtual void visitLeave(const KBookmarkGroup &) { ; }
private:
   typedef QValueList<KBookmark> KBookmarkList;
   QMap<QString, KBookmarkList> m_bk_map;
   KBookmarkManager *m_manager;
};

KBookmarkTextMap::KBookmarkTextMap( KBookmarkManager *manager ) {
   m_manager = manager;
}

void KBookmarkTextMap::update()
{
   m_bk_map.clear();
   KBookmarkGroup root = m_manager->root();
   traverse(root);
}

void KBookmarkTextMap::visit(const KBookmark &bk) {
   if (!bk.isSeparator()) {
      // todo - comment field
      QString text = bk.url().url() + " " + bk.text();
      m_bk_map[text].append(bk);
   }
}

void KBookmarkTextMap::visitEnter(const KBookmarkGroup &grp) {
   visit(grp);
}

QValueList<KBookmark> KBookmarkTextMap::find(const QString &text) const
{
   QValueList<KBookmark> matches;
   QValueList<QString> keys = m_bk_map.keys();
   for (QValueList<QString>::iterator it = keys.begin();
         it != keys.end(); ++it 
   ) {
      if ((*it).find(text,0,false) != -1)
         matches += m_bk_map[(*it)];
   }
   return matches;
}

Searcher* Searcher::s_self = 0;

static int m_currentResult;
static QStringList m_foundAddrs;

void Searcher::slotSearchNext()
{
   if (m_foundAddrs.empty())
      return;
   KEBListViewItem *item = ListView::self()->getItemAtAddress(m_foundAddrs[m_currentResult]);
   m_currentResult = ((m_currentResult+1) <= m_foundAddrs.count())
                    ? (m_currentResult+1) : 0;
   ListView::self()->clearSelection();
   ListView::self()->setCurrent(item);
   item->setSelected(true);
}

void Searcher::slotSearchTextChanged(const QString & text)
{
   if (!m_bktextmap)
      m_bktextmap = new KBookmarkTextMap(CurrentMgr::self()->mgr());
   m_bktextmap->update(); // FIXME - make this public and use it!!!
   QValueList<KBookmark> results = m_bktextmap->find(text);
   m_foundAddrs.clear();
   for (QValueList<KBookmark>::iterator it = results.begin(); it != results.end(); ++it )
      m_foundAddrs << (*it).address();
   // sort the addr list so we don't go "next" in a random order
   m_foundAddrs.sort();
   m_currentResult = 0;
   slotSearchNext();
}

#include "search.moc"
