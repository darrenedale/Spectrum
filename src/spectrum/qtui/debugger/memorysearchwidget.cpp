//
// Created by darren on 05/05/2021.
//

#include <cstdlib>
#include <format>
#include <iomanip>
#include <optional>
#include <print>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRegularExpression>
#include <QString>

#include "memorysearchwidget.h"
#include "../application.h"
#include "../widgetupdatesuspender.h"
#include "../../../util/assert.h"
#include "../../../util/debug.h"

using namespace std::string_literals;
using namespace Spectrum::QtUi::Debugger;

namespace
{
    /** Regular expression to split a string of multiple hex values. */
    const auto ValueSeparatorRegularExpression = QRegularExpression(R"((?:\s*(?:[,.:;]|\s)\s*))");

    /** Regular expression to match a hex value. */
    const auto ValueRegularExpression = QRegularExpression("^(?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5]|0x[0-9a-fA-F]?[0-9a-fA-F])$");

    std::optional<QByteArray> parseByteArray(const QString & text)
    {
        auto bytes = text.split(ValueSeparatorRegularExpression);

        if (!std::ranges::all_of(std::as_const(bytes), [](const QString & value) -> bool {
            const auto match = ValueRegularExpression.match(value);
            return match.hasMatch();
        })) {
            return {};
        }

        QByteArray ret;
        ret.reserve(bytes.size());

        std::ranges::transform(std::as_const(bytes), std::back_inserter(ret), [](const QString & value) {
            // NOTE base = 0 means it will interpret the 0x prefix as a hex number
            return static_cast<char>(value.toInt(nullptr, 0));
        });

        return ret;
    }

    std::string to_string(const MemorySearchWidget::SearchType & searchType)
    {
        switch (searchType)
        {
            case MemorySearchWidget::SearchType::UnsignedByte:
                return "UnsignedByte"s;

            case MemorySearchWidget::SearchType::UnsignedWord:
                return "UnsignedWord"s;

            case MemorySearchWidget::SearchType::SignedByte:
                return "SignedByte"s;

            case MemorySearchWidget::SearchType::SignedWord:
                return "SignedWord"s;

            case MemorySearchWidget::SearchType::String:
                return "String"s;

            case MemorySearchWidget::SearchType::ByteArray:
                return "ByteArray"s;

            default:
                sp_assert(false, "detected addition of SearchType enumerator with value {} not handled in to_string", static_cast<std::uint16_t>(searchType));
                std::abort();
        }
    }
};


/**
 * Formatter for all the enums, based on the to_string() implementation for each.
 * @tparam T must be one of the above enum types.
 */
template<>
struct std::formatter<MemorySearchWidget::SearchType>
{
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& context)
    {
        auto it = context.begin();

        if (it != context.end() && *it != '}')
        {
            throw std::format_error("invalid format args");
        }

        return it;
    }

    template<class FormatContext>
    FormatContext::iterator format(const MemorySearchWidget::SearchType & type, FormatContext & context) const
    {
        return std::ranges::copy(::to_string(type), context.out()).out;
    }
};

MemorySearchWidget::MemorySearchWidget(QWidget * parent)
: m_searchType(),
  m_stringValue(),
  m_byteValue(),
  m_wordValue()
{
    m_searchType.addItem(tr("Unsigned byte"), static_cast<int>(SearchType::UnsignedByte));
    m_searchType.addItem(tr("Signed byte"), static_cast<int>(SearchType::SignedByte));
    m_searchType.addItem(tr("Unsigned word"), static_cast<int>(SearchType::UnsignedWord));
    m_searchType.addItem(tr("Signed word"), static_cast<int>(SearchType::SignedWord));
    m_searchType.addItem(tr("String"), static_cast<int>(SearchType::String));
    m_searchType.addItem(tr("Byte array"), static_cast<int>(SearchType::ByteArray));

    m_byteValue.setRange(0, 255);
    m_wordValue.setRange(0, 65535);
    m_wordValue.setVisible(false);
    m_stringValue.setVisible(false);

    m_byteValue.installEventFilter(this);
    m_wordValue.installEventFilter(this);
    m_stringValue.installEventFilter(this);

    connect(&m_searchType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MemorySearchWidget::onSearchTypeChanged);

    auto * layout = new QHBoxLayout();
    auto * label = new QLabel();
    label->setPixmap(Application::icon(QStringLiteral("edit-find"), QStringLiteral("search")).pixmap(32));//static_cast<int>(font().pixelSize() * 1.5)));
    layout->addWidget(label, 0);
    layout->addWidget(&m_searchType, 0);
    layout->addWidget(&m_byteValue, 0);
    layout->addWidget(&m_wordValue, 0);
    layout->addWidget(&m_stringValue, 0);

    setLayout(layout);
}

MemorySearchWidget::~MemorySearchWidget() = default;

