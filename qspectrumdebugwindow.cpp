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
:	QWidget(parent),
	m_spectrum(0),
	m_af(0),
	m_bc(0),
	m_de(0),
	m_hl(0),
	m_ix(0),
	m_iy(0),
	m_afshadow(0),
	m_bcshadow(0),
	m_deshadow(0),
	m_hlshadow(0),
	m_pc(0),
	m_sp(0),
	m_a(0),
	m_b(0),
	m_c(0),
	m_d(0),
	m_e(0),
	m_h(0),
	m_l(0),
	m_ashadow(0),
	m_bshadow(0),
	m_cshadow(0),
	m_dshadow(0),
	m_eshadow(0),
	m_hshadow(0),
	m_lshadow(0),
	m_ixh(0),
	m_ixl(0),
	m_iyh(0),
	m_iyl(0),
	m_instruction(0) {
	createWidgets();
	connectWidgets();
}


QSpectrumDebugWindow::QSpectrumDebugWindow( Spectrum * spectrum, QWidget * parent )
:	QWidget(parent),
	m_spectrum(spectrum),
	m_af(0),
	m_bc(0),
	m_de(0),
	m_hl(0),
	m_ix(0),
	m_iy(0),
	m_afshadow(0),
	m_bcshadow(0),
	m_deshadow(0),
	m_hlshadow(0),
	m_pc(0),
	m_sp(0),
	m_a(0),
	m_b(0),
	m_c(0),
	m_d(0),
	m_e(0),
	m_h(0),
	m_l(0),
	m_ashadow(0),
	m_bshadow(0),
	m_cshadow(0),
	m_dshadow(0),
	m_eshadow(0),
	m_hshadow(0),
	m_lshadow(0),
	m_ixh(0),
	m_ixl(0),
	m_iyh(0),
	m_iyl(0),
	m_instruction(0) {
	createWidgets();
	connectWidgets();
}


QSpinBox * QSpectrumDebugWindow::createRegisterSpinBox( int bits ) {
	QSpinBox * ret = new QSpinBox();
	ret->setMinimum(0);
	ret->setMaximum((1 << bits) - 1);
	connect(ret, SIGNAL(valueChanged(int)), this, SLOT(setRegister()));
	return ret;
}


void QSpectrumDebugWindow::createWidgets( void ) {
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
	QHBoxLayout * plainRegisters = new QHBoxLayout();
	QHBoxLayout * shadowRegisters = new QHBoxLayout();
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

	QVBoxLayout * myLayout = new QVBoxLayout();
	myLayout->addLayout(plainRegisters);
	myLayout->addLayout(shadowRegisters);
	myLayout->addWidget(m_instruction);

	setLayout(myLayout);
}


void QSpectrumDebugWindow::connectWidgets( void ) {
}


void QSpectrumDebugWindow::setRegister( void ) {
	if(!m_spectrum) return;

	Z80 * cpu = dynamic_cast<Z80 *>(m_spectrum->cpu());
	if(!cpu) return;

		if(sender() == m_af) cpu->setAf(m_af->value());
		else if(sender() == m_bc) cpu->setBc(m_bc->value());
		else if(sender() == m_de) cpu->setDe(m_de->value());
		else if(sender() == m_hl) cpu->setHl(m_hl->value());
		else if(sender() == m_ix) cpu->setIx(m_ix->value());
		else if(sender() == m_iy) cpu->setIy(m_iy->value());
		else if(sender() == m_afshadow) cpu->setAfShadow(m_afshadow->value());
		else if(sender() == m_bcshadow) cpu->setBcShadow(m_bcshadow->value());
		else if(sender() == m_deshadow) cpu->setDeShadow(m_deshadow->value());
		else if(sender() == m_hlshadow) cpu->setHlShadow(m_hlshadow->value());
		else if(sender() == m_pc) cpu->setPc(m_pc->value());
		else if(sender() == m_sp) cpu->setSp(m_sp->value());
		else if(sender() == m_a) cpu->setA(m_a->value());
		else if(sender() == m_b) cpu->setB(m_b->value());
		else if(sender() == m_c) cpu->setC(m_c->value());
		else if(sender() == m_d) cpu->setD(m_d->value());
		else if(sender() == m_e) cpu->setE(m_e->value());
		else if(sender() == m_h) cpu->setH(m_h->value());
		else if(sender() == m_l) cpu->setL(m_l->value());
		else if(sender() == m_ashadow) cpu->setAShadow(m_ashadow->value());
		else if(sender() == m_bshadow) cpu->setBShadow(m_bshadow->value());
		else if(sender() == m_cshadow) cpu->setCShadow(m_cshadow->value());
		else if(sender() == m_dshadow) cpu->setDShadow(m_dshadow->value());
		else if(sender() == m_eshadow) cpu->setEShadow(m_eshadow->value());
		else if(sender() == m_hshadow) cpu->setHShadow(m_hshadow->value());
		else if(sender() == m_lshadow) cpu->setLShadow(m_lshadow->value());
//		else if(sender() == m_ixh) cpu->setIxh(m_ixh->value());
//		else if(sender() == m_ixl) cpu->seIxl(m_ixl->value());
//		else if(sender() == m_iyh) cpu->setIyh(m_iyh->value());
//		else if(sender() == m_iyl) cpu->setIyl(m_iyl->value());
}


void QSpectrumDebugWindow::updateStateDisplay( void ) {
	if(!m_spectrum) return;

	Z80 * cpu = dynamic_cast<Z80 *>(m_spectrum->cpu());
	if(!cpu) return;

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


void QSpectrumDebugWindow::setMemoryDisplayStart( int address ) {

}
