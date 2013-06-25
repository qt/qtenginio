#include "todosmodel.h"

#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonobject.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>

#include <Enginio/enginioreply.h>

TodosModel::TodosModel(QObject *parent)
    : EnginioModel(parent)
    , TitleRole()
    , DoneRole()
{
    QObject::connect(this, &EnginioModel::modelReset, this, &TodosModel::updateRoles);
}

QVariant TodosModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
        return EnginioModel::data(index, TitleRole).value<QJsonValue>().toString();

    if (role == Qt::FontRole) {
        bool completed = EnginioModel::data(index, DoneRole).value<QJsonValue>().toBool();
        QFont font;
        font.setPointSize(20);
        font.setStrikeOut(completed);
        return font;
    }

    if (role == Qt::TextColorRole) {
        bool completed = EnginioModel::data(index, DoneRole).value<QJsonValue>().toBool();
        return completed ? QColor("#999") : QColor("#333");
    }

    if (role == DoneRole)
        return EnginioModel::data(index, DoneRole).value<QJsonValue>().toBool();

    if (role == TitleRole)
        return EnginioModel::data(index, TitleRole).value<QJsonValue>().toString();

    return EnginioModel::data(index, role);
}

bool TodosModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        EnginioReply *reply = setProperty(index.row(), "title", value);
        QObject::connect(reply, &EnginioReply::finished, reply, &EnginioReply::deleteLater);
        return true;
    }
    return false;
}

void TodosModel::updateRoles()
{
    static QByteArray titleRoleName = QByteArrayLiteral("title");
    static QByteArray doneRoleName = QByteArrayLiteral("completed");
    QHash<int, QByteArray> roleNames = EnginioModel::roleNames();
    foreach(int role, roleNames.keys()) {
        QByteArray roleName = roleNames.value(role);
        if (roleName == titleRoleName) {
            TitleRole = role;
        } else if (roleName == doneRoleName) {
            DoneRole = role;
        }
    }
}