MemorySearchWidget::SearchType MemorySearchWidget::searchType() const
{
    bool ok;
    auto type = m_searchType.currentData().toInt(&ok);
    sp_assert(ok, "invalid search type ({}) {} found in MemorySearchWidget - can't convert to int", m_searchType.currentData().typeName(), m_searchType.currentData().toString().toStdString());
    return static_cast<SearchType>(type);
}

void MemorySearchWidget::setSearchType(MemorySearchWidget::SearchType type)
{
    const auto idx = m_searchType.findData(static_cast<int>(type));
    sp_assert(-1 != idx, "search type {} not found in MemorySearchWidget", type);
    m_searchType.setCurrentIndex(idx);
}

QByteArray MemorySearchWidget::stringValue() const
{
    auto value = m_stringValue.text();
    QByteArray ret;
    ret.reserve(value.size());

    for (const auto & ch : value) {
        auto unicode = ch.unicode();

        if (0x00a3 == unicode) {
            // £ U+00a3 Pound Sign
            ret += 96;
        } else if (QChar(0x00a9) == ch) {
            // © U+00a9 Copyright Sign
            ret += 127;
        } else if (32 <= unicode && 127 > unicode) {
            // ASCII and Spectrum charset equivalence
            ret += static_cast<QByteArray::value_type>(unicode & 0x00ff);
        } else {
            Util::debugln("unicode codepoint U+{:04x} cannot be represented in the Spectrum charset", static_cast<int>(unicode));
            ret += '\0';
        }
    }

    return ret;
}

void MemorySearchWidget::setStringValue(const QByteArray & value)
{
    m_stringValue.setText(value);
}

void MemorySearchWidget::keyPressEvent(QKeyEvent * ev)
{
    if (Qt::Key::Key_F3 == ev->key()) {
        emitSearchRequest();
        ev->accept();
        return;
    }

    QWidget::keyPressEvent(ev);
}

bool MemorySearchWidget::eventFilter(QObject * subject, QEvent * ev)
{
    if (ev->type() == QEvent::KeyPress) {
        if (
            const auto * keyEvent = reinterpret_cast<QKeyEvent *>(ev);
            Qt::Key::Key_Enter == keyEvent->key() || Qt::Key::Key_Return == keyEvent->key()
        ) {
            emitSearchRequest();
        }
    }

    return false;
}

void MemorySearchWidget::onSearchTypeChanged()
{
    switch (searchType()) {
        case SearchType::UnsignedByte:
            m_byteValue.setRange(0, 255);
            m_byteValue.setVisible(true);
            m_wordValue.setVisible(false);
            m_stringValue.setVisible(false);
            break;

        case SearchType::SignedByte:
            m_byteValue.setRange(-128, 127);
            m_byteValue.setVisible(true);
            m_wordValue.setVisible(false);
            m_stringValue.setVisible(false);
            break;

        case SearchType::UnsignedWord:
            m_wordValue.setRange(0, 65535);
            m_byteValue.setVisible(false);
            m_wordValue.setVisible(true);
            m_stringValue.setVisible(false);
            break;

        case SearchType::SignedWord:
            m_wordValue.setRange(-32768, 32767);
            m_byteValue.setVisible(false);
            m_wordValue.setVisible(true);
            m_stringValue.setVisible(false);
            break;

        case SearchType::String:
            m_byteValue.setVisible(false);
            m_wordValue.setVisible(false);
            m_stringValue.setVisible(true);
            m_stringValue.setToolTip(tr("Enter a string of valid Spectrum characters."));
            m_stringValue.setPlaceholderText(tr("Spectrum string..."));
            m_stringValue.setText({});
            break;

        case SearchType::ByteArray:
            m_byteValue.setVisible(false);
            m_wordValue.setVisible(false);
            m_stringValue.setVisible(true);
            m_stringValue.setToolTip(tr("Enter a list of byte values.<br>Separate individual bytes with either whitespace or punctuation (,.;:). You can use hexadecimal byte values using the <em>0x</em> prefix."));
            m_stringValue.setPlaceholderText(tr("Byte values..."));
            m_stringValue.setText({});
            break;

        default:
            sp_assert(false, "found invalid search type {} in MemorySearchWidget", searchType());
            std::abort();
    }
}

void MemorySearchWidget::emitSearchRequest()
{
    switch (searchType()) {
        case SearchType::UnsignedByte:
            Q_EMIT unsignedByteSearchRequested(unsignedByteValue());
            break;

        case SearchType::SignedByte:
            Q_EMIT signedByteSearchRequested(signedByteValue());
            break;

        case SearchType::UnsignedWord:
            Q_EMIT unsignedWordSearchRequested(signedWordValue());
            break;

        case SearchType::SignedWord:
            Q_EMIT signedWordSearchRequested(signedWordValue());
            break;

        case SearchType::String:
            Q_EMIT stringSearchRequested(stringValue());
            break;

        case SearchType::ByteArray: {
            if (const auto byteArray = parseByteArray(m_stringValue.text()); !byteArray) {
                Application::showNotification(tr("The array of bytes contains an invalid value."));
            } else {
                Q_EMIT stringSearchRequested(*byteArray);
            }
            break;
        }
    }
}
