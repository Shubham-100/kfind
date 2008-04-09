/***************************************************************************
 *   Copyright (C) 2008 by Fredrik Höglund <fredrik@kde.org>               *
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

#ifndef KTOOLTIP_P_H
#define KTOOLTIP_P_H

class KAbstractToolTipLabel;
class KStyleOptionToolTip;
class KToolTipDelegate;

class KToolTipManager
{
public:
    ~KToolTipManager();

    static KToolTipManager *instance() {
        if (!s_instance)
            s_instance = new KToolTipManager();

        return s_instance;
    }

    void showTip(const QPoint &pos, KToolTipItem *item);
    void hideTip();

    void initStyleOption(KStyleOptionToolTip *option) const;
    bool haveAlphaChannel() const;

    void setDelegate(KToolTipDelegate *delegate);
    KToolTipDelegate *delegate() const;

private:
    KToolTipManager();

    KAbstractToolTipLabel *label;
    KToolTipItem *currentItem;
    KToolTipDelegate *m_delegate;

#ifdef Q_WS_X11
    bool haveArgbVisual;
    Atom net_wm_cm_s0;
#endif

    static KToolTipManager *s_instance;
};

#endif
