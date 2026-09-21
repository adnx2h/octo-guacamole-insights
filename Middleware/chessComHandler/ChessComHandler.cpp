#include "ChessComHandler.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QDateTime>

ChessComHandler::ChessComHandler(QObject *parent) : QObject(parent) {}

void ChessComHandler::fetchRecentGames(const QString &username, const QString yr, const QString mnt) {
    if (username.trimmed().isEmpty()) {
        emit sgn_gamesFetchFailed("Username cannot be empty.");
        return;
    }
    m_username=username.trimmed();

    QDate currentDate = QDate::currentDate();
    QString year, month;

    if(yr=="0" && mnt=="0"){
        // Fetch games for current year/month from Chess.com API
         m_year = QString::number(currentDate.year());
         m_month = QString("%1").arg(currentDate.month(), 2, 10, QChar('0'));
    }else{
        m_year = yr;
        m_month = mnt;
    }
    QString urlStr = QString("https://api.chess.com/pub/player/%1/games/%2/%3")
                         .arg(username.trimmed().toLower(), m_year, m_month);

    QNetworkRequest request((QUrl(urlStr)));
    // Chess.com API requires a User-Agent header
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtChessAnalysisApp/1.0 (Contact: user@example.com)");

    QNetworkReply *reply = m_networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onGamesResponseReceived(reply);
    });
}

void ChessComHandler::onGamesResponseReceived(QNetworkReply *reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit sgn_gamesFetchFailed("Network error: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        emit sgn_gamesFetchFailed("Invalid JSON response received.");
        return;
    }

    QJsonArray gamesArray = doc.object()["games"].toArray();

    if(gamesArray.empty()){ //no games played in current month
        // Construct QDate using the current m_year and m_month (using day 1)
        QDate currentDate(m_year.toInt(), m_month.toInt(), 1);

        // Subtract 1 month from the active date
        QDate prevMonthDate = currentDate.addMonths(-1);

        // Safeguard: Stop searching if we step back more than 13 months from today
        if (prevMonthDate < QDate::currentDate().addMonths(-13)) {
            emit sgn_gamesFetchFailed("No recent games found in the last 13 months.");
            return;
        }

        QString year = QString::number(prevMonthDate.year());
        QString month = QString("%1").arg(prevMonthDate.month(), 2, 10, QChar('0'));

        m_year = year;
        m_month = month;

        fetchRecentGames(m_username, year, month);
        return;
    }
    QVariantList parsedGames;

    // Parse in reverse so newest games appear first
    for (int i = gamesArray.size() - 1; i >= 0; --i) {
        QJsonObject gameObj = gamesArray[i].toObject();

        QVariantMap map;
        map["pgn"] = gameObj["pgn"].toString();
        map["timeControl"] = gameObj["time_class"].toString();
        map["url"] = gameObj["url"].toString();

        QJsonObject whiteObj = gameObj["white"].toObject();
        QJsonObject blackObj = gameObj["black"].toObject();

        map["whiteUser"] = whiteObj["username"].toString();
        map["whiteRating"] = whiteObj["rating"].toInt();
        map["whiteResult"] = whiteObj["result"].toString();
        map["blackUser"] = blackObj["username"].toString();
        map["blackRating"] = blackObj["rating"].toInt();
        map["blackResult"] = blackObj["result"].toString();

        // Convert epoch timestamp to formatted date
        qint64 endTime = gameObj["end_time"].toInteger();
        map["date"] = QDateTime::fromSecsSinceEpoch(endTime).toString("yyyy-MM-dd HH:mm");

        parsedGames.append(map);
    }

    emit sgn_gamesFetchedSuccess(parsedGames);
}