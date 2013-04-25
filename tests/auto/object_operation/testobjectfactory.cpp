#include "testobjectfactory.h"
#include "selflinkedobject.h"
#include <QDebug>

TestObjectFactory::TestObjectFactory()
{
}

EnginioAbstractObject * TestObjectFactory::createObjectForType(
        const QString &objectType,
        const QString &id) const
{
    qDebug() << Q_FUNC_INFO << objectType << id;
    if (objectType == QString("objects.SelfLinkedObject"))
        return new SelfLinkedObject(id);

    // Same factory can be used to create other custom types

    return 0;
}
