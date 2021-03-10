//
// Created by darren on 10/03/2021.
//

#include <QGridLayout>

#include "keyboardmonitorwidget.h"
#include "../spectrum.h"
#include "../keyboard.h"

using namespace Spectrum::Qt;

KeyboardMonitorWidget::KeyboardMonitorWidget(Spectrum * spectrum, QWidget * parent)
:   QWidget(parent),
    m_spectrum(spectrum)
{
    m_1.setCheckable(true);
    m_1.setText(tr("1"));
    m_2.setCheckable(true);
    m_2.setText(tr("2"));
    m_3.setCheckable(true);
    m_3.setText(tr("3"));
    m_4.setCheckable(true);
    m_4.setText(tr("4"));
    m_5.setCheckable(true);
    m_5.setText(tr("5"));

    m_6.setCheckable(true);
    m_6.setText(tr("6"));
    m_7.setCheckable(true);
    m_7.setText(tr("7"));
    m_8.setCheckable(true);
    m_8.setText(tr("8"));
    m_9.setCheckable(true);
    m_9.setText(tr("9"));
    m_0.setCheckable(true);
    m_0.setText(tr("0"));

    m_q.setCheckable(true);
    m_q.setText(tr("Q"));
    m_w.setCheckable(true);
    m_w.setText(tr("W"));
    m_e.setCheckable(true);
    m_e.setText(tr("E"));
    m_r.setCheckable(true);
    m_r.setText(tr("R"));
    m_t.setCheckable(true);
    m_t.setText(tr("T"));

    m_y.setCheckable(true);
    m_y.setText(tr("Y"));
    m_u.setCheckable(true);
    m_u.setText(tr("U"));
    m_i.setCheckable(true);
    m_i.setText(tr("I"));
    m_o.setCheckable(true);
    m_o.setText(tr("O"));
    m_p.setCheckable(true);
    m_p.setText(tr("P"));

    m_a.setCheckable(true);
    m_a.setText(tr("A"));
    m_s.setCheckable(true);
    m_s.setText(tr("S"));
    m_d.setCheckable(true);
    m_d.setText(tr("D"));
    m_f.setCheckable(true);
    m_f.setText(tr("F"));
    m_g.setCheckable(true);
    m_g.setText(tr("G"));

    m_h.setCheckable(true);
    m_h.setText(tr("H"));
    m_j.setCheckable(true);
    m_j.setText(tr("J"));
    m_k.setCheckable(true);
    m_k.setText(tr("K"));
    m_l.setCheckable(true);
    m_l.setText(tr("L"));
    m_enter.setCheckable(true);
    m_enter.setText(tr("Enter"));

    m_capsShift.setCheckable(true);
    m_capsShift.setText(tr("Caps Shift"));
    m_z.setCheckable(true);
    m_z.setText(tr("Z"));
    m_x.setCheckable(true);
    m_x.setText(tr("X"));
    m_c.setCheckable(true);
    m_c.setText(tr("C"));
    m_v.setCheckable(true);
    m_v.setText(tr("V"));

    m_b.setCheckable(true);
    m_b.setText(tr("B"));
    m_n.setCheckable(true);
    m_n.setText(tr("N"));
    m_m.setCheckable(true);
    m_m.setText(tr("M"));
    m_symbolShift.setCheckable(true);
    m_symbolShift.setText(tr("Symbol Shift"));
    m_space.setCheckable(true);
    m_space.setText(tr("Space"));

    auto * mainLayout = new QGridLayout();
    auto * halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_1);
    halfRowLayout->addWidget(&m_2);
    halfRowLayout->addWidget(&m_3);
    halfRowLayout->addWidget(&m_4);
    halfRowLayout->addWidget(&m_5);
    mainLayout->addLayout(halfRowLayout, 0, 0);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_6);
    halfRowLayout->addWidget(&m_7);
    halfRowLayout->addWidget(&m_8);
    halfRowLayout->addWidget(&m_9);
    halfRowLayout->addWidget(&m_0);
    mainLayout->addLayout(halfRowLayout, 0, 1);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_q);
    halfRowLayout->addWidget(&m_w);
    halfRowLayout->addWidget(&m_e);
    halfRowLayout->addWidget(&m_r);
    halfRowLayout->addWidget(&m_t);
    mainLayout->addLayout(halfRowLayout, 1, 0);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_y);
    halfRowLayout->addWidget(&m_u);
    halfRowLayout->addWidget(&m_i);
    halfRowLayout->addWidget(&m_o);
    halfRowLayout->addWidget(&m_p);
    mainLayout->addLayout(halfRowLayout, 1, 1);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_a);
    halfRowLayout->addWidget(&m_s);
    halfRowLayout->addWidget(&m_d);
    halfRowLayout->addWidget(&m_f);
    halfRowLayout->addWidget(&m_g);
    mainLayout->addLayout(halfRowLayout, 2, 0);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_h);
    halfRowLayout->addWidget(&m_j);
    halfRowLayout->addWidget(&m_k);
    halfRowLayout->addWidget(&m_l);
    halfRowLayout->addWidget(&m_enter);
    mainLayout->addLayout(halfRowLayout, 2, 1);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_capsShift);
    halfRowLayout->addWidget(&m_z);
    halfRowLayout->addWidget(&m_x);
    halfRowLayout->addWidget(&m_c);
    halfRowLayout->addWidget(&m_v);
    mainLayout->addLayout(halfRowLayout, 3, 0);

    halfRowLayout = new QHBoxLayout();
    halfRowLayout->addWidget(&m_b);
    halfRowLayout->addWidget(&m_n);
    halfRowLayout->addWidget(&m_m);
    halfRowLayout->addWidget(&m_symbolShift);
    halfRowLayout->addWidget(&m_space);
    mainLayout->addLayout(halfRowLayout, 3, 1);

    setLayout(mainLayout);
    updateStateDisplay();

    m_updateTimer.setInterval(10);
    connect(&m_updateTimer, &QTimer::timeout, this, &KeyboardMonitorWidget::updateStateDisplay);
    m_updateTimer.start();
}

