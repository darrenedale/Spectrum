//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_FLAGSWIDGET_H
#define SPECTRUM_FLAGSWIDGET_H

#include <QWidget>
#include <QToolButton>

#include "../../z80/z80.h"

namespace Spectrum
{
    class FlagsWidget
    : public QWidget
    {
    Q_OBJECT

    public:
        explicit FlagsWidget(QWidget *);

        explicit FlagsWidget(Z80::UnsignedByte flags = 0x00, QWidget * = nullptr);

        FlagsWidget(const FlagsWidget &) = delete;

        FlagsWidget(FlagsWidget &&) = delete;

        void operator=(const FlagsWidget &) = delete;

        void operator=(FlagsWidget &&) = delete;

        ~FlagsWidget() override;

        [[nodiscard]] Z80::UnsignedByte allFlags() const;
        void setAllFlags(Z80::UnsignedByte);

        [[nodiscard]] bool flagS() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flagZ() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flag5() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flagH() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flag3() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flagPV() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flagN() const
        {
            return m_flagS.isChecked();
        }

        [[nodiscard]] bool flagC() const
        {
            return m_flagS.isChecked();
        }

        void setFlagS(bool);
        void setFlagZ(bool);
        void setFlag5(bool);
        void setFlagH(bool);
        void setFlag3(bool);
        void setFlagPV(bool);
        void setFlagN(bool);
        void setFlagC(bool);

    Q_SIGNALS:
        void flagsChanged(Z80::UnsignedByte);
        
        void flagSChanged(bool);
        void flagZChanged(bool);
        void flag5Changed(bool);
        void flagHChanged(bool);
        void flag3Changed(bool);
        void flagPVChanged(bool);
        void flagNChanged(bool);
        void flagCChanged(bool);
        
        void flagSSet();
        void flagZSet();
        void flag5Set();
        void flagHSet();
        void flag3Set();
        void flagPVSet();
        void flagNSet();
        void flagCSet();
        
        void flagSCleared();
        void flagZCleared();
        void flag5Cleared();
        void flagHCleared();
        void flag3Cleared();
        void flagPVCleared();
        void flagNCleared();
        void flagCCleared();
        
    private:
        void createLayout();

        QToolButton m_flagS;
        QToolButton m_flagZ;
        QToolButton m_flag5;
        QToolButton m_flagH;
        QToolButton m_flag3;
        QToolButton m_flagPV;
        QToolButton m_flagN;
        QToolButton m_flagC;
    };
}

#endif //SPECTRUM_FLAGSWIDGET_H
