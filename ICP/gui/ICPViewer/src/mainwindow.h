#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logparser.h"
#include <icpmatcher.h>

#include <QtGui>
#include <QtCore>
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

		bool eventFilter(QObject* obj, QEvent* event);

		void clear();

	public slots:
		void isFileValid(QString path);
		void fileBrowse();
		void openParser();
		void closeParser();

		void scanFile();

		void processPosition(PositionEntry* entry);
		void processLidar(LidarEntry* entry);

		void play();
		void pause();
		void stepFrame();

		//void mergePoints(QList<Point*> points);
		void drawMap();
		void drawKDTree(int depth=-1);
		void updateICPGraphics();

	protected:
		//void drawGrid(QGraphicsScene* scene, int range, bool )
		void drawKDNode(KDNode* node, int maxLevel);

	private:
		LogParser* parser;
		ICPMatcher matcher;

		LidarEntry* currentScan;
		LidarEntry* previousScan;
		QVector<LidarEntry*> previousScans;

		Transform correction;

		QVector<Point*> completeMap;
		QVector<PositionEntry*> positions;
		QVector<PositionEntry*> positionsICP;

		int lastRenderedMapIndex;
		int lastPositionIndex;

		Ui::MainWindow *ui;
		QGraphicsScene mapScene;
		QGraphicsScene icpScene;
		QGraphicsScene kdtreeScene;

		QTimer playTimer;
};

#endif // MAINWINDOW_H
