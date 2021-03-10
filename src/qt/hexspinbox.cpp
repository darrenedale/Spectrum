//
// Created by darren on 04/03/2021.
//

#include <cassert>

#include <QStringBuilder>
#include <QRegularExpression>
#include <QHelpEvent>
#include <QToolTip>

#include "hexspinbox.h"

HexSpinBox::HexSpinBox(QWidget * parent)
: HexSpinBox(4, QLatin1Char('0'), parent)
{}

HexSpinBox::HexSpinBox(int digits, QWidget * parent)
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
    assert(0 < digits);
    setPrefix(QStringLiteral("0x"));
    setMinimum(0);
    setMouseTracking(true);
}

HexSpinBox::~HexSpinBox() = default;

void HexSpinBox::setDigits(int digits)
{
    assert(0 < digits);

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

QString HexSpinBox::textFromValue(int value) const
{
    return QStringLiteral("%1").arg(value, digits(), 16, fillChar());
}

int HexSpinBox::valueFromText(const QString & text) const
{
    const QRegularExpression matcher(QStringLiteral("^\\s*") % QRegularExpression::escape(prefix()) % "\\s*([a-fA-F0-9]+)\\s*$");
    auto match = matcher.match(text);

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

QValidator::State HexSpinBox::validate(QString & text, int & pos) const
{
    return QValidator::Acceptable;
}

