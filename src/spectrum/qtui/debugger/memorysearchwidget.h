//
// Created by darren on 05/05/2021.
//

#ifndef SPECTRUM_QTUI_DEBUGGER_MEMORYSEARCHWIDGET_H
#define SPECTRUM_QTUI_DEBUGGER_MEMORYSEARCHWIDGET_H

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include "../../../z80/types.h"

namespace Spectrum::QtUi::Debugger
{
    /**
     * Widget to enable the user to enter a search term for a memory view.
     */
    class MemorySearchWidget
    : public QWidget
    {
        using UnsignedWord = ::Z80::UnsignedWord;
        using UnsignedByte = ::Z80::UnsignedByte;
        using SignedWord = ::Z80::SignedWord;
        using SignedByte = ::Z80::SignedByte;

    Q_OBJECT

    public:
        /**
         * Enumeration of the types of search available.
         */
        enum class SearchType
        {
            UnsignedByte = 0,       // search for a single unsigned byte value
            UnsignedWord,           // search for a 16-bit unsigned word value
            SignedByte,             // search for a single signed byte value
            SignedWord,             // search for a 16-bit signed word value
            String,                 // search for a string of bytes entered as text
            ByteArray,              // search for a string of bytes entered as an array of values
        };

        /**
         * Initialise a new search widget.
         *
         * @param parent The parent of the search widget.
         */
        explicit MemorySearchWidget(QWidget * parent = nullptr);
        MemorySearchWidget(MemorySearchWidget &&) = delete;
        MemorySearchWidget(const MemorySearchWidget &) = delete;
        void operator=(MemorySearchWidget &&) = delete;
        void operator=(const MemorySearchWidget &) = delete;

        /**
         * Destructor.
         */
        ~MemorySearchWidget() override;

        /**
         * Fetch the type of search currently selected.
         *
         * @return The search type.
         */
        [[nodiscard]] SearchType searchType() const;

        /**
         * Set the current search type.
         *
         * @param type The search type.
         */
        void setSearchType(SearchType type);

        /**
         * Fetch the unsigned word the user has entered as a search term.
         *
         * Calling this when the search widget is not in Unsigned Word mode produces undefined results.
         *
         * @return
         */
        [[nodiscard]] UnsignedWord unsignedWordValue() const
        {
            return static_cast<UnsignedWord>(m_wordValue.value());
        }

        /**
         * Set the the unsigned word search term.
         *
         * Calling this when the search widget is not in Unsigned Word mode produces undefined results.
         *
         * @return
         */
        void setUnsignedWordValue(UnsignedWord value)
        {
            m_wordValue.setValue(value);
        }

        /**
         * Fetch the signed word the user has entered as a search term.
         *
         * Calling this when the search widget is not in Signed Word mode produces undefined results.
         *
         * @return
         */
        [[nodiscard]] SignedWord signedWordValue() const
        {
            return static_cast<SignedWord>(m_wordValue.value());
        }

        /**
         * Set the the signed word search term.
         *
         * Calling this when the search widget is not in Signed Word mode produces undefined results.
         *
         * @return
         */
        void setSignedWordValue(SignedWord value)
        {
            m_wordValue.setValue(value);
        }

        /**
         * Fetch the unsigned byte the user has entered as a search term.
         *
         * Calling this when the search widget is not in Unsigned Byte mode produces undefined results.
         *
         * @return
         */
        [[nodiscard]] UnsignedByte unsignedByteValue() const
        {
            return static_cast<UnsignedByte>(m_byteValue.value());
        }

        /**
         * Set the the unsigned byte search term.
         *
         * Calling this when the search widget is not in Unsigned Byte mode produces undefined results.
         *
         * @return
         */
        void setUnsignedByteValue(UnsignedByte value)
        {
            m_byteValue.setValue(value);
        }

        /**
         * Fetch the signed byte the user has entered as a search term.
         *
         * Calling this when the search widget is not in Signed Byte mode produces undefined results.
         *
         * @return
         */
        [[nodiscard]] SignedByte signedByteValue() const
        {
            return static_cast<SignedByte>(m_byteValue.value());
        }

        /**
         * Set the the signed byte search term.
         *
         * Calling this when the search widget is not in Signed Byte mode produces undefined results.
         *
         * @return
         */
        void setSignedByteValue(SignedByte value)
        {
            m_byteValue.setValue(value);
        }

        /**
         * The current search string.
         *
         * The string is provided as an array of bytes to search for in the memory. The user's input will be converted to bytes in the Spectrum's 8-bit
         * character set.
         *
         * @return The string of bytes.
         */
        [[nodiscard]] QByteArray stringValue() const;

        /**
         * Set the search string.
         *
         * @param value The string.
         */
        void setStringValue(const QByteArray & value);

    Q_SIGNALS:
        /**
         * Emitted when the user has requested a string search.
         *
         * @param query The bytes of the user's search string in the Spectrum character set.
         */
        void stringSearchRequested(const QByteArray & query);

        /**
         * Emitted when the user has requested an unsigned word search.
         *
         * @param query The sought value.
         */
        void unsignedWordSearchRequested(::Z80::UnsignedWord query);

        /**
         * Emitted when the user has requested an signed word search.
         *
         * @param query The sought value.
         */
        void signedWordSearchRequested(::Z80::SignedWord query);

        /**
         * Emitted when the user has requested an unsigned byte search.
         *
         * @param query The sought value.
         */
        void unsignedByteSearchRequested(::Z80::UnsignedByte query);

        /**
         * Emitted when the user has requested an signed byte search.
         *
         * @param query The sought value.
         */
        void signedByteSearchRequested(::Z80::SignedByte query);

    protected:
        /**
         * Handler for key presses on the widget.
         *
         * Currently F3 triggers a search with the current term.
         *
         * @param ev The keyboard event.
         */
        void keyPressEvent(QKeyEvent * ev) override;

        /**
         * Listens for return/enter key presses on value entry widget and triggers signals.
         *
         * @param subject
         * @param ev
         * @return
         */
        bool eventFilter(QObject * subject, QEvent * ev) override;

        /**
         * Internal handler for when the user changes the search type.
         */
        void onSearchTypeChanged();

        /**
         * Helper to emit the appropriate search request signal for the current search type and value.
         */
        void emitSearchRequest();

    private:
        /**
         * The search type widget.
         */
        QComboBox m_searchType;

        /**
         * The widget to capture the user's string search query.
         */
        QLineEdit m_stringValue;

        /**
         * The widget to capture the user's string 8-bit search value.
         */
        QSpinBox m_byteValue;

        /**
         * The widget to capture the user's 16-bit search value.
         */
        QSpinBox m_wordValue;
    };
}

#endif //SPECTRUM_QTUI_DEBUGGER_MEMORYSEARCHWIDGET_H
