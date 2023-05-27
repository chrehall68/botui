#include "Options.h"
#include <QCoreApplication>
#ifdef NETWORK_ENABLED
#include "org_freedesktop_NetworkManager_Connection_Active.h"
#include "NetworkSettingsWidget.h"
#include "NetworkStatusWidget.h"
#include "ui_NetworkSettingsWidget.h"
#include "MenuBar.h"
#include "RootController.h"
#include "StatusBar.h"
#include "Device.h"
#include "ConnectWidget.h"
#include "ManageNetworksWidget.h"
#include "NetworkManager.h"
#include "SystemUtils.h"
#include "Execute.h"

#include <QTimer>

#include <QDebug>

NetworkSettingsWidget::NetworkSettingsWidget(Device *device, QWidget *parent)
	: StandardWidget(device, parent), ui(new Ui::NetworkSettingsWidget)
{
	ui->setupUi(this);
	performStandardSetup(tr("Network Settings"));

	enableCoolOffTimer = new QTimer(this);
	enableCoolOffTimer->setSingleShot(true);

	ui->ConnectButton->setEnabled(false);
	QObject::connect(ui->ConnectButton, SIGNAL(clicked()), SLOT(connect()));
	QObject::connect(ui->ManageButton, SIGNAL(clicked()), SLOT(manage()));

	QObject::connect(ui->TournamentModeButton, SIGNAL(clicked()), SLOT(TournamentMode()));
	NetworkManager::ref().connect(ui->connectionModeSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));

	// TODO: put back after we support client mode WiFi
	ui->ConnectButton->setVisible(true);
	ui->ManageButton->setVisible(true);
	ui->security->setVisible(false);
	ui->securityLabel->setVisible(false);

	QObject::connect(&NetworkManager::ref(),
					 SIGNAL(stateChanged(const NetworkManager::State &, const NetworkManager::State &)),
					 SLOT(stateChanged(const NetworkManager::State &, const NetworkManager::State &)));

	QTimer *updateTimer = new QTimer(this);
	QObject::connect(updateTimer, SIGNAL(timeout()), SLOT(updateInformation()));
	updateTimer->start(10000);

	updateInformation();
}

void NetworkSettingsWidget::TournamentMode()
{
	system("sudo iwconfig wlan0 txpower 1");
	QMessageBox msgBox;
	msgBox.setText("Tournament Mode activated");
	msgBox.exec();
}

void NetworkSettingsWidget::indexChanged(int index)
{
	NetworkManager::ref().turnOn();

	if (index == 1) // AP mode
	{

		NetworkManager::ref().enableAP();
		ui->ConnectButton->setEnabled(false);
	}
	else if (index == 2) // Wifi on (client mode)
	{
		
		if (NetworkManager::ref().currentActiveConnectionName() == "APName")
		{
			NetworkManager::ref().disableAP();
		}
		ui->ConnectButton->setEnabled(true);
	}
	else if (index == 3) // Wifi off
	{
		NetworkManager::ref().turnOff();
		ui->ConnectButton->setEnabled(false);
	}
}

NetworkSettingsWidget::~NetworkSettingsWidget()
{
	delete ui;
	delete enableCoolOffTimer;
}

void NetworkSettingsWidget::connect() // Connects to network
{
	RootController::ref().presentWidget(new ConnectWidget(device()));
}

void NetworkSettingsWidget::manage() // Forget or add network to history
{
	RootController::ref().presentWidget(new ManageNetworksWidget(device()));
}

void NetworkSettingsWidget::updateInformation()
{
	const bool on = NetworkManager::ref().isOn(); //
	ui->ssid->setText(NetworkManager::ref().currentActiveConnectionName());
	ui->ip->setText(NetworkManager::ref().ip4Address());

	const QString id = device()->id();
	const QString serial = device()->serial();
	// if (!id.isEmpty())
	// {
	// 	const QString password = SystemUtils::sha256(id).left(6) + "00";
	// 	const QString ssid = serial + "-wombat";
	// 	ui->ssid->setText(ssid);
	// 	ui->password->setText(password);
	// }

	if (NetworkManager::ref().isActiveConnectionOn() == true) // if there's an active connection
	{
		if (ui->connectionModeSelect->currentText() == "Client Mode") // if Wombat in client mode
		{
			ui->state->setText(on ? tr("ON") : tr("OFF"));
			ui->ssid->setText(NetworkManager::ref().currentActiveConnectionName());
			ui->ip->setText(NetworkManager::ref().ip4Address());
		}
		if (ui->connectionModeSelect->currentText() == "AP Mode")
		{ // if Wombat is in AP Mode
			ui->ssid->setText(NetworkManager::ref().currentActiveConnectionName());
			ui->ip->setText(NetworkManager::ref().ip4Address());
		}
	}
	if (ui->connectionModeSelect->currentText() == "Wifi Off")
	{
		ui->state->setText("OFF");
		ui->ssid->setText(" ");
		ui->ip->setText(" ");
	}
	Network active = NetworkManager::ref().active();
	// ui->ssid->setText(active.ssid());
	ui->security->setText(active.securityString());
	// const QString ip = NetworkManager::ref().ipAddress();
	// ui->ip->setText(ip.isEmpty() ? tr("No IP") : ip);
}

void NetworkSettingsWidget::stateChanged(const NetworkManager::State &newState, const NetworkManager::State &oldState)
{
	qDebug() << "State Changed to" << newState;
	QTimer::singleShot(300, this, SLOT(updateInformation()));
}

#endif
