//
// Created by darren on 05/05/2021.
//

#include <QHBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QRegularExpression>
#include "../application.h"
#include "../widgetupdatesuspender.h"
#include "memorysearchwidget.h"

using namespace Spectrum::QtUi::Debugger;

namespace
{
    std::optional<QByteArray> parseByteArray(const QString & text)
    {
        static const auto rxSeparator = QRegularExpression(R"((?:\s*(?:[,.:;]|\s)\s*))");
        auto bytes = text.split(rxSeparator);

        if (!std::all_of(bytes.cbegin(), bytes.cend(), [](const QString & value) -> bool {
            static const auto rxValue = QRegularExpression("^(?:[01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5]|0x[0-9a-fA-F]?[0-9a-fA-F])$");
            auto match = rxValue.match(value);
            return match.hasMatch();
        })) {
            return {};
        }

        QByteArray ret;
        ret.reserve(bytes.size());

        std::transform(bytes.cbegin(), bytes.cend(), std::back_inserter(ret), [](const QString & value) {
            // NOTE base = 0 means it will interpret the 0x prefix as a hex number
            return static_cast<char>(value.toInt(nullptr, 0));
        });

        return ret;
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
    assert(ok);
    return static_cast<SearchType>(type);
}

void MemorySearchWidget::setSearchType(MemorySearchWidget::SearchType type)
{
    auto idx = m_searchType.findData(static_cast<int>(type));
    assert(-1 != idx);
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
        } else if (0x00a9 == ch) {
            // © U+00a9 Copyright Sign
            ret += 127;
        } else if (32 <= unicode && 127 > unicode) {
            // ASCII and Spectrum charset equivalence
            ret += static_cast<QByteArray::value_type>(unicode & 0x00ff);
        } else {
            Util::debug << "unicode codepoint U+" << std::hex << std::setfill('0') << std::setw(4) << unicode << " cannot be represented in the Spectrum charset\n";
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
        auto * keyEvent = reinterpret_cast<QKeyEvent *>(ev);

        if (Qt::Key::Key_Enter == keyEvent->key() || Qt::Key::Key_Return == keyEvent->key()) {
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
            Util::debug << "found invalid search type in search type combo\n";
            assert(false);
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
            auto byteArray = parseByteArray(m_stringValue.text());

            if (!byteArray) {
                Application::showNotification(tr("The array of bytes contains an invalid value."));
            } else {
                Q_EMIT stringSearchRequested(*byteArray);
            }
            break;
        }
    }
}
