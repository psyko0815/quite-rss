/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef RSSLISTING_H
#define RSSLISTING_H

#include <QtGui>
#include <QtSql>
#include <QtWebKit>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
QT_END_NAMESPACE

class RSSListing : public QMainWindow
{
    Q_OBJECT
public:
    RSSListing(QWidget *widget = 0);
    ~RSSListing();

public slots:
    void addFeed();
    void deleteFeed();
    void importFeeds();
    void finished(QNetworkReply *reply);
    void readyRead();
    void metaDataChanged();
    void itemActivated(QTreeWidgetItem * item);
    void error(QNetworkReply::NetworkError);
    void slotFeedsTreeClicked(QModelIndex index);
    void slotFeedsTreeDoubleClicked(QModelIndex index);
    void slotFeedViewClicked(QModelIndex index);
    void slotFeedsTreeKeyUpDownPressed();
    void slotFeedKeyUpDownPressed();
    void toggleToolBar(bool checked);
    void toggleQueryResults(bool checked);
    void showOptionDlg();
    void receiveMessage(const QString&);
    void slotPlaceToTray();
    void slotActivationTray(QSystemTrayIcon::ActivationReason reason);
    void slotShowWindows();
    void slotClose();
    void slotCloseApp();
    void myEmptyWorkingSet();

protected:
     bool eventFilter(QObject *obj, QEvent *ev);
     virtual void closeEvent(QCloseEvent*);
     virtual void changeEvent(QEvent*);

private:
    void parseXml();
    void get(const QUrl &url);
    void createActions();
    void createMenu();
    void createToolBar();
    void readSettings ();
    void writeSettings();
    void createTrayMenu();

    QSettings *settings_;
    QXmlStreamReader xml;
    QString currentTag;
    QString itemString;
    QString titleString;
    QString linkString;
    QString descriptionString;
    QString commentsString;
    QString pubDateString;
    QString guidString;

    QNetworkProxy networkProxy_;
    QNetworkAccessManager manager_;
    QUrl currentUrl_;
    QNetworkReply *currentReply_;

    QSqlDatabase db_;
    QSqlTableModel *feedsModel_;
    QSqlTableModel *newsModel_;

    QAction *addFeedAct_;
    QAction *deleteFeedAct_;
    QAction *importFeedsAct_;
    QAction *toolBarToggle_;
    QAction *treeWidgetToggle_;
    QAction *optionsAct_;
    QAction *exitAct_;
    QMenu *fileMenu_;
    QMenu *viewMenu_;
    QMenu *feedMenu_;
    QMenu *toolsMenu_;
    QMenu *trayMenu_;
    QToolBar *toolBar_;

    QTabWidget *feedsTabWidget_;
    QTreeView *feedsView_;
    QTabWidget *newsTabWidget_;
    QTableView *newsView_;

    QTreeWidget *treeWidget_;

    QWebView *webView_;

    QSystemTrayIcon *traySystem;
    int oldState;

signals:
    void signalFeedsTreeKeyUpDownPressed();
    void signalFeedKeyUpDownPressed();
    void signalPlaceToTray();
    void signalCloseApp();
};

#endif

