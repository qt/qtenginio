#ifndef TESTOBJECTFACTORY_H
#define TESTOBJECTFACTORY_H

#include "enginioabstractobjectfactory.h"

class TestObjectFactory : public EnginioAbstractObjectFactory
{
public:
    TestObjectFactory();
    EnginioAbstractObject * createObjectForType(const QString &objectType,
                                                const QString &id) const;
};

#endif // TESTOBJECTFACTORY_H
