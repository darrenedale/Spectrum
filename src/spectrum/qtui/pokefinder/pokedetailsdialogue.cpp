//
// Created by darren on 11/10/2026.
//

#include "pokedetailsdialogue.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpressionValidator>

#include "../filenamewidget.h"
#include "../hexspinbox.h"

using namespace Spectrum::QtUi;

namespace
{
    constexpr auto WindowTitleCreate = "Save poke";
    constexpr auto WindowTitleAdd = "Add poke";
}

PokeDetailsDialogue::PokeDetailsDialogue(QWidget *parent) noexcept
: PokeDetailsDialogue(QLatin1String(""), 0, 0, Mode::Create, parent)
{}

PokeDetailsDialogue::PokeDetailsDialogue(const QString & name, const Z80::UnsignedWord address, const Z80::UnsignedByte value, const Mode mode, QWidget * parent) noexcept
: QDialog(parent),
  m_mode(mode),
  m_address(address),
  m_filenameWidget(std::make_unique<FilenameWidget>(FilenameWidget::Type::Save, this)),
  m_nameWidget(std::make_unique<QLineEdit>(name, this)),
  m_addressDisplay(std::make_unique<QLabel>(this)),
  m_valueWidget(std::make_unique<QSpinBox>(this))
{
    setWindowTitle(Mode::Create == mode ? tr(WindowTitleCreate) : tr(WindowTitleAdd));
    m_nameWidget->setPlaceholderText(tr("Poke name"));

    m_nameWidget->setValidator(
        new QRegularExpressionValidator(QRegularExpression("\\s*\\w+.*"), m_nameWidget.get())
    );

    m_valueWidget->setRange(0, 255);
    m_valueWidget->setValue(value);

    m_filenameWidget->setFilter(QStringLiteral("Poke Files (*.pok)"));
    setAddress(address);
    layoutWidget();
}

PokeDetailsDialogue::~PokeDetailsDialogue() noexcept = default;

void PokeDetailsDialogue::setMode(const Mode mode) noexcept
{
    m_mode = mode;
    setWindowTitle(Mode::Create == mode ? tr(WindowTitleCreate) : tr(WindowTitleAdd));
    m_filenameWidget->setType(mode == Mode::Create ? FilenameWidget::Type::Save : FilenameWidget::Type::Open);
}

void PokeDetailsDialogue::setName(const QString& name) noexcept
{
    m_nameWidget->setText(name);
}

QString PokeDetailsDialogue::name() const noexcept
{
    return m_nameWidget->text();
}

void PokeDetailsDialogue::setFilename(const QString& name) noexcept
{
    m_filenameWidget->setFilename(name);
}

QString PokeDetailsDialogue::filename() const noexcept
{
    return m_filenameWidget->filename();
}

Z80::UnsignedWord PokeDetailsDialogue::address() const noexcept
{
    return m_address;
}

void PokeDetailsDialogue::setAddress(const Z80::UnsignedWord address) noexcept
{
    m_address = address;
    m_addressDisplay->setText(QStringLiteral("0x%1").arg(address, 4, 16, '0'));
}

Z80::UnsignedByte PokeDetailsDialogue::value() const noexcept
{
    return m_valueWidget->value();
}

void PokeDetailsDialogue::setValue(const Z80::UnsignedByte value) noexcept
{
    m_valueWidget->setValue(value);
}

void PokeDetailsDialogue::layoutWidget() noexcept
{
    auto * groupLayout = new QVBoxLayout(this);
    groupLayout->addWidget(m_filenameWidget.get(), 0);

    auto * layout = new QHBoxLayout(this);
    layout->addWidget(m_nameWidget.get(), 1);
    layout->addWidget(new QLabel(tr("Address")), 0);
    layout->addWidget(m_addressDisplay.get(), 0);
    layout->addWidget(new QLabel(tr("Value")), 0);
    layout->addWidget(m_valueWidget.get(), 0);
    groupLayout->addLayout(layout);

    auto * groupBox = new QGroupBox(tr("Poke"), this);
    groupBox->setLayout(groupLayout);

    auto * controls = new QDialogButtonBox(this);
    controls->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

    auto * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(groupBox, 0);
    mainLayout->addStretch(1);
    mainLayout->addWidget(controls, 0);
    setLayout(mainLayout);

    auto geometry = this->geometry();
    geometry.setWidth(400);
    geometry.setHeight(200);
    setGeometry(geometry);

    connect(controls, &QDialogButtonBox::accepted, this, &PokeDetailsDialogue::accept);
    connect(controls, &QDialogButtonBox::rejected, this, &PokeDetailsDialogue::reject);
}
