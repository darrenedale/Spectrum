
#include <print>

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QToolBar>
#include <QVBoxLayout>

#include "pokefinderwindow.h"

#include <QHeaderView>

#include "pokefindermodel.h"
#include "../application.h"
#include "../thread.h"
#include "../../../util/assert.h"
#include "../../../util/debug.h"

using namespace Spectrum::QtUi::PokeFinder;

PokeFinderWindow::PokeFinderWindow(QWidget * parent )
: PokeFinderWindow(nullptr, parent)
{}

PokeFinderWindow::PokeFinderWindow(Thread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start"), Application::icon(QStringLiteral("resume"))), tr("Resume")),
  m_before(new QSpinBox()),
  m_scanBefore(new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), "")),
  m_after(new QSpinBox()),
  m_scanAfter(new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), "")),
  m_addressList(new QTreeView()),
  m_clear(new QPushButton(QIcon(QStringLiteral("edit-clear-list")), tr("Clear"))),
  m_pokeFinder(thread->spectrum().memory())
{
    setWindowTitle(tr("Spectrum Poke Finder"));
    
    m_before->setMinimum(0);
    m_before->setMaximum(0xffff);
    m_before->setToolTip(tr("Search for all addresses in the Spectrum's memory that contain this value."));

    m_scanBefore->setToolTip("Start a new poke search by looking for all addresses in the Spectrum memory that contain this value.");

    m_after->setMinimum(0);
    m_after->setMaximum(0xffff);
    m_before->setToolTip(tr("Search for this value in all addresses in the Spectrum's memory that were identified as containing the <i>before</i> value."));

    m_scanAfter->setToolTip("Search for addresses that have changed from the <i>before</i> value to this value.");
    m_scanAfter->setEnabled(false);

    m_clear->setToolTip(tr("Clear all search results and values and start a new search."));

    auto * model = new PokeFinderModel();
    model->setHorizontalHeaderLabels({tr("Address"), tr("Before"), tr("After")});
    m_addressList->setModel(model);

    createToolbars();
    layoutWidget();

	connect(m_thread, &Thread::paused, this, &PokeFinderWindow::threadPaused, Qt::UniqueConnection);
	connect(m_thread, &Thread::resumed, this, &PokeFinderWindow::threadResumed, Qt::UniqueConnection);
	connect(m_thread, &Thread::spectrumChanged, this, &PokeFinderWindow::threadSpectrumChanged, Qt::UniqueConnection);

	connectWidgets();

	if (m_thread->isPaused()) {
	    threadPaused();
	} else {
	    threadResumed();
	}
}

PokeFinderWindow::~PokeFinderWindow() = default;

void PokeFinderWindow::createToolbars()
{
    auto * toolBar = addToolBar(tr("Poke Finder"));
    toolBar->setObjectName(QStringLiteral("pokeFinder-toolBar"));
    toolBar->addAction(&m_pauseResume);
}

