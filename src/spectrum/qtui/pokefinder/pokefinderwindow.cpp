
#include <print>
#include <string>

#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "pokefindermodel.h"
#include "pokefindersortfiltermodel.h"
#include "pokefinderwindow.h"

#include <filesystem>
#include <QSettings>

#include "pokedetailsdialogue.h"
#include "../application.h"
#include "../pokewidget.h"
#include "../thread.h"
#include "../../pokedefinition.h"
#include "../../io/pokfilereader.h"
#include "../../io/pokfilewriter.h"
#include "../../../util/assert.h"
#include "../../../util/debug.h"

using namespace std::string_literals;
using namespace Spectrum::QtUi::PokeFinder;

namespace
{
    using namespace Spectrum::Io;
    using namespace Spectrum::QtUi;

    constexpr int SoughtValueSentinel = -1;

    /** Helper to create a new .pok file with a given poke. */
    void createPokFile(const Spectrum::PokeDefinition & poke, const std::string & filename) noexcept
    {
        auto writer = PokFileWriter(filename);
        writer.addPoke(poke);
        writer.close();

        Application::showNotification(
            QObject::tr("Created .pok file \"%1\".")
#if QT_VERSION > QT_VERSION_CHECK(6, 9, 0)
            .arg(filename)
#else
            .arg(filename.data())
#endif
        );
    }

    /** Helper to append a poke to an existing .pok file. */
    void addToPokFile(const Spectrum::PokeDefinition & poke, const std::string & filename) noexcept
    {
        assert(std::filesystem::exists(filename));
        std::vector pokes = {poke};

        {
            auto reader = PokFileReader(filename);

            while (reader.hasMorePokes()) {
                pokes.push_back(std::move(*reader.nextPoke()));
            }
        }

        auto writer = PokFileWriter(filename);

        for (const auto & writePoke : pokes) {
            writer.addPoke(writePoke);
        }

        writer.close();

        Application::showNotification(
            QObject::tr("Added poke to .pok file \"%1\".")
    #if QT_VERSION > QT_VERSION_CHECK(6, 9, 0)
            .arg(filename)
    #else
            .arg(filename.data())
    #endif
        );
    }
}

PokeFinderWindow::PokeFinderWindow(QWidget * parent)
: PokeFinderWindow(nullptr, parent)
{}

