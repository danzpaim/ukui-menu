/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "ukuimenuinterface.h"
#include <QDir>
#include <QDebug>
#include <QCollator>
#include <QLocale>
#include <QStringList>
#include <QJsonArray>
#include <QJsonObject>
#include <unistd.h>
#include "ukui_chineseletter.h"

UkuiMenuInterface::UkuiMenuInterface()
{
    if (!g_projectCodeName.contains("V10SP1-edu")) {
    } else {
        QString path = QDir::homePath() + "/.config/ukui/ukui-menu.ini";
        setting = new QSettings(path, QSettings::IniFormat);
        QString syspath = QDir::homePath() + "/.config/ukui/menusysapplist.ini";
        syssetting = new QSettings(syspath, QSettings::IniFormat);
    }
}

QVector<QStringList> UkuiMenuInterface::appInfoVector = QVector<QStringList>();
QVector<QString> UkuiMenuInterface::desktopfpVector = QVector<QString>();
QVector<QString> UkuiMenuInterface::collectAppVector = QVector<QString>();
QVector<QStringList> UkuiMenuInterface::alphabeticVector = QVector<QStringList>();
QVector<QStringList> UkuiMenuInterface::functionalVector = QVector<QStringList>();
QVector<QString> UkuiMenuInterface::allAppVector = QVector<QString>();
QStringList UkuiMenuInterface::androidDesktopfnList = QStringList();
QVector<QString> UkuiMenuInterface::tencentInitVector = QVector<QString>();
QVector<QString> UkuiMenuInterface::customizedVector = QVector<QString>();
QVector<QString> UkuiMenuInterface::thirdPartyVector = QVector<QString>();
QVector<QString> UkuiMenuInterface::applicationVector = QVector<QString>();

UkuiMenuInterface::~UkuiMenuInterface()
{
    if (!g_projectCodeName.contains("V10SP1-edu")) {
    } else {
        if (setting) {
            delete setting;
        }

        if (syssetting) {
            delete syssetting;
        }

        setting = nullptr;
        syssetting = nullptr;
    }
}

QStringList UkuiMenuInterface::getFunctionClassName()
{
    QStringList functionList;
    functionList.append(QObject::tr("Office"));
    functionList.append(QObject::tr("Development"));
    functionList.append(QObject::tr("Image"));
    functionList.append(QObject::tr("Video"));
    functionList.append(QObject::tr("Internet"));
    functionList.append(QObject::tr("Game"));
    functionList.append(QObject::tr("Education"));
    functionList.append(QObject::tr("Social"));
    functionList.append(QObject::tr("System"));
    functionList.append(QObject::tr("Safe"));
    functionList.append(QObject::tr("Others"));
    return functionList;
}
//文件递归查询
void UkuiMenuInterface::recursiveSearchFile(const QString &_filePath)
{
    if (!g_projectCodeName.contains("V10SP1-edu")) {
        QDir dir(_filePath);

        if (!dir.exists()) {
            return;
        }

        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::DirsFirst);
        QFileInfoList list = dir.entryInfoList();

        if (list.size() < 1) {
            return;
        }

        int i = 0;

        //递归算法的核心部分
        do {
            QFileInfo fileInfo = list.at(i);
            //如果是文件夹，递归
            bool isDir = fileInfo.isDir();

            if (isDir) {
                recursiveSearchFile(fileInfo.filePath());
            } else {
                //过滤后缀不是.desktop的文件
                QString filePathStr = fileInfo.filePath();

                if (!filePathStr.endsWith(".desktop")) {
                    i++;
                    continue;
                }

                QByteArray fpbyte = filePathStr.toLocal8Bit();
                char *filepath = fpbyte.data();

                if (0 != access(filepath, R_OK)) { //判断文件是否可读
                    i++;
                    continue;
                }

                keyfile = g_key_file_new();

                if (!g_key_file_load_from_file(keyfile, filepath, flags, error)) {
                    return;
                }

                char *ret_1 = g_key_file_get_locale_string(keyfile, "Desktop Entry", "NoDisplay", nullptr, nullptr);

                if (ret_1 != nullptr) {
                    QString str = QString::fromLocal8Bit(ret_1);

                    if (str.contains("true")) {
                        g_key_file_free(keyfile);
                        i++;
                        continue;
                    }
                }

                char *ret_2 = g_key_file_get_locale_string(keyfile, "Desktop Entry", "NotShowIn", nullptr, nullptr);

                if (ret_2 != nullptr) {
                    QString str = QString::fromLocal8Bit(ret_2);

                    if (str.contains("UKUI")) {
                        g_key_file_free(keyfile);
                        i++;
                        continue;
                    }
                }

                //过滤LXQt、KDE
                char *ret = g_key_file_get_locale_string(keyfile, "Desktop Entry", "OnlyShowIn", nullptr, nullptr);

                if (ret != nullptr) {
                    QString str = QString::fromLocal8Bit(ret);

                    if (str.contains("LXQt") || str.contains("KDE")) {
                        g_key_file_free(keyfile);
                        i++;
                        continue;
                    }
                }

                g_key_file_free(keyfile);
                m_filePathList.append(filePathStr);
            }

            i++;
        } while (i < list.size());
    } else {
        GError **error = nullptr;
        GKeyFileFlags flags = G_KEY_FILE_NONE;
        GKeyFile *keyfile = g_key_file_new();
        QDir dir(_filePath);

        if (!dir.exists()) {
            return;
        }

        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::DirsFirst);
        QFileInfoList list = dir.entryInfoList();
        list.removeAll(QFileInfo("/usr/share/applications/screensavers"));

        if (list.size() < 1) {
            return;
        }

        int i = 0;

        //递归算法的核心部分
        do {
            QFileInfo fileInfo = list.at(i);
            //如果是文件夹，递归
            bool isDir = fileInfo.isDir();

            if (isDir) {
                recursiveSearchFile(fileInfo.filePath());
            } else {
                //过滤后缀不是.desktop的文件
                QString filePathStr = fileInfo.filePath();

                if (!filePathStr.endsWith(".desktop")) {
                    i++;
                    continue;
                }

                QByteArray fpbyte = filePathStr.toLocal8Bit();
                char *filepath = fpbyte.data();
                g_key_file_load_from_file(keyfile, filepath, flags, error);
                char *ret_1 = g_key_file_get_locale_string(keyfile, "Desktop Entry", "NoDisplay", nullptr, nullptr);

                if (ret_1 != nullptr) {
                    QString str = QString::fromLocal8Bit(ret_1);

                    if (str.contains("true")) {
                        i++;
                        continue;
                    }
                }

                char *ret_2 = g_key_file_get_locale_string(keyfile, "Desktop Entry", "NotShowIn", nullptr, nullptr);

                if (ret_2 != nullptr) {
                    QString str = QString::fromLocal8Bit(ret_2);

                    if (str.contains("UKUI")) {
                        i++;
                        continue;
                    }
                }

                //过滤LXQt、KDE
                char *ret = g_key_file_get_locale_string(keyfile, "Desktop Entry", "OnlyShowIn", nullptr, nullptr);

                if (ret != nullptr) {
                    QString str = QString::fromLocal8Bit(ret);

                    if (str.contains("LXQt") || str.contains("KDE")) {
                        i++;
                        continue;
                    }
                }

                //过滤中英文名为空的情况
                QLocale cn;
                QString language = cn.languageToString(cn.language());

                if (QString::compare(language, "Chinese") == 0) {
                    char *nameCh = g_key_file_get_string(keyfile, "Desktop Entry", "Name[zh_CN]", nullptr);
                    char *nameEn = g_key_file_get_string(keyfile, "Desktop Entry", "Name", nullptr);

                    if (QString::fromLocal8Bit(nameCh).isEmpty() && QString::fromLocal8Bit(nameEn).isEmpty()) {
                        i++;
                        continue;
                    }
                } else {
                    char *name = g_key_file_get_string(keyfile, "Desktop Entry", "Name", nullptr);

                    if (QString::fromLocal8Bit(name).isEmpty()) {
                        i++;
                        continue;
                    }
                }

                m_filePathList.append(filePathStr);
            }

            i++;
        } while (i < list.size());

        g_key_file_free(keyfile);
    }
}