void KeyboardMonitorWidget::setSpectrum(Spectrum * spectrum)
{
    m_spectrum = spectrum;
    updateStateDisplay();
}

void KeyboardMonitorWidget::updateStateDisplay()
{
    if (!m_spectrum || !m_spectrum->keyboard()) {
        m_1.setChecked(false);
        m_2.setChecked(false);
        m_3.setChecked(false);
        m_4.setChecked(false);
        m_5.setChecked(false);

        m_6.setChecked(false);
        m_7.setChecked(false);
        m_8.setChecked(false);
        m_9.setChecked(false);
        m_0.setChecked(false);

        m_q.setChecked(false);
        m_w.setChecked(false);
        m_e.setChecked(false);
        m_r.setChecked(false);
        m_t.setChecked(false);

        m_y.setChecked(false);
        m_u.setChecked(false);
        m_i.setChecked(false);
        m_o.setChecked(false);
        m_p.setChecked(false);

        m_a.setChecked(false);
        m_s.setChecked(false);
        m_d.setChecked(false);
        m_f.setChecked(false);
        m_g.setChecked(false);

        m_h.setChecked(false);
        m_j.setChecked(false);
        m_k.setChecked(false);
        m_l.setChecked(false);
        m_enter.setChecked(false);

        m_capsShift.setChecked(false);
        m_z.setChecked(false);
        m_x.setChecked(false);
        m_c.setChecked(false);
        m_v.setChecked(false);

        m_b.setChecked(false);
        m_n.setChecked(false);
        m_m.setChecked(false);
        m_symbolShift.setChecked(false);
        m_space.setChecked(false);
    } else {
        auto * keyboard = m_spectrum->keyboard();
        m_1.setChecked(keyboard->keyState(Keyboard::Key::Num1));
        m_2.setChecked(keyboard->keyState(Keyboard::Key::Num2));
        m_3.setChecked(keyboard->keyState(Keyboard::Key::Num3));
        m_4.setChecked(keyboard->keyState(Keyboard::Key::Num4));
        m_5.setChecked(keyboard->keyState(Keyboard::Key::Num5));

        m_6.setChecked(keyboard->keyState(Keyboard::Key::Num6));
        m_7.setChecked(keyboard->keyState(Keyboard::Key::Num7));
        m_8.setChecked(keyboard->keyState(Keyboard::Key::Num8));
        m_9.setChecked(keyboard->keyState(Keyboard::Key::Num9));
        m_0.setChecked(keyboard->keyState(Keyboard::Key::Num0));

        m_q.setChecked(keyboard->keyState(Keyboard::Key::Q));
        m_w.setChecked(keyboard->keyState(Keyboard::Key::W));
        m_e.setChecked(keyboard->keyState(Keyboard::Key::E));
        m_r.setChecked(keyboard->keyState(Keyboard::Key::R));
        m_t.setChecked(keyboard->keyState(Keyboard::Key::T));

        m_y.setChecked(keyboard->keyState(Keyboard::Key::Y));
        m_u.setChecked(keyboard->keyState(Keyboard::Key::U));
        m_i.setChecked(keyboard->keyState(Keyboard::Key::I));
        m_o.setChecked(keyboard->keyState(Keyboard::Key::O));
        m_p.setChecked(keyboard->keyState(Keyboard::Key::P));

        m_a.setChecked(keyboard->keyState(Keyboard::Key::A));
        m_s.setChecked(keyboard->keyState(Keyboard::Key::S));
        m_d.setChecked(keyboard->keyState(Keyboard::Key::D));
        m_f.setChecked(keyboard->keyState(Keyboard::Key::F));
        m_g.setChecked(keyboard->keyState(Keyboard::Key::G));

        m_h.setChecked(keyboard->keyState(Keyboard::Key::H));
        m_j.setChecked(keyboard->keyState(Keyboard::Key::J));
        m_k.setChecked(keyboard->keyState(Keyboard::Key::K));
        m_l.setChecked(keyboard->keyState(Keyboard::Key::L));
        m_enter.setChecked(keyboard->keyState(Keyboard::Key::Enter));

        m_capsShift.setChecked(keyboard->keyState(Keyboard::Key::CapsShift));
        m_z.setChecked(keyboard->keyState(Keyboard::Key::Z));
        m_x.setChecked(keyboard->keyState(Keyboard::Key::X));
        m_c.setChecked(keyboard->keyState(Keyboard::Key::C));
        m_v.setChecked(keyboard->keyState(Keyboard::Key::V));

        m_b.setChecked(keyboard->keyState(Keyboard::Key::B));
        m_n.setChecked(keyboard->keyState(Keyboard::Key::N));
        m_m.setChecked(keyboard->keyState(Keyboard::Key::M));
        m_symbolShift.setChecked(keyboard->keyState(Keyboard::Key::SymbolShift));
        m_space.setChecked(keyboard->keyState(Keyboard::Key::Space));
    }
}
