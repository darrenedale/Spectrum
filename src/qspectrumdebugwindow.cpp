#include "qspectrumdebugwindow.h"

#include "spectrum.h"
#include "z80.h"
#include <QSpinBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

using namespace Spectrum;

QSpectrumDebugWindow::QSpectrumDebugWindow( QWidget * parent )
:	QSpectrumDebugWindow(nullptr, parent)
{
}


QSpectrumDebugWindow::QSpectrumDebugWindow( Spectrum * spectrum, QWidget * parent )
:	QWidget(parent),
	m_spectrum(spectrum),
	m_af(nullptr),
	m_bc(nullptr),
	m_de(nullptr),
	m_hl(nullptr),
	m_ix(nullptr),
	m_iy(nullptr),
	m_afshadow(nullptr),
	m_bcshadow(nullptr),
	m_deshadow(nullptr),
	m_hlshadow(nullptr),
	m_pc(nullptr),
	m_sp(nullptr),
	m_a(nullptr),
	m_b(nullptr),
	m_c(nullptr),
	m_d(nullptr),
	m_e(nullptr),
	m_h(nullptr),
	m_l(nullptr),
	m_ashadow(nullptr),
	m_bshadow(nullptr),
	m_cshadow(nullptr),
	m_dshadow(nullptr),
	m_eshadow(nullptr),
	m_hshadow(nullptr),
	m_lshadow(nullptr),
	m_ixh(nullptr),
	m_ixl(nullptr),
	m_iyh(nullptr),
	m_iyl(nullptr),
	m_instruction(nullptr) {
	createWidgets();
	connectWidgets();
}

QSpinBox * QSpectrumDebugWindow::createRegisterSpinBox(int bits) const
{
	auto * ret = new QSpinBox();
	ret->setMinimum(0);
	ret->setMaximum((1 << bits) - 1);
	connect(ret, qOverload<int>(&QSpinBox::valueChanged), this, &QSpectrumDebugWindow::setRegister);
	return ret;
}

void QSpectrumDebugWindow::createWidgets()
{
	m_af = createRegisterSpinBox(16);
	m_bc = createRegisterSpinBox(16);
	m_de = createRegisterSpinBox(16);
	m_hl = createRegisterSpinBox(16);
	m_ix = createRegisterSpinBox(16);
	m_iy = createRegisterSpinBox(16);
	m_afshadow = createRegisterSpinBox(16);
	m_bcshadow = createRegisterSpinBox(16);
	m_deshadow = createRegisterSpinBox(16);
	m_hlshadow = createRegisterSpinBox(16);
	m_pc = createRegisterSpinBox(16);
	m_sp = createRegisterSpinBox(16);
	m_a = createRegisterSpinBox(8);
	m_b = createRegisterSpinBox(8);
	m_c = createRegisterSpinBox(8);
	m_d = createRegisterSpinBox(8);
	m_e = createRegisterSpinBox(8);
	m_h = createRegisterSpinBox(8);
	m_l = createRegisterSpinBox(8);
	m_ashadow = createRegisterSpinBox(8);
	m_bshadow = createRegisterSpinBox(8);
	m_cshadow = createRegisterSpinBox(8);
	m_dshadow = createRegisterSpinBox(8);
	m_eshadow = createRegisterSpinBox(8);
	m_hshadow = createRegisterSpinBox(8);
	m_lshadow = createRegisterSpinBox(8);
	m_ixh = createRegisterSpinBox(8);
	m_ixl = createRegisterSpinBox(8);
	m_iyh = createRegisterSpinBox(8);
	m_iyl = createRegisterSpinBox(8);
	m_instruction = new QLineEdit();

	/*
	 * layout
	 */
	auto * plainRegisters = new QHBoxLayout();
	auto * shadowRegisters = new QHBoxLayout();
	QLabel * myLabel;

	/* normal registers */
	myLabel = new QLabel("A");
	myLabel->setBuddy(m_a);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_a);

	myLabel = new QLabel("B");
	myLabel->setBuddy(m_b);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_b);

	myLabel = new QLabel("C");
	myLabel->setBuddy(m_c);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_c);

	myLabel = new QLabel("D");
	myLabel->setBuddy(m_d);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_d);

	myLabel = new QLabel("E");
	myLabel->setBuddy(m_e);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_e);

	myLabel = new QLabel("H");
	myLabel->setBuddy(m_h);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_h);

	myLabel = new QLabel("L");
	myLabel->setBuddy(m_l);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_l);

	myLabel = new QLabel("AF");
	myLabel->setBuddy(m_af);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_af);

	myLabel = new QLabel("BC");
	myLabel->setBuddy(m_bc);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_bc);

	myLabel = new QLabel("DE");
	myLabel->setBuddy(m_de);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_de);

	myLabel = new QLabel("IX");
	myLabel->setBuddy(m_ix);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_ix);

	myLabel = new QLabel("IY");
	myLabel->setBuddy(m_iy);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_iy);

	myLabel = new QLabel("HL");
	myLabel->setBuddy(m_hl);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_hl);

	myLabel = new QLabel("SP");
	myLabel->setBuddy(m_sp);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_sp);

	myLabel = new QLabel("PC");
	myLabel->setBuddy(m_pc);
	plainRegisters->addWidget(myLabel);
	plainRegisters->addWidget(m_pc);

	/* shadow registers */
	myLabel = new QLabel("A'");
	myLabel->setBuddy(m_ashadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_ashadow);

	myLabel = new QLabel("B'");
	myLabel->setBuddy(m_bshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_bshadow);

	myLabel = new QLabel("C'");
	myLabel->setBuddy(m_cshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_cshadow);

	myLabel = new QLabel("D'");
	myLabel->setBuddy(m_dshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_dshadow);

	myLabel = new QLabel("E'");
	myLabel->setBuddy(m_eshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_eshadow);

	myLabel = new QLabel("H'");
	myLabel->setBuddy(m_hshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_hshadow);

	myLabel = new QLabel("L'");
	myLabel->setBuddy(m_lshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_lshadow);

	myLabel = new QLabel("AF'");
	myLabel->setBuddy(m_afshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_afshadow);

	myLabel = new QLabel("BC'");
	myLabel->setBuddy(m_bcshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_bcshadow);

	myLabel = new QLabel("DE'");
	myLabel->setBuddy(m_deshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_deshadow);

	myLabel = new QLabel("IX");
	myLabel->setBuddy(m_ix);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_ix);

	myLabel = new QLabel("IY");
	myLabel->setBuddy(m_iy);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_iy);

	myLabel = new QLabel("HL'");
	myLabel->setBuddy(m_hlshadow);
	shadowRegisters->addWidget(myLabel);
	shadowRegisters->addWidget(m_hlshadow);

	auto * myLayout = new QVBoxLayout();
	myLayout->addLayout(plainRegisters);
	myLayout->addLayout(shadowRegisters);
	myLayout->addWidget(m_instruction);

	setLayout(myLayout);
}


