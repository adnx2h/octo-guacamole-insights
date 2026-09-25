#ifndef CHESSCOMHANDLER_H
#define CHESSCOMHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

class ChessComHandler : public QObject
{
    Q_OBJECT

public:
    explicit ChessComHandler(QObject *parent = nullptr);

    // Entry point: Fetch recent games for a given user
    Q_INVOKABLE void fetchRecentGames(const QString &username, const QString m_year = "0", const QString m_month = "0");

signals:
    void sgn_gamesFetchedSuccess(const QVariantList &gamesList);
    void sgn_gamesFetchFailed(const QString &errorMessage);

private slots:
    void onGamesResponseReceived(QNetworkReply *reply);

private:
    QNetworkAccessManager m_networkManager;
    QString m_username, m_year, m_month;
};

#endif // CHESSCOMHANDLER_H
