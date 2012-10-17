#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include <icpmatcher.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	parser=NULL;
	lastPositionIndex = 0;
	lastRenderedMapIndex = 0;
	currentScan = NULL;
	previousScan = NULL;

    ui->setupUi(this);

	//Setup graphics scenes
	ui->mapGraphicsView->setScene(&mapScene);
	ui->icpGraphicsView->setScene(&icpScene);
	ui->kdTreeGraphicsView->setScene(&kdtreeScene);

	mapScene.setBackgroundBrush(Qt::black);
	icpScene.setBackgroundBrush(Qt::black);
	kdtreeScene.setBackgroundBrush(Qt::black);
	/*QRadialGradient gradient(0, 0, 1000);
	gradient.setSpread(QGradient::PadSpread);
	mapScene.setBackgroundBrush(gradient);*/

	ui->mapGraphicsView->setInteractive(true);
	ui->mapGraphicsView->installEventFilter(this);
	ui->mapGraphicsView->verticalScrollBar()->installEventFilter(this);

	ui->icpGraphicsView->setInteractive(true);
	ui->icpGraphicsView->installEventFilter(this);
	ui->icpGraphicsView->verticalScrollBar()->installEventFilter(this);

	ui->kdTreeGraphicsView->setInteractive(true);
	ui->kdTreeGraphicsView->installEventFilter(this);
	ui->kdTreeGraphicsView->verticalScrollBar()->installEventFilter(this);

	/*QPen pen(Qt::yellow);
	mapScene.addLine(0, -5000, 0, 5000, pen);
	mapScene.addLine(-5000, 0, 5000, 0, pen);*/

	ui->statsProgress->setVisible(false);


	connect(ui->logPathLine, SIGNAL(textChanged(QString)), this, SLOT(isFileValid(QString)));
	connect(ui->logBrowseButton, SIGNAL(clicked()), this, SLOT(fileBrowse()));
	connect(ui->openLogButton, SIGNAL(clicked()), this, SLOT(openParser()));
	connect(ui->logCloseButton, SIGNAL(clicked()), this, SLOT(closeParser()));
	connect(ui->gatherStatsButton, SIGNAL(clicked()), this, SLOT(scanFile()));

	connect(ui->actionStep, SIGNAL(triggered()), this, SLOT(stepFrame()));
	connect(ui->actionPlay, SIGNAL(triggered()), this, SLOT(play()));
	connect(ui->actionPause, SIGNAL(triggered()), this, SLOT(pause()));

	connect(ui->kdRenderDepthSpin, SIGNAL(valueChanged(int)), this, SLOT(drawKDTree(int)));

	//Hide unsupported features
	ui->kdTreeTreeWidget->setVisible(false);
	ui->scrollArea->setVisible(false);
}

MainWindow::~MainWindow()
{
	if(parser != NULL){
		delete parser;
	}
    delete ui;
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event){
	if(event->type() == QEvent::Wheel){
			if(obj == ui->mapGraphicsView || obj == ui->icpGraphicsView || obj == ui->kdTreeGraphicsView){
					QWheelEvent* wheel = (QWheelEvent*) event;

					//Ensure that [-240 <= delta <= +240]
					qreal delta = fmax(fmin(240, (qreal)wheel->delta()), -240);

					qreal scale = ((delta / 4.0) / 120.0) + 1.0;

					QTransform t = ((QGraphicsView*)obj)->transform();
					t.scale(scale, scale);

					//Prevent excessive zooming
					if(t.m11() > 0.01 && t.m11() < 1000.0){
							((QGraphicsView*)obj)->setTransform(t);
					}


					return true;
			}
			if(obj == ui->mapGraphicsView->verticalScrollBar() ||
			  obj == ui->icpGraphicsView->verticalScrollBar() ||
			  obj == ui->kdTreeGraphicsView->verticalScrollBar()){
					return true;
			}
	}
	return false;
}

void MainWindow::clear(){
	while(!positions.isEmpty()){
		delete positions.last();
		positions.pop_back();
	}

	while(!completeMap.isEmpty()){
		delete completeMap.last();
		completeMap.pop_back();
	}

	mapScene.clear();
}

void MainWindow::isFileValid(QString path){
	QFile file(path);

	if(file.exists()){
		ui->statisticsBox->setEnabled(true);
		ui->openLogButton->setEnabled(true);
		qDebug() << "Is Valid" << path;
	}
	else{
		ui->statisticsBox->setEnabled(false);
		ui->openLogButton->setEnabled(false);
		qDebug() << "Not Valid" << path;
	}
}

