/***********************************************************************
 * SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
 * SPDX-FileCopyrightText: 2008-2009 Martin Sandsmark <martin.sandsmark@kde.org>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***********************************************************************/

#pragma once

#include "fileTree.h"

#include <QPixmap>
#include <QRectF>
#include <QString>

namespace RadialMap
{
class Segment;

class Map
{
public:
    Map();
    ~Map();

    void make(const Folder *, bool = false);
    bool resize(const QRectF &);

    bool isNull() const
    {
        return (m_signature.isEmpty());
    }
    void invalidate();

    qreal height() const
    {
        return m_rect.height();
    }
    qreal width() const
    {
        return m_rect.width();
    }
    QPixmap pixmap() const
    {
        return m_pixmap;
    }

    void saveSvg(const QString &path);

    friend class Widget;
    friend class Item;

private:
    void paint(QPaintDevice *paintDevice = nullptr);
    void paint(QPainter *painter);
    void colorise();
    void setRingBreadth();
    void findVisibleDepth(const Folder *dir, uint currentDepth = 0);
    bool build(const Folder *const dir, const uint depth = 0, uint a_start = 0, const uint a_end = 5760);

    QVector<QList<Segment *>> m_signature;

    const Folder *m_root{};
    uint m_minSize{};
    QVector<FileSize> m_limits;
    QRectF m_rect;
    uint m_visibleDepth; /// visible level depth of system
    QPixmap m_pixmap;
    int m_ringBreadth;
    uint m_innerRadius; /// radius of inner circle
    QString m_centerText;
    qreal m_dpr = 1.0;

    uint MAP_2MARGIN;

    Q_DISABLE_COPY_MOVE(Map);
};
} // namespace RadialMap
