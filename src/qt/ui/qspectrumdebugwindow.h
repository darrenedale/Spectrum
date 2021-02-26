#ifndef QSPECTRUMDEBUGWINDOW_H
#define QSPECTRUMDEBUGWINDOW_H

#include <QWidget>

class QSpinBox;
class QLineEdit;

namespace Spectrum
{
	class Spectrum;

	class QSpectrumDebugWindow
	:	public QWidget
	{
		Q_OBJECT

		public:
			explicit QSpectrumDebugWindow( QWidget * = nullptr );
			explicit QSpectrumDebugWindow( Spectrum * spectrum, QWidget * = nullptr );

	protected:
	    void showEvent(QShowEvent *) override;
	    void closeEvent(QCloseEvent *) override;

		public Q_SLOTS:
			void updateStateDisplay();
			void setMemoryDisplayStart(int);

		private Q_SLOTS:
			void setRegister(int);

		private:
			QSpinBox * createRegisterSpinBox(int) const;
			void createWidgets();
			void connectWidgets();

			Spectrum * m_spectrum;
			int m_displayMemAddr;

			QSpinBox * m_af, * m_bc, * m_de, * m_hl, * m_ix, * m_iy, * m_afshadow, * m_bcshadow, * m_deshadow, * m_hlshadow, * m_pc, * m_sp;
			QSpinBox * m_a, * m_b, * m_c, * m_d, * m_e, * m_h, * m_l, * m_ashadow, * m_bshadow, * m_cshadow, * m_dshadow, * m_eshadow, * m_hshadow, * m_lshadow, * m_ixh, * m_ixl, * m_iyh, * m_iyl;
			QLineEdit * m_instruction;
	};
}

#endif // QSPECTRUMDEBUGWINDOW_H
