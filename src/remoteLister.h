/***********************************************************************
 * SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
 * SPDX-FileCopyrightText: 2008-2009 Martin Sandsmark <martin.sandsmark@kde.org>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***********************************************************************/

#pragma once

#include "fileTree.h"
#include <KDirLister>

namespace Filelight
{
class ScanManager;

class RemoteLister : public KDirLister
{
    Q_OBJECT
public:
    RemoteLister(const QUrl &url, ScanManager *manager, QObject *parent = nullptr);
    ~RemoteLister() override;

Q_SIGNALS:
    void branchCompleted(Folder *tree);

private Q_SLOTS:
    void onCompleted();
    void onCanceled();

private:
    struct Store *m_root;
    struct Store *m_store;
    ScanManager *m_manager;

    Q_DISABLE_COPY_MOVE(RemoteLister)
};
} // namespace Filelight
