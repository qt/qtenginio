#include "imagemodel.h"
#include "qabstractitemmodel.h"
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qvariant.h>
#include <qicon.h>
#include <QtCore/qdatetime.h>

ImageModel::ImageModel(QObject *parent)
    : EnginioModel(parent)
{
    connect(this, SIGNAL(modelReset()),
            this, SLOT(reset()));
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));
}

void ImageModel::reset()
{
    rowsInserted(QModelIndex(), 0, rowCount() - 1);
}

void ImageModel::rowsInserted(const QModelIndex &, int start, int end)
{
    for (int row = start; row <= end; ++row) {
        QJsonValue rowData = EnginioModel::data(index(row)).value<QJsonValue>();
        QString fileId = rowData.toObject().value("file").toObject().value("id").toString();
        if (m_images.contains(fileId))
            continue;
        ImageObject *image = new ImageObject(enginio(), rowData.toObject().value("file").toObject());
        m_images.insert(fileId, image);
        connect(image, SIGNAL(imageChanged(QString)), this, SLOT(imageChanged(QString)));
    }
}

void ImageModel::imageChanged(const QString &fileId)
{
    for (int row = 0; row < rowCount(); ++row) {
        if (data(index(row), FileId) == fileId) {
            QModelIndex changedIndex = index(row);
            emit dataChanged(changedIndex, changedIndex);
        }
    }
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()
            || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    QJsonObject rowData = EnginioModel::data(index).value<QJsonValue>().toObject();

    switch (role) {
    case FileId: {
        return rowData.value("file").toObject().value("id").toString();
    }
    case Qt::DecorationRole: {
        QString fileId = rowData.value("file").toObject().value("id").toString();
        if (m_images.contains(fileId))
            return m_images.value(fileId)->thumbnail();
        return QVariant();
    }
    case Qt::SizeHintRole: {
        QString fileId = rowData.value("file").toObject().value("id").toString();
        if (m_images.contains(fileId))
            return m_images.value(fileId)->thumbnail().size();
        return QVariant();
    }
    case FileName:
        return rowData.value("file").toObject().value("fileName").toString();
    case FileSize:
        return QString::number(rowData.value("file").toObject().value("fileSize").toDouble());
    case CreationTime:
        return QDateTime::fromString(rowData.value("file").toObject().value("createdAt").toString(), Qt::ISODate);
    }

    return QVariant();
}
