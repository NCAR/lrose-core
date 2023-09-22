/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
    treemodel.cpp

    Provides a simple tree model to show how to create and use hierarchical
    models.
*/

#include "ScriptEditorHelpModel.hh"
#include "ScriptEditorHelpItem.hh"

#include <QStringList>

ScriptEditorHelpModel::ScriptEditorHelpModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    rootItem = new ScriptEditorHelpItem({tr("Command"), tr("Description")});
    setupModelData(data.split('\n'), rootItem);
}

ScriptEditorHelpModel::~ScriptEditorHelpModel()
{
    delete rootItem;
}

int ScriptEditorHelpModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<ScriptEditorHelpItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant ScriptEditorHelpModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    ScriptEditorHelpItem *item = static_cast<ScriptEditorHelpItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags ScriptEditorHelpModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant ScriptEditorHelpModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex ScriptEditorHelpModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ScriptEditorHelpItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ScriptEditorHelpItem*>(parent.internalPointer());

    ScriptEditorHelpItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex ScriptEditorHelpModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    ScriptEditorHelpItem *childItem = static_cast<ScriptEditorHelpItem*>(index.internalPointer());
    ScriptEditorHelpItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int ScriptEditorHelpModel::rowCount(const QModelIndex &parent) const
{
    ScriptEditorHelpItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ScriptEditorHelpItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void ScriptEditorHelpModel::setupModelData(const QStringList &lines, ScriptEditorHelpItem *parent)
{
    QVector<ScriptEditorHelpItem*> parents;
    QVector<int> indentations;
    parents << parent;
    indentations << 0;

    int number = 0;

    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].at(position) != ' ')
                break;
            position++;
        }

        const QString lineData = lines[number].mid(position).trimmed();

        if (!lineData.isEmpty()) {
            // Read the column data from the rest of the line.
            // TODO - FIX this
            // const QStringList columnStrings =
            //     lineData.split(QLatin1Char('\t'), Qt::SkipEmptyParts);
            const QStringList columnStrings = lineData.split(QLatin1Char('\t'));
            // default is behavior =  Qt::SkipEmptyParts);
            QVector<QVariant> columnData;
            columnData.reserve(columnStrings.count());
            for (const QString &columnString : columnStrings)
                columnData << columnString;

            if (position > indentations.last()) {
                // The last child of the current parent is now the new parent
                // unless the current parent has no children.

                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Append a new item to the current parent's list of children.
            parents.last()->appendChild(new ScriptEditorHelpItem(columnData, parents.last()));
        }
        ++number;
    }
}

