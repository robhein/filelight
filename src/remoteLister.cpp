/***********************************************************************
 * SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
 * SPDX-FileCopyrightText: 2008-2009 Martin Sandsmark <martin.sandsmark@kde.org>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***********************************************************************/

#include "remoteLister.h"
#include "filelight_debug.h"
#include "scan.h"

#include <QList>
#include <QWidget>

namespace Filelight
{

// TODO: delete all this stuff!

// You need to use a single DirLister.
// One per folder breaks KIO (seemingly) and also uses un-godly amounts of memory!

struct Store {
    using List = QList<Store *>;

    /// location of the folder
    const QUrl url;
    /// the folder on which we are operating
    Folder *folder = nullptr;
    /// so we can reference the parent store
    Store *parent = nullptr;
    /// directories in this folder that need to be scanned before we can propagate()
    List stores;

    Store(const QUrl &u, const QString &name, Store *s)
        : url(u)
        , folder(new Folder((name + QLatin1Char('/')).toUtf8().constData()))
        , parent(s)
    {
    }
    ~Store() = default;

    Store *propagate()
    {
        /// returns the next store available for scanning

        qCDebug(FILELIGHT_LOG) << "propagate: " << url;

        if (parent) {
            parent->folder->append(folder);
            if (parent->stores.isEmpty()) {
                return parent->propagate();
            }
            return parent;
        }

        // we reached the root, let's get our next folder scanned
        return this;
    }

private:
    Q_DISABLE_COPY_MOVE(Store)
};

RemoteLister::RemoteLister(const QUrl &url, ScanManager *manager, QObject *parent)
    : KDirLister(parent)
    , m_root(new Store(url, url.url(), nullptr))
    , m_store(m_root)
    , m_manager(manager)
{
    setShowingDotFiles(true); // Stupid KDirLister API function names

    // Use SIGNAL(result(KIO::Job*)) instead and then use Job::error()
    connect(this, static_cast<void (KCoreDirLister::*)()>(&KCoreDirLister::completed), this, &RemoteLister::onCompleted);
    connect(this, static_cast<void (KCoreDirLister::*)()>(&KCoreDirLister::canceled), this, &RemoteLister::onCanceled);
}

RemoteLister::~RemoteLister()
{
    delete m_root;
}

void RemoteLister::onCanceled()
{
    qCDebug(FILELIGHT_LOG) << "Canceled";
    Q_EMIT branchCompleted(nullptr);
    deleteLater();
}

void RemoteLister::onCompleted()
{
    // m_folder is set to the folder we should operate on

    const KFileItemList items = KDirLister::items();
    for (const auto &item : items) {
        if (item.isDir()) {
            m_store->stores += new Store(item.url(), item.name(), m_store);
        } else {
            m_store->folder->append(item.name().toUtf8().constData(), item.size());
            m_manager->m_totalSize += item.size();
        }

        m_manager->m_files++;
    }

    if (m_store->stores.isEmpty()) {
        // no directories to scan, so we need to append ourselves to the parent folder
        // propagate() will return the next ancestor that has stores left to be
        // scanned, or root if we are done
        Store *newStore = m_store->propagate();
        if (newStore != m_store) {
            // We need to clean up old stores
            delete m_store;
            m_store = newStore;
        }
    }

    if (!m_store->stores.isEmpty()) {
        Store::List::Iterator first = m_store->stores.begin();
        const QUrl url((*first)->url);
        Store *currentStore = m_store;

        // we should operate with this store next time this function is called
        m_store = *first;

        // we don't want to handle this store again
        currentStore->stores.erase(first);

        // this returns _immediately_
        qCDebug(FILELIGHT_LOG) << "scanning: " << url;
        openUrl(url);
    } else {
        qCDebug(FILELIGHT_LOG) << "I think we're done";

        Q_ASSERT(m_root == m_store);
        Q_EMIT branchCompleted(m_store->folder);

        deleteLater();
    }
}
}
