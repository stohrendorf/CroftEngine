#include "downloadprogress.h"

#include "ui_downloadprogress.h"

#include <fstream>
#include <gsl/gsl-lite.hpp>
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
}

DownloadProgress::~DownloadProgress()
{
  delete ui;
  m_reply->deleteLater();
  m_reply = nullptr;
  m_accessManager->deleteLater();
  m_accessManager = nullptr;
}

void DownloadProgress::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  ui->progressBar->setMaximum(bytesTotal);
  ui->progressBar->setValue(bytesReceived);
}

void DownloadProgress::finished()
{
  gsl_Assert(m_reply != nullptr);
  if(m_reply->error() != QNetworkReply::NetworkError::NoError)
  {
    close();
    return;
  }

  std::error_code ec;
  std::filesystem::create_directories(m_target.parent_path(), ec);
  const auto dowloadedData = m_reply->readAll();
  std::ofstream tmp{m_target, std::ios::binary | std::ios::trunc};
  Expects(tmp.is_open());
  tmp.write(dowloadedData.data(), dowloadedData.size());
  emit downloaded(m_target);
  close();
}

void DownloadProgress::start()
{
  if(m_reply != nullptr)
    return;

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
  QMessageBox::critical(
    this, tr("Download Failed"), tr("The download failed with an error: %1").arg(m_reply->errorString()));
}
} // namespace launcher
