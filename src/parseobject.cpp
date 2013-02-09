#include <QDebug>
#include <QDesktopServices>
#include <QTextDocumentFragment>

#include "parseobject.h"
#include "VersionNo.h"
#include "db_func.h"

ParseObject::ParseObject(QString dataDirPath, QObject *parent)
  : QObject(parent),
    dataDirPath_(dataDirPath)
{
}

void ParseObject::slotParse(const QByteArray &xmlData, const QString &feedUrl)
{
  if (!dataDirPath_.isEmpty()) {
    QFile file(dataDirPath_ + QDir::separator() + "lastfeed.dat");
    file.open(QIODevice::WriteOnly);
    file.write(xmlData);
    file.close();
  }

  QString currentTag;
  QString currentTagText;
  QStack<QString> tagsStack;
  QString titleString;
  QString linkBaseString;
  QString linkString;
  QString linkAlternateString;
  QString authorString;
  QString authorUriString;
  QString authorEmailString;
  QString rssDescriptionString;
  QString commentsString;
  QString rssPubDateString;
  QString rssGuidString;
  QString atomIdString;
  QString atomUpdatedString;
  QString atomSummaryString;
  QString contentString;
  QString categoryString;
  QString enclosureUrl;
  QString enclosureType;
  QString enclosureLength;

  qDebug() << "=================== parseXml:start ============================";

  // поиск идентификатора ленты и режима дубликатов новостей в таблице лент
  int parseFeedId = 0;
  bool duplicateNewsMode = false;

  QSqlQuery q;
  q.prepare("SELECT id, duplicateNewsMode FROM feeds WHERE xmlUrl LIKE :xmlUrl");
  q.bindValue(":xmlUrl", feedUrl);
  q.exec();
  while (q.next()) {
    parseFeedId = q.value(0).toInt();
    duplicateNewsMode = q.value(1).toBool();
  }

  // идентификатор не найден (например, во время запроса удалили ленту)
  if (0 == parseFeedId) {
    qDebug() << QString("Feed '%1' not found").arg(feedUrl);
    emit feedUpdated(parseFeedId, false, 0);
    return;
  }

  qDebug() << QString("Feed '%1' found with id = %2").arg(feedUrl).
              arg(parseFeedId);

  // собственно сам разбор
  bool feedChanged = false;
  QSqlDatabase db = QSqlDatabase::database();
  db.transaction();
  int itemCount = 0;
  QXmlStreamReader xml(xmlData.trimmed());
  xml.setNamespaceProcessing(false);
  bool isHeader = true;  //!< флаг заголовка ленты - элементы до первой новости

  xml.readNext();
  if (xml.documentVersion().isEmpty()) {
    xml.clear();
    xml.addData(QString::fromLocal8Bit(xmlData.trimmed()));
  } else {
    if (xml.documentEncoding().toString().contains("utf-8", Qt::CaseInsensitive)) {
      xml.clear();
      xml.addData(QString::fromUtf8(xmlData.trimmed()));
    }
  }

  xml.readNext();
  while (!xml.atEnd()) {
    if (xml.isStartElement()) {
      tagsStack.push(currentTag);
      currentTag = xml.name().toString();

      if (currentTag == "rss")
        qDebug() << "Feed type: RSS";
      if (currentTag == "feed") {
        qDebug() << "Feed type: Atom";
        linkBaseString = xml.attributes().value("xml:base").toString();
      }

      if (currentTag == "enclosure") {
        enclosureUrl = xml.attributes().value("url").toString();
        enclosureType = xml.attributes().value("type").toString();
        enclosureLength = xml.attributes().value("length").toString();
      }

      if (currentTag == "item") {  // RSS
        if (isHeader) {
          rssPubDateString = parseDate(rssPubDateString, feedUrl);

          QSqlQuery q;
          QString qStr("UPDATE feeds "
                       "SET title=?, description=?, htmlUrl=?, "
                       "author_name=?, pubdate=? "
                       "WHERE id==?");
          q.prepare(qStr);
          q.addBindValue(titleString.simplified());
          q.addBindValue(rssDescriptionString);
          q.addBindValue(linkString);
          q.addBindValue(authorString.simplified());
          q.addBindValue(rssPubDateString);
          q.addBindValue(parseFeedId);
          q.exec();
          qDebug() << "q.exec(" << q.lastQuery() << ")";
          qDebug() << q.boundValues();
          qDebug() << q.lastError().number() << ": " << q.lastError().text();
        }
        isHeader = false;
        titleString.clear();
        linkString.clear();
        authorString.clear();
        rssDescriptionString.clear();
        commentsString.clear();
        rssPubDateString.clear();
        rssGuidString.clear();
        contentString.clear();
        categoryString.clear();
      }
      if (currentTag == "entry") {  // Atom
        atomUpdatedString = parseDate(atomUpdatedString, feedUrl);

        if (isHeader) {
          QSqlQuery q;
          QString qStr ("UPDATE feeds "
                        "SET title=?, description=?, htmlUrl=?, "
                        "author_name=?, author_email=?, "
                        "author_uri=?, pubdate=? "
                        "WHERE id==?");
          q.prepare(qStr);
          q.addBindValue(titleString.simplified());
          q.addBindValue(atomSummaryString);
          QString linkStr;
          if (!linkString.isEmpty()) linkStr = linkString;
          else linkStr = linkAlternateString;
          if (QUrl(linkStr).host().isEmpty()) linkStr = linkBaseString + linkStr;
          q.addBindValue(linkStr);
          q.addBindValue(authorString);
          q.addBindValue(authorEmailString);
          q.addBindValue(authorUriString);
          q.addBindValue(atomUpdatedString);
          q.addBindValue(parseFeedId);
          q.exec();
          qDebug() << "q.exec(" << q.lastQuery() << ")";
          qDebug() << q.boundValues();
          qDebug() << q.lastError().number() << ": " << q.lastError().text();
        }
        isHeader = false;
        titleString.clear();
        linkString.clear();
        linkAlternateString.clear();
        authorString.clear();
        authorUriString.clear();
        authorEmailString.clear();
        atomIdString.clear();
        atomUpdatedString.clear();
        atomSummaryString.clear();
        contentString.clear();
        categoryString.clear();
      }
      if ((currentTag == "link") && // Atom
          ((tagsStack.top() == "feed") || (tagsStack.top() == "entry"))) {
        if (xml.attributes().value("type").toString() == "text/html") {
          if (xml.attributes().value("rel").toString() == "self")
            linkString = xml.attributes().value("href").toString();
          if (xml.attributes().value("rel").toString() == "alternate")
            linkAlternateString = xml.attributes().value("href").toString();
        } else if (linkAlternateString.isNull()) {
          if (!(xml.attributes().value("rel").toString() == "self"))
            linkAlternateString = xml.attributes().value("href").toString();
        }
      }
      if (isHeader) {
        if (xml.namespaceUri().isEmpty()) qDebug() << itemCount << ":" << currentTag;
        else qDebug() << itemCount << ":" << xml.qualifiedName();
        for (int i = 0 ; i < xml.attributes().count(); ++i)
          qDebug() << "      " << xml.attributes().at(i).name() << "=" << xml.attributes().at(i).value();
      }
      currentTagText.clear();
      //      qDebug() << tagsStack << currentTag;
    } else if (xml.isEndElement()) {
      // rss::item
      if (xml.name() == "item") {
        rssPubDateString = parseDate(rssPubDateString, feedUrl);

        titleString = QTextDocumentFragment::fromHtml(titleString.simplified()).
            toPlainText();

        // поиск дубликата статей в базе
        QSqlQuery q;
        QString qStr;
        qDebug() << "guid:     " << rssGuidString;
        qDebug() << "link_href:" << linkString;
        qDebug() << "title:"     << titleString;
        qDebug() << "published:" << rssPubDateString;

        qStr.clear();
        if (!rssPubDateString.isEmpty()) {  // поиск по pubDate
          if (!duplicateNewsMode)
            qStr.append("AND published=:published");
        } else {
          qStr.append("AND title LIKE :title");
        }

        if (!rssGuidString.isEmpty()) {       // поиск по guid
          q.prepare(QString("SELECT * FROM news WHERE feedId=:id AND guid=:guid %1").arg(qStr));
          q.bindValue(":guid", rssGuidString);
        } else if (!linkString.isEmpty()) {   // поиск по link_href
          q.prepare(QString("SELECT * FROM news WHERE feedId=:id AND link_href=:link_href %1").arg(qStr));
          q.bindValue(":link_href", linkString);
        } else {
          q.prepare(QString("SELECT * FROM news WHERE feedId=:id %1").arg(qStr));
        }
        q.bindValue(":id", parseFeedId);
        if (!rssPubDateString.isEmpty()) {    // поиск по pubDate
          if (!duplicateNewsMode)
            q.bindValue(":published", rssPubDateString);
        } else {
          q.bindValue(":title", titleString);
        }
        q.exec();

        // проверка правильности запроса
        if (q.lastError().isValid())
          qDebug() << "ERROR: " << q.lastError().text();
        else {
          // если дубликата нет, добавляем статью в базу
          if (!q.next()) {
            qStr = QString("INSERT INTO news("
                           "feedId, description, content, guid, title, author_name, published, received, link_href, category, "
                           "enclosure_url, enclosure_type, enclosure_length) "
                           "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            q.prepare(qStr);
            q.addBindValue(parseFeedId);
            q.addBindValue(rssDescriptionString);
            q.addBindValue(contentString);
            q.addBindValue(rssGuidString);
            q.addBindValue(titleString);
            q.addBindValue(QTextDocumentFragment::fromHtml(authorString.simplified()).toPlainText());
            if (rssPubDateString.isEmpty())
              rssPubDateString = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            q.addBindValue(rssPubDateString);
            q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
            q.addBindValue(linkString);
            q.addBindValue(QTextDocumentFragment::fromHtml(categoryString.simplified()).toPlainText());
            q.addBindValue(enclosureUrl);
            q.addBindValue(enclosureType);
            q.addBindValue(enclosureLength);
            q.exec();
            qDebug() << "q.exec(" << q.lastQuery() << ")";
            qDebug() << "       " << parseFeedId;
            qDebug() << "       " << rssDescriptionString;
            qDebug() << "       " << contentString;
            qDebug() << "       " << rssGuidString;
            qDebug() << "       " << titleString;
            qDebug() << "       " << authorString;
            qDebug() << "       " << rssPubDateString;
            qDebug() << "       " << QDateTime::currentDateTime().toString();
            qDebug() << "       " << linkString;
            qDebug() << "       " << categoryString;
            qDebug() << "       " << enclosureUrl;
            qDebug() << "       " << enclosureType;
            qDebug() << "       " << enclosureLength;
            qDebug() << q.lastError().number() << ": " << q.lastError().text();
            feedChanged = true;
          }
        }
        q.finish();

        ++itemCount;
      }
      // atom::feed
      else if (xml.name() == "entry") {
        atomUpdatedString = parseDate(atomUpdatedString, feedUrl);

        titleString = QTextDocumentFragment::fromHtml(titleString.simplified()).
            toPlainText();

        // поиск дубликата статей в базе
        QSqlQuery q;
        QString qStr;
        qDebug() << "atomId:" << atomIdString;
        qDebug() << "title:" << titleString;
        qDebug() << "published:" << atomUpdatedString;

        qStr.clear();
        if (!atomUpdatedString.isEmpty()) {  // поиск по pubDate
          if (!duplicateNewsMode)
            qStr.append("AND published=:published");
        } else {
          qStr.append("AND title LIKE :title");
        }

        if (!atomIdString.isEmpty()) {       // поиск по guid
          if (duplicateNewsMode) {
            q.prepare("SELECT * FROM news WHERE feedId=:id AND guid=:guid");
          } else {
            q.prepare(QString("SELECT * FROM news WHERE feedId=:id AND guid=:guid %1").arg(qStr));
            if (!atomUpdatedString.isEmpty()) {    // поиск по pubDate
              q.bindValue(":published", atomUpdatedString);
            } else {
              q.bindValue(":title", titleString);
            }
          }
          q.bindValue(":guid", atomIdString);
        } else {
          q.prepare(QString("SELECT * FROM news WHERE feedId=:id %1").arg(qStr));
          if (!atomUpdatedString.isEmpty()) {    // поиск по pubDate
            if (!duplicateNewsMode)
              q.bindValue(":published", atomUpdatedString);
          } else {
            q.bindValue(":title", titleString);
          }
        }
        q.bindValue(":id", parseFeedId);
        q.exec();

        // проверка правильности запроса
        if (q.lastError().isValid())
          qDebug() << "ERROR: q.exec(" << qStr << ") -> " << q.lastError().text();
        else {
          // если дубликата нет, добавляем статью в базу
          if (!q.next()) {
            qStr = QString("INSERT INTO news("
                           "feedId, description, content, guid, title, author_name, "
                           "author_uri, author_email, published, received, "
                           "link_href, link_alternate, category, "
                           "enclosure_url, enclosure_type, enclosure_length) "
                           "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
            q.prepare(qStr);
            q.addBindValue(parseFeedId);
            q.addBindValue(atomSummaryString);
            q.addBindValue(contentString);
            q.addBindValue(atomIdString);
            q.addBindValue(titleString);
            q.addBindValue(QTextDocumentFragment::fromHtml(authorString.simplified()).toPlainText());
            q.addBindValue(authorUriString);
            q.addBindValue(authorEmailString);
            if (atomUpdatedString.isEmpty())
              atomUpdatedString = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            q.addBindValue(atomUpdatedString);
            q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
            if (!linkString.isEmpty() && QUrl(linkString).host().isEmpty())
              linkString = linkBaseString + linkString;
            q.addBindValue(linkString);
            if (!linkAlternateString.isEmpty() && QUrl(linkAlternateString).host().isEmpty())
              linkAlternateString = linkBaseString + linkAlternateString;
            q.addBindValue(linkAlternateString);
            q.addBindValue(QTextDocumentFragment::fromHtml(categoryString.simplified()).toPlainText());
            q.addBindValue(enclosureUrl);
            q.addBindValue(enclosureType);
            q.addBindValue(enclosureLength);
            q.exec();
            qDebug() << "q.exec(" << q.lastQuery() << ")";
            qDebug() << "       " << parseFeedId;
            qDebug() << "       " << atomSummaryString;
            qDebug() << "       " << contentString;
            qDebug() << "       " << atomIdString;
            qDebug() << "       " << titleString;
            qDebug() << "       " << authorString;
            qDebug() << "       " << authorUriString;
            qDebug() << "       " << authorEmailString;
            qDebug() << "       " << atomUpdatedString;
            qDebug() << "       " << QDateTime::currentDateTime().toString();
            qDebug() << "       " << linkString;
            qDebug() << "       " << linkAlternateString;
            qDebug() << "       " << categoryString;
            qDebug() << "       " << enclosureUrl;
            qDebug() << "       " << enclosureType;
            qDebug() << "       " << enclosureLength;
            qDebug() << q.lastError().number() << ": " << q.lastError().text();
            feedChanged = true;
          }
        }
        q.finish();

        ++itemCount;
      }
      if (isHeader) {
        if (!currentTagText.isEmpty()) qDebug() << itemCount << "::   " << currentTagText;
      }
      currentTag = tagsStack.pop();
      //      qDebug() << tagsStack << currentTag;
    } else if (xml.isCharacters() && !xml.isWhitespace()) {
      currentTagText += xml.text().toString();

      if (currentTag == "title") {
        if ((tagsStack.top() == "channel") ||  // RSS
            (tagsStack.top() == "item") ||
            (tagsStack.top() == "feed") ||     // Atom
            (tagsStack.top() == "entry"))
          titleString += xml.text().toString();
        //        if (tagsStack.top() == "image")
        //          imageTitleString += xml.text().toString();
      }
      else if ((currentTag == "link") &&
               ((tagsStack.top() == "channel") || (tagsStack.top() == "item")))
        linkString = xml.text().toString();
      else if (currentTag == "author")  //rss
        authorString += xml.text().toString();
      else if (currentTag == "creator")  //rss::dc:creator
        authorString += xml.text().toString();
      else if (currentTag == "name")   //atom::author
        authorString += xml.text().toString();
      else if (currentTag == "uri")    //atom::uri
        authorUriString += xml.text().toString();
      else if (currentTag == "email")  //atom::email
        authorEmailString += xml.text().toString();
      else if (currentTag == "description") {
        if ((tagsStack.top() == "channel") ||
            (tagsStack.top() == "item"))
          rssDescriptionString += xml.text().toString();
      }
      else if (currentTag == "comments")
        commentsString += xml.text().toString();
      else if (currentTag == "pubDate")
        rssPubDateString += xml.text().toString();
      else if (currentTag == "date")
        rssPubDateString += xml.text().toString();
      else if (currentTag == "guid")
        rssGuidString += xml.text().toString();
      else if (currentTag == "encoded")  //rss::content:encoded
        contentString += xml.text().toString();

      else if (currentTag == "id")
        atomIdString += xml.text().toString();
      else if (currentTag == "updated")
        atomUpdatedString += xml.text().toString();
      else if (currentTag == "summary")
        atomSummaryString += xml.text().toString();
      else if (currentTag == "content")
        contentString += xml.text().toString();
      else if (currentTag == "category")
        categoryString = xml.text().toString();
    }
    xml.readNext();
  }

  if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
    QString str = QString("XML ERROR: Line=%1, ErrorString=%2").
        arg(xml.lineNumber()).arg(xml.errorString());
    qDebug() << str;
  }

  // Устанавливаем время обновления ленты
  q.prepare("UPDATE feeds SET updated=? WHERE id=?");
  q.addBindValue(QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                       "yyyy-MM-ddTHH:mm:ss"));
  q.addBindValue(parseFeedId);
  q.exec();

  int newCount = 0;
  if (feedChanged) {
    setUserFilter(parseFeedId);
    newCount = recountFeedCounts(parseFeedId);
  }

  db.commit();
  qDebug() << "=================== parseXml:finish ===========================";

  emit feedUpdated(parseFeedId, feedChanged, newCount);
}

