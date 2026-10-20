//
// Created by darren on 11/10/2026.
//

#ifndef SPECTRUM_QTUI_POKEDIALOGUEWIDGET_H
#define SPECTRUM_QTUI_POKEDIALOGUEWIDGET_H

#include <QDialog>

#include "../../../z80/types.h"
#include "../hexspinbox.h"

class QLabel;
class QLineEdit;
class QString;
class QWidget;

namespace Spectrum::QtUi
{
    class FilenameWidget;
    class HexSpinBox;

    /** Capture the details of a poke prior to saving it to a .pok file. */
    class PokeDetailsDialogue
    : public QDialog
    {
        Q_OBJECT

        public:
            enum class Mode: std::uint8_t
            {
                Create,
                Add,
            };

            /** Default construct a poke details dialogue with empty filename and name, 0x0000 address and 0x00 value. */
            explicit PokeDetailsDialogue(QWidget * = nullptr) noexcept;

            /** Construct a poke details dialogue with name, address, value and mode. */
            PokeDetailsDialogue(const QString &, ::Z80::UnsignedWord, ::Z80::UnsignedByte, Mode = Mode::Create, QWidget * = nullptr) noexcept;

            /** Poke details dialogues can't be copied. */
            PokeDetailsDialogue(const PokeDetailsDialogue &) = delete;

            /** Poke details dialogues can't be moved. */
            PokeDetailsDialogue(PokeDetailsDialogue &&) = delete;

            /** Poke details dialogues can't be copy assigned. */
            PokeDetailsDialogue operator=(const PokeDetailsDialogue &) = delete;

            /** Poke details dialogues can't be move assigned. */
            PokeDetailsDialogue operator=(PokeDetailsDialogue &&) = delete;

            /** The destructor is defaulted in the implementation file. */
            ~PokeDetailsDialogue() noexcept override;

            /** Set the filename in the dialogue. */
            void setFilename(const QString & name) noexcept;

            /** Fetch the filename from the dialogue. */
            [[nodiscard]]
            QString filename() const noexcept;

            /** Set the poke name in the dialogue. */
            void setName(const QString & name) noexcept;

            /** Fetch the poke name from the dialogue. */
            [[nodiscard]]
            QString name() const noexcept;

            /** Fetch the poke address from the dialogue. */
            [[nodiscard]]
            ::Z80::UnsignedWord address() const noexcept;

            /** Set the poke address in the dialogue. */
            void setAddress(::Z80::UnsignedWord) noexcept;

            /** Fetch the poke value from the dialogue. */
            [[nodiscard]]
            ::Z80::UnsignedByte value() const noexcept;

            /** Set the poke value in the dialogue. */
            void setValue(::Z80::UnsignedByte) noexcept;

            /** Fetch the storage mode from the dialogue. */
            [[nodiscard]]
            Mode mode() const noexcept
            {
                return m_mode;
            }

            /** Set the storage mode in the dialogue. */
            void setMode(Mode) noexcept;

        private:
            void layoutWidget() noexcept;

            Mode m_mode;
            ::Z80::UnsignedWord m_address;
            std::unique_ptr<FilenameWidget> m_filenameWidget;
            std::unique_ptr<QLineEdit>(m_nameWidget);
            std::unique_ptr<QLabel>(m_addressDisplay);
            std::unique_ptr<QSpinBox>(m_valueWidget);
    };
} // Spectrum::QtUi

#endif //SPECTRUM_QTUI_POKEDIALOGUEWIDGET_H