PokeFinderWindow::PokeFinderWindow(Thread * thread, QWidget * parent )
: QMainWindow(parent),
  m_thread(thread),
  m_pauseResume(QIcon::fromTheme(QStringLiteral("media-playback-start"), Application::icon(QStringLiteral("resume"))), tr("Resume")),
  m_excludeRom(QIcon::fromTheme(QStringLiteral("lock"), Application::icon(QStringLiteral("lock"))), tr("Don't search ROM")),
  m_hideMismatches(QIcon::fromTheme(QStringLiteral("filter"), Application::icon(QStringLiteral("filter"))),tr("Keep mismatches visible")),
  m_before(new QSpinBox()),
  m_scanBefore(new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), "")),
  m_after(new QSpinBox()),
  m_scanAfter(new QPushButton(QIcon::fromTheme(QStringLiteral("view-refresh")), "")),
  m_addressList(new QTreeView()),
  m_clear(new QPushButton(QIcon::fromTheme(QStringLiteral("edit-clear-all"), Application::icon(QStringLiteral("clear"))), tr("Clear"))),
  m_pokeFinder(thread->spectrum().memory())
{
    setWindowTitle(tr("Spectrum Poke Finder"));

    m_pauseResume.setShortcuts({Qt::Key::Key_Pause, Qt::Key::Key_Escape, tr("Ctrl+P")});

    m_before->setMinimum(-1);
    m_before->setMaximum(0xffff);
    m_before->setToolTip(tr("Search for all addresses in the Spectrum's memory that contain this value."));
    m_before->setSpecialValueText(tr("None"));
    m_before->setValue(SoughtValueSentinel);

    m_scanBefore->setToolTip("Start a new poke search by looking for all addresses in the Spectrum memory that contain this value.");

    m_after->setMinimum(-1);
    m_after->setMaximum(0xffff);
    m_after->setSpecialValueText(tr("None"));
    m_after->setToolTip(tr("Search for this value in all addresses in the Spectrum's memory that were identified as containing the <i>before</i> value."));
    m_after->setValue(SoughtValueSentinel);

    m_scanAfter->setToolTip("Search for addresses that have changed from the <i>before</i> value to this value.");
    m_scanAfter->setEnabled(false);

    m_clear->setToolTip(tr("Clear all search results and values and start a new search."));
    m_clear->setShortcut(tr("Ctrl+C"));

    m_excludeRom.setCheckable(true);
    m_excludeRom.setChecked(true);
    m_excludeRom.setToolTip(tr("Exclude addresses in the Spectrum ROM address space (0x0000 - 0x3fff) from the search results."));
    m_hideMismatches.setCheckable(true);
    m_hideMismatches.setChecked(true);
    m_hideMismatches.setToolTip(tr("Filter out addresses that don't match the \"after\" value."));

    auto * model = new PokeFinderModel();
    model->setHorizontalHeaderLabels({tr("Address"), tr("Before"), tr("After")});
    auto * proxyModel = new PokeFinderSortFilterModel();
    proxyModel->setSourceModel(model);
    proxyModel->sort(1, Qt::AscendingOrder);
    proxyModel->hideNonMatchingAddresses(true);
    proxyModel->hideRomAddresses(true);
    m_addressList->setModel(proxyModel);
    m_addressList->setContextMenuPolicy(Qt::CustomContextMenu);

    createToolbars();
    layoutWidget();
	connectWidgets();

    connect(m_thread, &Thread::paused, this, &PokeFinderWindow::threadPaused, Qt::UniqueConnection);
    connect(m_thread, &Thread::resumed, this, &PokeFinderWindow::threadResumed, Qt::UniqueConnection);
    connect(m_thread, &Thread::spectrumChanged, this, &PokeFinderWindow::threadSpectrumChanged, Qt::UniqueConnection);

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
    toolBar->addSeparator();
    toolBar->addAction(&m_excludeRom);
    toolBar->addAction(&m_hideMismatches);
}

