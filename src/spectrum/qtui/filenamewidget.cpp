//
// Created by darren on 12/10/2026.
//

#include <QLineEdit>
#include <QToolButton>

#include "filenamewidget.h"

#include <QFileDialog>
#include <QHBoxLayout>

#include "application.h"

using namespace Spectrum::QtUi;

FilenameWidget::FilenameWidget(const Type type, QWidget * parent) noexcept
: FilenameWidget(type, QLatin1String(""), parent)
{
}

FilenameWidget::FilenameWidget(const Type type, const QString & fileName, QWidget * parent) noexcept
: QWidget(parent),
  m_type(type),
  m_filename(std::make_unique<QLineEdit>(this)),
  m_choose(std::make_unique<QToolButton>(this))
{
    m_filename->setPlaceholderText(tr("File name"));

    if (Type::Save == m_type) {
        m_choose->setIcon(QIcon::fromTheme(
            QStringLiteral("file-save"),
            Application::icon(QStringLiteral("save"))
        ));
    } else {
        m_choose->setIcon(QIcon::fromTheme(
            QStringLiteral("file-open"),
            Application::icon(QStringLiteral("open"))
        ));
    }

    layoutWidget();
    connectWidgets();
}

FilenameWidget::~FilenameWidget() noexcept = default;

void FilenameWidget::setType(const Type type) noexcept
{
    m_type = type;

    if (Type::Save == m_type) {
        m_choose->setIcon(QIcon::fromTheme(
            QStringLiteral("file-save"),
            Application::icon(QStringLiteral("save"))
        ));
    } else {
        m_choose->setIcon(QIcon::fromTheme(
            QStringLiteral("file-open"),
            Application::icon(QStringLiteral("open"))
        ));
    }
}

bool FilenameWidget::hasFilename() const noexcept
{
    return !m_filename->text().isEmpty();
}

void FilenameWidget::setFilename(const QString & name) noexcept
{
    m_filename->setText(name);
}

QString FilenameWidget::filename() const noexcept
{
    return m_filename->text();
}

void FilenameWidget::setFilter(const QString & filter) noexcept
{
    m_filter = filter;
}

void FilenameWidget::layoutWidget() noexcept
{
    auto * layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_filename.get(), 1);
    layout->addWidget(m_choose.get(), 0);
    setLayout(layout);
}

void FilenameWidget::connectWidgets() noexcept
{
    connect(m_choose.get(), &QToolButton::clicked, this, &FilenameWidget::chooseFileActivated);
    connect(m_filename.get(), &QLineEdit::textChanged, this, &FilenameWidget::filenameChanged);

    connect(m_choose.get(), &QToolButton::clicked, [this]() {
        QString fileName;

        if (Type::Open == m_type) {
            fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open"),
                m_filename->text(),
                m_filter,
                nullptr
            );
        } else {
            fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save"),
                m_filename->text(),
                m_filter,
                nullptr
            );
        }

        if (fileName.isEmpty()) {
            return;
        }

        setFilename(fileName);
    });
}
