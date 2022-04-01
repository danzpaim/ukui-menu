#include "buriedpointdatasend.h"
#include <QHostInfo>
#include <QString>

Q_GLOBAL_STATIC(BuriedPointDataSend, buriedPointDataSend)

BuriedPointDataSend::BuriedPointDataSend()
{
    QString path = QDir::homePath() + "/menuUploadMessage/";
    m_sendDataInterface = new UploadMessageInterface(getgid(), "ukui-menu", "menuData", path);
}

BuriedPointDataSend::~BuriedPointDataSend()
{
    if (m_sendDataInterface != nullptr) {
        delete  m_sendDataInterface;
        m_sendDataInterface = nullptr;
    }
}

BuriedPointDataSend *BuriedPointDataSend::getInstance()
{
    return buriedPointDataSend();
}

QString BuriedPointDataSend::getCurrentTime()
{
    QString tempDateTimeStr;
    QDateTime tempDateTime = QDateTime::currentDateTime().toTimeZone(QTimeZone(8 * 3600));
    tempDateTimeStr = tempDateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    return tempDateTimeStr;
}

void BuriedPointDataSend::setPoint(const pointDataStruct &data)
{
    int curNum = 1;
    QStringList applist = QStringList();
    QString keyValue = data.module + data.function;

    if (m_functionCount.keys().contains(keyValue)) {
        curNum = m_functionCount.value(keyValue);
        curNum ++;
    }

    m_functionCount.insert(keyValue, curNum);

    if (!data.otherFunction.isEmpty()) {
        if (m_applist.keys().contains(keyValue)) {
            applist = m_applist.value(keyValue);
        }

        applist.append(data.otherFunction.at(0));
        m_applist.insert(keyValue, applist);
    }

    QString time = getCurrentTime();
    QJsonObject jsonObj;
    QJsonArray otherFunction = QJsonArray::fromStringList(data.otherFunction);
    jsonObj.insert("module", QJsonValue(data.module));
    jsonObj.insert("function", QJsonValue(data.function));
    jsonObj.insert("functionNum", QJsonValue(QString::number(curNum)));
    jsonObj.insert("otherFunction", otherFunction);
    jsonObj.insert("errorLevel", QJsonValue(data.errorLevel));
    jsonObj.insert("errorOutput", QJsonValue(data.errorOutput));
    jsonObj.insert("timeStamp", time);
    //  将数据转化为QString
    QString informationData(QJsonDocument(jsonObj).toJson(QJsonDocument::Compact));
    m_sendDataInterface->UploadMessage(informationData);
}


