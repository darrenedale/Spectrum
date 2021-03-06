//
// Created by darren on 04/03/2021.
//

#ifndef SPECTRUM_HEXSPINBOX_H
#define SPECTRUM_HEXSPINBOX_H

#include <QSpinBox>

namespace Spectrum
{
    class HexSpinBox
    : public QSpinBox
    {
        Q_OBJECT

    public:
        explicit HexSpinBox(QWidget * parent = nullptr);
        explicit HexSpinBox(int digits, QWidget * parent = nullptr);
        explicit HexSpinBox(int digits, const QChar & fillChar, QWidget * parent = nullptr);
        explicit HexSpinBox(const QChar & fillChar, QWidget * parent = nullptr);
        HexSpinBox(const HexSpinBox &) = delete;
        HexSpinBox(HexSpinBox &&) = delete;
        void operator=(const HexSpinBox &) = delete;
        void operator=(HexSpinBox &&) = delete;
        ~HexSpinBox() override;

        [[nodiscard]] int digits() const
        {
            return m_digits;
        }

        [[nodiscard]] const QChar & fillChar() const
        {
            return m_fill;
        }

        void setDigits(int);
        void setFillChar(const QChar &);

    protected:
        bool event(QEvent *) override;

        [[nodiscard]] QString textFromValue(int value) const override;
        [[nodiscard]] int valueFromText(const QString & text) const override;
        [[nodiscard]] QValidator::State validate(QString & text, int & pos) const override;

    private:
        QChar m_fill;
        int m_digits;
    };
}

#endif //SPECTRUM_HEXSPINBOX_H