//获取系统desktop文件路径
QStringList UkuiMenuInterface::getDesktopFilePath()
{
    if (!g_projectCodeName.contains("V10SP1-edu")) {
        m_filePathList.clear();
        QString jsonPath = QDir::homePath() + "/.config/ukui-menu-security-config.json";
        QFile file(jsonPath);

        if (file.exists()) {
            file.open(QIODevice::ReadOnly);
            QByteArray readBy = file.readAll();
            QJsonParseError error;
            QJsonDocument readDoc = QJsonDocument::fromJson(readBy, &error);

            if (!readDoc.isNull() && error.error == QJsonParseError::NoError) {
                QJsonObject obj = readDoc.object().value("ukui-menu").toObject();

                if (obj.value("mode").toString() == "whitelist") {
                    QJsonArray blArray = obj.value("whitelist").toArray();
                    QJsonArray enArray = blArray.at(0).toObject().value("entries").toArray();

                    for (int index = 0; index < enArray.size(); index++) {
                        QJsonObject obj = enArray.at(index).toObject();
                        m_filePathList.append(obj.value("path").toString());
                        //                    qDebug()<<obj.value("path").toString();
                    }

                    return m_filePathList;
                } else if (obj.value("mode").toString() == "blacklist") {
                    getAndroidApp();
                    recursiveSearchFile("/usr/share/applications/");
                    recursiveSearchFile("/var/lib/snapd/desktop/applications/");
                    recursiveSearchFile("/var/lib/flatpak/exports/share/applications/");
                    QJsonArray blArray = obj.value("blacklist").toArray();
                    QJsonArray enArray = blArray.at(0).toObject().value("entries").toArray();

                    for (int index = 0; index < enArray.size(); index++) {
                        QJsonObject obj = enArray.at(index).toObject();
                        m_filePathList.removeAll(obj.value("path").toString());
                        //                    qDebug()<<obj.value("path").toString();
                    }
                } else {
                    getAndroidApp();
                    recursiveSearchFile("/usr/share/applications/");
                    recursiveSearchFile("/var/lib/snapd/desktop/applications/");
                    recursiveSearchFile("/var/lib/flatpak/exports/share/applications/");
                }
            }

            file.close();
        } else {
            getAndroidApp();
            recursiveSearchFile("/usr/share/applications/");
            recursiveSearchFile("/var/lib/snapd/desktop/applications/");
            recursiveSearchFile("/var/lib/flatpak/exports/share/applications/");
        }

        m_filePathList.removeAll("/usr/share/applications/software-properties-livepatch.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-color-select.desktop");
        m_filePathList.removeAll("/usr/share/applications/blueman-adapters.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-user-guide.desktop");
        m_filePathList.removeAll("/usr/share/applications/nm-connection-editor.desktop");
        m_filePathList.removeAll("/usr/share/applications/debian-uxterm.desktop");
        m_filePathList.removeAll("/usr/share/applications/debian-xterm.desktop");
        m_filePathList.removeAll("/usr/share/applications/fcitx.desktop");
        m_filePathList.removeAll("/usr/share/applications/fcitx-configtool.desktop");
        m_filePathList.removeAll("/usr/share/applications/onboard-settings.desktop");
        m_filePathList.removeAll("/usr/share/applications/info.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-power-preferences.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-power-statistics.desktop");
        m_filePathList.removeAll("/usr/share/applications/software-properties-drivers.desktop");
        m_filePathList.removeAll("/usr/share/applications/software-properties-gtk.desktop");
        m_filePathList.removeAll("/usr/share/applications/gnome-session-properties.desktop");
        m_filePathList.removeAll("/usr/share/applications/org.gnome.font-viewer.desktop");
        m_filePathList.removeAll("/usr/share/applications/xdiagnose.desktop");
        m_filePathList.removeAll("/usr/share/applications/gnome-language-selector.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-notification-properties.desktop");
        m_filePathList.removeAll("/usr/share/applications/transmission-gtk.desktop");
        m_filePathList.removeAll("/usr/share/applications/mpv.desktop");
        m_filePathList.removeAll("/usr/share/applications/system-config-printer.desktop");
        m_filePathList.removeAll("/usr/share/applications/org.gnome.DejaDup.desktop");
        m_filePathList.removeAll("/usr/share/applications/yelp.desktop");
        //v10
        m_filePathList.removeAll("/usr/share/applications/time.desktop");
        m_filePathList.removeAll("/usr/share/applications/network.desktop");
        m_filePathList.removeAll("/usr/share/applications/shares.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-power-statistics.desktop");
        m_filePathList.removeAll("/usr/share/applications/display-im6.desktop");
        m_filePathList.removeAll("/usr/share/applications/display-im6.q16.desktop");
        m_filePathList.removeAll("/usr/share/applications/openjdk-8-policytool.desktop");
        m_filePathList.removeAll("/usr/share/applications/kylin-io-monitor.desktop");
        m_filePathList.removeAll("/usr/share/applications/wps-office-uninstall.desktop");
        m_filePathList.removeAll("/usr/share/applications/wps-office-misc.desktop");
        m_filePathList.removeAll("/usr/share/applications/kylin-installer.desktop");
        QStringList desktopList;

        for (int i = 0; i < m_filePathList.count(); ++i) {
            QString filepath = m_filePathList.at(i);
            int list_index = filepath.lastIndexOf('/');
            QString desktopName = filepath.right(filepath.length() - list_index - 1);

            if (desktopList.contains(desktopName)) {
                m_filePathList.removeAll(filepath);
                i--;
            } else {
                desktopList.append(desktopName);
            }
        }

        return m_filePathList;
    } else {
        m_filePathList.clear();
        getAndroidApp();
        recursiveSearchFile("/usr/share/applications/");
        m_filePathList.removeAll("/usr/share/applications/software-properties-livepatch.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-color-select.desktop");
        m_filePathList.removeAll("/usr/share/applications/blueman-adapters.desktop");
        m_filePathList.removeAll("/usr/share/applications/blueman-manager.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-user-guide.desktop");
        m_filePathList.removeAll("/usr/share/applications/nm-connection-editor.desktop");
        m_filePathList.removeAll("/usr/share/applications/debian-uxterm.desktop");
        m_filePathList.removeAll("/usr/share/applications/debian-xterm.desktop");
        m_filePathList.removeAll("/usr/share/applications/im-config.desktop");
        m_filePathList.removeAll("/usr/share/applications/fcitx.desktop");
        m_filePathList.removeAll("/usr/share/applications/fcitx-configtool.desktop");
        m_filePathList.removeAll("/usr/share/applications/onboard-settings.desktop");
        m_filePathList.removeAll("/usr/share/applications/info.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-power-preferences.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-power-statistics.desktop");
        m_filePathList.removeAll("/usr/share/applications/software-properties-drivers.desktop");
        m_filePathList.removeAll("/usr/share/applications/software-properties-gtk.desktop");
        m_filePathList.removeAll("/usr/share/applications/gnome-session-properties.desktop");
        m_filePathList.removeAll("/usr/share/applications/org.gnome.font-viewer.desktop");
        m_filePathList.removeAll("/usr/share/applications/xdiagnose.desktop");
        m_filePathList.removeAll("/usr/share/applications/gnome-language-selector.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-notification-properties.desktop");
        m_filePathList.removeAll("/usr/share/applications/transmission-gtk.desktop");
        m_filePathList.removeAll("/usr/share/applications/mpv.desktop");
        m_filePathList.removeAll("/usr/share/applications/system-config-printer.desktop");
        m_filePathList.removeAll("/usr/share/applications/org.gnome.DejaDup.desktop");
        m_filePathList.removeAll("/usr/share/applications/yelp.desktop");
        m_filePathList.removeAll("/usr/share/applications/peony-computer.desktop");
        m_filePathList.removeAll("/usr/share/applications/peony-home.desktop");
        m_filePathList.removeAll("/usr/share/applications/peony-trash.desktop");
        //filePathList.removeAll("/usr/share/applications/peony.desktop");
        //*过滤*//
        m_filePathList.removeAll("/usr/share/applications/recoll-searchgui.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-about.desktop");
        m_filePathList.removeAll("/usr/share/applications/org.gnome.dfeet.desktop");
        m_filePathList.removeAll("/usr/share/applications/ukui-feedback.desktop");
        m_filePathList.removeAll("/usr/share/applications/users.desktop");
        //    filePathList.removeAll("/usr/share/applications/mate-terminal.desktop");
        m_filePathList.removeAll("/usr/share/applications/vim.desktop");
        m_filePathList.removeAll("/usr/share/applications/mpv.desktop");
        m_filePathList.removeAll("/usr/share/applications/engrampa.desktop");
        m_filePathList.removeAll("/usr/share/applications/hp-document.desktop");
        //    filePathList.removeAll("/usr/share/applications/kylin-user-guide.desktop");
        m_filePathList.removeAll("/usr/share/applications/wps-office-prometheus.desktop");
        //    filePathList.removeAll("/usr/share/applications/indicator-china-weather.desktop");    //禁用麒麟天气
        //v10
        m_filePathList.removeAll("/usr/share/applications/mate-about.desktop");
        m_filePathList.removeAll("/usr/share/applications/time.desktop");
        m_filePathList.removeAll("/usr/share/applications/network.desktop");
        m_filePathList.removeAll("/usr/share/applications/shares.desktop");
        m_filePathList.removeAll("/usr/share/applications/mate-power-statistics.desktop");
        m_filePathList.removeAll("/usr/share/applications/display-im6.desktop");
        m_filePathList.removeAll("/usr/share/applications/display-im6.q16.desktop");
        m_filePathList.removeAll("/usr/share/applications/openjdk-8-policytool.desktop");
        m_filePathList.removeAll("/usr/share/applications/kylin-io-monitor.desktop");
        m_filePathList.removeAll("/usr/share/applications/wps-office-uninstall.desktop");
        m_filePathList.removeAll("/usr/share/applications/wps-office-misc.desktop");
        /*加入的*/
        m_filePathList.append("/usr/share/applications/mate-calc.desktop");
        m_filePathList = getInstalledAppList();
        return m_filePathList;
    }
}

