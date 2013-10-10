/* ============================================================
* QuiteRSS is a open-source cross-platform RSS/Atom news feeds reader
* Copyright (C) 2011-2013 QuiteRSS Team <quiterssteam@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#ifndef PARSEOBJECT_H
#define PARSEOBJECT_H

#include <QtSql>
#include <QDateTime>
#include <QDomDocument>
#include <QQueue>
#include <QObject>
#include <QUrl>

struct FeedItemStruct {
  QString title;
  QString updated;
  QString link;
  QString linkBase;
  QString language;
  QString author;
  QString authorUri;
  QString authorEmail;
  QString description;
};

struct NewsItemStruct {
  QString id;
  QString title;
  QString updated;
  QString link;
  QString linkAlternate;
  QString language;
  QString author;
  QString authorUri;
  QString authorEmail;
  QString description;
  QString content;
  QString category;
  QString eUrl;
  QString eType;
  QString eLength;
  QString comments;
};

struct FeedCountStruct{
  int feedId;
  int unreadCount;
  int newCount;
  int undeleteCount;
  QString updated;
  QString lastBuildDate;
  QString htmlUrl;
  QString xmlUrl;
  QString title;
};

Q_DECLARE_METATYPE(FeedCountStruct)

class RSSListing;

class ParseObject : public QObject
{
  Q_OBJECT
public:
  explicit ParseObject(QObject *parent);

public slots:
  void slotGetFeed(int feedId, QString feedUrl, QDateTime date, int auth);
  void slotGetFeedsFolder(QString query);
  void slotGetAllFeeds();
  void getUrlDone(int result, int feedId, QString feedUrlStr,
                  QString error, QByteArray data,
                  QDateTime dtReply, QString codecName);
  void parseXml(QByteArray data, int feedId,
                QDateTime dtReply, QString codecName);

signals:
  void showProgressBar(int value);
  void loadProgress(int value, bool clear = false);
  void signalRequestUrl(int feedId, QString urlString,
                        QDateTime date, QString userInfo);
  void signalReadyParse(const QByteArray &xml, const int &feedId,
                        const QDateTime &dtReply, const QString &codecName);
  void finishUpdateFeed();
  void feedUpdated(int feedId, bool changed, int newCount,
                   QString status, bool finish);
  void feedCountsUpdate(FeedCountStruct counts);

private slots:
  bool addFeedInQueue(int feedId, const QString &feedUrl,
                      const QDateTime &date, int auth);
  void getQueuedXml();
  void slotParse(const QByteArray &xmlData, const int &feedId,
                 const QDateTime &dtReply, const QString &codecName);

private:
  void finishUpdate(int feedId, bool changed,
                    int newCount, QString status);

  void parseAtom(const QString &feedUrl, const QDomDocument &doc);
  void parseRss(const QString &feedUrl, const QDomDocument &doc);
  void addAtomNewsIntoBase(NewsItemStruct &newsItem);
  void addRssNewsIntoBase(NewsItemStruct &newsItem);
  QString toPlainText(const QString &text);
  QString parseDate(const QString &dateString, const QString &urlString);
  int recountFeedCounts(int feedId, const QString &feedUrl,
                        const QString &updated, const QString &lastBuildDate);

  RSSListing *rssl_;
  QList<int> feedIdList_;
  int updateFeedsCount_;
  QTimer *parseTimer_;
  int currentFeedId_;
  QQueue<int> idsQueue_;
  QQueue<QByteArray> xmlsQueue_;
  QQueue<QDateTime> dtReadyQueue_;
  QQueue<QString> codecNameQueue_;

  int parseFeedId_;
  bool duplicateNewsMode_;
  bool feedChanged_;

};

#endif // PARSEOBJECT_H