void PokeFinderWindow::layoutWidget()
{
	auto * values = new QGroupBox(tr("Search values"));
	auto * layout = new QHBoxLayout();
    values->setToolTip(tr("Enter the values you want to search for while the Spectrum program is running."));

    auto * before = new QVBoxLayout();
    auto * label = new QLabel(tr("Before"));
    before->addWidget(label);
    label->setBuddy(m_before);
    auto * inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_before, 1);
    inputLayout->addWidget(m_scanBefore, 0);
    before->addLayout(inputLayout);
    m_before->setToolTip(tr("The value to search for before the state change in the Spectrum program.\n\nTypically this will be the number of lives before you lose one.\n\nCtrl+1."));
    label->setToolTip(m_before->toolTip());

    auto * after = new QVBoxLayout();
    after->addWidget(new QLabel(tr("After")));
    inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_after, 1);
    inputLayout->addWidget(m_scanAfter, 0);
    after->addLayout(inputLayout);
    m_after->setToolTip(tr("The value to search for after the state change in the Spectrum program.\n\nTypically this will be the number of lives after you lose one.\n\nCtrl+2"));
    label->setToolTip(m_after->toolTip());

    layout->addLayout(before);
    layout->addLayout(after);
    values->setLayout(layout);

    auto * controlBox = new QHBoxLayout();
    controlBox->addStretch(10);
    controlBox->addWidget(m_clear, 1);

	auto * mainLayout = new QVBoxLayout();
	mainLayout->addWidget(values, 0);
    mainLayout->addWidget(m_addressList, 1);
	mainLayout->addLayout(controlBox, 0);

    auto * centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void PokeFinderWindow::connectWidgets()
{
    connect(&m_pauseResume, &QAction::triggered, this, &PokeFinderWindow::pauseResumeTriggered);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Re-scan the Spectrum memory for the "before" value when it changes
    connect(m_before, &QSpinBox::valueChanged, this, &PokeFinderWindow::rescanBeforeValue);

    // Re-scan the matching "before" addresses the "after" value when it changes
    connect(m_after, &QSpinBox::valueChanged, this, &PokeFinderWindow::rescanAfterValue);
#else
    // Qt5 needs the valueChanged overload to be explicitly resolved

    // Re-scan the Spectrum memory for the "before" value when it changes
    connect(m_before, QOverload<int>::of(&QSpinBox::valueChanged), this, &PokeFinderWindow::rescanBeforeValue);

    // Re-scan the matching "before" addresses the "after" value when it changes
    connect(m_after, QOverload<int>::of(&QSpinBox::valueChanged), this, &PokeFinderWindow::rescanAfterValue);
#endif

    connect(m_before, &QSpinBox::textChanged, this, [this](const QString & valueText) {
        if (valueText.isEmpty()) {
            m_before->setValue(SoughtValueSentinel);
        }
    });

    connect(m_after, &QSpinBox::textChanged, this, [this](const QString & valueText) {
        if (valueText.isEmpty()) {
            m_after->setValue(SoughtValueSentinel);
        }
    });

    connect(m_scanBefore, &QAbstractButton::clicked, this, [this]() {
        const auto beforeValue = m_before->value();
        m_pokeFinder.setBefore(static_cast<PokeFinder::Word>(beforeValue));

        addressListModel()->setBeforeAddresses(beforeValue, m_pokeFinder.matchingBeforeAddresses());
        addressListProxyModel()->setFilterRegularExpression("");

        m_scanAfter->setEnabled(true);
    });

    connect(m_scanAfter, &QAbstractButton::clicked, this, [this]() {
        const auto afterValue = m_after->value();
        addressListModel()->setAfterAddresses(afterValue, m_pokeFinder.matchingAfterAddresses(afterValue));

        auto * proxyModel = addressListProxyModel();
        proxyModel->setFilterRegularExpression("\\d+");
        proxyModel->setFilterKeyColumn(2);
    });

    connect(m_clear, &QAbstractButton::clicked, this, [this] {
        m_before->setValue(SoughtValueSentinel);
        m_after->setValue(SoughtValueSentinel);

        auto * model = addressListModel();
        model->removeRows(0, model->rowCount());
        addressListProxyModel()->setFilterRegularExpression("");
        m_scanAfter->setEnabled(false);
    });

    connect(m_addressList, &QWidget::customContextMenuRequested, this, &PokeFinderWindow::showAddressListContextMenu);
    connect(&m_hideMismatches, &QAction::triggered, addressListProxyModel(), &PokeFinderSortFilterModel::hideNonMatchingAddresses);
    connect(&m_excludeRom, &QAction::triggered, addressListProxyModel(), &PokeFinderSortFilterModel::hideRomAddresses);
}

void PokeFinderWindow::closeEvent(QCloseEvent * ev) noexcept
{
    saveSettings();
    QMainWindow::closeEvent(ev);
}

void PokeFinderWindow::showEvent(QShowEvent * event) noexcept
{
    loadSettings();
    m_before->clear();
    m_after->clear();
    m_before->setFocus();
    QMainWindow::showEvent(event);
}