QStringList UkuiMenuInterface::getInstalledAppList()   //获取已安装应用列表
{
    /*显示的应用列表*/
    //1、系统默认应用
    //2、腾讯应用与安装的系统应用
    //3、考虑用户隔离
    //应用列表的所有应用由：系统应用+应用商店安装了的应用
    //    QDBusInterface desktopfpListiface("cn.kylinos.SSOBackend",
    //                             "/cn/kylinos/SSOBackend",
    //                             "cn.kylinos.SSOBackend.applications",
    //                             QDBusConnection::systemBus());
    //    QString username=getUserName();
    //    QDBusReply<QString> reply = desktopfpListiface.call("GetDesktopAppList",username);
    //1、获取系统应用列表
    //filePathList;
    QStringList ifFileDesktopList;
    /*新的应用列表*/
    myDebug() << "sysapplistnum初始化默认应用列表" << m_filePathList.count();

    for (int i = 0; i < m_filePathList.count(); i++) { //过滤 得到真实存在的应用
        QString tmp = m_filePathList.at(i);
        QFileInfo fileInfo(tmp);

        if (!fileInfo.isFile()) { //判断是否存在
            //qDebug()<<tmp;
            continue;
        }

        ifFileDesktopList.append(tmp);
    }

    //ifFileDesktopList  所有当前存在的应用
    /*得到系统默认应用*/
    //判断当前是否已经得到了默认应用
    syssetting->beginGroup("ukui-menu-sysapplist");
    int sysapplistnum = syssetting->allKeys().count();
    syssetting->sync();
    syssetting->endGroup();
    //qDebug()<<"sysapplistnum初始化默认应用列表3"<<ifFileDesktopList.count();
    int num = ifFileDesktopList.count();

    for (int i = 0; i < num; i++) {
        QString tmp = ifFileDesktopList.at(i);
        QString str = ifFileDesktopList.at(i);
        QStringList list = str.split('/');
        str = list[list.size() - 1];

        if (tmp.indexOf("tencent") == -1) { //所有不是腾讯的系统应用7
            if (sysapplistnum == 0) { //没有初始化默认应用列表
                syssetting->beginGroup("ukui-menu-sysapplist");
                myDebug() << "isnottencent" << str;
                syssetting->setValue(str, 0);
                syssetting->sync();
                syssetting->endGroup();
            }
        }/*else{

            //用户隔离/etc/skel/桌面
            QString tmp=QString("%1%2").arg("/usr/share/applications/").arg(str);
            QString appid=getTencentAppid(tmp);
            qDebug()<<"appid"<<tmp<<appid;

            if(reply.value().indexOf(appid)!=-1)
            {
                qDebug()<<"当前用户可见的腾讯应用"<<tmp;
                //ifFileDesktopList.append(QString("%1%2").arg("/usr/share/applications/").arg(str));//tmp
            }else{
                ifFileDesktopList.removeAll(tmp);
            }
        }*/
    }

    m_filePathList.clear();
    return ifFileDesktopList;
}
//创建应用信息容器(intel SP1共用)
QVector<QStringList> UkuiMenuInterface::createAppInfoVector()
{
    desktopfpVector.clear();
    QVector<QStringList> appInfoVector;
    QVector<QStringList> vector;
    vector.append(QStringList() << "office" << "Office" << "Calculator" << "Spreadsheet" << "Presentation" << "WordProcessor" << "TextEditor"); //0办公
    vector.append(QStringList() << "develop" << "Development"); //1开发
    vector.append(QStringList() << "graphic" << "Graphics"); //2图像
    vector.append(QStringList() << "video" << "Audio" << "Video"); //3影音
    vector.append(QStringList() << "network" << "Network"); //4网络
    vector.append(QStringList() << "game" << "Game"); //5游戏
    vector.append(QStringList() << "education" << "Education"); //6教育
    vector.append(QStringList() << "social" << "Messaging"); //7社交
    vector.append(QStringList() << "system" << "System" << "Settings" << "Security"); //8系统
    vector.append(QStringList() << "safe"); //9安全
    vector.append(QStringList() << "others"); //10其他
    QStringList desktopfpList = getDesktopFilePath();
    QSqlDatabase db = QSqlDatabase::database("MainThreadDataBase");
    QSqlQuery sql(db);

    for (int i = 0; i < desktopfpList.count(); i++) {
        QStringList appInfoList;
        QString desktopfp = desktopfpList.at(i);
        QString name = getAppName(desktopfpList.at(i));

        if (!name.isEmpty()) {
            QString englishName = getAppEnglishName(desktopfpList.at(i));
            QString letter = getAppNameInitial(desktopfpList.at(i));
            QString letters = getAppNameInitials(desktopfpList.at(i));
            desktopfpVector.append(desktopfp);
            appInfoList << desktopfp << name << englishName << letter << letters;

            if (!g_projectCodeName.contains("V10SP1-edu")) {
                QString desktopfpExecName = getAppExec(desktopfpList.at(i));
                desktopfpExecName = desktopfpExecName.mid(desktopfpExecName.lastIndexOf("/") + 1);
                desktopfpExecName = desktopfpExecName.left(desktopfpExecName.lastIndexOf(" "));
                sql.exec(QString("select name_zh from appCategory where app_name=\"%1\" ").arg(desktopfpExecName));

                if (sql.next()) {
                    for (int j = 0; j < vector.size(); j++) {
                        if (vector.at(j).contains(sql.value(0).toString())) {
                            appInfoList.append(QString::number(j));
                        }
                    }

                    appInfoVector.append(appInfoList);
                    continue;
                }
            }

            bool is_owned = false;

            for (int j = 0; j < vector.size(); j++) {
                if (matchingAppCategories(desktopfpList.at(i), vector.at(j))) { //有对应分类
                    is_owned = true;
                    appInfoList.append(QString::number(j));
                }
            }

            if (!is_owned) { //该应用无对应分类
                appInfoList.append(QString::number(10));
            }

            appInfoVector.append(appInfoList);
        }
    }

    return appInfoVector;
}
//获取tencent应用名
QString UkuiMenuInterface::getTencentAppid(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *Appid = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Appid", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(Appid);
}
//获取应用名称
QString UkuiMenuInterface::getAppName(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *name = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Name", nullptr, nullptr);
    QString namestr = QString::fromLocal8Bit(name);
    g_key_file_free(keyfile);
    return namestr;
}
//获取英应用英文名
QString UkuiMenuInterface::getAppEnglishName(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *name = g_key_file_get_string(keyfile, "Desktop Entry", "Name", nullptr);
    QString namestr = QString::fromLocal8Bit(name);
    return namestr;
}
//获取应用分类
QString UkuiMenuInterface::getAppCategories(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *category = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Categories", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(category);
}
//获取应用图标
QString UkuiMenuInterface::getAppIcon(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *icon = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Icon", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(icon);
}
//获取应用命令
QString UkuiMenuInterface::getAppExec(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *exec = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Exec", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(exec);
}
//获取应用注释
QString UkuiMenuInterface::getAppComment(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *comment = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Comment", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(comment);
}
//获取应用类型
QString UkuiMenuInterface::getAppType(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *type = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Type", nullptr, nullptr);
    g_key_file_free(keyfile);
    return QString::fromLocal8Bit(type);
}
bool UkuiMenuInterface::cmpApp(QStringList &arg_1, QStringList &arg_2)
{
    QLocale local;
    QString language = local.languageToString(local.language());

    if (QString::compare(language, "Chinese") == 0) {
        local = QLocale(QLocale::Chinese);
    } else {
        local = QLocale(QLocale::English);
    }

    QCollator collator(local);

    if (collator.compare(arg_1.at(1), arg_2.at(1)) < 0) {
        return true;
    } else {
        return false;
    }
}
bool UkuiMenuInterface::initAppIni()
{
    if (false) {
        QVector<QStringList> appInitVector;
        QVector<QStringList> tencentInitVectorList;
        QVector<QStringList> customizedVectorList;
        QVector<QStringList> thirdPartyVectorList;
        QString tencent_math = "/usr/share/applications/tencent-math-precise-practice.desktop";
        QString tencent_chinese = "/usr/share/applications/tencent-chinese-precise-practice.desktop";
        QString tencent_english = "/usr/share/applications/tencent-english-precise-practice.desktop";
        QVector<QStringList> precise_practiceVector;
        QStringList math;
        QStringList english;
        QStringList chainese;
        setting->beginGroup("application");
        QStringList desktopfnList = setting->allKeys();
        setting->endGroup();

        if (desktopfnList.count() == 0) {
            for (int i = 0; i < appInfoVector.count(); i++) {
                //qDebug()<<"appInfoVector"<<appInfoVector.count()<<i;
                QString tmp = appInfoVector.at(i).at(0);

                if (tmp.indexOf("tencent") != -1) {
                    if (tmp == tencent_math) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        math = appInfoVector.at(i);
                        continue;
                    }

                    if (tmp == tencent_chinese) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        chainese = appInfoVector.at(i);
                        continue;
                    }

                    if (tmp == tencent_english) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        english = appInfoVector.at(i);
                        continue;
                    }

                    tencentInitVectorList.append(appInfoVector.at(i));//所有是腾讯的系统应用
                } else if (tmp.indexOf("wps") != -1) {
                    thirdPartyVectorList.append(appInfoVector.at(i));
                } else if (tmp.indexOf("eye") != -1) {
                    customizedVectorList.append(appInfoVector.at(i));
                } else if (tmp.indexOf("mdm") != -1) {
                    customizedVectorList.append(appInfoVector.at(i));
                } else {
                    appInitVector.append(appInfoVector.at(i));
                }
            }

            qSort(appInitVector.begin(), appInitVector.end(), cmpApp); //按中英文字母排序
            qSort(tencentInitVectorList.begin(), tencentInitVectorList.end(), cmpApp); //按中英文字母排序

            //腾讯应用的精准类应用处理
            if (precise_practiceVector.contains(english)) {
                tencentInitVectorList.insert(0, english);
            }

            if (precise_practiceVector.contains(math)) {
                tencentInitVectorList.insert(0, math);
            }

            if (precise_practiceVector.contains(chainese)) {
                tencentInitVectorList.insert(0, chainese);
            }

            qSort(customizedVectorList.begin(), customizedVectorList.end(), cmpApp); //按中英文字母排序
            qSort(thirdPartyVectorList.begin(), thirdPartyVectorList.end(), cmpApp); //按中英文字母排序
            setting->beginGroup("tencent");

            for (int i = 0; i < tencentInitVectorList.count(); i++) {
                QString str = tencentInitVectorList.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                //qDebug()<<str;
                setting->setValue(str, i);
            }

            setting->sync();
            setting->endGroup();
            //        int a=tencentInitVectorList.count();
            setting->beginGroup("customized");

            for (int i = 0; i < customizedVectorList.count(); i++) {
                QString str = customizedVectorList.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i);
            }

            setting->sync();
            setting->endGroup();
            //        int b=customizedVector.count();
            setting->beginGroup("thirdParty");

            for (int i = 0; i < thirdPartyVectorList.count(); i++) {
                QString str = thirdPartyVectorList.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i);
            }

            setting->sync();
            setting->endGroup();
            //        int c=thirdPartyVectorList.count();
            setting->beginGroup("application");

            for (int i = 0; i < appInitVector.count(); i++) { //赋值
                QString str = appInitVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i);
            }

            setting->sync();
            setting->endGroup();
            return 1;
        }

        return 0;
    } else {
        QVector<QStringList> appInitVector;
        QVector<QStringList> tencentInitVector;
        QVector<QStringList> customizedVector;
        QVector<QStringList> thirdPartyVector;
        QVector<QStringList> preorderAppVector;
        QString tencent_math = "/usr/share/applications/tencent-math-precise-practice.desktop";
        QString tencent_chinese = "/usr/share/applications/tencent-chinese-precise-practice.desktop";
        QString tencent_english = "/usr/share/applications/tencent-english-precise-practice.desktop";
        QString smallPluginManage = "/usr/share/applications/small-plugin-manage.desktop";
        QVector<QStringList> precise_practiceVector;
        QStringList math;
        QStringList english;
        QStringList chainese;
        setting->beginGroup("application");
        QStringList desktopfnList = setting->allKeys();
        setting->endGroup();

        if (desktopfnList.count() == 0) {
            for (int i = 0; i < desktopfpVector.count(); i++) {
                //qDebug()<<"appInfoVector"<<appInfoVector.count()<<i;
                QString tmp = appInfoVector.at(i).at(0);

                if (tmp.indexOf("tencent") != -1) {
                    if (tmp == tencent_math) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        math = appInfoVector.at(i);
                        continue;
                    }

                    if (tmp == tencent_chinese) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        chainese = appInfoVector.at(i);
                        continue;
                    }

                    if (tmp == tencent_english) {
                        precise_practiceVector.append(appInfoVector.at(i));
                        english = appInfoVector.at(i);
                        continue;
                    }

                    tencentInitVector.append(appInfoVector.at(i));//所有是腾讯的系统应用
                } else if (tmp.indexOf("wps") != -1) {
                    thirdPartyVector.append(appInfoVector.at(i));
                } else if (tmp.indexOf("eye") != -1) {
                    customizedVector.append(appInfoVector.at(i));
                } else if (tmp.indexOf("mdm") != -1) {
                    customizedVector.append(appInfoVector.at(i));
                } else if (tmp.contains(smallPluginManage)) {
                    preorderAppVector.append(appInfoVector.at(i));
                } else {
                    appInitVector.append(appInfoVector.at(i));
                }
            }

            qSort(appInitVector.begin(), appInitVector.end(), cmpApp); //按中英文字母排序
            qSort(tencentInitVector.begin(), tencentInitVector.end(), cmpApp); //按中英文字母排序

            //腾讯应用的精准类应用处理
            if (precise_practiceVector.contains(english)) {
                tencentInitVector.insert(0, english);
            }

            if (precise_practiceVector.contains(math)) {
                tencentInitVector.insert(0, math);
            }

            if (precise_practiceVector.contains(chainese)) {
                tencentInitVector.insert(0, chainese);
            }

            qSort(customizedVector.begin(), customizedVector.end(), cmpApp); //按中英文字母排序
            qSort(thirdPartyVector.begin(), thirdPartyVector.end(), cmpApp); //按中英文字母排序
            setting->beginGroup("application");

            for (int i = 0; i < preorderAppVector.count(); i++) {
                QString str = preorderAppVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                //qDebug()<<str;
                setting->setValue(str, i);
            }

            int firstVectorCount = preorderAppVector.count();

            for (int i = 0; i < tencentInitVector.count(); i++) {
                QString str = tencentInitVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                //qDebug()<<str;
                setting->setValue(str, i + firstVectorCount);
            }

            int a = tencentInitVector.count();

            for (int i = 0; i < customizedVector.count(); i++) {
                QString str = customizedVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i + a);
            }

            int b = customizedVector.count();

            for (int i = 0; i < thirdPartyVector.count(); i++) {
                QString str = thirdPartyVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i + a + b);
            }

            int c = thirdPartyVector.count();

            for (int i = 0; i < appInitVector.count(); i++) { //赋值
                QString str = appInitVector.at(i).at(0).section(' ', 0, 0);
                QStringList list = str.split('/');
                str = list[list.size() - 1];
                setting->setValue(str, i + a + b + c);
            }

            int d = appInitVector.count();
            setting->setValue("kylin-user-guide.desktop", a + b + c + d);
            setting->sync();
            setting->endGroup();
            return 1;
        }

        return 0;
    }
}
QVector<QString> UkuiMenuInterface::getAllClassification()
{
    QVector<QString> allAppVector;
    QVector<QString> commonVector;
    QVector<QStringList> appVector;
    allAppVector.clear();
    commonVector.clear();
    appVector.clear();
    commonVector = getCommonUseApp();
    int index = 0;

    Q_FOREACH (QString desktopfp, desktopfpVector) {
        if (!commonVector.contains(desktopfp)) {
            appVector.append(appInfoVector.at(index));
        }

        index++;
    }

    qSort(appVector.begin(), appVector.end(), cmpApp);

    Q_FOREACH (QString desktopfp, commonVector) {
        allAppVector.append(desktopfp);
    }

    for (int index = 0; index < appVector.size(); index++) {
        allAppVector.append(appVector.at(index).at(0));
    }

    return allAppVector;
}
QVector<QString> UkuiMenuInterface::getCommonUseApp()
{
    //    QDateTime dt=QDateTime::currentDateTime();
    //    int currentDateTime=dt.toTime_t();
    //    int nDaySec=24*60*60;
    //    setting->beginGroup("datetime");
    //    QStringList dateTimeKeys=setting->allKeys();
    //    QStringList timeOutKeys;
    //    timeOutKeys.clear();
    //    for(int i=0;i<dateTimeKeys.count();i++)
    //    {
    //        if((currentDateTime-setting->value(dateTimeKeys.at(i)).toInt())/nDaySec >= 4)
    //        {
    //            timeOutKeys.append(dateTimeKeys.at(i));
    //        }
    //    }
    //    setting->endGroup();
    //    for(int i=0;i<timeOutKeys.count();i++)
    //    {
    //        setting->beginGroup("application");
    //        setting->remove(timeOutKeys.at(i));
    //        setting->sync();
    //        setting->endGroup();
    //        setting->beginGroup("datetime");
    //        setting->remove(timeOutKeys.at(i));
    //        setting->sync();
    //        setting->endGroup();
    //    }
    //    setting->beginGroup("lockapplication");
    //    QStringList lockdesktopfnList=setting->allKeys();
    //    for(int i=0;i<lockdesktopfnList.count()-1;i++)
    //        for(int j=0;j<lockdesktopfnList.count()-1-i;j++)
    //        {
    //            int value_1=setting->value(lockdesktopfnList.at(j)).toInt();
    //            int value_2=setting->value(lockdesktopfnList.at(j+1)).toInt();
    //            if(value_1 > value_2)
    //            {
    //                QString tmp=lockdesktopfnList.at(j);
    //                lockdesktopfnList.replace(j,lockdesktopfnList.at(j+1));
    //                lockdesktopfnList.replace(j+1,tmp);
    //            }
    //        }
    //    setting->endGroup();
    //    setting->beginGroup("application");
    //    QStringList desktopfnList=setting->allKeys();
    //    for(int i=0;i<desktopfnList.count()-1;i++)
    //        for(int j=0;j<desktopfnList.count()-1-i;j++)
    //        {
    //            int value_1=setting->value(desktopfnList.at(j)).toInt();
    //            int value_2=setting->value(desktopfnList.at(j+1)).toInt();
    //            if(value_1 < value_2)
    //            {
    //                QString tmp=desktopfnList.at(j);
    //                desktopfnList.replace(j,desktopfnList.at(j+1));
    //                desktopfnList.replace(j+1,tmp);
    //            }
    //        }
    //    setting->endGroup();
    //    QVector<QString> data;
    //    Q_FOREACH(QString desktopfn,lockdesktopfnList)
    //    {
    //        QString desktopfp;
    //        if(androidDesktopfnList.contains(desktopfn))
    //            desktopfp=QString(QDir::homePath()+"/.local/share/applications/"+desktopfn);
    //        else
    //            desktopfp=QString("/usr/share/applications/"+desktopfn);
    //        QFileInfo fileInfo(desktopfp);
    //        if(!fileInfo.isFile()|| !desktopfpVector.contains(desktopfp))
    //            continue;
    //        data.append(desktopfp);
    //    }
    //    Q_FOREACH(QString desktopfn,desktopfnList)
    //    {
    //        QString desktopfp;
    //        if(androidDesktopfnList.contains(desktopfn))
    //            desktopfp=QString(QDir::homePath()+"/.local/share/applications/"+desktopfn);
    //        else
    //            desktopfp=QString("/usr/share/applications/"+desktopfn);
    //        QFileInfo fileInfo(desktopfp);
    //        if(!fileInfo.isFile() || !desktopfpVector.contains(desktopfp))
    //            continue;
    //        data.append(desktopfp);
    //    }
    if (!g_projectCodeName.contains("V10SP1-edu")) {
        QVector<QString> data;

        Q_FOREACH (QString desktopfn, getLockAppList()) {
            QString desktopfp;

            if (androidDesktopfnList.contains(desktopfn)) {
                desktopfp = QString(QDir::homePath() + "/.local/share/applications/" + desktopfn);
            } else {
                desktopfp = QString("/usr/share/applications/" + desktopfn);
            }

            QFileInfo fileInfo(desktopfp);

            if (!fileInfo.isFile() || !desktopfpVector.contains(desktopfp)) {
                continue;
            }

            data.append(desktopfp);
        }

        Q_FOREACH (QString desktopfn, getUnlockAllList()) {
            QString desktopfp;

            if (androidDesktopfnList.contains(desktopfn)) {
                desktopfp = QString(QDir::homePath() + "/.local/share/applications/" + desktopfn);
            } else {
                desktopfp = QString("/usr/share/applications/" + desktopfn);
            }

            QFileInfo fileInfo(desktopfp);

            if (!fileInfo.isFile() || !desktopfpVector.contains(desktopfp)) {
                continue;
            }

            data.append(desktopfp);
        }

        return data;
    }
}