QString ParseObject::parseDate(QString dateString, QString urlString)
{
  QDateTime dt;
  QString temp;
  QString timeZone;

  if (dateString.isEmpty()) return QString();

  QString ds = dateString.simplified();
  QLocale locale(QLocale::C);

  if (ds.indexOf(',') != -1) {
    ds = ds.remove(0, ds.indexOf(',')+1).simplified();
  }

  for (int i = 0; i < 2; i++, locale = QLocale::system()) {
    temp     = ds.left(23);
    timeZone = ds.mid(temp.length(), 3);
    dt = locale.toDateTime(temp, "yyyy-MM-ddTHH:mm:ss.z");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp     = ds.left(19);
    timeZone = ds.mid(temp.length(), 3);
    dt = locale.toDateTime(temp, "yyyy-MM-ddTHH:mm:ss");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(23);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "yyyy-MM-dd HH:mm:ss.z");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(19);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "yyyy-MM-dd HH:mm:ss");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(20);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "dd MMM yyyy HH:mm:ss");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(19);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "d MMM yyyy HH:mm:ss");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(11);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "dd MMM yyyy");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(10);
    timeZone = ds.mid(temp.length()+1, 3);
    dt = locale.toDateTime(temp, "d MMM yyyy");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    temp = ds.left(10);
    timeZone = ds.mid(temp.length(), 3);
    dt = locale.toDateTime(temp, "yyyy-MM-dd");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");

    // @HACK(arhohryakov:2012.01.01):
    // Формат "dd MMM yy HH:mm:ss" не распознаётся автоматически. Приводим
    // его к формату "dd MMM yyyy HH:mm:ss"
    QString temp2;
    temp2 = ds;  // чтобы сохранить нетронутую ds для вывода, если будет ошибка
    if (70 < ds.mid(7, 2).toInt()) temp2.insert(7, "19");
    else temp2.insert(7, "20");
    temp = temp2.left(20);
    timeZone = ds.mid(temp.length()+1-2, 3);  // "-2", т.к. вставили 2 символа
    dt = locale.toDateTime(temp, "dd MMM yyyy HH:mm:ss");
    if (dt.isValid()) return locale.toString(dt.addSecs(timeZone.toInt() * -3600), "yyyy-MM-ddTHH:mm:ss");
  }

  qWarning() << __LINE__ << "parseDate: error with" << dateString << urlString;
  return QString();
}