void MainWindow::fileBrowse(){
	QFileDialog dialog(this);
	dialog.setNameFilter(tr("Log Files (*.log)"));

	QStringList fileNames;
	if(dialog.exec()){
		fileNames = dialog.selectedFiles();

		if(fileNames.length() > 1){
			QMessageBox msgBox;
			msgBox.setText("Only one log can be opened at a time.");
			msgBox.setDefaultButton(QMessageBox::Ok);
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}

		if(fileNames.length() == 1){
			ui->logPathLine->setText(fileNames[0]);
			isFileValid(fileNames[0]);
			openParser();
		}
		else{
			qDebug() << "Incorrect number of files selected"<<fileNames.length();
		}
	}
}

void MainWindow::openParser(){
	if(parser != NULL){
		closeParser();
	}

	parser = new LogParser(ui->logPathLine->text());

	connect(parser, SIGNAL(lidarDataAvail(LidarEntry*)), this, SLOT(processLidar(LidarEntry*)));
	connect(parser, SIGNAL(positionDataAvail(PositionEntry*)), this, SLOT(processPosition(PositionEntry*)));

	ui->toolBar->setEnabled(true);
	ui->logCloseButton->setEnabled(true);
	ui->mapTab->setEnabled(true);
	ui->kdtreeTab->setEnabled(true);
	ui->icpTab->setEnabled(true);
	//scanFile();
}

void MainWindow::closeParser(){
	ui->logCloseButton->setEnabled(false);
	ui->toolBar->setEnabled(false);
	ui->mapTab->setEnabled(false);
	ui->kdtreeTab->setEnabled(false);
	ui->icpTab->setEnabled(false);

	clear();

	if(parser != NULL){
		delete parser;
		parser = NULL;

		matcher.clear();
	}
}

void MainWindow::scanFile(){
	LogParser p(ui->logPathLine->text());

	EntryHeader* hdr=p.getCurrentHeader();

	QDateTime start = QDateTime::currentDateTime();
	QDateTime end = QDateTime(QDate(0, 0, 0));

	qDebug() << start.toString() << "\t" << end.toString();

	QMultiMap<QString, uint32_t> sensors;

	ui->statsProgress->setVisible(true);
	ui->statsProgress->setMinimum(0);
	ui->statsProgress->setMaximum(p.byteSize());

	connect(&p, SIGNAL(pos(int)), ui->statsProgress, SLOT(setValue(int)));

	int i=0;

	while(p.readHeader()){
		if(sensors.find(hdr->interface, hdr->index) == sensors.end()){
			sensors.insert(hdr->interface, hdr->index);
		}
		i++;

		if(hdr->time < start){
			start = hdr->time;
		}

		if(hdr->time > end){
			end = hdr->time;
		}
	}

	qDebug() << "Read "<<i << "records found "<<sensors.size()<<"unique sensors";
	QMap<QString, uint32_t>::iterator iter = sensors.begin();

	for(; iter != sensors.end(); iter++){
		qDebug() << "Sensor" <<iter.key() << " Index" << iter.value();
	}

	int duration = start.secsTo(end);

	qDebug() << start.toString() << "\t" << end.toString();

	QTime t;
	t = t.addSecs(duration);
	ui->runtimeEdit->setText(t.toString());

	QString temp = QString("%1").arg(i);
	ui->entryLine->setText(temp);

	temp = QString("%1").arg(sensors.size());
	ui->deviceCountEdit->setText(temp);

	temp.clear();
	QList<QString> types = sensors.uniqueKeys();
	for(i=0; i<types.size(); i++){
		temp.append(types[i]);
		temp.append(" ");
	}
	ui->sensorTypesEdit->setText(temp);

	disconnect(&p, SIGNAL(pos(int)), ui->statsProgress, SLOT(setValue(int)));
	ui->statsProgress->setVisible(false);
}

void MainWindow::processPosition(PositionEntry* entry){
	qDebug() << "processPosition()" << entry->hdr.index;
	//delete entry;

	if(entry->hdr.index == 0){
		positions.push_back(entry);
	}
	stepFrame();
}

