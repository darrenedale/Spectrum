//
// Created by darren on 25/02/2021.
//

#include <fstream>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QApplication>
#include "mainwindow.h"
#include "../qtui/qimagedisplaydevice.h"

using namespace Spectrum::ScreenView;

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

MainWindow::~MainWindow() = default;

void MainWindow::loadScreen(const QString & fileName)
{
    m_fileName->setText(fileName);
    std::ifstream in(fileName.toStdString());

    if (!in) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("The file '%1' could not be opened.").arg(QFileInfo(fileName).fileName()));
        return;
    }

    in.read(reinterpret_cast<std::ifstream::char_type *>(m_screenData.data()), Spectrum::DisplayFile::extent);

    if (in.fail()) {
        QMessageBox::critical(this, QApplication::applicationDisplayName(), tr("The file '%1' could not be read.").arg(QFileInfo(fileName).fileName()));
        return;
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
    display.redrawDisplay(DisplayFile(m_screenData.data(), DisplayFile::extent));
    display.setBorder(m_borderColour->colour(), m_borderColour->isBright());
    m_display->setImage(display.image());
}
