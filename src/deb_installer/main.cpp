/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "debinstaller.h"
#include "utils.h"

#include <QCommandLineParser>
#include <QDebug>
#include <QTimer>
#include <QSharedMemory>

#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DLog>

#include <QDBusConnection>
#include <QDBusInterface>
#include "singleInstallerApplication.h"

DWIDGET_USE_NAMESPACE
#ifdef DUTIL_USE_NAMESPACE
DUTIL_USE_NAMESPACE
#else
DCORE_USE_NAMESPACE
#endif

#define RECENT_PATH QDir::homePath() + "/.local/share/recently-used.xbel"

#define DAPPLICATION_XSTRING(s) DAPPLICATION_STRING(s)
#define DAPPLICATION_STRING(s) #s

int main(int argc, char *argv[])
{
    SingleInstallerApplication app(argc, argv);

    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-deb-installer");
    app.setApplicationVersion(DApplication::buildVersion("3.0"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/acknowledgments/deepin-package-manager/");
    app.setProductIcon(QIcon::fromTheme("deepin-deb-installer"));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    //    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    app.loadTranslator();
    app.setProductName(QApplication::translate("main", "Package Installer"));
    app.setApplicationDisplayName(QApplication::translate("main", "Package Installer"));
    app.setApplicationDescription(QApplication::translate(
                                      "main",
                                      "Package Installer helps users install and remove local packages, and supports bulk installation."));

    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");

    DApplicationSettings settings;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << qApp->applicationName() << "started, version = " << qApp->applicationVersion();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.DebInstaller")) {
        dbus.registerObject("/com/deepin/DebInstaller", &app, QDBusConnection::ExportScriptableSlots);
        app.parseCmdLine();
        app.activateWindow();
        return app.exec();
    } else {
        QCommandLineParser parser;
        parser.process(app);
        QList<QVariant> debInstallPathList;
        debInstallPathList << parser.positionalArguments();
        QDBusInterface notification("com.deepin.DebInstaller", "/com/deepin/DebInstaller", "com.deepin.DebInstaller", QDBusConnection::sessionBus());
        QDBusMessage msg = notification.callWithArgumentList(QDBus::AutoDetect, "InstallerDeb", debInstallPathList);
        return 0;
    }
}
