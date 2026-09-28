//
// Created by darren on 04/03/2021.
//

#include <print>

#include <QStringBuilder>
#include <QRegularExpression>
#include <QHelpEvent>
#include <QToolTip>

#include "hexspinbox.h"
#include "../../util/assert.h"

using namespace Spectrum;

namespace
{
    /** Regular expression to match a hex value in a string. */
    const auto HexValueRegularExpression = QRegularExpression(QStringLiteral("^\\s*0x\\s*([a-fA-F0-9]+)\\s*$"));
}

HexSpinBox::HexSpinBox(QWidget * parent)
: HexSpinBox(4, QLatin1Char('0'), parent)
{}

HexSpinBox::HexSpinBox(const int digits, QWidget * parent)
: HexSpinBox(digits, QLatin1Char('0'), parent)
{}

HexSpinBox::HexSpinBox(const QChar & fillChar, QWidget * parent)
: HexSpinBox(4, fillChar, parent)
{}

HexSpinBox::HexSpinBox(int digits, const QChar & fillChar, QWidget * parent)
: QSpinBox(parent),
  m_digits(digits),
  m_fill(fillChar)
{
    sp_assert(0 < digits, "digits < 1 provided to Spectrum::HexSpinBox::HexSpinBox");
    setPrefix(QStringLiteral("0x"));
    setMinimum(0);

    int max = 0;

    while (digits--) {
        max = (max << 4) | 0xf;
    }

    setMaximum(max);
    setMouseTracking(true);
}

HexSpinBox::~HexSpinBox() = default;

void HexSpinBox::setDigits(const int digits)
{
    sp_assert(0 < digits, "digits < 1 provided to Spectrum::HexSpinBox::setDigits");

    if (digits == m_digits) {
        return;
    }

    m_digits = digits;
    update();
}

bool HexSpinBox::event(QEvent * event)
{
    if (QEvent::Type::ToolTip == event->type()) {
        const auto & value = this->value();
        QToolTip::showText(
                dynamic_cast<QHelpEvent *>(event)->globalPos(),
                tr("<p>Hex: <strong>%1</strong></p><p>Binary: %2</p><p>Octal: %3</p><p>Decimal: %4</p>")
                    .arg(value, digits(), 16, fillChar())
                    .arg(value, digits() * 4, 2, fillChar())
                    .arg(value, 0, 8, fillChar())
                    .arg(value));
        return true;
    }

    return QSpinBox::event(event);
}

QString HexSpinBox::textFromValue(const int value) const
{
    return QStringLiteral("%1").arg(value, digits(), 16, fillChar());
}

int HexSpinBox::valueFromText(const QString & text) const
{
    const auto match = HexValueRegularExpression.match(text);

    if (!match.hasMatch()) {
        return 0;
    }

    return match.captured(1).toInt(nullptr, 16);
}

void HexSpinBox::setFillChar(const QChar & ch)
{
    if (m_fill == ch) {
        return;
    }

    m_fill = ch;
    update();
}

QValidator::State HexSpinBox::validate(QString &, int &) const
{
    return QValidator::Acceptable;
}