void QSpectrumDebugWindow::connectWidgets()
{
}


void QSpectrumDebugWindow::setRegister(int value)
{
	if (!m_spectrum) {
	    return;
	}

	auto * cpu = m_spectrum->z80();

	if(!cpu) {
        return;
    }

    if(sender() == m_af) cpu->setAf(value);
    else if(sender() == m_bc) cpu->setBc(value);
    else if(sender() == m_de) cpu->setDe(value);
    else if(sender() == m_hl) cpu->setHl(value);
    else if(sender() == m_ix) cpu->setIx(value);
    else if(sender() == m_iy) cpu->setIy(value);
    else if(sender() == m_afshadow) cpu->setAfShadow(value);
    else if(sender() == m_bcshadow) cpu->setBcShadow(value);
    else if(sender() == m_deshadow) cpu->setDeShadow(value);
    else if(sender() == m_hlshadow) cpu->setHlShadow(value);
    else if(sender() == m_pc) cpu->setPc(value);
    else if(sender() == m_sp) cpu->setSp(value);
    else if(sender() == m_a) cpu->setA(value);
    else if(sender() == m_b) cpu->setB(value);
    else if(sender() == m_c) cpu->setC(value);
    else if(sender() == m_d) cpu->setD(value);
    else if(sender() == m_e) cpu->setE(value);
    else if(sender() == m_h) cpu->setH(value);
    else if(sender() == m_l) cpu->setL(value);
    else if(sender() == m_ashadow) cpu->setAShadow(value);
    else if(sender() == m_bshadow) cpu->setBShadow(value);
    else if(sender() == m_cshadow) cpu->setCShadow(value);
    else if(sender() == m_dshadow) cpu->setDShadow(value);
    else if(sender() == m_eshadow) cpu->setEShadow(value);
    else if(sender() == m_hshadow) cpu->setHShadow(value);
    else if(sender() == m_lshadow) cpu->setLShadow(value);
//		else if(sender() == m_ixh) cpu->setIxh(value);
//		else if(sender() == m_ixl) cpu->seIxl(value);
//		else if(sender() == m_iyh) cpu->setIyh(value);
//		else if(sender() == m_iyl) cpu->setIyl(value);
}


void QSpectrumDebugWindow::updateStateDisplay()
{
	if(!m_spectrum) {
	    return;
	}

	auto * cpu = m_spectrum->z80();

	if (!cpu) {
	    return;
	}

	m_af->setValue(cpu->afRegisterValue());
	m_bc->setValue(cpu->bcRegisterValue());
	m_de->setValue(cpu->deRegisterValue());
	m_hl->setValue(cpu->hlRegisterValue());
	m_ix->setValue(cpu->ixRegisterValue());
	m_iy->setValue(cpu->iyRegisterValue());
	m_afshadow->setValue(cpu->afShadowRegisterValue());
	m_bcshadow->setValue(cpu->bcShadowRegisterValue());
	m_deshadow->setValue(cpu->deShadowRegisterValue());
	m_hlshadow->setValue(cpu->hlShadowRegisterValue());
	m_pc->setValue(cpu->pc());
	m_sp->setValue(cpu->stackPointer());
	m_a->setValue(cpu->aRegisterValue());
	m_b->setValue(cpu->bRegisterValue());
	m_c->setValue(cpu->cRegisterValue());
	m_d->setValue(cpu->dRegisterValue());
	m_e->setValue(cpu->eRegisterValue());
	m_h->setValue(cpu->hRegisterValue());
	m_l->setValue(cpu->lRegisterValue());
	m_ashadow->setValue(cpu->aShadowRegisterValue());
	m_bshadow->setValue(cpu->bShadowRegisterValue());
	m_cshadow->setValue(cpu->cShadowRegisterValue());
	m_dshadow->setValue(cpu->dShadowRegisterValue());
	m_eshadow->setValue(cpu->eShadowRegisterValue());
	m_hshadow->setValue(cpu->hShadowRegisterValue());
	m_lshadow->setValue(cpu->lShadowRegisterValue());
	m_ixh->setValue(cpu->ixhRegisterValue());
	m_ixl->setValue(cpu->ixlRegisterValue());
	m_iyh->setValue(cpu->iyhRegisterValue());
	m_iyl->setValue(cpu->iylRegisterValue());
}


void QSpectrumDebugWindow::setMemoryDisplayStart(int address)
{

}
