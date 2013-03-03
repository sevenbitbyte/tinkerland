#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serialdevice.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
		
	protected:
		void populatePortPathCombo();

		BaudRateType stringToBaud(QString str) const;

	protected slots:
		void serialPortPathChangeSlot(QString portPath);
		void openPortSlot();

		void systemStatusSlot(SystemStatusPacket status);
		void deviceBrokenPipeSlot();
		void deviceBrokenPipeReconnectSlot();

		void enableAutoRotateSlot(bool enable);

		void visibleTabChangedSlot(int index);

		void accelValueSlot(AccelValuePacket values);
		void accelSamplesPerSecSlot(float samples);

	private:
		Ui::MainWindow *ui;
		SerialDevice* device;

		struct ErrorState{
			QFileInfo originFileInfo;
			QDateTime originCreateTime;
			QDateTime startTime;

			enum ErrorType { Err_None, Err_BrokenPipe };

			ErrorType type;
		};

		ErrorState errorState;
};

#endif // MAINWINDOW_H
