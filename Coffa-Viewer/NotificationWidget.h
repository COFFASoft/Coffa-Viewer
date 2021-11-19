#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGraphicsDropShadowEffect>

class NotificationWidget : public QDialog
{
	Q_OBJECT;
public:
	NotificationWidget();

	NotificationWidget(QString aMessage);

	void setMessage(QString aMessage);

private:
	QString theMessage;
	QLabel* theLabel;
	QLabel* theImageLabel;
};