void PokeFinderWindow::layoutWidget()
{
	// lives
	auto * lives = new QGroupBox(tr("Lives"));
	auto * layout = new QHBoxLayout();

    auto * before = new QVBoxLayout();
    before->addWidget(new QLabel(tr("Before")));
    auto * inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_before, 1);
    inputLayout->addWidget(m_scanBefore, 0);
    before->addLayout(inputLayout);

    auto * after = new QVBoxLayout();
    after->addWidget(new QLabel(tr("After")));
    inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_after, 1);
    inputLayout->addWidget(m_scanAfter, 0);
    after->addLayout(inputLayout);

    layout->addLayout(before);
    layout->addLayout(after);
    lives->setLayout(layout);

    auto * controlBox = new QHBoxLayout();
    controlBox->addStretch(10);
    controlBox->addWidget(m_clear, 1);

	auto * mainLayout = new QVBoxLayout();
    auto * guidance = new QLabel(tr("Run the game, then pause it when you can see the number of lives. Enter the lives in the <i>before</i> widget, then lose a life. Enter the new lives in the <i>after</i> widget and the poke finder will try to find the memory addresses where it's stored."));
    guidance->setWordWrap(true);
    mainLayout->addWidget(guidance, 0);
	mainLayout->addWidget(lives, 0);
    mainLayout->addWidget(m_addressList, 1);
	mainLayout->addLayout(controlBox, 0);

    auto * centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void PokeFinderWindow::connectWidgets()
{
    connect(&m_pauseResume, &QAction::triggered, this, &PokeFinderWindow::pauseResumeTriggered);

    connect(m_before, &QSpinBox::valueChanged, this, [this](const int lives) {
        m_pokeFinder.setLives(static_cast<PokeFinder::Word>(lives));
        auto * model = dynamic_cast<PokeFinderModel *>(m_addressList->model());
        sp_assert(model != nullptr, "address list model should be PokeFinderModel, found {}", typeid(m_addressList->model()).name());
        model->setBeforeAddresses(lives, m_pokeFinder.matchingBeforeAddresses());
        m_scanAfter->setEnabled(true);
    });

    connect(m_scanBefore, &QPushButton::clicked, this, [this](const int lives) {
        m_pokeFinder.setLives(static_cast<PokeFinder::Word>(lives));
        auto * model = dynamic_cast<PokeFinderModel *>(m_addressList->model());
        sp_assert(model != nullptr, "address list model should be PokeFinderModel, found {}", typeid(m_addressList->model()).name());
        model->setBeforeAddresses(lives, m_pokeFinder.matchingBeforeAddresses());
        m_scanAfter->setEnabled(true);
    });

    connect(m_after, &QSpinBox::editingFinished, this, [this] {
        auto * model = dynamic_cast<PokeFinderModel *>(m_addressList->model());
        sp_assert(model != nullptr, "address list model should be PokeFinderModel, found {}", typeid(m_addressList->model()).name());
        const auto lives = m_after->value();
        model->setAfterAddresses(lives, m_pokeFinder.matchingAfterAddresses(lives));
        m_scanAfter->setEnabled(true);
        // TODO sort
    });

    connect(m_scanAfter, &QPushButton::clicked, this, [this]() {
        auto * model = dynamic_cast<PokeFinderModel *>(m_addressList->model());
        sp_assert(model != nullptr, "address list model should be PokeFinderModel, found {}", typeid(m_addressList->model()).name());
        const auto lives = m_after->value();
        model->setAfterAddresses(lives, m_pokeFinder.matchingAfterAddresses(lives));
        // TODO sort
    });

    connect(m_clear, &QPushButton::clicked, this, [this] {
        m_before->clear();
        m_after->clear();
        m_addressList->model()->removeRows(0, m_addressList->model()->rowCount());
        m_scanAfter->setEnabled(false);
    });
}

void PokeFinderWindow::closeEvent(QCloseEvent * ev)
{
    // TODO settings

    QMainWindow::closeEvent(ev);
}

void PokeFinderWindow::showEvent(QShowEvent * event)
{
    // TODO settings

    setGeometry({geometry().x(), geometry().y(), 500, 400});
    QMainWindow::showEvent(event);
}

void PokeFinderWindow::pauseResumeTriggered()
{
    sp_assert(m_thread, "detected null thread in PokeFinderWindow::pauseResumeTriggered()");

    if (m_thread->isPaused()) {
        m_thread->resume();
    } else {
        m_thread->pause();
    }
}

void PokeFinderWindow::threadPaused()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start"), Application::icon(QStringLiteral("resume"))));
    m_pauseResume.setText(tr("Resume"));
    centralWidget()->setEnabled(true);
}

void PokeFinderWindow::threadResumed()
{
    m_pauseResume.setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause"), Application::icon(QStringLiteral("pause"))));
    m_pauseResume.setText(tr("Pause"));
    centralWidget()->setEnabled(false);
}

void PokeFinderWindow::threadSpectrumChanged()
{
    const auto & spectrum = m_thread->spectrum();
    m_pokeFinder.setMemory(spectrum.memory());
}
