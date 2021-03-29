//
// Created by darren on 23/03/2021.
//

#ifndef SPECTRUM_PROGRAMPOINTERSWIDGET_H
#define SPECTRUM_PROGRAMPOINTERSWIDGET_H

#include "hexspinbox.h"
#include "../../z80/types.h"
#include "../../z80/registers.h"

namespace Spectrum::QtUi
{
    class ProgramPointersWidget
    : public QWidget
    {
        Q_OBJECT

    public:
        explicit ProgramPointersWidget(QWidget * = nullptr);
        ProgramPointersWidget(const ProgramPointersWidget &) = delete;
        ProgramPointersWidget(ProgramPointersWidget &) = delete;
        void operator=(const ProgramPointersWidget &) = delete;
        void operator=(ProgramPointersWidget &&) = delete;
        ~ProgramPointersWidget() override;

        void setRegisters(const ::Z80::Registers &);
        void setRegister(::Z80::Register16, ::Z80::UnsignedWord);
        [[nodiscard]] ::Z80::UnsignedWord registerValue(::Z80::Register16) const;

        //
        // manage actions for PC/SP context menus
        //
        void addProgramCounterAction(QAction * action)
        {
            m_pc.addAction(action);
        }

        void addStackPointerAction(QAction * action)
        {
            m_sp.addAction(action);
        }

        void removeProgramCounterAction(QAction * action)
        {
            m_pc.removeAction(action);
        }

        void removeStackPointerAction(QAction * action)
        {
            m_sp.removeAction(action);
        }
        
        void insertProgramCounterAction(QAction * before, QAction * action)
        {
            m_pc.insertAction(before, action);
        }
        
        void insertStackPointerAction(QAction * before, QAction * action)
        {
            m_sp.insertAction(before, action);
        }

        void insertProgramCounterAction(QAction * before, QList<QAction *> actions)
        {
            m_pc.insertActions(before, std::move(actions));
        }

        void insertStackPointerActions(QAction * before, QList<QAction *> actions)
        {
            m_sp.insertActions(before, std::move(actions));
        }

        [[nodiscard]] QList<QAction *> programCounterActions() const
        {
            return m_pc.actions();
        }

        [[nodiscard]] QList<QAction *> stackPointerActions() const
        {
            return m_sp.actions();
        }

    Q_SIGNALS:
        void registerChanged(::Z80::Register16 reg, ::Z80::UnsignedWord value);

    private:
        HexSpinBox m_sp;
        HexSpinBox m_pc;

    };
}

#endif //SPECTRUM_PROGRAMPOINTERSWIDGET_H
