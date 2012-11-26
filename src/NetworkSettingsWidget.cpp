#include "NetworkSettingsWidget.h"
#include "ui_NetworkSettingsWidget.h"
#include "MenuBar.h"
#include "RootController.h"
#include "StatusBar.h"
#include "Device.h"
#include "ConnectWidget.h"
#include "ManageNetworksWidget.h"
#include "NetworkManager.h"

#include <QTimer>

#include <QDebug>

NetworkSettingsWidget::NetworkSettingsWidget(Device *device, QWidget *parent)
	: StandardWidget(device, parent),
	ui(new Ui::NetworkSettingsWidget)
{
	ui->setupUi(this);
	performStandardSetup(tr("Network Settings"));
	
	ui->turnOn->setVisible(false);
	ui->turnOff->setVisible(false);
	
	QObject::connect(ui->connect, SIGNAL(clicked()), SLOT(connect()));
	QObject::connect(ui->manage, SIGNAL(clicked()), SLOT(manage()));
	QObject::connect(ui->turnOn, SIGNAL(clicked()), SLOT(turnOn()));
	QObject::connect(ui->turnOff, SIGNAL(clicked()), SLOT(turnOff()));
	
	QObject::connect(&NetworkManager::ref(),
		SIGNAL(stateChanged(const NetworkManager::State &, const NetworkManager::State &)),
		SLOT(stateChanged(const NetworkManager::State &, const NetworkManager::State &)));
	
	updateInformation();
}

NetworkSettingsWidget::~NetworkSettingsWidget()
{
	delete ui;
}

void NetworkSettingsWidget::connect()
{
	RootController::ref().presentWidget(new ConnectWidget(m_device));
}

void NetworkSettingsWidget::manage()
{
	RootController::ref().presentWidget(new ManageNetworksWidget(m_device));
}

void NetworkSettingsWidget::turnOn()
{
	// m_device->networkingProvider()->setNetworkState(NetworkingProvider::NetworkOn);
}

void NetworkSettingsWidget::turnOff()
{
	// m_device->networkingProvider()->setNetworkState(NetworkingProvider::NetworkOff);
}

void NetworkSettingsWidget::updateInformation()
{
	Network active = NetworkManager::ref().active();
	ui->ssid->setText(active.ssid());
	ui->security->setText(active.securityString());
	ui->ip->setText(NetworkManager::ref().ipAddress());
}

void NetworkSettingsWidget::stateChanged(const NetworkManager::State &newState, const NetworkManager::State &oldState)
{
	qDebug() << "State Changed to" << newState;
	QTimer::singleShot(300, this, SLOT(updateInformation()));
}