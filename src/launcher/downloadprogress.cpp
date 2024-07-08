#include "downloadprogress.h"

#include "ui_downloadprogress.h"

#include <boost/log/trivial.hpp>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <ios>
#include <QMessageBox>
#include <utility>

namespace launcher
{
DownloadProgress::DownloadProgress(QUrl url, std::filesystem::path target, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DownloadProgress)
    , m_url{std::move(url)}
    , m_accessManager{new QNetworkAccessManager{this}}
    , m_target{std::move(target)}
{
  ui->setupUi(this);
  ui->url->setText(m_url.toString());
  connect(m_accessManager, &QNetworkAccessManager::finished, this, &DownloadProgress::finished);
  setWindowFlag(Qt::WindowType::Dialog);

  if(std::filesystem::is_regular_file(m_target))
  {
    emit downloaded(m_target);
    close();
  }
}

DownloadProgress::~DownloadProgress()
{
  delete ui;
  m_reply->deleteLater();
  m_reply = nullptr;
  m_accessManager->deleteLater();
  m_accessManager = nullptr;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void DownloadProgress::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  ui->progressBar->setMaximum(gsl::narrow<int>(bytesTotal));
  ui->progressBar->setValue(gsl::narrow<int>(bytesReceived));
}

void DownloadProgress::finished()
{
  gsl_Assert(m_reply != nullptr);
  if(m_reply->error() != QNetworkReply::NetworkError::NoError)
  {
    BOOST_LOG_TRIVIAL(error) << "download failed: " << m_reply->errorString().toStdString();
    close();
    return;
  }

  std::filesystem::create_directories(m_target.parent_path());
  const auto downloadedData = m_reply->readAll();
  std::ofstream tmp{m_target, std::ios::binary | std::ios::trunc};
  gsl_Assert(tmp.is_open());
  tmp.write(downloadedData.data(), downloadedData.size());
  emit downloaded(m_target);
  close();
}

void DownloadProgress::start()
{
  if(m_reply != nullptr)
    return;

  if(std::filesystem::is_regular_file(m_target))
  {
    emit downloaded(m_target);
    close();
    return;
  }

  gsl_Assert(m_accessManager != nullptr);
  m_reply = m_accessManager->get(QNetworkRequest(m_url));
  connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadProgress::downloadProgress);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
  connect(m_reply, &QNetworkReply::errorOccurred, this, &DownloadProgress::errorOccurred);
#else
  connect(
    m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), this, &DownloadProgress::errorOccurred);
#endif
}

void DownloadProgress::errorOccurred(QNetworkReply::NetworkError /*error*/)
{
  gsl_Assert(m_reply != nullptr);
  BOOST_LOG_TRIVIAL(error) << "download failed: " << m_reply->errorString().toStdString();
  QMessageBox::critical(
    this, tr("Download Failed"), tr("The download failed with an error: %1").arg(m_reply->errorString()));
}
} // namespace launcher
