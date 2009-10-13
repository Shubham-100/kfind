/***************************************************************************
 *   Copyright (C) 2009 by Peter Penz <peter.penz@gmx.at>                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "edittagsdialog_p.h"

#include <klocale.h>

EditTagsDialog::EditTagsDialog(const QList<Nepomuk::Tag>& tags,
                               QWidget* parent,
                               Qt::WFlags flags) :
    KDialog(parent, flags),
    m_tags(tags)
{

    const QString caption = (tags.count() > 0) ?
                            i18nc("@title:window", "Change Tags") :
                            i18nc("@title:window", "Add Tags");
    setCaption(caption);
    setButtons(KDialog::Ok | KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
}

EditTagsDialog::~EditTagsDialog()
{
}

QList<Nepomuk::Tag> EditTagsDialog::tags() const
{
    return m_tags;
}

#include "edittagsdialog_p.moc"
