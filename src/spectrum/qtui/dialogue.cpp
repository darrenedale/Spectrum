//
// Created by darren on 24/04/2021.
//

#include <QGridLayout>
#include <QAbstractButton>
#include <QScreen>
#include "dialogue.h"
#include "../../util/debug.h"
#include "widgetupdatesuspender.h"

using namespace Spectrum::QtUi;

namespace
{
    // this is how big the icon is in pixels ...
    constexpr const int DefaultIconExtent = 40;

    // ... at this pixel density
    constexpr const double DefaultIconExtentDpi = 96.0;
}

Dialogue::Dialogue(const QString & message, const QString & title, QWidget * parent)
: QDialog(parent),
  m_message(message),
  m_title(title),
  m_icon(),
  m_iconExtent(DefaultIconExtent),
  m_iconLabel(),
  m_controls()
{
    m_message.setWordWrap(true);

    auto metrics = fontMetrics();

    auto * mainLayout = new QGridLayout();
    mainLayout->setSpacing(metrics.horizontalAdvance(QLatin1Char('W')));

    auto font = this->font();
    font.setPointSizeF(font.pointSizeF() * 1.2);
    font.setBold(true);
    m_title.setFont(font);

    setLayout(mainLayout);
    rebuildLayout();

    connect(&m_controls, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(&m_controls, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

Dialogue::Dialogue(const QString & message, QWidget * parent)
: Dialogue(message, {}, parent)
{}

Dialogue::Dialogue(QWidget * parent)
: Dialogue({}, {}, parent)
{}

Dialogue::~Dialogue() = default;

void Dialogue::showEvent(QShowEvent * ev)
{
    // work out the icon size based on the screen pixel density
    auto iconExtent = static_cast<int>((screen()->logicalDotsPerInchX() / DefaultIconExtentDpi) * DefaultIconExtent);
    Util::debug << "screen DPI: " << screen()->logicalDotsPerInchX() << '\n';
    Util::debug << "Icon size: " << iconExtent << '\n';

    if (iconExtent != m_iconExtent) {
        m_iconExtent = iconExtent;
        m_iconLabel.setPixmap(m_icon.pixmap(m_iconExtent));
    }

    QDialog::showEvent(ev);
}

void Dialogue::setTitle(const QString & title)
{
    m_title.setText(title);
    rebuildLayout();
}

QPushButton * Dialogue::addButton(QDialogButtonBox::StandardButton button)
{
    if (m_controls.parent() != layout()) {
        layout()->addWidget(&m_controls);
    }

    auto * ret = m_controls.addButton(button);
    rebuildLayout();
    return ret;
}

QPushButton * Dialogue::addButton(const QString & text, QDialogButtonBox::ButtonRole role)
{
    if (m_controls.parent() != layout()) {
        layout()->addWidget(&m_controls);
    }

    auto * ret = m_controls.addButton(text, role);
    rebuildLayout();
    return ret;
}

void Dialogue::addButton(QAbstractButton * button, QDialogButtonBox::ButtonRole role)
{
    assert(button);

    if (m_controls.buttons().contains(button)) {
        Util::debug << "button is already in button box ";

        if (m_controls.buttonRole(button) == role) {
            Util::debug << "and has the provided role, nothing to do\n";
            return;
        }

        Util::debug << "with a different role, removing and re-adding\n";
        m_controls.removeButton(button);
    }

    m_controls.addButton(button, role);
    rebuildLayout();
}

void Dialogue::clearButtons()
{
    m_controls.clear();
    rebuildLayout();
}

void Dialogue::removeButton(QAbstractButton * button)
{
    m_controls.removeButton(button);
    rebuildLayout();
}

void Dialogue::setIcon(const QIcon & icon)
{
    m_icon = icon;

    if (!m_icon.isNull()) {
        m_iconLabel.setPixmap(m_icon.pixmap(m_iconExtent));
    }

    rebuildLayout();
}

void Dialogue::rebuildLayout()
{
    // don't update the widget until we've re-built the layout
    WidgetUpdateSuspender suspender(*this);

    m_title.setParent(nullptr);
    m_message.setParent(nullptr);
    m_iconLabel.setParent(nullptr);
    m_controls.setParent(nullptr);

    auto * layout = qobject_cast<QGridLayout *>(this->layout());
    int row = 0;
    int col = 0;

    if (!m_icon.isNull()) {
        layout->addWidget(&m_iconLabel, row, col);
        layout->setColumnStretch(col, 0);
        ++col;
    }

    if (!title().isEmpty()) {
        layout->addWidget(&m_title, row, col);
        layout->setRowStretch(row, 0);
        ++row;
    }

    layout->addWidget(&m_message, row, col);
    layout->setRowStretch(row, 0);
    ++row;

    // one empty row for expandable space
    layout->setRowStretch(row, 10);
    ++row;

    if (!m_controls.buttons().isEmpty()) {
        layout->addWidget(&m_controls, row, 0, 1, (col + 1));
        layout->setRowStretch(row, 0);
        ++row;
    }

    layout->setColumnStretch(col, 10);
}