QVector<QString> UkuiMenuInterface::sortDesktopList(QString group)
{
    setting->beginGroup(group);
    QStringList desktopfnList = setting->allKeys();

    for (int i = 0; i < desktopfnList.count() - 1; i++)
        for (int j = 0; j < desktopfnList.count() - 1 - i; j++) { //冒泡排序常用的应用从大到小排列desktopfnList
            int value_1 = setting->value(desktopfnList.at(j)).toInt();
            int value_2 = setting->value(desktopfnList.at(j + 1)).toInt();

            if (value_1 > value_2) {
                QString tmp = desktopfnList.at(j);
                desktopfnList.replace(j, desktopfnList.at(j + 1));
                desktopfnList.replace(j + 1, tmp);
            }
        }

    setting->sync();
    setting->endGroup();
    QVector<QString> data;

    Q_FOREACH (QString desktopfn, desktopfnList) {
        QString desktopfp;

        if (androidDesktopfnList.contains(desktopfn)) {
            desktopfp = QString(QDir::homePath() + "/.local/share/applications/" + desktopfn);
        } else {
            desktopfp = QString("/usr/share/applications/" + desktopfn);
        }

        data.append(desktopfp);
    }

    return data;
}
QVector<QString> UkuiMenuInterface::getCollectApp()
{
    QVector<QString> data;

    Q_FOREACH (QString desktopfn, getCollectAppList()) {
        QString desktopfp;

        if (androidDesktopfnList.contains(desktopfn)) {
            desktopfp = QString(QDir::homePath() + "/.local/share/applications/" + desktopfn);
        } else {
            desktopfp = QString("/usr/share/applications/" + desktopfn);
        }

        QFileInfo fileInfo(desktopfp);

        if (!fileInfo.isFile() || !desktopfpVector.contains(desktopfp)) {
            continue;
        }

        data.append(desktopfp);
    }

    return data;
}
QVector<QString> UkuiMenuInterface::getLockApp()
{
    setting->beginGroup("lockapplication");
    QStringList lockdesktopfnList = setting->allKeys();

    for (int i = 0; i < lockdesktopfnList.count() - 1; i++) //冒泡排序锁住的应用从小到大排列lockdesktopfnList
        for (int j = 0; j < lockdesktopfnList.count() - 1 - i; j++) {
            int value_1 = setting->value(lockdesktopfnList.at(j)).toInt();
            int value_2 = setting->value(lockdesktopfnList.at(j + 1)).toInt();

            if (value_1 > value_2) {
                QString tmp = lockdesktopfnList.at(j);
                lockdesktopfnList.replace(j, lockdesktopfnList.at(j + 1));
                lockdesktopfnList.replace(j + 1, tmp);
            }
        }

    setting->endGroup();
    QVector<QString> data;

    Q_FOREACH (QString desktopfn, lockdesktopfnList) {
        QString desktopfp;
        //        if(androidDesktopfnList.contains(desktopfn))//如果锁的应用在安卓列表
        //            desktopfp=QString(QDir::homePath()+"/.local/share/applications/"+desktopfn);
        //        else
        desktopfp = QString("/usr/share/applications/" + desktopfn);
        //        QFileInfo fileInfo(desktopfp);
        //        if(!fileInfo.isFile())//判断是否存在
        //            continue;
        data.append(desktopfp);//加入data
    }

    return data;
}
QVector<QStringList> UkuiMenuInterface::getAlphabeticClassification()
{
    QVector<QStringList> data;
    QStringList appnameList;
    appnameList.clear();
    QVector<QStringList> appVector[27];
    int index = 0;

    while (index < appInfoVector.size()) {
        QString appname = appInfoVector.at(index).at(1);
        QString appnamepy = UkuiChineseLetter::getPinyins(appname);

        if (!appnamepy.isEmpty()) {
            char c = appnamepy.at(0).toLatin1();

            switch (c) {
                case 'A':
                    appVector[0].append(appInfoVector.at(index));
                    break;

                case 'B':
                    appVector[1].append(appInfoVector.at(index));
                    break;

                case 'C':
                    appVector[2].append(appInfoVector.at(index));
                    break;

                case 'D':
                    appVector[3].append(appInfoVector.at(index));
                    break;

                case 'E':
                    appVector[4].append(appInfoVector.at(index));
                    break;

                case 'F':
                    appVector[5].append(appInfoVector.at(index));
                    break;

                case 'G':
                    appVector[6].append(appInfoVector.at(index));
                    break;

                case 'H':
                    appVector[7].append(appInfoVector.at(index));
                    break;

                case 'I':
                    appVector[8].append(appInfoVector.at(index));
                    break;

                case 'J':
                    appVector[9].append(appInfoVector.at(index));
                    break;

                case 'K':
                    appVector[10].append(appInfoVector.at(index));
                    break;

                case 'L':
                    appVector[11].append(appInfoVector.at(index));
                    break;

                case 'M':
                    appVector[12].append(appInfoVector.at(index));
                    break;

                case 'N':
                    appVector[13].append(appInfoVector.at(index));
                    break;

                case 'O':
                    appVector[14].append(appInfoVector.at(index));
                    break;

                case 'P':
                    appVector[15].append(appInfoVector.at(index));
                    break;

                case 'Q':
                    appVector[16].append(appInfoVector.at(index));
                    break;

                case 'R':
                    appVector[17].append(appInfoVector.at(index));
                    break;

                case 'S':
                    appVector[18].append(appInfoVector.at(index));
                    break;

                case 'T':
                    appVector[19].append(appInfoVector.at(index));
                    break;

                case 'U':
                    appVector[20].append(appInfoVector.at(index));
                    break;

                case 'V':
                    appVector[21].append(appInfoVector.at(index));
                    break;

                case 'W':
                    appVector[22].append(appInfoVector.at(index));
                    break;

                case 'X':
                    appVector[23].append(appInfoVector.at(index));
                    break;

                case 'Y':
                    appVector[24].append(appInfoVector.at(index));
                    break;

                case 'Z':
                    appVector[25].append(appInfoVector.at(index));
                    break;

                default:
                    appVector[26].append(appInfoVector.at(index));
                    break;
            }
        }

        index++;
    }

    for (int i = 0; i < 26; i++) {
        QStringList desktopfpList;
        desktopfpList.clear();
        qSort(appVector[i].begin(), appVector[i].end(), cmpApp);

        for (int j = 0; j < appVector[i].size(); j++) {
            desktopfpList.append(appVector[i].at(j).at(0));
        }

        data.append(desktopfpList);
    }

    QVector<QStringList> otherVector;
    QVector<QStringList> numberVector;

    for (int i = 0; i < appVector[26].count(); i++) {
        QString appname = appVector[26].at(i).at(2);
        QChar c = appname.at(0);

        if (c < 48 || (c > 57 && c < 65) || c > 90) {
            otherVector.append(appVector[26].at(i));
        } else {
            numberVector.append(appVector[26].at(i));
        }
    }

    qSort(otherVector.begin(), otherVector.end(), cmpApp);
    qSort(numberVector.begin(), numberVector.end(), cmpApp);
    QStringList otherfpList;
    otherfpList.clear();

    for (int i = 0; i < otherVector.size(); i++) {
        otherfpList.append(otherVector.at(i).at(0));
    }

    QStringList numberfpList;
    numberfpList.clear();

    for (int i = 0; i < numberVector.size(); i++) {
        numberfpList.append(numberVector.at(i).at(0));
    }

    data.append(otherfpList);
    data.append(numberfpList);
    return data;
}
QVector<QStringList> UkuiMenuInterface::getFunctionalClassification()
{
    QVector<QStringList> appVector[11];
    int index = 0;

    while (index < appInfoVector.size()) {
        int count = appInfoVector.at(index).size() - 5;

        for (int i = 0; i < count; i++) {
            int category = appInfoVector.at(index).at(5 + i).toInt();

            switch (category) {
                case 0:
                    appVector[0].append(appInfoVector.at(index));
                    break;

                case 1:
                    appVector[1].append(appInfoVector.at(index));
                    break;

                case 2:
                    appVector[2].append(appInfoVector.at(index));
                    break;

                case 3:
                    appVector[3].append(appInfoVector.at(index));
                    break;

                case 4:
                    appVector[4].append(appInfoVector.at(index));
                    break;

                case 5:
                    appVector[5].append(appInfoVector.at(index));
                    break;

                case 6:
                    appVector[6].append(appInfoVector.at(index));
                    break;

                case 7:
                    appVector[7].append(appInfoVector.at(index));
                    break;

                case 8:
                    appVector[8].append(appInfoVector.at(index));
                    break;

                case 9:
                    appVector[9].append(appInfoVector.at(index));
                    break;

                case 10:
                    appVector[10].append(appInfoVector.at(index));
                    break;

                default:
                    break;
            }
        }

        index++;
    }

    QVector<QStringList> data;
    data.clear();

    for (int i = 0; i < 11; i++) {
        QStringList desktopfpList;
        desktopfpList.clear();
        qSort(appVector[i].begin(), appVector[i].end(), cmpApp);

        for (int j = 0; j < appVector[i].size(); j++) {
            desktopfpList.append(appVector[i].at(j).at(0));
        }

        data.append(desktopfpList);
    }

    return data;
}
bool UkuiMenuInterface::matchingAppCategories(QString desktopfp, QStringList categorylist)
{
    QString category = getAppCategories(desktopfp);
    int index;

    for (index = 0; index < categorylist.count(); index++) {
        if (category.contains(categorylist.at(index), Qt::CaseInsensitive)) {
            return true;
        }
    }

    if (index == categorylist.count()) {
        return false;
    }

    return false;
}
void UkuiMenuInterface::getAndroidApp()
{
    androidDesktopfnList.clear();
    QVector<QStringList> androidVector;
    androidVector.clear();
    QString path = QDir::homePath() + "/.local/share/applications/";
    QDir dir(path);

    if (!dir.exists()) {
        return;
    }

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();

    if (list.size() < 1) {
        return;
    }

    int i = 0;
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();

    do {
        QFileInfo fileInfo = list.at(i);

        if (!fileInfo.isFile()) {
            i++;
            continue;
        }

        //过滤后缀不是.desktop的文件
        QString filePathStr = fileInfo.filePath();

        if (!filePathStr.endsWith(".desktop")) {
            i++;
            continue;
        }

        QByteArray fpbyte = filePathStr.toLocal8Bit();
        char *filepath = fpbyte.data();
        g_key_file_load_from_file(keyfile, filepath, flags, error);
        char *ret_1 = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Categories", nullptr, nullptr);

        if (ret_1 != nullptr) {
            //            QString str=QString::fromLocal8Bit(ret_1);
            //            if(!str.contains("Android"))
            //            {
            //                i++;
            //                continue;
            //            }
            //            else
            //            {
            m_filePathList.append(filePathStr);
            androidDesktopfnList.append(fileInfo.fileName());
            //            }
        }

        i++;
    } while (i < list.size());

    g_key_file_free(keyfile);
}
QString UkuiMenuInterface::getAppNameInitials(QString desktopfp)
{
    QString firstLetters;
    QString appname = getAppName(desktopfp);
    QStringList appnamestr = appname.split(" ");
    QString letters;

    Q_FOREACH (QString name, appnamestr) {
        letters.clear();
        letters = UkuiChineseLetter::getFirstLettersAll(name);

        if (letters.isEmpty()) {
            letters = UkuiChineseLetter::getFirstLetter(name);
        }

        firstLetters.append(letters);
    }

    return firstLetters;
}
QString UkuiMenuInterface::getAppNameInitial(QString desktopfp)
{
    return UkuiChineseLetter::getFirstLetter(getAppName(desktopfp));
}
//获取应用拼音
QString UkuiMenuInterface::getAppNamePinyin(QString appname)
{
    return UkuiChineseLetter::getPinyins(appname);
}
bool UkuiMenuInterface::checkKreApp(QString desktopfp)
{
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();
    QByteArray fpbyte = desktopfp.toLocal8Bit();
    char *filepath = fpbyte.data();
    g_key_file_load_from_file(keyfile, filepath, flags, error);
    char *value = g_key_file_get_locale_string(keyfile, "Desktop Entry", "environment", nullptr, nullptr);
    g_key_file_free(keyfile);

    if (!value) {
        return false;
    } else if (QString::compare(QString::fromLocal8Bit(value), "kre-environment") == 0) {
        return true;
    }

    return false;
}
//获取指定类型应用列表
QStringList UkuiMenuInterface::getSpecifiedCategoryAppList(QString categorystr)
{
    QByteArray categorybyte = categorystr.toLocal8Bit();
    char *category = categorybyte.data();
    QStringList desktopfpList = getDesktopFilePath();
    QStringList appnameList;
    appnameList.clear();

    for (int index = 0; index < desktopfpList.count(); index++) {
        QString appcategorystr = getAppCategories(desktopfpList.at(index));

        if (!appcategorystr.isEmpty()) {
            QByteArray appcategorybyte = appcategorystr.toLocal8Bit();
            char *appcategory = appcategorybyte.data();
            int len = static_cast<int>(strlen(appcategory) - 6);

            for (int i = 0; i < len; i++) {
                if (strncmp(appcategory + i, category, strlen(category)) == 0) {
                    QString appname = getAppName(desktopfpList.at(index));

                    if (QString::compare(appname, "访问提示") == 0) {
                        appnameList.append(appname);
                    }

                    break;
                }
            }
        }
    }

    return appnameList;
}
