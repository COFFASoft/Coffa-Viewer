#include "NotificationWidget.h"

NotificationWidget::NotificationWidget()
{
	setFixedSize(255, 105);
	setStyleSheet("QDialog{background: white;}");
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowModality(Qt::WindowModal);

	QWidget* styleWidget = new QWidget(this);
	styleWidget->setFixedSize(250, 100);
	styleWidget->move(2.5, 2.5);
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
	theLabel->setFixedSize(200, 80);
	theLabel->setText(theMessage);
	theLabel->setAlignment(Qt::AlignVCenter);
	theLabel->setStyleSheet("QLabel{background: white; border-radius:0px; border: 0px solid white;}");
	theLabel->setFont(aFont);
}

NotificationWidget::NotificationWidget(QString aMessage)
{
	theMessage = aMessage;
	setFixedSize(255, 105);
	setStyleSheet("QDialog{background: white;}");
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowModality(Qt::WindowModal);
	
	QWidget* styleWidget = new QWidget(this);
	styleWidget->setFixedSize(250, 100);
	styleWidget->move(2.5, 2.5);
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
	theLabel->setFixedSize(200, 80);
	theLabel->setText(theMessage);
	theLabel->setAlignment(Qt::AlignVCenter);
	theLabel->setStyleSheet("QLabel{background: white; border-radius:0px; border: 0px solid white;}");
	theLabel->setFont(aFont);
		
}

void NotificationWidget::setMessage(QString aMessage)
{
	theMessage = aMessage;
	theLabel->setText(theMessage);
}

