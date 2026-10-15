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

    class PokeDetailsDialogue
    : public QDialog
    {
        Q_OBJECT

        public:
            explicit PokeDetailsDialogue(QWidget * parent = nullptr) noexcept;

            PokeDetailsDialogue(const QString &, ::Z80::UnsignedWord, ::Z80::UnsignedByte, QWidget * parent = nullptr) noexcept;
            PokeDetailsDialogue(const PokeDetailsDialogue &) = delete;
            PokeDetailsDialogue(PokeDetailsDialogue &&) = delete;
            PokeDetailsDialogue operator=(const PokeDetailsDialogue &) = delete;
            PokeDetailsDialogue operator=(PokeDetailsDialogue &&) = delete;
            ~PokeDetailsDialogue() noexcept override;

            void setFilename(const QString & name) noexcept;

            [[nodiscard]]
            QString filename() const noexcept;

            void setName(const QString & name) noexcept;

            [[nodiscard]]
            QString name() const noexcept;

            [[nodiscard]]
            ::Z80::UnsignedWord address() const noexcept;

            void setAddress(::Z80::UnsignedWord address) noexcept;

            [[nodiscard]]
            ::Z80::UnsignedByte value() const noexcept;

            void setValue(::Z80::UnsignedByte value) noexcept;

        private:
            void layoutWidget() noexcept;

            ::Z80::UnsignedWord m_address;
            std::unique_ptr<FilenameWidget> m_filenameWidget;
            std::unique_ptr<QLineEdit>(m_nameWidget);
            std::unique_ptr<QLabel>(m_addressDisplay);
            std::unique_ptr<HexSpinBox>(m_valueWidget);
    };
} // Spectrum::QtUi

#endif //SPECTRUM_QTUI_POKEDIALOGUEWIDGET_H
