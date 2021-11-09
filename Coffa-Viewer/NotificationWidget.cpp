#include "NotificationWidget.h"

NotificationWidget::NotificationWidget()
{
	setFixedSize(355, 155);
	setStyleSheet("QDialog{background: white;}");
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowModality(Qt::WindowModal);

	QWidget* styleWidget = new QWidget(this);
	styleWidget->setFixedSize(350, 150);
	styleWidget->move(2.5, 2.5);
	//styleWidget->setStyleSheet("QWidget{background: white; border-radius:15px;}");
	styleWidget->setStyleSheet("QWidget{background: white; border-radius:10px; border: 1px solid rgba(200, 200, 200,1);}");

	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(10);
	effect->setXOffset(1.5);
	effect->setYOffset(1.5);
	effect->setColor(QColor(232, 232, 232));

	styleWidget->setGraphicsEffect(effect);

	QFont aFont;
	aFont.setPointSize(11);


	theLabel = new QLabel(styleWidget);
	theLabel->move(10, 10);
	theLabel->setFixedSize(200, 130);
	theLabel->setText(theMessage);
	theLabel->setAlignment(Qt::AlignVCenter);
	theLabel->setStyleSheet("QLabel{background: white; border-radius:0px; border: 0px solid white;}");
	theLabel->setFont(aFont);

	//QImage Welim(":/Coffa_Project/Resources/loading_GIF.gif");
	//QPixmap Welpix = QPixmap::fromImage(Welim);
	QMovie* movie = new QMovie(":/Coffa_Project/Resources/loading_GIF.gif");
	theImageLabel = new QLabel(styleWidget);
	theImageLabel->move(220, 10);
	theImageLabel->setMovie(movie);
	theImageLabel->setFixedSize(100, 100);
	theImageLabel->setScaledContents(true);
	theImageLabel->setAlignment(Qt::AlignCenter);
	theImageLabel->show();

	movie->start();
}

NotificationWidget::NotificationWidget(QString aMessage)
{
	theMessage = aMessage;
	setFixedSize(355, 155);
	setStyleSheet("QDialog{background: white;}");
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowModality(Qt::WindowModal);
	
	QWidget* styleWidget = new QWidget(this);
	styleWidget->setFixedSize(350, 150);
	styleWidget->move(2.5, 2.5);
	//styleWidget->setStyleSheet("QWidget{background: white; border-radius:15px;}");
	styleWidget->setStyleSheet("QWidget{background: white; border-radius:10px; border: 1px solid rgba(200, 200, 200,1);}");
	
	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect;
	effect->setBlurRadius(10);
	effect->setXOffset(1.5);
	effect->setYOffset(1.5);
	effect->setColor(QColor(232, 232, 232));

	styleWidget->setGraphicsEffect(effect);

	QFont aFont;
	aFont.setPointSize(11);

	theLabel = new QLabel(styleWidget);
	theLabel->move(10, 10);
	theLabel->setFixedSize(200, 130);
	theLabel->setText(theMessage);
	theLabel->setAlignment(Qt::AlignVCenter);
	theLabel->setStyleSheet("QLabel{background: white; border-radius:0px; border: 0px solid white;}");

	theLabel->setFont(aFont);

	QMovie* movie = new QMovie(":/Coffa_Project/Resources/loading_GIF.gif");
	theImageLabel = new QLabel(styleWidget);
	theImageLabel->move(220, 10);
	theImageLabel->setMovie(movie);
	movie->start();
	theImageLabel->setFixedSize(64, 64);
	//theImageLabel->setScaledContents(true);
	theImageLabel->setAlignment(Qt::AlignCenter);
	theImageLabel->setStyleSheet("QLabel{background: white; border-radius:0px; border: 0px solid white;}");
	theImageLabel->show();
	
}

void NotificationWidget::setMessage(QString aMessage)
{
	theMessage = aMessage;
	theLabel->setText(theMessage);
}

