#include <enginiomodel.h>
#include <imageobject.h>

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H
class ImageModel : public EnginioModel
{
    Q_OBJECT
public:
    ImageModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;

    enum Roles {
        FileId = Qt::UserRole + 1,
        Image,
        FileName,
        FileSize,
        CreationTime
    };

public slots:
    void rowsInserted(const QModelIndex &, int start, int end);
    void reset();
    void imageChanged(const QString &fileId);

private:
    QMap<QString, ImageObject*> m_images;

};

#endif