void MainWindow::processLidar(LidarEntry* entry){
	qDebug() << "processLidar()";

	previousScan = currentScan;
	currentScan = entry;

	drawKDTree();

	Transform t;
	if(positions.size() > 0){
		t = Transform(positions[positions.size()-1]->pose);

		if(positionsICP.isEmpty()){
			positionsICP.push_back(positions[positions.size()-1]);
		}

		qDebug() << "t.x="<<t.xDelta<<" t.y="<<t.yDelta<<" t.r="<<t.rDelta;
	}
	else{
		qDebug() << "No Position";
		return;
	}

	Transform c = matcher.computeTransform(entry->points.toStdVector());
	qDebug() << "c.x="<<c.xDelta<<" c.y="<<c.yDelta<<"ct.r="<<c.rDelta;

	//correction.sum(c);

	PositionEntry* currentP = positionsICP.last();
	PositionEntry* p=new PositionEntry;
	p->pose.x = currentP->pose.x - c.xDelta;
	p->pose.y = currentP->pose.y - c.yDelta;
	p->pose.a = currentP->pose.a - c.rDelta;
	positionsICP.push_back(p);

	for(int i=0; i<entry->points.size(); i++){
		entry->points[i]->applyTransform((const Transform*)&t, NULL);
		completeMap.push_back(entry->points[i]);
	}

	drawMap();
	updateICPGraphics();
}

void MainWindow::play(){
	connect(&playTimer, SIGNAL(timeout()), this, SLOT(stepFrame()));

	playTimer.setSingleShot(false);
	playTimer.start(25);
}

void MainWindow::pause(){
	playTimer.stop();
}

void MainWindow::stepFrame(){
	if(parser != NULL){
		parser->readLogEntry();
	}
}

void MainWindow::drawMap(){
	if(ui->tabWidget->currentWidget() != ui->mapTab){
		return;
	}

	mapScene.clear();

	int i = 0; //lastRenderedMapIndex;
	for(; i<completeMap.size(); i++){
		Point* p = completeMap.at(i);

		QPen pen(Qt::green);

		mapScene.addEllipse(p->x, p->y, 0.01, 0.01, pen);

		/*if(positions.size() > 0){
			PositionEntry* current = positions[positions.size()-1];
			mapScene.addLine(current->pose.x, current->pose.y, p->x, p->y, pen);
		}*/

		//delete p;
	}
	completeMap.clear();
	//lastRenderedMapIndex = i;

	//i = lastPositionIndex;
	i=1;
	for(; i<positions.size(); i++){
		if(i<1){
			continue;
		}
		PositionEntry* current = positions[i];
		PositionEntry* previous = positions[i-1];
		QPen pen(Qt::red);

		mapScene.addLine(current->pose.x, current->pose.y, previous->pose.x, previous->pose.y, pen);
	}
	lastPositionIndex = i;

	i=1;
	for(; i<positionsICP.size(); i++){
		if(i<1){
			continue;
		}
		PositionEntry* current = positionsICP[i];
		PositionEntry* previous = positionsICP[i-1];
		QPen pen(Qt::blue);

		mapScene.addLine(current->pose.x, current->pose.y, previous->pose.x, previous->pose.y, pen);
	}

	if(positions.size() > 0){
		PositionEntry* current = positions[positions.size()-1];

		QPen pen(Qt::yellow);
		mapScene.addEllipse(current->pose.x-0.05, current->pose.y-0.05, 0.1, 0.1, pen);

		double x2 = current->pose.x + (0.1 * cos(current->pose.a));
		double y2 = current->pose.y + (0.1 * sin(current->pose.a));
		mapScene.addLine(current->pose.x, current->pose.y, x2, y2, pen);
	}

	mapScene.setSceneRect(mapScene.itemsBoundingRect());
	ui->mapGraphicsView->resetMatrix();
	ui->mapGraphicsView->scale(0.1, 0.1);
	ui->mapGraphicsView->fitInView(mapScene.sceneRect(), Qt::KeepAspectRatio);
	ui->mapGraphicsView->centerOn(mapScene.sceneRect().center());

}