/** @brief Обновление счётчиков ленты и всех родительских категорий
 *
 *  Обновляются поля: количество непрочитанных новостей,
 *  количество новых новостей, дата последнего обновления у категорий
 * @param db - база данных
 * @param feedId - идентификатор ленты
 *----------------------------------------------------------------------------*/
int ParseObject::recountFeedCounts(int feedId)
{
  QSqlQuery q;
  QString qStr;

  int feedParId = 0;
  q.exec(QString("SELECT parentId FROM feeds WHERE id=='%1'").arg(feedId));
  if (q.next()) feedParId = q.value(0).toInt();

  int undeleteCount = 0;
  int unreadCount = 0;
  int newNewsCount = 0;

  // Подсчет всех новостей (не помеченных удаленными)
  qStr = QString("SELECT count(id) FROM news WHERE feedId=='%1' AND deleted==0").
      arg(feedId);
  q.exec(qStr);
  if (q.next()) undeleteCount = q.value(0).toInt();

  // Подсчет непрочитанных новостей
  qStr = QString("SELECT count(read) FROM news WHERE feedId=='%1' AND read==0 AND deleted==0").
      arg(feedId);
  q.exec(qStr);
  if (q.next()) unreadCount = q.value(0).toInt();

  // Подсчет новых новостей
  qStr = QString("SELECT count(new) FROM news WHERE feedId=='%1' AND new==1 AND deleted==0").
      arg(feedId);
  q.exec(qStr);
  if (q.next()) newNewsCount = q.value(0).toInt();

  int unreadCountOld = 0;
  int newCountOld = 0;
  int undeleteCountOld = 0;
  qStr = QString("SELECT unread, newCount, undeleteCount FROM feeds WHERE id=='%1'").
      arg(feedId);
  q.exec(qStr);
  if (q.next()) {
    unreadCountOld = q.value(0).toInt();
    newCountOld = q.value(1).toInt();
    undeleteCountOld = q.value(2).toInt();
  }

  if ((unreadCount == unreadCountOld) && (newNewsCount == newCountOld) &&
      (undeleteCount == undeleteCountOld)) {
    return 0;
  }

  // Установка количества непрочитанных новостей в ленту
  // Установка количества новых новостей в ленту
  qStr = QString("UPDATE feeds SET unread='%1', newCount='%2', undeleteCount='%3' "
                 "WHERE id=='%4'").
      arg(unreadCount).arg(newNewsCount).arg(undeleteCount).arg(feedId);
  q.exec(qStr);

  // Пересчитываем счетчики для всех родителей
  int l_feedParId = feedParId;
  while (l_feedParId) {
    QString updated;
    int newCount = 0;

    qStr = QString("SELECT sum(unread), sum(newCount), sum(undeleteCount), "
                   "max(updated) FROM feeds WHERE parentId=='%1'").
        arg(l_feedParId);
    q.exec(qStr);
    if (q.next()) {
      unreadCount   = q.value(0).toInt();
      newCount      = q.value(1).toInt();
      undeleteCount = q.value(2).toInt();
      updated       = q.value(3).toString();
    }
    qStr = QString("UPDATE feeds SET unread='%1', newCount='%2', undeleteCount='%3', "
                   "updated='%4' WHERE id=='%5'").
        arg(unreadCount).arg(newCount).arg(undeleteCount).arg(updated).
        arg(l_feedParId);
    q.exec(qStr);

    q.exec(QString("SELECT parentId FROM feeds WHERE id==%1").arg(l_feedParId));
    if (q.next()) l_feedParId = q.value(0).toInt();
  }

  return (newNewsCount - newCountOld);
}
