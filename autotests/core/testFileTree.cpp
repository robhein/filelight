/***********************************************************************
 * SPDX-FileCopyrightText: 2020 Shubham <aryan100jangid@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 ***********************************************************************/

#include "testFileTree.h"

TestFileTree::TestFileTree()
    : fl(std::make_unique<File>("./autotests/core/dummy.txt", 20))
{
}

void TestFileTree::testFileName()
{
    const QString fname = fl->displayName();
    QCOMPARE(QStringLiteral("./autotests/core/dummy.txt"), fname);
}

void TestFileTree::testFileSize()
{
    const quint64 fsize = fl->size();
    QVERIFY(fsize > 0);
}

void TestFileTree::testFilePath()
{
    auto folder = std::make_shared<Folder>("./autotests/core/");
    const QString fpath = fl->displayPath(folder);
    QVERIFY(!fpath.isEmpty());
}

void TestFileTree::testDuplicate()
{
    auto foo = std::make_shared<Folder>("foo/");
    auto bar = std::make_shared<Folder>("bar/");
    auto file = std::make_shared<Folder>("file/");
    auto light = std::make_shared<Folder>("light/");
    bar->append("onion", 1024);
    light->append("torch", 128);
    file->append(light);
    foo->append(bar);
    foo->append(file);
    foo->append("shallot", 512);
    auto other = foo->duplicate();
    qDebug() << other->size();
    QCOMPARE(other->size(), foo->size());
    QCOMPARE(other->children(), foo->children());
}

QTEST_MAIN(TestFileTree)