void MainWindow::drawKDNode(KDNode* node, int maxLevel){
	if(node->left != NULL){
		drawKDNode(node->left, maxLevel);
	}
	if(node->right != NULL){
		drawKDNode(node->right, maxLevel);
	}

	QPen pen(Qt::green);
	kdtreeScene.addEllipse(node->location.x-0.005, node->location.y-0.005, 0.01, 0.01, pen);

	if(node->depth < maxLevel){

		QRectF bounds = QRectF(node->topLeft.x, node->topLeft.y-node->getHeight(), node->getWidth(), node->getHeight());

		pen = QPen(Qt::yellow);
		kdtreeScene.addRect(bounds, pen);

		if(node->direction == X_Axis){
			QPointF top(node->location.x, node->topLeft.y-node->getHeight());
			QPointF bottom(node->location.x, node->topLeft.y);

			pen = QPen(Qt::blue);

			QLineF line(top, bottom);
			kdtreeScene.addLine(line, pen);
		}
		else{
			QPointF left(node->topLeft.x, node->location.y);
			QPointF right(node->topLeft.x+node->getWidth(), node->location.y);

			pen = QPen(Qt::red);

			QLineF line(left, right);
			kdtreeScene.addLine(line, pen);
		}
	}
}

void MainWindow::drawKDTree(int depth){
	if(ui->tabWidget->currentWidget() != ui->kdtreeTab || currentScan == NULL){
		return;
	}

	kdtreeScene.clear();

	KDNode* n = KDNode::buildKDTree(currentScan->points.toStdVector());

	drawKDNode(n, ui->kdRenderDepthSpin->value());

	ui->kdDepthLine->setText(QString("%1").arg(KDNode::getMaxDepth(n)));
	ui->kdNodeCountLine->setText(QString("%1").arg(KDNode::getNodeCount(n)));

	kdtreeScene.setSceneRect(kdtreeScene.itemsBoundingRect());
	ui->kdTreeGraphicsView->resetMatrix();
	ui->kdTreeGraphicsView->scale(0.1, 0.1);
	ui->kdTreeGraphicsView->fitInView(kdtreeScene.sceneRect(), Qt::KeepAspectRatio);
	ui->kdTreeGraphicsView->centerOn(kdtreeScene.sceneRect().center());

	delete n;
}

void MainWindow::updateICPGraphics(){
	if(ui->tabWidget->currentWidget() != ui->icpTab || previousScan == NULL || currentScan == NULL){
		return;
	}

	icpScene.clear();

	QMap<QGraphicsEllipseItem*, Point*> prevPointMap;
	QMap<QGraphicsEllipseItem*, Point*> currentPointMap;

	int i = 0;

	//Draw previous scan
	for(; i<previousScan->points.size(); i++){
		Point* p = previousScan->points[i];

		QPen pen(Qt::green);

		prevPointMap.insert(icpScene.addEllipse(p->x-0.005, p->y-0.005, 0.01, 0.01, pen), p);
	}

	//Draw current scan
	for(i=0; i<currentScan->points.size(); i++){
		Point* p = currentScan->points[i];

		QPen pen(Qt::blue);

		currentPointMap.insert(icpScene.addEllipse(p->x-0.005, p->y-0.005, 0.01, 0.01, pen), p);
	}

	Transform t;
	ICPMatcher m;

	if(ui->icpAutoCheckBox->isChecked()){
		int iterations = ui->maxICPIterations->value();
		double error = ui->minICPError->value();

		m.computeTransform(previousScan->points.toStdVector(), iterations, error);
		t = m.computeTransform(currentScan->points.toStdVector(), iterations, error);
		map<Point*, KDNode*> pairs = m.getMatchingPairs();

		map<Point*, KDNode*>::iterator pairIter = pairs.begin();

		for(; pairIter != pairs.end(); pairIter++){
			Point* p = pairIter->first;
			KDNode* n = pairIter->second;

			QPen pen(Qt::white);
			QGraphicsLineItem* line = icpScene.addLine(p->x, p->y, n->location.x, n->location.y, pen);
			line->setOpacity(0.2);
		}
	}

	ui->currentICPDeltaX->setText(QString("%1").arg(t.xDelta));
	ui->currentICPDeltaY->setText(QString("%1").arg(t.yDelta));
	ui->currentICPDeltaTheta->setText(QString("%1").arg(t.rDelta));
	ui->currentICPIterations->setText(QString("%1").arg(m.getNumIterations()));
	ui->currentICPMatchError->setText(QString("%1").arg(m.getMatchError()));

	icpScene.setSceneRect(icpScene.itemsBoundingRect());
	ui->icpGraphicsView->resetMatrix();
	ui->icpGraphicsView->scale(0.1, 0.1);
	ui->icpGraphicsView->fitInView(icpScene.sceneRect(), Qt::KeepAspectRatio);
	ui->icpGraphicsView->centerOn(icpScene.sceneRect().center());
}