void PokeFinderWindow::keyReleaseEvent(QKeyEvent * event) noexcept
{
    if (event->key() == Qt::Key_1 && event->modifiers() & Qt::ControlModifier) {
        m_before->setFocus();
        m_before->selectAll();
    } else if (event->key() == Qt::Key_2 && event->modifiers() & Qt::ControlModifier) {
        m_after->setFocus();
        m_after->selectAll();
    }
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

void PokeFinderWindow::loadSettings() noexcept
{
    QSettings settings;

    // We prefix the group name rather than nesting groups because we save as .ini files, and .ini files don't support group
    // nesting, so Qt prefixes each setting with the full sub-group path using \ as a separator. It doesn't matter much here
    // because only Qt will read these main window settings, but with core emulator settings and potential other UIs we want
    // the config file to be parseable without having to handle the Qt-isms
#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-pokefinderwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-pokefinderwindow"));
#endif

    Util::debug("PokeFinderWindow::loadSettings()");
    if (settings.contains(QStringLiteral("geometry"))) {
        if (const auto rect = settings.value(QStringLiteral("geometry")).toRect(); rect.isValid()) {
            setGeometry(rect);
        }
    }

    if (settings.contains(QStringLiteral("exclude-rom"))) {
        if (const auto setting = settings.value(QStringLiteral("exclude-rom")); setting.canConvert<bool>()) {
            m_excludeRom.setChecked(setting.toBool());
        }
    }

    if (settings.contains(QStringLiteral("hide-mismatches"))) {
        if (const auto setting = settings.value(QStringLiteral("hide-mismatches")); setting.canConvert<bool>()) {
            m_hideMismatches.setChecked(setting.toBool());
        }
    }

    settings.endGroup();
}

void PokeFinderWindow::saveSettings() const noexcept
{
    QSettings settings;

    // We prefix the group name rather than nesting groups because we save as .ini files, and .ini files don't support group
    // nesting, so Qt prefixes each setting with the full sub-group path using \ as a separator. It doesn't matter much here
    // because only Qt will read these main window settings, but with core emulator settings and potential other UIs we want
    // the config file to be parseable without having to handle the Qt-isms
#if defined(USE_QT_5)
    settings.beginGroup(QStringLiteral("qt5-pokefinderwindow"));
#else
    settings.beginGroup(QStringLiteral("qt6-pokefinderwindow"));
#endif

    Util::debug("PokeFinderWindow::loadSettings()");
    settings.setValue(QStringLiteral("geometry"), geometry());
    settings.setValue(QStringLiteral("exclude-rom"), m_excludeRom.isChecked());
    settings.setValue(QStringLiteral("hide-mismatches"), m_hideMismatches.isChecked());
    settings.endGroup();
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

PokeFinderSortFilterModel * PokeFinderWindow::addressListProxyModel() const noexcept
{
    auto * model =  dynamic_cast<PokeFinderSortFilterModel *>(m_addressList->model());
    sp_assert(nullptr != model, "address list model should be PokeFinderSortFilterModel, found {}", typeid(m_addressList->model()).name());
    return model;
}

PokeFinderModel * PokeFinderWindow::addressListModel() const noexcept
{
    auto * model = dynamic_cast<PokeFinderModel *>(addressListProxyModel()->sourceModel());
    sp_assert(nullptr != model, "address list proxy model's source model should be PokeFinderModel, found {}", typeid(m_addressList->model()).name());
    return model;
}

void PokeFinderWindow::rescanBeforeValue(const int value) noexcept
{
    Util::debug("before widget value changed");

    auto * model = addressListModel();

    if (SoughtValueSentinel == value) {
        model->removeRows(0, model->rowCount());
        m_scanAfter->setEnabled(false);
        return;
    }

    m_pokeFinder.setBefore(static_cast<PokeFinder::Word>(value));
    model->setBeforeAddresses(value, m_pokeFinder.matchingBeforeAddresses());
    addressListProxyModel()->setFilterRegularExpression("");
    m_scanAfter->setEnabled(true);
}

void PokeFinderWindow::rescanAfterValue(const int value) noexcept
{
    Util::debug("after widget value edited");

    auto * proxyModel = addressListProxyModel();
    auto * model = addressListModel();

    if (SoughtValueSentinel == value) {
        model->setBeforeAddresses(m_before->value(), m_pokeFinder.matchingBeforeAddresses());
        return;
    }

    model->setAfterAddresses(value, m_pokeFinder.matchingAfterAddresses(value));
    m_scanAfter->setEnabled(true);

    proxyModel->setFilterRegularExpression("\\d+");
    proxyModel->setFilterKeyColumn(2);
}

void PokeFinderWindow::showAddressListContextMenu(const QPoint & pos) noexcept
{
    auto index = m_addressList->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    index = index.siblingAtColumn(2);

    if (!m_addressList->model()->data(index, PokeFinderModel::IsMatchRole).toBool()) {
        Util::debug("item at R{}C{} is not a match", index.row(), index.column());
        return;
    }

    bool ok;
    const auto address = m_addressList->model()->data(index, PokeFinderModel::AddressRole).toUInt(&ok);

    if (!ok) {
        Util::debug("AddressRole data for address list item at R{}C{} is not a valid address", index.row(), index.column());
        return;
    }

    const auto value = m_addressList->model()->data(index, PokeFinderModel::ValueRole).toUInt(&ok);

    if (!ok) {
        Util::debugln("ValueRole data for address list item at R{}C{} is not a valid value", index.row(), index.column());
        return;
    }

    Util::debugln("opening context menu for match address {:#06x} at R{}C{}", address, index.row(), index.column());

    // build the context menu
    QMenu contextMenu(m_addressList);
    contextMenu.addSection(tr("Poke"));

    auto * pokeWidget = new PokeWidget(address, m_thread->spectrum().memory()->readByte(address), PokeWidget::ValueBase::Decimal, this);
    auto * widgetAction = new QWidgetAction(this);
    pokeWidget->focusValue();
    widgetAction->setDefaultWidget(pokeWidget);
    contextMenu.addAction(widgetAction);

    auto * createPokeFileAction = new QAction(tr("Create .pok file"));
    auto * addToPokeFileAction = new QAction(tr("Add to .pok file"));
    contextMenu.addAction(createPokeFileAction);
    contextMenu.addAction(addToPokeFileAction);

    connect(pokeWidget, &PokeWidget::pokeClicked, [this](const ::Z80::UnsignedWord pokeAddress, const ::Z80::UnsignedByte value) {
        m_thread->spectrum().memory()->writeByte(pokeAddress, value);

        Application::showNotification(
            tr("Poked 0x%1 to address 0x%2.")
            .arg(value, 2, 16, '0')
            .arg(pokeAddress, 4, 16, '0')
        );
    });

    connect(createPokeFileAction, &QAction::triggered, [this, address, value]() {
        Util::debugln("create .pok file triggered");
        showSavePokeDialogue(address, value);
    });

    connect(addToPokeFileAction, &QAction::triggered, [this, address, value]() {
        Util::debugln("create .pok file triggered");
        showSavePokeDialogue(address, value, PokeDetailsDialogue::Mode::Add);
    });

    contextMenu.exec(QCursor::pos());
}

void PokeFinderWindow::showSavePokeDialogue(const ::Z80::UnsignedWord address, const ::Z80::UnsignedByte value, PokeDetailsDialogue::Mode mode) noexcept
{
    auto pokeDialogue = PokeDetailsDialogue(this);
    pokeDialogue.setMode(mode);
    pokeDialogue.setAddress(address);
    pokeDialogue.setValue(value);

    if (QDialog::Accepted != pokeDialogue.exec()) {
        return;
    }

    const auto filename = pokeDialogue.filename().toStdString();

    if (filename.empty()) {
        return;
    }

    if (PokeDetailsDialogue::Mode::Add == mode && !std::filesystem::exists(filename)) {
        mode = PokeDetailsDialogue::Mode::Create;
        Application::showNotification(tr("The poke file %1 does not exist, creating a new file.").arg(
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            filename
#else
            QString::fromStdString(filename)
#endif
        ));
    }

    auto pokeName = pokeDialogue.name().toStdString();
    const auto pokeValue = pokeDialogue.value();

    if (pokeName.empty()) {
        const auto start = filename.find_last_of('/');
        auto end = filename.find_last_of('.');

        if (std::string::npos == start) {
            end = 0;
        }

        if (std::string::npos == end) {
            end = filename.length();
        }

        pokeName = filename.substr(start + 1, end - start - 1);

        if (pokeName.empty()) {
            pokeName = "Rainbow-generated poke"s;
        }
    }

    PokeDefinition pokeDefinition;
    pokeDefinition.setName(pokeName);

    pokeDefinition.addPoke(PokeDefinition::Poke{
        .address = static_cast<::Z80::UnsignedWord>(address),
        .bytes = {pokeValue},
    });

    if (PokeDetailsDialogue::Mode::Create == mode) {
        createPokFile(pokeDefinition, filename);
    } else {
        addToPokFile(pokeDefinition, filename);
    }
}
