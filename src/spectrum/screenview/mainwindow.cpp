//
// Created by darren on 25/02/2021.
//

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include "mainwindow.h"
#include "../qtui/qimagedisplaydevice.h"

using namespace ScreenView;

MainWindow::MainWindow(QWidget * parent)
: QMainWindow(parent),
  m_chooseFile(std::make_unique<QPushButton>(QIcon::fromTheme(QStringLiteral("document-open")), tr(""))),
  m_fileName(std::make_unique<QLineEdit>()),
  m_display(std::make_unique<Spectrum::ImageWidget>()),
  m_borderColour(std::make_unique<ColourCombo>()),
  m_screenData()
{
    auto * mainLayout = new QVBoxLayout(this);
    auto * controlsLayout = new QHBoxLayout(this);

    controlsLayout->addWidget(m_fileName.get());
    controlsLayout->addWidget(m_chooseFile.get());
    controlsLayout->addWidget(new QLabel(tr("Border")));
    controlsLayout->addWidget(m_borderColour.get());

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(m_display.get());

    auto * centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    m_borderColour->setColour(Spectrum::Colour::White, false);

    connect(m_chooseFile.get(), &QPushButton::clicked, this, &MainWindow::chooseScreenFile);

    connect(m_fileName.get(), &QLineEdit::editingFinished, [this]() {
        loadScreen(m_fileName->text());
    });

    connect(m_borderColour.get(), &ColourCombo::colourSelected, this, &MainWindow::updateScreen);
    updateScreen();
}

void MainWindow::loadScreen(const QString & fileName)
{
    m_fileName->setText(fileName);
    FILE * inFile = std::fopen(fileName.toUtf8().data(), "rb");

    if (!inFile) {
        // TODO notify failed to open
        return;
    }

    std::size_t bytesToRead = DisplayFileSize;
    auto * buffer = m_screenData;

    while (!std::feof(inFile) && 0 < bytesToRead) {
        auto bytesRead = std::fread(buffer, sizeof(std::uint8_t), bytesToRead, inFile);

        if (0 == bytesRead) {
            // TODO notify read error
            return;
        }

        buffer += bytesRead;
        bytesToRead -= bytesRead;
    }

    updateScreen();
}

void MainWindow::chooseScreenFile()
{
    static QString lastDir;

    if (lastDir.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QStringLiteral("mainwindow"));
        lastDir = settings.value(QStringLiteral("lastOpenDir")).toString();

        if (lastDir.isEmpty()) {
            lastDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open spectrum screen file"), lastDir);

    if (fileName.isEmpty()) {
        return;
    }

    // TODO extract path from filename for lastDir
    lastDir = QFileInfo(fileName).absolutePath();
    QSettings settings;
    settings.beginGroup(QStringLiteral("mainwindow"));
    settings.setValue(QStringLiteral("lastOpenDir"), lastDir);
    settings.endGroup();
    loadScreen(fileName);
}

void MainWindow::showEvent(QShowEvent * ev)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("mainwindow"));
    setGeometry({settings.value(QStringLiteral("position")).toPoint(), settings.value(QStringLiteral("size")).toSize()});
    QWidget::showEvent(ev);
}

void MainWindow::closeEvent(QCloseEvent * ev)
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("mainwindow"));
    settings.setValue(QStringLiteral("position"), pos());
    settings.setValue(QStringLiteral("size"), size());
    QWidget::closeEvent(ev);
}

void MainWindow::updateScreen()
{
    Spectrum::QtUi::QImageDisplayDevice display;
    display.redrawDisplay(m_screenData);
    display.setBorder(m_borderColour->colour(), m_borderColour->isBright());
    m_display->setImage(display.image());
}

MainWindow::~MainWindow() = default;
