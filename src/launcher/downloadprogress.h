#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QWidget>
// https://bugreports.qt.io/browse/QTBUG-73263
#include <filesystem>

namespace Ui
{
class DownloadProgress;
}

namespace launcher
{
class DownloadProgress : public QWidget
{
  Q_OBJECT

public:
  explicit DownloadProgress(QUrl url, std::filesystem::path target, QWidget* parent = nullptr);
  ~DownloadProgress() override;

  void start();

signals:
  void downloaded(std::filesystem::path target);

private slots:
  void finished();
  void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
  void errorOccurred(QNetworkReply::NetworkError error);

private:
  Ui::DownloadProgress* ui;
  const QUrl m_url;
  QNetworkAccessManager* m_accessManager;
  QNetworkReply* m_reply = nullptr;
  std::filesystem::path m_target;
};
} // namespace setup
