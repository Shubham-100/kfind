/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KONQ_UNDO_P_H
#define KONQ_UNDO_P_H

#include <qstack.h>
#include <QUndoCommand>

struct KonqBasicOperation
{
  typedef QStack<KonqBasicOperation> Stack;

  KonqBasicOperation()
  { m_valid = false; }

  bool m_valid;
  bool m_directory;
  bool m_renamed;
  bool m_link;
  KUrl m_src;
  KUrl m_dst;
  QString m_target;
};

// ### I considered inheriting this from QUndoCommand.
// ### but since it is being copied by value in the code, we can't use that.
// ### also it would need to implement undo() itself (well, it can call the undomanager for it)
class KonqCommand
{
public:
    typedef QStack<KonqCommand> Stack;

    KonqCommand()
    { m_valid = false; }

    // TODO
    //KonqCommand( Type type, KonqBasicOperation::Stack& opStack, const KUrl::List& src, const KUrl& dest )
    //  : m_type( type ), m_opStack( opStack ), m_src( src ), m_dest( dest )
    // {
    //     // if using QUndoCommand: setText(...); // see code in KonqUndoManager::undoText()
    // }

    //virtual void undo() {} // TODO
    //virtual void redo() {} // TODO

    bool m_valid;

    KonqUndoManager::CommandType m_type;
    KonqBasicOperation::Stack m_opStack;
    KUrl::List m_src;
    KUrl m_dst;
};

// This class listens to a job, collects info while it's running (for copyjobs)
// and when the job terminates, on success, it calls addCommand in the undomanager.
class KonqCommandRecorder : public QObject
{
  Q_OBJECT
public:
  KonqCommandRecorder( KonqUndoManager::CommandType op, const KUrl::List &src, const KUrl &dst, KIO::Job *job );
  virtual ~KonqCommandRecorder();

private Q_SLOTS:
  void slotResult( KJob *job );

  void slotCopyingDone( KIO::Job *, const KUrl &from, const KUrl &to, bool directory, bool renamed );
  void slotCopyingLinkDone( KIO::Job *, const KUrl &from, const QString &target, const KUrl &to );

private:
  KonqCommand m_cmd;
};


#endif /* KONQ_UNDO_P_H */

